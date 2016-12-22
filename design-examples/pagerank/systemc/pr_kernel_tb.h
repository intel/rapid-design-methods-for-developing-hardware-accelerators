// See LICENSE for license details.
#ifndef KERNEL_TB_H_
#define KERNEL_TB_H_

/*[[[cog
     import cog
     from cog_acctempl import *
     from kernel_tb_params import *
  ]]]*/
//[[[end]]]

#include "systemc.h"
#include "clock_generator.h"
#include "ga_tlm_fifo.h"

class KernelTest : public sc_module
{
public:
  sc_in_clk clk;
  sc_in<bool> rst;
  sc_in<bool> done;
  sc_out<bool> start;

  // functional IOs

  /*[[[cog
       for p in inps:
         cog.outl("ga::tlm_fifo_in<%s > %s;" % (p.reqTy(), p.reqNmT()))
         cog.outl("ga::tlm_fifo_out<%s > %s;" % (p.respTy(), p.respNmT()))
         cog.outl("")
       for p in outs:
         cog.outl("ga::tlm_fifo_in<%s > %s;" % (p.reqTy(), p.reqNmT()))
         cog.outl("ga::tlm_fifo_in<%s > %s;" % (p.dataTy(), p.dataNmT()))
         cog.outl("")
    ]]]*/
  ga::tlm_fifo_in<MemTypedReadReqType<VertexData> > vtxReqIn;
  ga::tlm_fifo_out<MemTypedReadRespType<VertexData> > vtxRespOut;

  ga::tlm_fifo_in<MemTypedReadReqType<VertexIdType> > ovIdReqIn;
  ga::tlm_fifo_out<MemTypedReadRespType<VertexIdType> > ovIdRespOut;

  ga::tlm_fifo_in<MemSingleReadReqType<PageRankType,pagerank_hls::Tag> > ovprReqIn;
  ga::tlm_fifo_out<MemSingleReadRespType<PageRankType,pagerank_hls::Tag> > ovprRespOut;

  ga::tlm_fifo_in<MemTypedReadReqType<PageRankType> > lvprReqIn;
  ga::tlm_fifo_out<MemTypedReadRespType<PageRankType> > lvprRespOut;

  ga::tlm_fifo_in<MemTypedWriteReqType<PageRankType> > prWriteReqIn;
  ga::tlm_fifo_in<MemTypedWriteDataType<PageRankType> > prWriteDataIn;

  //[[[end]]]
  sc_out<Config> config_out;

  Config config;

  SC_HAS_PROCESS(KernelTest);

  //scide_waive SCIDE.8.15
  KernelTest(sc_module_name modname) :
    sc_module(modname)
  , clk("clk")
  , rst("rst"), done("done"), start("start")
  /*[[[cog
       for p in inps:
         cog.outl(""", %s("%s"), %s("%s")""" % (p.reqNmT(),p.reqNmT(),p.respNmT(),p.respNmT()))
       for p in outs:
         cog.outl(""", %s("%s"), %s("%s")""" % (p.reqNmT(),p.reqNmT(),p.dataNmT(),p.dataNmT()))
    ]]]*/
  , vtxReqIn("vtxReqIn"), vtxRespOut("vtxRespOut")
  , ovIdReqIn("ovIdReqIn"), ovIdRespOut("ovIdRespOut")
  , ovprReqIn("ovprReqIn"), ovprRespOut("ovprRespOut")
  , lvprReqIn("lvprReqIn"), lvprRespOut("lvprRespOut")
  , prWriteReqIn("prWriteReqIn"), prWriteDataIn("prWriteDataIn")
  //[[[end]]]
  {
    SC_CTHREAD(init_proc, clk.pos());
    async_reset_signal_is(rst, false);

    /*[[[cog
         for p in inps + outs:
           cog.outl("SC_CTHREAD(%s_proc, clk.pos());" % (p.nm,))
           cog.outl("async_reset_signal_is(rst, false);")
           cog.outl("")
      ]]]*/
    SC_CTHREAD(vtx_proc, clk.pos());
    async_reset_signal_is(rst, false);

    SC_CTHREAD(ovId_proc, clk.pos());
    async_reset_signal_is(rst, false);

    SC_CTHREAD(ovpr_proc, clk.pos());
    async_reset_signal_is(rst, false);

    SC_CTHREAD(lvpr_proc, clk.pos());
    async_reset_signal_is(rst, false);

    SC_CTHREAD(prWrite_proc, clk.pos());
    async_reset_signal_is(rst, false);

    //[[[end]]]
    /*[[[cog
         for p in inps:
           cog.outl("%s.clk_rst( clk, rst);" % (p.reqNmT(),))
           cog.outl("%s.clk_rst( clk, rst);" % (p.respNmT(),))
      ]]]*/
    vtxReqIn.clk_rst( clk, rst);
    vtxRespOut.clk_rst( clk, rst);
    ovIdReqIn.clk_rst( clk, rst);
    ovIdRespOut.clk_rst( clk, rst);
    ovprReqIn.clk_rst( clk, rst);
    ovprRespOut.clk_rst( clk, rst);
    lvprReqIn.clk_rst( clk, rst);
    lvprRespOut.clk_rst( clk, rst);
    //[[[end]]]

    /*[[[cog
         for p in outs:
           cog.outl("%s.clk_rst( clk, rst);" % (p.reqNmT(),))
           cog.outl("%s.clk_rst( clk, rst);" % (p.dataNmT(),))
      ]]]*/
    prWriteReqIn.clk_rst( clk, rst);
    prWriteDataIn.clk_rst( clk, rst);
    //[[[end]]]

  }

  /*[[[cog
       for p in inps:
         if type(p) is not TypedRead: continue
         cog.outl("void %s_proc() {" % (p.nm,))
         cog.outl("  %s.reset_get();" % (p.reqNmT(),))
         cog.outl("  %s.reset_put();" % (p.respNmT(),))
         cog.outl("  wait();")
         cog.outl("  while (1) {")
         cog.outl("    %s read_req;" % (p.reqTy(),))
         cog.outl("    %s.get(read_req);" % (p.reqNmT(),))
         cog.outl("")
         cog.outl("    const %s *ptr = reinterpret_cast<const %s*>( read_req.addr);" % (p.ty,p.ty))
         cog.outl("")
         cog.outl("    for (unsigned i=0; i<read_req.size; ++i) {")
         cog.outl("      %s read_resp;" % (p.respTy(),))
         cog.outl("")
         cog.outl("      read_resp.data = ptr[i];")
         cog.outl("")
         cog.outl("      %s.put(read_resp);" % (p.respNmT(),))
         cog.outl("      wait();")
         cog.outl("    }")
         cog.outl("  }")
         cog.outl("}")
         cog.outl("")
    ]]]*/
  void vtx_proc() {
    vtxReqIn.reset_get();
    vtxRespOut.reset_put();
    wait();
    while (1) {
      MemTypedReadReqType<VertexData> read_req;
      vtxReqIn.get(read_req);

      const VertexData *ptr = reinterpret_cast<const VertexData*>( read_req.addr);

      for (unsigned i=0; i<read_req.size; ++i) {
        MemTypedReadRespType<VertexData> read_resp;

        read_resp.data = ptr[i];

        vtxRespOut.put(read_resp);
        wait();
      }
    }
  }

  void ovId_proc() {
    ovIdReqIn.reset_get();
    ovIdRespOut.reset_put();
    wait();
    while (1) {
      MemTypedReadReqType<VertexIdType> read_req;
      ovIdReqIn.get(read_req);

      const VertexIdType *ptr = reinterpret_cast<const VertexIdType*>( read_req.addr);

      for (unsigned i=0; i<read_req.size; ++i) {
        MemTypedReadRespType<VertexIdType> read_resp;

        read_resp.data = ptr[i];

        ovIdRespOut.put(read_resp);
        wait();
      }
    }
  }

  void lvpr_proc() {
    lvprReqIn.reset_get();
    lvprRespOut.reset_put();
    wait();
    while (1) {
      MemTypedReadReqType<PageRankType> read_req;
      lvprReqIn.get(read_req);

      const PageRankType *ptr = reinterpret_cast<const PageRankType*>( read_req.addr);

      for (unsigned i=0; i<read_req.size; ++i) {
        MemTypedReadRespType<PageRankType> read_resp;

        read_resp.data = ptr[i];

        lvprRespOut.put(read_resp);
        wait();
      }
    }
  }

    //[[[end]]]
  /*[[[cog
       for p in inps:
         if type(p) is not SingleRead: continue
         cog.outl("void %s_proc() {" % (p.nm,))
         cog.outl("  %s.reset_get();" % (p.reqNmT(),))
         cog.outl("  %s.reset_put();" % (p.respNmT(),))
         cog.outl("  wait();")
         cog.outl("  while (1) {")
         cog.outl("    %s read_req;" % (p.reqTy(),))
         cog.outl("    %s.get(read_req);" % (p.reqNmT(),))
         cog.outl("")
         cog.outl("    const %s *ptr = reinterpret_cast<const %s*>( read_req.addr);" % (p.ty,p.ty))
         cog.outl("")
         cog.outl("    %s read_resp;" % (p.respTy(),))
         cog.outl("    read_resp.data = *ptr;")
         cog.outl("    read_resp.utag = read_req.utag;")
         cog.outl("")
         cog.outl("    %s.put(read_resp);" % (p.respNmT(),))
         cog.outl("    wait();")
         cog.outl("  }")
         cog.outl("}")
         cog.outl("")
    ]]]*/
  void ovpr_proc() {
    ovprReqIn.reset_get();
    ovprRespOut.reset_put();
    wait();
    while (1) {
      MemSingleReadReqType<PageRankType,pagerank_hls::Tag> read_req;
      ovprReqIn.get(read_req);

      const PageRankType *ptr = reinterpret_cast<const PageRankType*>( read_req.addr);

      MemSingleReadRespType<PageRankType,pagerank_hls::Tag> read_resp;
      read_resp.data = *ptr;
      read_resp.utag = read_req.utag;

      ovprRespOut.put(read_resp);
      wait();
    }
  }

  //[[[end]]]
  /*[[[cog
       for p in outs:
         cog.outl("void %s_proc() {" % (p.nm,))
         cog.outl("  %s.reset_get();" % (p.reqNmT(),))
         cog.outl("  %s.reset_get();" % (p.dataNmT(),))
         cog.outl("  wait();")
         cog.outl("  while (1) {")
         cog.outl("    %s write_req;" % (p.reqTy(),))
         cog.outl("    %s.get(write_req);" % (p.reqNmT(),))
         cog.outl("")
         cog.outl("    %s *ptr = reinterpret_cast<%s*>( write_req.addr);" % (p.ty, p.ty))
         cog.outl("")
         cog.outl("    for (unsigned i=0; i<write_req.size; ++i) {")
         cog.outl("      %s write_data;" % (p.dataTy(),))
         cog.outl("      %s.get(write_data);" % (p.dataNmT(),))
         cog.outl("      ptr[i] = write_data.data;")
         cog.outl("      wait();")
         cog.outl("    }")
         cog.outl("  }")
         cog.outl("}")
         cog.outl("")
    ]]]*/
  void prWrite_proc() {
    prWriteReqIn.reset_get();
    prWriteDataIn.reset_get();
    wait();
    while (1) {
      MemTypedWriteReqType<PageRankType> write_req;
      prWriteReqIn.get(write_req);

      PageRankType *ptr = reinterpret_cast<PageRankType*>( write_req.addr);

      for (unsigned i=0; i<write_req.size; ++i) {
        MemTypedWriteDataType<PageRankType> write_data;
        prWriteDataIn.get(write_data);
        ptr[i] = write_data.data;
        wait();
      }
    }
  }

  //[[[end]]]
  void init_proc() {
    start = false;
    size_t wait_cycles_after_last_input = 0;
    config_out.write(config);
    wait();
    while (1) {
        start = true;
        if (done) {
          wait_cycles_after_last_input++;
          if (wait_cycles_after_last_input > 1000) {
            sc_stop();
          }
        }
        wait();
      }
    }
};


#include "fpga_app_sw.h"
class AcclApp: public sc_module, public FpgaAppSwAlloc {
public:

  ClockGenerator clkgen;
  KernelTest tb;
  /*[[[cog
       cog.outl("%s kernel;" % module_name)
    ]]]*/
  pagerank_hls kernel;
  //[[[end]]]
  sc_signal<bool> clk;
  sc_signal<bool> rst;
  sc_signal<Config> config_ch;
  sc_signal<bool> start_ch;
  sc_signal<bool> done_ch;

  // channel for memory read port
  /*[[[cog
       for p in inps:
         cog.outl("ga::tlm_fifo<%s,2> %s;" % (p.reqTy(),p.reqNm()))
         cog.outl("ga::tlm_fifo<%s,2> %s;" % (p.respTy(),p.respNm()))
         cog.outl("")
    ]]]*/
  ga::tlm_fifo<MemTypedReadReqType<VertexData>,2> vtxReq;
  ga::tlm_fifo<MemTypedReadRespType<VertexData>,2> vtxResp;

  ga::tlm_fifo<MemTypedReadReqType<VertexIdType>,2> ovIdReq;
  ga::tlm_fifo<MemTypedReadRespType<VertexIdType>,2> ovIdResp;

  ga::tlm_fifo<MemSingleReadReqType<PageRankType,pagerank_hls::Tag>,2> ovprReq;
  ga::tlm_fifo<MemSingleReadRespType<PageRankType,pagerank_hls::Tag>,2> ovprResp;

  ga::tlm_fifo<MemTypedReadReqType<PageRankType>,2> lvprReq;
  ga::tlm_fifo<MemTypedReadRespType<PageRankType>,2> lvprResp;

  //[[[end]]]
  // channel for memory write port
  /*[[[cog
       for p in outs:
         cog.outl("ga::tlm_fifo<%s,2> %s;" % (p.reqTy(),p.reqNm()))
         cog.outl("ga::tlm_fifo<%s,2> %s;" % (p.dataTy(),p.dataNm()))
    ]]]*/
  ga::tlm_fifo<MemTypedWriteReqType<PageRankType>,2> prWriteReq;
  ga::tlm_fifo<MemTypedWriteDataType<PageRankType>,2> prWriteData;
  //[[[end]]]

  SC_HAS_PROCESS(AcclApp);

  AcclApp(sc_module_name name=sc_core::sc_gen_unique_name("AcclApp")) :
    clkgen("clkgen"), tb("tb"),
    kernel("kernel"),
    clk("clk"), rst("rst"),
    start_ch("start_ch"), done_ch("done_ch")
    /*[[[cog
         for p in inps:
           cog.outl(""", %s("%s"), %s("%s")""" % (p.reqNm(),p.reqNm(),p.respNm(),p.respNm()))
         for p in outs:
           cog.outl(""", %s("%s"), %s("%s")""" % (p.reqNm(),p.reqNm(),p.dataNm(),p.dataNm()))
      ]]]*/
    , vtxReq("vtxReq"), vtxResp("vtxResp")
    , ovIdReq("ovIdReq"), ovIdResp("ovIdResp")
    , ovprReq("ovprReq"), ovprResp("ovprResp")
    , lvprReq("lvprReq"), lvprResp("lvprResp")
    , prWriteReq("prWriteReq"), prWriteData("prWriteData")
    //[[[end]]]
  {
    tb.clk(clk);
    clkgen.clk(clk);
    tb.rst(rst);
    clkgen.rst(rst);

    kernel.clk(clk);
    kernel.rst(rst);

    tb.start(start_ch);
    kernel.start(start_ch);

    tb.done(done_ch);
    kernel.done(done_ch);

    tb.config_out(config_ch);
    kernel.config(config_ch);

    /*[[[cog
         for p in inps:
           cog.outl("tb.%s(%s);" % (p.reqNmT(),p.reqNm()))
           cog.outl("kernel.%s(%s);" % (p.reqNmK(),p.reqNm()))
           cog.outl("tb.%s(%s);" % (p.respNmT(),p.respNm()))
           cog.outl("kernel.%s(%s);" % (p.respNmK(),p.respNm()))
           cog.outl("")
      ]]]*/
    tb.vtxReqIn(vtxReq);
    kernel.vtxReqOut(vtxReq);
    tb.vtxRespOut(vtxResp);
    kernel.vtxRespIn(vtxResp);

    tb.ovIdReqIn(ovIdReq);
    kernel.ovIdReqOut(ovIdReq);
    tb.ovIdRespOut(ovIdResp);
    kernel.ovIdRespIn(ovIdResp);

    tb.ovprReqIn(ovprReq);
    kernel.ovprReqOut(ovprReq);
    tb.ovprRespOut(ovprResp);
    kernel.ovprRespIn(ovprResp);

    tb.lvprReqIn(lvprReq);
    kernel.lvprReqOut(lvprReq);
    tb.lvprRespOut(lvprResp);
    kernel.lvprRespIn(lvprResp);

    //[[[end]]]
    /*[[[cog
         for p in outs:
           cog.outl("tb.%s(%s);" % (p.reqNmT(),p.reqNm()))
           cog.outl("kernel.%s(%s);" % (p.reqNmK(),p.reqNm()))
           cog.outl("kernel.%s(%s);" % (p.dataNmK(),p.dataNm()))
           cog.outl("tb.%s(%s);" % (p.dataNmT(),p.dataNm()))
           cog.outl("")
      ]]]*/
    tb.prWriteReqIn(prWriteReq);
    kernel.prWriteReqOut(prWriteReq);
    kernel.prWriteDataOut(prWriteData);
    tb.prWriteDataIn(prWriteData);

    //[[[end]]]
  }

  virtual void  compute( const void * config_ptr,
                         const unsigned int config_size) {
    tb.config = *static_cast<const Config *>(config_ptr);
    clkgen.reset_state = true;
    sc_start();
    //reset done
    cout << "HW compute.." << endl;
    sc_start();
  }

  virtual void join() {}

};
#endif /* KERNEL_TB_H_ */
