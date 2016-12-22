// See LICENSE for license details.

#ifndef __PAGERANK_HLS_H__
#define __PAGERANK_HLS_H__


#include "systemc.h"
#include "pr_types.h"
#include "ga_tlm_fifo.h"
#include "spl_mem_network.h"
#include "types.h"
#include "hls_utils.h"
#ifdef __SYSTEMC_AFU__
#ifdef __SCL_FLEX_CH__
#include "scl_flex_channel.h"
#else
#include "ctos_tlm.h"
using namespace ctos_tlm;
#include "ctos_flex_channels.h"
#endif //__SCL_FLEX_CH__
#endif //__SYSTEMC_AFU__

// out-of-order buffer to store partial state of vertex page rank computation
// it allows to receive neighbor pagerank responses out of order
template <size_t BUFFER_SIZE, typename CONTROL_STATE, typename DATA_STATE>
struct OooState {
  typedef DATA_STATE DataState;
  typedef CONTROL_STATE ControlState;
  typedef typename SizeT<BUFFER_SIZE>::Type Tag;

  DataState state[BUFFER_SIZE];
  ControlState control;

  ga::ga_storage_fifo<Tag, BUFFER_SIZE> credit_fifo;

  // return true if successful
  bool alloc(Tag &tag) {
    bool found = credit_fifo.nb_get(tag);
    if (!found) return false;
    control.init(tag);
    return true;
  }

  bool can_compute(const Tag &tag) {
    return control.can_compute(tag);
  }

  void compute(const Tag &tag, PageRankType next_pr) {

  }

  bool is_full() const {
    return credit_fifo.nb_can_get();
  }

  void reset_free () {
    credit_fifo.reset_put();
  }
  void reset_alloc () {
    credit_fifo.reset_get();
  }
  void free(const Tag &tag) {
    control.valid[tag] = false;
    assert(credit_fifo.nb_can_put());
    credit_fifo.nb_put(tag);
  }
};
template <size_t SIZE>
struct PrControlState {
  SC_SIGNAL_MULTIW(bool, valid[SIZE]);
  SC_SIGNAL_MULTIW(bool, vtxInfoRcvd[SIZE]);
  SC_SIGNAL_MULTIW(bool, noNeighb[SIZE]);
  SC_SIGNAL_MULTIW(bool, lprRcvd[SIZE]);
  SC_SIGNAL_MULTIW(bool, prAccumDone[SIZE]);
  SC_SIGNAL_MULTIW(bool, initPr[SIZE]);
  SC_SIGNAL_MULTIW(EdgeId, edgeRemaining[SIZE]);

  void reset(size_t tag) {
    assert(tag < SIZE);
    valid[tag] = false;
  }
  void init(size_t tag) {
    valid[tag] = true;
    vtxInfoRcvd[tag] = false;
    lprRcvd[tag] = false;
    prAccumDone[tag] = false;
  }
};

struct PrDataState {
  VertexIdType vid;
  VertexData vinfo;
  PageRankType pr;
  PageRankType lpr;
};

class pagerank_hls : public sc_module
{
public:
  sc_in_clk clk;
  sc_in<bool> rst;
  //// functional ports////
  sc_in<Config> config;
  sc_in<bool> start;
  sc_out<bool> done;

  enum {
    VTX_ROW_SIZE = 128
  };
  typedef typename SizeT<VTX_ROW_SIZE>::Type Tag;


  // memory ports
  ga::tlm_fifo_out<MemTypedReadReqType<VertexData> > vtxReqOut;
  ga::tlm_fifo_in<MemTypedReadRespType<VertexData> > vtxRespIn;

  ga::tlm_fifo_out<MemTypedReadReqType<VertexIdType> > ovIdReqOut;
  ga::tlm_fifo_in<MemTypedReadRespType<VertexIdType> > ovIdRespIn;

  ga::tlm_fifo_out<MemSingleReadReqType<PageRankType, Tag> > ovprReqOut;
  ga::tlm_fifo_in<MemSingleReadRespType<PageRankType, Tag> > ovprRespIn;

  ga::tlm_fifo_out<MemTypedReadReqType<PageRankType> > lvprReqOut;
  ga::tlm_fifo_in<MemTypedReadRespType<PageRankType> > lvprRespIn;

  ga::tlm_fifo_out<MemTypedWriteReqType<PageRankType> > prWriteReqOut;
  ga::tlm_fifo_out<MemTypedWriteDataType<PageRankType> > prWriteDataOut;
  ga::ga_storage_fifo<Tag, VTX_ROW_SIZE> applyQ;
  ga::ga_storage_fifo<Tag, VTX_ROW_SIZE> gatherQ;
  ga::ga_storage_fifo<PageRankType, 2> writePrQ;
  ga::ga_storage_fifo<Tag, 2> commitQ;




  OooState<VTX_ROW_SIZE,PrControlState<VTX_ROW_SIZE>, PrDataState> oooState;

  //// state variables ////
  PRreal error;


  SC_HAS_PROCESS(pagerank_hls);

  pagerank_hls(sc_module_name modname) :
    sc_module(modname)
  , clk("clk"), rst("rst"), config("config"),
      start("start"), done("done"), vtxReqOut("vtxReqOut"),
      vtxRespIn("vtxRespIn"), ovIdReqOut("ovIdReqOut"),
      ovIdRespIn("ovIdRespIn"), ovprReqOut("ovprReqOut"),
      ovprRespIn("ovprRespIn"), lvprReqOut("lvprReqOut"),
      lvprRespIn("lvprRespIn"), prWriteReqOut("prWriteReqOut"),
      prWriteDataOut("prWriteDataOut"), commitQ("commitQ") {
    vtxRespIn.clk_rst(clk, rst);
    ovprRespIn.clk_rst(clk, rst);
    ovIdRespIn.clk_rst(clk, rst);
    prWriteReqOut.clk_rst(clk, rst);
    vtxReqOut.clk_rst(clk, rst);
    prWriteDataOut.clk_rst(clk, rst);
    ovprReqOut.clk_rst(clk, rst);
    ovIdReqOut.clk_rst(clk, rst);
    lvprRespIn.clk_rst(clk, rst);
    lvprReqOut.clk_rst(clk, rst);
    SC_CTHREAD(vertex_fetcher, clk.pos());
    async_reset_signal_is(rst, false);
    SC_CTHREAD(apply_proc, clk.pos());
    async_reset_signal_is(rst, false);
    SC_CTHREAD(commit_proc, clk.pos());
    async_reset_signal_is(rst, false);
    SC_CTHREAD(gather_proc, clk.pos());
    async_reset_signal_is(rst, false);
    SC_CTHREAD(apply_proc2, clk.pos());
    async_reset_signal_is(rst, false);
  }

  // requests and receives local vertex info and data, allocates a splace in oooState buffer
  // it also accumulates request for ovid in order to use the memory BW more efficiently (less waste in cacheline responses)
  void vertex_fetcher() {
    bool vtxAndEdgeRequestSent = false;
    {
      lvprRespIn.reset_get();
      lvprReqOut.reset_put();
      vtxReqOut.reset_put();
      vtxRespIn.reset_get();
      ovIdReqOut.reset_put();
      oooState.reset_alloc();
      gatherQ.reset_put();

    }
    // we request big chunks of edgeinfos to reduce waste in cachelines
    VtxId vtxIter = 0;
    unsigned ovd_offset = 0;
    unsigned ovd_size = 0;
    Tag vtx_sent_b4_ovid_sent = 0;
    wait();
    while (1) {
      {
        if (start && !vtxAndEdgeRequestSent && vtxReqOut.nb_can_put() && lvprReqOut.nb_can_put() ) {
          MemTypedReadReqType<VertexData> vtxReq(config.read().getVDAddr(config.read().vtxOffset), config.read().vtxCnt);
          MemTypedReadReqType<PageRankType> prReq(config.read().getReadPRAddr(config.read().vtxOffset), config.read().vtxCnt);
          vtxReqOut.nb_put(vtxReq);
          lvprReqOut.nb_put(prReq);
          vtxAndEdgeRequestSent = true;
          vtxIter = 0;
          ovd_offset = 0;
          ovd_size = 0;

        }

        if (!start.read() && done.read()) {
          vtxAndEdgeRequestSent = false;
        }

        if (vtxRespIn.nb_can_get() && lvprRespIn.nb_can_get() && gatherQ.nb_can_put() && ovIdReqOut.nb_can_put()) {
          MemTypedReadRespType<VertexData> vtxResp;
          MemTypedReadRespType<PageRankType> prResp;
          vtxRespIn.nb_get(vtxResp);
          lvprRespIn.nb_get(prResp);
          DBG_OUT << "vertex_fetcher: received vtx data resp and lvpr for vtxId " << config.read().vtxOffset+vtxIter << endl;

          Tag tag;
          while (!oooState.alloc(tag)) {
            DBG_OUT << "STALL, CANT ALLOC OOO state" << endl;
            wait();
          }

          oooState.state[tag].vid = vtxIter;
          oooState.state[tag].vinfo = vtxResp.data;
          oooState.state[tag].lpr = prResp.data;
          //oooState.state[tag].pr = 0;
          oooState.control.edgeRemaining[tag] = vtxResp.data.ovdSize;
          oooState.control.lprRcvd[tag] = true;
          oooState.control.vtxInfoRcvd[tag] = true;
          oooState.control.noNeighb[tag] = (vtxResp.data.ovdSize == 0);
          oooState.control.initPr[tag] = false;
          gatherQ.nb_put(tag);

          if (vtxIter == 0) {
            ovd_offset = vtxResp.data.ovdOffset;
          }
          ovd_size += vtxResp.data.ovdSize;

          // the condition on the vtx_sent_b4_ovid_sent is to avoid a deadlock that is possible
          // when the oooState buffer gets full with vertices before we could sent the edgeinfo requests below
          if ((vtxIter == config.read().vtxCnt-1 || ovd_size >= 128 || vtx_sent_b4_ovid_sent >= 32) && ovd_size != 0) {
            ovIdReqOut.nb_put(MemTypedReadReqType<VertexIdType>(config.read().getVIDAddr(ovd_offset),ovd_size));
            ovd_offset = vtxResp.data.ovdOffset + vtxResp.data.ovdSize;
            ovd_size = 0;
            vtx_sent_b4_ovid_sent = 0;
          }
          ++vtx_sent_b4_ovid_sent;
          DBG_OUT << "vertex_fetcher: vertex " <<  vtxIter << " started at tag = " << tag << " neighbors = " << vtxResp.data.ovdSize << endl;
          // increment vtx id
          ++vtxIter;
        } else {
          //if (vtxIter != config.read().vtxCnt)
          DBG_OUT << "STALL due to SMTH is FULL " << vtxRespIn.nb_can_get() << " " << lvprRespIn.nb_can_get() << " " << gatherQ.nb_can_put() << " " << ovIdReqOut.nb_can_put() <<  endl;
        }
      }
      wait();
    }
  }

  void gather_proc() {
    {
      gatherQ.reset_get();
      applyQ.reset_put();
      ovIdRespIn.reset_get();
      ovprReqOut.reset_put();
    }
    bool activeVtx = false;
    Tag tag = 0;
    VertexData vtxData;
    unsigned neighbToGo;
    wait();
    while (1) {
      {
        // iterate over neighbor ids to request neighbor PRs
        if (activeVtx && ovIdRespIn.nb_can_get() && ovprReqOut.nb_can_put()) {
          // last neighbor
          if (neighbToGo == 1 || neighbToGo == 0)
            activeVtx = false;

          MemTypedReadRespType<VertexIdType> ovIdResp;
          ovIdRespIn.nb_get(ovIdResp);
          ovprReqOut.nb_put(MemSingleReadReqType<PageRankType, Tag>(config.read().getReadPRAddr(ovIdResp.data.vid), tag));
          DBG_OUT << "gather_proc: vertex " <<  oooState.state[tag].vid << " started at tag = " << tag << " send other PR req for neighbor " << vtxData.ovdSize-neighbToGo << " out of " << vtxData.ovdSize << endl;
          neighbToGo--;
        }
        if (!activeVtx && gatherQ.nb_can_get() && applyQ.nb_can_put() ) {
          gatherQ.nb_get(tag);
          applyQ.nb_put(tag);
          vtxData = oooState.state[tag].vinfo;
          neighbToGo = vtxData.ovdSize;
          if (neighbToGo !=0) activeVtx = true;
          DBG_OUT << "gather_proc: next active vertex with " <<  vtxData.ovdSize << " neighbors " <<  endl;
        } else {
          DBG_OUT << "STALL due to gatherQ.nb_can_get() && applyQ.nb_can_put()  " << gatherQ.nb_can_get() << " " << applyQ.nb_can_put() <<  endl;
        }

      }
      wait();
    }
  }

  // pagerank accumulation is done in this process
  void apply_proc() {
    bool readConfig = false;
    Config conf;
    size_t bubble_counter = 0;
    size_t active_counter = 0;
    ovprRespIn.reset_get();
    wait();
    while (1) {
      {
        if (start && !readConfig) {
          readConfig = true;
          conf = config.read();
        }
        MemSingleReadRespType<PageRankType, Tag> prResp;
        while (!ovprRespIn.nb_can_get() ) {
          //cout << "SMTH FULL " << ovprRespIn.nb_can_get() << endl;
          bubble_counter++;
          wait();
        }

        ovprRespIn.nb_get(prResp);
        Tag tag = prResp.utag;
        assert(oooState.control.valid[tag] == true);
        assert(oooState.control.prAccumDone[tag] == false);
        PageRankType ovpr = prResp.data;
        // init if the first neighbor
        if (oooState.control.initPr[tag] == false) {
          oooState.state[tag].pr.setPr(ovpr.getPr());
          oooState.control.initPr[tag] = true;
        } else {
          oooState.state[tag].pr.setPr(oooState.state[tag].pr.getPr() + ovpr.getPr());
        }

        if (oooState.control.edgeRemaining[tag].read() == 1) {
          oooState.control.prAccumDone[tag] = true;
        }
        oooState.control.edgeRemaining[tag].write(oooState.control.edgeRemaining[tag].read() - 1);
        DBG_OUT << "apply_proc: ovpr arrived for tag " << tag << " vid = " << oooState.state[tag].vid.vid << " neighbors remaining = " << oooState.control.edgeRemaining[tag].read() - 1 << endl;

        active_counter++;

      }
      wait();
    }
  }

  // after accumulation we do multiplication and send to commit process
  // waits guarded by SYNTHESIS ifdef are to instruct HLS about the point where we want to do pipelining
  void apply_proc2() {
    {
      applyQ.reset_get();
      commitQ.reset_put();
      writePrQ.reset_put();
    }
    Config conf;
    PRreal lBETA=0.85;
    wait();
    while (1) {
      {
        Tag tag = applyQ.get();
        conf = config.read();
#ifdef __SYNTHESIS__
        wait();
#endif
        while (!oooState.control.noNeighb[tag] && !oooState.control.prAccumDone[tag])
          wait();

        // finalize PR computation
        PRreal pr = (oooState.control.noNeighb[tag]) ? 0.0 :  oooState.state[tag].pr.getPr();
        PRreal betaSum = lBETA*pr;
        PRreal betaSumOffset = conf.getRankOffset() + betaSum;
        pr = betaSumOffset*oooState.state[tag].vinfo.getOneOverVtxDegree();
#ifdef __SYNTHESIS__
        wait();
        wait();
        wait();
#endif

        while (!commitQ.nb_can_put() && !writePrQ.nb_can_put())
          wait();
        commitQ.nb_put(tag);
        writePrQ.nb_put(pr);
      }
      wait();
    }
  }

  // completes computation and frees oooState buffer credit
  // writes happen in order and in a streaming fashion
  void commit_proc() {
    done.write(false);
    bool sendWrReq = false;
    Config conf;
    size_t edgeCount = 0;

    size_t reset_ooo_buffer = VTX_ROW_SIZE;
    {
      commitQ.reset_get();
      oooState.reset_free();
      prWriteReqOut.reset_put();
      prWriteDataOut.reset_put();
      writePrQ.reset_get();
    }

    wait();
    while (1) {
      {
        if (start && !sendWrReq && prWriteReqOut.nb_can_put()) {
          prWriteReqOut.nb_put(MemTypedWriteReqType<PageRankType>(config.read().getWritePRAddr(config.read().vtxOffset), config.read().vtxCnt));
          sendWrReq = true;
          conf = config.read();
          //cout << "WRITE REQ AT OFFSET " << config.read().vtxOffset << " COUNT " <<  config.read().vtxCnt << endl;
        }
        if (!start.read() && done.read()) {
          sendWrReq = false;
          done = false;
        }

        //if (done == false) {
        if (reset_ooo_buffer != 0) {
          --reset_ooo_buffer;
          oooState.free(reset_ooo_buffer);
        } else if (!done && commitQ.nb_can_get() && writePrQ.nb_can_get() && prWriteDataOut.nb_can_put()) {
          Tag tag;
          commitQ.nb_get(tag);
          PageRankType pr;
          writePrQ.nb_get(pr);
          prWriteDataOut.nb_put(MemTypedWriteDataType<PageRankType>(pr));

          edgeCount+=oooState.state[tag].vinfo.ovdSize;


          if (oooState.state[tag].vid == conf.vtxCnt-1) {
            done = true;
            cout << "conf.iterCount=" << conf.iterCount << endl;
            cout << "conf.vtxOffset=" << conf.vtxOffset << endl;
            cout << "conf.vtxCnt =" << conf.vtxCnt << endl;

            //#ifndef __SYNTHESIS__
            if(conf.iterCount == 0)
              cout << "EDGE PROCESSED: " << edgeCount << endl;
            cout << "TIME DONE: " << sc_time_stamp() << " " << endl;
            cout << "EDGE THROUGHPUT: " << float((ITER_COUNT-conf.iterCount)*edgeCount)/(sc_time_stamp().value()/1000000) << endl;
            //#endif

          }

          //free tag
          oooState.free(tag);
          DBG_OUT << "Committed vertex " << oooState.state[tag].vid.vid << ", returning credit tag = " << tag << endl;

        }
        //}
      }
      wait();
    }
  }



};

//#define PR_CACHE

#include "accelerator_interface.h"
// pagerank_acc pr;
// pr.spl_rd_resp_in.valid
// pr.spl_rd_resp_in.ready
// pr.spl_rd_resp_in.data
class pagerank_acc : public accelerator_interface<Config> {
public:
  //  // primary IOs come from accelelatore_interface
  typedef LoadUnitParams<VertexData, 24, 1<<30, 2> VertexDataLoadParams;
  typedef LoadUnitParams<VertexIdType, 32, 1<<30> VertexIdLoadParams;
  typedef LoadUnitParams<PageRankType, 20, 1<<30> LVPageRankLoadParams;
  typedef LoadUnitSingleReqParams<PageRankType, pagerank_hls::Tag, 128> PageRankLoadParams;

  typedef LoadStoreUnitSingleReqParams<PageRankType, pagerank_hls::Tag, 64, 2, 128, 32> PageRankLoadStoreParams;

  typedef StoreUnitParams<PageRankType> PageRankStoreParams;
  // load/store units
  AccIn<VertexDataLoadParams>  vd_mem_in;
  AccIn<VertexIdLoadParams> vid_mem_in;
  AccIn<LVPageRankLoadParams> lvpr_mem_in;
  AccOut<PageRankStoreParams> pr_mem_out;

#ifndef PR_CACHE
  AccIn<PageRankLoadParams> pr_mem_in;
#else
  AccInOut<PageRankLoadStoreParams> pr_mem_in;
#endif
  // arbiters between ld/st units
  MemArbiter<1, SplMemWriteReqType, SplMemWriteRespType> wr_arbiter;
  MemArbiter<4, SplMemReadReqType, SplMemReadRespType> rd_arbiter;

  // main compute block
  pagerank_hls pr;

  //
  SimpleGate<3, GATE_AND> and_gate;
  IdleMonitor<8> idle_monitor;


  // channels to connect components above
  sc_signal<bool> wr_arb_idle, rd_arb_idle, acc_done, overall_done;

  AccIn<VertexDataLoadParams>::ChannelToArbiter vd_mem_arb_ch;
  AccIn<VertexIdLoadParams>::ChannelToArbiter vid_mem_arb_ch;
#ifndef PR_CACHE
  AccIn<PageRankLoadParams>::ChannelToArbiter pr_mem_in_arb_ch;
#else
  AccInOut<PageRankLoadStoreParams>::ChannelToArbiter pr_mem_in_arb_ch;
#endif
  AccIn<LVPageRankLoadParams>::ChannelToArbiter lvpr_mem_in_arb_ch;
  AccOut<PageRankStoreParams >::ChannelToArbiter pr_mem_out_arb_ch;

  ga::tlm_fifo<MemTypedReadReqType<VertexData> > vtx_req_ch;
  ga::tlm_fifo<MemTypedReadRespType<VertexData> > vtx_resp_ch;

  ga::tlm_fifo<MemTypedReadReqType<VertexIdType> > ovid_req_ch;
  ga::tlm_fifo<MemTypedReadRespType<VertexIdType> > ovid_resp_ch;

  ga::tlm_fifo<MemTypedReadReqType<PageRankType> > lvpr_req_ch;
  ga::tlm_fifo<MemTypedReadRespType<PageRankType> > lvpr_resp_ch;

  ga::tlm_fifo<MemSingleReadReqType<PageRankType, pagerank_hls::Tag> > ovpr_req_ch;
  ga::tlm_fifo<MemSingleReadRespType<PageRankType, pagerank_hls::Tag> > ovpr_resp_ch;
  ga::tlm_fifo<MemSingleWriteReqType<PageRankType, pagerank_hls::Tag> > ovpr_wr_req_ch;


  ga::tlm_fifo<MemTypedWriteReqType<PageRankType> > pr_write_req_ch;
  ga::tlm_fifo<MemTypedWriteDataType<PageRankType> > pr_write_data_ch;


  SC_HAS_PROCESS(pagerank_acc);

  pagerank_acc(sc_module_name name = sc_gen_unique_name("pagerank_acc")) : accelerator_interface<Config>(name),
      vd_mem_in("vd_mem_in"), vid_mem_in("vid_mem_in"), lvpr_mem_in(
          "lvpr_mem_in"), pr_mem_out(
              "pr_mem_out"), pr_mem_in("pr_mem_in"), wr_arbiter("wr_arbiter"), rd_arbiter("rd_arbiter"), pr(
                  "pr"), and_gate("and_gate"), idle_monitor(
                      "idle_monitor"), wr_arb_idle("wr_arb_idle"), rd_arb_idle(
                          "rd_arb_idle"), acc_done("acc_done"), overall_done("overall_done"), vtx_req_ch(
                              "vtx_req_ch"), vtx_resp_ch("vtx_resp_ch"), ovid_req_ch(
                                  "ovid_req_ch"), ovid_resp_ch(
                                      "ovid_resp_ch"), lvpr_req_ch("lvpr_req_ch"), lvpr_resp_ch(
                                          "lvpr_resp_ch"), ovpr_req_ch(
                                              "ovpr_req_ch"), ovpr_resp_ch("ovpr_resp_ch"), pr_write_req_ch(
                                                  "pr_write_req_ch"), pr_write_data_ch("pr_write_data_ch") {

    vd_mem_arb_ch.bindArbiter<4>(rd_arbiter,0,vd_mem_in);
    vid_mem_arb_ch.bindArbiter<4>(rd_arbiter,1,vid_mem_in);
    lvpr_mem_in_arb_ch.bindArbiter<4>(rd_arbiter,3,lvpr_mem_in);
    pr_mem_out_arb_ch.bindArbiter<1>(wr_arbiter,0,pr_mem_out);
#ifndef PR_CACHE
    pr_mem_in_arb_ch.bindArbiter<4>(rd_arbiter,2,pr_mem_in);
#else
    pr_mem_in_arb_ch.bindArbiter<4, 2>(rd_arbiter,2,pr_mem_in);
#endif
    rd_arbiter.clk(clk);
    rd_arbiter.rst(rst);
    wr_arbiter.clk(clk);
    wr_arbiter.rst(rst);
    vd_mem_in.clk(clk);
    vd_mem_in.rst(rst);
    vid_mem_in.clk(clk);
    vid_mem_in.rst(rst);
    pr_mem_in.clk(clk);
    pr_mem_in.rst(rst);
    lvpr_mem_in.clk(clk);
    lvpr_mem_in.rst(rst);
    pr_mem_out.clk(clk);
    pr_mem_out.rst(rst);
    idle_monitor.clk(clk);
    idle_monitor.rst(rst);

    pr.clk(clk);
    pr.rst(rst);
    pr.start(start);
    pr.config(config);

    pr.vtxReqOut(vtx_req_ch);
    pr.vtxRespIn(vtx_resp_ch);
    vd_mem_in.acc_req_in(vtx_req_ch);
    vd_mem_in.acc_resp_out(vtx_resp_ch);

    pr.ovIdReqOut(ovid_req_ch);
    pr.ovIdRespIn(ovid_resp_ch);
    vid_mem_in.acc_req_in(ovid_req_ch);
    vid_mem_in.acc_resp_out(ovid_resp_ch);

    pr.ovprReqOut(ovpr_req_ch);
    pr.ovprRespIn(ovpr_resp_ch);
#ifndef PR_CACHE
    pr_mem_in.acc_req_in(ovpr_req_ch);
    pr_mem_in.acc_resp_out(ovpr_resp_ch);
#else
    pr_mem_in.acc_rd_req_in(ovpr_req_ch);
    pr_mem_in.acc_wr_req_in(ovpr_wr_req_ch);
    pr_mem_in.acc_rd_resp_out(ovpr_resp_ch);
#endif
    pr.lvprReqOut(lvpr_req_ch);
    pr.lvprRespIn(lvpr_resp_ch);
    lvpr_mem_in.acc_req_in(lvpr_req_ch);
    lvpr_mem_in.acc_resp_out(lvpr_resp_ch);


    pr.prWriteReqOut(pr_write_req_ch);
    pr.prWriteDataOut(pr_write_data_ch);
    pr_mem_out.acc_req_in(pr_write_req_ch);
    pr_mem_out.acc_data_in(pr_write_data_ch);


    rd_arbiter.out_req_fifo(spl_rd_req);
    rd_arbiter.in_resp_fifo(spl_rd_resp);

    wr_arbiter.out_req_fifo(spl_wr_req);
    wr_arbiter.in_resp_fifo(spl_wr_resp);

    // idle/done ANDing
    wr_arbiter.idle(wr_arb_idle);
    and_gate.ins[0](wr_arb_idle);
    rd_arbiter.idle(rd_arb_idle);
    and_gate.ins[1](rd_arb_idle);
    pr.done(acc_done);
    and_gate.ins[2](acc_done);
    and_gate.out(overall_done);
    idle_monitor.in_idle(overall_done);
    idle_monitor.out_idle(done);


  }


};


#include "multi_acc_template.h"
// scheduler algorithm divides all vertices into chunk_size_in_vtx chunks and then send those chunks to accelerator units,
// when one accelerator unit completes the work, the next chunk is allocated. after all work is exhausted, it asserts done signal
template <size_t N>
class pagerank_sched : public sc_module, public acc_scheduler_interface<N, Config> {
public:

  //SimpleGate<N, GATE_AND> and_gate;

  SC_HAS_PROCESS(pagerank_sched);
  pagerank_sched(sc_module_name name = sc_gen_unique_name("pagerank_sched")) : sc_module(name),
      acc_scheduler_interface<N, Config>(){
    SC_CTHREAD(schedule_proc, this->clk.pos());
    async_reset_signal_is(this->rst, false);

  }

  void schedule_proc() {
    bool in_work[N];
    {
      SCHEDULE_RESET_UNROLL: for (unsigned int i = 0; i < N; ++i) {
        this->acc_start[i] = false;
        in_work[i] = false;
      }
    }
    unsigned int chunk_size_in_vtx = PageRankType::BitCnt * 32; // important to make it PageRank aligned to avoid partial writes when writing a pagerank values from different AUs
    unsigned int proccessed_vtx_count = 0;
    this->done = false;
    wait();
    while (1) {
      {
        if (this->start.read()) {
          Config conf = this->config.read();
          Config acc_conf;
          acc_conf.copy(conf);
          size_t i = 0;
          chunk_size_in_vtx = std::min(chunk_size_in_vtx, conf.vtxCnt);
          CONFIG_BROADCAST_WHILE: while(proccessed_vtx_count < conf.vtxCnt) {

            // we will stall in this loop until the job is assigned
            if (!this->acc_done[i].read() && !this->acc_start[i].read()) {
              acc_conf.vtxOffset = conf.vtxOffset + proccessed_vtx_count;
              acc_conf.vtxCnt = chunk_size_in_vtx;
              proccessed_vtx_count += chunk_size_in_vtx;
              chunk_size_in_vtx = std::min(chunk_size_in_vtx, conf.vtxCnt - proccessed_vtx_count);

              this->acc_config[i].write(acc_conf);
              this->acc_start[i] = true;
              in_work[i] = true;
            }

            if (this->acc_done[i].read() && this->acc_start[i].read()) {
              this->acc_start[i] = false;
              in_work[i] = false;
            }
            i = (i + 1)%N;
            wait();
          }
          bool all_done = true;
          OVERALL_DONE_UNROLL: for (unsigned i = 0; i < N; ++i) {
            bool accdone = (!in_work[i])? true: this->acc_done[i] && this->acc_start[i];
            all_done = all_done && accdone;
          }
          if (all_done) {
            this->done = true;
          }
        }
      }
      wait();
    }
  }
};

#if defined (USE_CTOS) && (__SYNTHESIS__)
SC_MODULE_EXPORT(pagerank_acc);
//SC_MODULE_EXPORT(pagerank_hls);
#endif


#endif //__PAGERANK_HLS_H__

