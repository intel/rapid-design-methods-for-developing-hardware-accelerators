// See LICENSE for license details.

#ifndef SPL_MEM_NETWORK_H_
#define SPL_MEM_NETWORK_H_

#include "systemc.h"
#include "types.h"
#include "accio.h"
#include "acc_typed_io.h"
#include "acc_mem_arbiter.h"

template <size_t NUM_OF_READERS, size_t NUM_OF_WRITERS>
class SplMemNetwork : public sc_module {
public:
  sc_in_clk clk;
  sc_in<bool> rst;

  AccMemIn<> acc_in[NUM_OF_READERS];
  AccMemOut acc_out[NUM_OF_WRITERS];

  MemArbiter<NUM_OF_WRITERS, SplMemWriteReqType, SplMemWriteRespType> wr_arbiter;
  MemArbiter<NUM_OF_READERS, SplMemReadReqType, SplMemReadRespType> rd_arbiter;

  ga::tlm_fifo<SplMemReadRespType> spl_rd_resp_fifo[NUM_OF_READERS];
  ga::tlm_fifo<SplMemReadReqType> spl_rd_req_fifo[NUM_OF_READERS];
  ga::tlm_fifo<SplMemWriteRespType> spl_wr_resp_fifo[NUM_OF_WRITERS];
  ga::tlm_fifo<SplMemWriteReqType> spl_wr_req_fifo[NUM_OF_WRITERS];


  ga::tlm_fifo_hier_in<AccMemWriteReqType> acc_wr_req[NUM_OF_WRITERS];
  ga::tlm_fifo_hier_in<AccMemWriteDataType> acc_wr_data[NUM_OF_WRITERS];
  ga::tlm_fifo_hier_in<AccMemReadReqType> acc_rd_req[NUM_OF_READERS];
  ga::tlm_fifo_hier_out<AccMemReadRespType> acc_rd_resp[NUM_OF_READERS];

  ga::tlm_fifo_hier_in<SplMemReadRespType> spl_rd_resp;
  ga::tlm_fifo_hier_out<SplMemReadReqType> spl_rd_req;
  ga::tlm_fifo_hier_in<SplMemWriteRespType> spl_wr_resp;
  ga::tlm_fifo_hier_out<SplMemWriteReqType> spl_wr_req;

  SC_HAS_PROCESS(SplMemNetwork);

  SplMemNetwork(sc_module_name name) :
      clk("clk"), rst("rst"), wr_arbiter("wr_arbiter"), rd_arbiter(
          "rd_arbiter"), spl_rd_resp("spl_rd_resp"), spl_rd_req("spl_rd_req"), spl_wr_resp(
          "spl_wr_resp"), spl_wr_req("spl_wr_req") {
    for (unsigned i = 0; i < NUM_OF_READERS; ++i) {
      acc_in[i].clk(clk);
      acc_in[i].rst(rst);
      acc_in[i].acc_req_in(acc_rd_req[i]);
      acc_in[i].acc_resp_out(acc_rd_resp[i]);
      acc_in[i].spl_req_out(spl_rd_req_fifo[i]);
      acc_in[i].spl_resp_in(spl_rd_resp_fifo[i]);
      rd_arbiter.in_req_fifo[i](spl_rd_req_fifo[i]);
      rd_arbiter.out_resp_fifo[i](spl_rd_resp_fifo[i]);
    }

    for (unsigned i = 0; i < NUM_OF_WRITERS; ++i) {
      acc_out[i].clk(clk);
      acc_out[i].rst(rst);
      acc_out[i].acc_req_in(acc_wr_req[i]);
      acc_out[i].acc_data_in(acc_wr_data[i]);
      acc_out[i].spl_req_out(spl_wr_req_fifo[i]);
      acc_out[i].spl_resp_out(spl_wr_resp_fifo[i]);
      wr_arbiter.in_req_fifo[i](spl_wr_req_fifo[i]);
      wr_arbiter.out_resp_fifo[i](spl_wr_resp_fifo[i]);
    }

    rd_arbiter.clk(clk);
    rd_arbiter.rst(rst);
    wr_arbiter.clk(clk);
    wr_arbiter.rst(rst);

    rd_arbiter.out_req_fifo(spl_rd_req);
    rd_arbiter.in_resp_fifo(spl_rd_resp);
    wr_arbiter.out_req_fifo(spl_wr_req);
    wr_arbiter.in_resp_fifo(spl_wr_resp);

  }

};

template <typename LDUNIT_PARAMS>
class AccIn : public sc_module {
#ifdef STRATUS
  HLS_INLINE_MODULE;
#endif
public:
  typedef typename LDUNIT_PARAMS::type AccDataType;

  AccMemIn<LDUNIT_PARAMS::cl_request_buffer_size> acc_mem_in;
  AccMemTypedIn<AccDataType, LDUNIT_PARAMS::utype_request_buffer_size, LDUNIT_PARAMS::utype_max_request_burst_count> acc_typed_in;

  sc_in_clk clk;
  sc_in<bool> rst;

  // functional ports
  ga::tlm_fifo_hier_out<SplMemReadReqType> spl_req_out;
  ga::tlm_fifo_hier_in<SplMemReadRespType> spl_resp_in;

  ga::tlm_fifo_hier_out<MemTypedReadRespType<AccDataType> > acc_resp_out;
  ga::tlm_fifo_hier_in<MemTypedReadReqType<AccDataType> > acc_req_in;

  // interconnects
  ga::tlm_fifo<AccMemReadReqType> inter_req;
  ga::tlm_fifo<AccMemReadRespType> inter_resp;


  SC_HAS_PROCESS(AccIn);

  AccIn(sc_module_name name) :
      acc_mem_in(sc_gen_unique_name(name)), acc_typed_in(sc_gen_unique_name(name)), clk("clk"), rst(
          "rst"),spl_req_out("spl_req_out"), spl_resp_in("spl_resp_in"), acc_resp_out(
          "acc_resp_out"), acc_req_in("acc_req_in"), inter_req("inter_req"), inter_resp(
          "inter_resp") {


    acc_mem_in.clk(clk);
    acc_mem_in.rst(rst);
    acc_mem_in.acc_resp_out(inter_resp);
    acc_mem_in.acc_req_in(inter_req);
    acc_mem_in.spl_req_out(spl_req_out);
    acc_mem_in.spl_resp_in(spl_resp_in);


    acc_typed_in.clk(clk);
    acc_typed_in.rst(rst);
    acc_typed_in.mem_req_out(inter_req);
    acc_typed_in.mem_resp_in(inter_resp);
    acc_typed_in.acc_resp_out(acc_resp_out);
    acc_typed_in.acc_req_in(acc_req_in);

  }

  struct ChannelToArbiter {
    ga::tlm_fifo<SplMemReadReqType> spl_req_ch;
    ga::tlm_fifo<SplMemReadRespType> spl_resp_ch;

    ChannelToArbiter() :
      spl_req_ch("spl_req_ch"), spl_resp_ch("spl_resp_ch") {}

    template <size_t NUM_OF_READERS>
    void bindArbiter(MemArbiter<NUM_OF_READERS, SplMemReadReqType, SplMemReadRespType> &arb, size_t arb_index, AccIn &acc_in) {
      assert(arb_index < NUM_OF_READERS);
      arb.in_req_fifo[arb_index](spl_req_ch);
      arb.out_resp_fifo[arb_index](spl_resp_ch);
      acc_in.spl_req_out(spl_req_ch);
      acc_in.spl_resp_in(spl_resp_ch);
    }
  };

};

//template <typename T, size_t REQ_BUF_SIZE=16, size_t MAX_BURST_COUNT=256, size_t UTYPE_REQ_BUF_SIZE=REQ_BUF_SIZE>

template <typename STUNIT_PARAMS>
class AccOut : public sc_module {
#ifdef STRATUS
  HLS_INLINE_MODULE;
#endif
public:

  typedef typename STUNIT_PARAMS::type AccDataType;

  AccMemOut acc_mem_out;
  AccMemTypedOut<AccDataType> acc_typed_out;

  sc_in_clk clk;
  sc_in<bool> rst;

  // functional ports
  ga::tlm_fifo_hier_out<SplMemWriteReqType> spl_req_out;
  ga::tlm_fifo_hier_in<SplMemWriteRespType> spl_resp_in;

  ga::tlm_fifo_hier_in<MemTypedWriteReqType<AccDataType> > acc_req_in;
  ga::tlm_fifo_hier_in<MemTypedWriteDataType<AccDataType> > acc_data_in;

  // interconnects
  ga::tlm_fifo<AccMemWriteReqType> inter_req;
  ga::tlm_fifo<AccMemWriteDataType> inter_data;


  SC_HAS_PROCESS(AccOut);

  AccOut(sc_module_name name) :
      sc_module(name), acc_mem_out(sc_gen_unique_name(name)), acc_typed_out(sc_gen_unique_name(name)), clk("clk"), rst("rst"), spl_req_out("spl_req_out"), spl_resp_in(
          "spl_resp_in"), acc_req_in("acc_req_in"), acc_data_in("acc_data_in"), inter_req(
          "inter_req"), inter_data("inter_data") {
    acc_mem_out.clk(clk);
    acc_mem_out.rst(rst);
    acc_mem_out.acc_data_in(inter_data);
    acc_mem_out.acc_req_in(inter_req);
    acc_mem_out.spl_req_out(spl_req_out);
    acc_mem_out.spl_resp_out(spl_resp_in);


    acc_typed_out.clk(clk);
    acc_typed_out.rst(rst);
    acc_typed_out.mem_req_out(inter_req);
    acc_typed_out.mem_data_out(inter_data);
    acc_typed_out.acc_data_in(acc_data_in);
    acc_typed_out.acc_req_in(acc_req_in);
  }

  struct ChannelToArbiter {
    ga::tlm_fifo<SplMemWriteReqType> spl_req_ch;
    ga::tlm_fifo<SplMemWriteRespType> spl_resp_ch;

    template <size_t NUM_OF_WRITERS>
    void bindArbiter(MemArbiter<NUM_OF_WRITERS, SplMemWriteReqType, SplMemWriteRespType> &arb, size_t arb_index, AccOut &acc_out) {
      assert(arb_index < NUM_OF_WRITERS);
      arb.in_req_fifo[arb_index](spl_req_ch);
      arb.out_resp_fifo[arb_index](spl_resp_ch);
      acc_out.spl_req_out(spl_req_ch);
      acc_out.spl_resp_in(spl_resp_ch);
    }
  };

};

#include "acc_single_io.h"


template <size_t NUM_OF_READERS, size_t NUM_OF_WRITERS>
struct SplMemToTypedIOChannel {
  ga::tlm_fifo<AccMemReadRespType> acc_rd_resp_fifo[NUM_OF_READERS];
  ga::tlm_fifo<AccMemReadReqType> acc_rd_req_fifo[NUM_OF_READERS];
  ga::tlm_fifo<AccMemWriteReqType> acc_wr_req_fifo[NUM_OF_WRITERS];
  ga::tlm_fifo<AccMemWriteDataType> acc_wr_data_fifo[NUM_OF_WRITERS];

  template <typename T>
  void bindReader(SplMemNetwork<NUM_OF_READERS, NUM_OF_WRITERS> &ntw, AccMemTypedIn<T> &memin, size_t index) {
    assert(index < NUM_OF_READERS);
    ntw.acc_rd_req[index](acc_rd_req_fifo[index]);
    memin.mem_req_out(acc_rd_req_fifo[index]);
    ntw.acc_rd_resp[index](acc_rd_resp_fifo[index]);
    memin.mem_resp_in(acc_rd_resp_fifo[index]);
  }
  template <typename T>
  void bindWriter(SplMemNetwork<NUM_OF_READERS, NUM_OF_WRITERS> &ntw, AccMemTypedOut<T> &memout, size_t index) {
    assert(index < NUM_OF_WRITERS);
    ntw.acc_wr_req[index](acc_wr_req_fifo[index]);
    memout.mem_req_out(acc_wr_req_fifo[index]);
    ntw.acc_wr_data[index](acc_wr_data_fifo[index]);
    memout.mem_data_out(acc_wr_data_fifo[index]);
  }
};


//#if defined (USE_CTOS) && (__SYNTHESIS__)
//SC_MODULE_EXPORT(SplMemNetwork<2,1>);
//#endif

//#ifdef __CTOS__
//int sc_main(int argc, char *argv[]) {
//  SplMemNetwork<1,2> splmem("SPLMEM");
//  return 0;
//}
//#endif



#endif /* SPL_MEM_NETWORK_H_ */
