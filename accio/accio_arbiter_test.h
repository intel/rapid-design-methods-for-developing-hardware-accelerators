// See LICENSE for license details.
/*
 * accio_arbiter_test.h
 *
 *  Created on: Jan 28, 2016
 *      Author: aayupov
 */

#ifndef ACCIO_ARBITER_TEST_H_
#define ACCIO_ARBITER_TEST_H_

#include "ga_tlm_fifo.h"
#include "acc_mem_arbiter.h"
#include "acc_typed_io.h"
#include "spl_mem_network.h"
#include <queue>

#ifdef CTOS_MODEL
#include "splmem_ctos_wrapper.h"
#endif


struct AccDataType {
  UInt64 data;
  AccDataType() {}
  AccDataType(int d) : data(d) {}
  AccDataType(UInt64 d) : data(d) {}

  enum {
    BitCnt = 64
  };
  static size_t getBitCnt() {
    return BitCnt;
  }

  static size_t numberOfFields() {
    return 1;
  }

  static size_t fieldWidth(size_t index) {
    switch(index) {
    case 0:
      return 64;
    default:
      assert(0);
    }
    return 0;
  }
  void putField(size_t index, UInt64 d) {
    switch(index) {
    case 0:
      data = d;
      break;
    default:
      assert(0);
    }
  }

  UInt64 getField(size_t index) const {
    switch(index) {
    case 0:
      return data;
    default:
      assert(0);
    }
    return 0;
  }

  inline friend void sc_trace(sc_trace_file* tf, const AccDataType& d,
      const std::string& name) {
  }

  inline friend std::ostream& operator<<(std::ostream& os, const AccDataType& d) {
    os << "data: " << d.data << std::endl;
    return os;
  }

  inline bool operator==(const AccDataType& rhs) const {
    bool result = true;
    result = result && (data == rhs.data);
    return result;
  }
};


class AccTb : public sc_module {
public:
  sc_in_clk clk;
  sc_in<bool> rst;

  struct TbInType {
    std::queue<MemTypedReadReqType<AccDataType> > rd_req1;
    std::queue<MemTypedReadReqType<AccDataType> > rd_req2;
    std::queue<MemTypedWriteReqType<AccDataType> > wr_req1;
    std::queue<MemTypedWriteDataType<AccDataType> > wr_data1;
  };
  struct TbOutType {
    std::queue<MemTypedReadRespType<AccDataType> > rd_resp1;
    std::queue<MemTypedReadRespType<AccDataType> > rd_resp2;
  };
  // comes from top
  std::array<CacheLineType, tb_params::DRAM_SIZE_IN_CLS> &dram; // 128KB
  TbInType &test_in;
  TbOutType &test_out;
  // comes from top ends
  bool active;

  // functional IOs
  ga::tlm_fifo_out<MemTypedReadReqType<AccDataType> > acc_rd_req_out1;
  ga::tlm_fifo_out<MemTypedReadReqType<AccDataType> > acc_rd_req_out2;
  ga::tlm_fifo_in<MemTypedReadRespType<AccDataType> > acc_rd_resp_in1;
  ga::tlm_fifo_in<MemTypedReadRespType<AccDataType> > acc_rd_resp_in2;
  ga::tlm_fifo_out<MemTypedWriteReqType<AccDataType> > acc_wr_req_out1;
  ga::tlm_fifo_out<MemTypedWriteDataType<AccDataType> > acc_wr_data_out1;
  ga::tlm_fifo_in<SplMemReadReqType> spl_rd_req_in;
  ga::tlm_fifo_out<SplMemReadRespType> spl_rd_resp_out;
  ga::tlm_fifo_in<SplMemWriteReqType> spl_wr_req_in;
  ga::tlm_fifo_in<SplMemWriteRespType> spl_wr_resp_out;



  SC_HAS_PROCESS(AccTb);

  AccTb(sc_module_name modname, std::array<CacheLineType, tb_params::DRAM_SIZE_IN_CLS> &dram, TbInType &test_in, TbOutType &test_out) :
    sc_module(modname), clk("clk"), rst("rst"),   dram(dram), test_in(test_in), test_out(test_out), active(false),
      acc_rd_req_out1("acc_rd_req_out1"),
      acc_rd_req_out2("acc_rd_req_out2"), acc_rd_resp_in1("acc_rd_resp_in1"),
      acc_rd_resp_in2("acc_rd_resp_in2"), acc_wr_req_out1("acc_wr_req_out1"),
      acc_wr_data_out1("acc_wr_data_out1"), spl_rd_req_in("spl_rd_req_in"),
      spl_rd_resp_out("spl_rd_resp_out"), spl_wr_req_in("spl_wr_req_in"),
      spl_wr_resp_out("spl_wr_resp_out") {
    SC_CTHREAD(accelerator_request, clk.pos());
    async_reset_signal_is(rst, false);
    SC_CTHREAD(accelerator_cmd, clk.pos());
    async_reset_signal_is(rst, false);
    SC_CTHREAD(spl_response, clk.pos());
    async_reset_signal_is(rst, false);
    spl_wr_resp_out.clk_rst(clk, rst);
    acc_rd_resp_in2.clk_rst(clk, rst);
    acc_rd_resp_in1.clk_rst(clk, rst);
    spl_rd_req_in.clk_rst(clk, rst);
    spl_wr_req_in.clk_rst(clk, rst);
    acc_wr_data_out1.clk_rst(clk, rst);
    acc_wr_req_out1.clk_rst(clk, rst);
    spl_rd_resp_out.clk_rst(clk, rst);
    acc_rd_req_out1.clk_rst(clk, rst);
    acc_rd_req_out2.clk_rst(clk, rst);
  }

  void accelerator_request() {
    active = false;
    acc_rd_req_out1.reset_put();
    acc_rd_req_out2.reset_put();
    acc_wr_req_out1.reset_put();
    acc_wr_data_out1.reset_put();
    size_t wait_cycles_after_last_input = 0;
    {
    }
    wait();
    while (1) {
      if (active) {
        if (!test_in.rd_req1.empty()&& acc_rd_req_out1.nb_can_put()) {
          acc_rd_req_out1.nb_put(test_in.rd_req1.front());
          DBG_OUT << sc_time_stamp() << " accio_arbiter_tb:acc_thread send rd1 request with size " << (unsigned int)test_in.rd_req1.front().size << " and address "<< test_in.rd_req1.front().addr << endl;
          test_in.rd_req1.pop();
        }
        if (!test_in.rd_req2.empty() && acc_rd_req_out2.nb_can_put()) {
          acc_rd_req_out2.nb_put(test_in.rd_req2.front());
          DBG_OUT << sc_time_stamp() << " accio_arbiter_tb:acc_thread send rd2 request with size " << (unsigned int)test_in.rd_req2.front().size << " and address "<< test_in.rd_req2.front().addr << endl;
          test_in.rd_req2.pop();
        }
        if (!test_in.wr_req1.empty() && acc_wr_req_out1.nb_can_put()) {
          MemTypedWriteReqType<AccDataType> wr_req = test_in.wr_req1.front();
          acc_wr_req_out1.nb_put(wr_req);
          test_in.wr_req1.pop();
          DBG_OUT << sc_time_stamp() << " accio_arbiter_tb:acc_thread send wr request with size " << (unsigned int)wr_req.size << " and address "<< wr_req.addr << endl;

          for (unsigned i = 0; i < wr_req.size; ++i) {
            assert(!test_in.wr_data1.empty());
            MemTypedWriteDataType<AccDataType> data = test_in.wr_data1.front();
            test_in.wr_data1.pop();
            acc_wr_data_out1.put(data);
            wait();
            DBG_OUT << sc_time_stamp() << " accio_arbiter_tb:acc_thread send wr request with data " << data.data << endl;
          }
        }
      }

      if (active && test_in.wr_req1.empty() && test_in.rd_req1.empty() && test_in.rd_req2.empty())
      {
        wait_cycles_after_last_input++;
        if (wait_cycles_after_last_input > 100) {
          cout << "DONE <- " << " accio_arbiter" << endl;
          active = false;
          sc_pause();
        }

      }
      wait();
    }
  }

  void accelerator_cmd() {
    {
      acc_rd_resp_in1.reset_get();
      acc_rd_resp_in2.reset_get();
    }
    wait();
    while (1) {
      {
        if (acc_rd_resp_in1.nb_can_get() ) {
          test_out.rd_resp1.push(acc_rd_resp_in1.get());
        }
        if (acc_rd_resp_in2.nb_can_get() ) {
          test_out.rd_resp2.push(acc_rd_resp_in2.get());
        }
      }
      wait();
    }
  }

  void spl_response() {
    {
      spl_rd_resp_out.reset_put();
      spl_rd_req_in.reset_get();
      spl_wr_req_in.reset_get();
    }
    wait();
    while (1) {
      {
        if(spl_rd_req_in.nb_can_get()) {
          SplMemReadReqType splReq = spl_rd_req_in.get();
          //spl should be able to always send the response. arbiter and upstream logic should take care of that
          assert(spl_rd_resp_out.nb_can_put());
          SplMemReadRespType splResp;
          splResp.io_unit_id = splReq.io_unit_id;
          splResp.tag = splReq.tag;

          splResp.data = dram.at(splReq.addr/64);
          spl_rd_resp_out.put(splResp);
          DBG_OUT << sc_time_stamp() << " accio_arbiter_tb:spl_thread SPL response generated and sent with data "<< splResp.data << endl;
        }
        if(spl_wr_req_in.nb_can_get()) {
          SplMemWriteReqType splReq = spl_wr_req_in.get();
          DBG_OUT << sc_time_stamp() << " accio_arbiter_tb:spl_thread SPL write request received with address " << splReq.addr << " data " << splReq.data << endl;
          // we do read modify write
          CacheLineType read_cl =dram.at(splReq.addr/64);
          memcpy(&read_cl, &splReq.data+splReq.offset, splReq.width);

          dram.at(splReq.addr/64) = read_cl;
        }

      }
      wait();
    }
  }
};

class AccTbTop : public sc_module
{
public:

  ClockGenerator clkgen;
  AccTb acc_read_tb;
#ifdef CTOS_MODEL
  mem_ntw_ctos_wrapper mem_ntw;
  //("HevcPakHlsCABACBs_syn", CTOS_TARGET_SUFFIX(CTOS_MODEL),"",false);
#else
  SplMemNetwork<2,1> mem_ntw;
#endif

  AccMemTypedIn<AccDataType> acc_in1;
  AccMemTypedIn<AccDataType> acc_in2;
  AccMemTypedOut<AccDataType> acc_out1;
  sc_signal<bool> clk_ch;
  sc_signal<bool> rst_ch;

  sc_signal<bool> rd_arb_idle;
  sc_signal<bool> wr_arb_idle;

  ga::tlm_fifo<MemTypedReadRespType<AccDataType>, 2> rd_resp_out_ch1;
  ga::tlm_fifo<MemTypedReadReqType<AccDataType>, 2> rd_req_out_ch1;
  ga::tlm_fifo<MemTypedReadRespType<AccDataType>, 2> rd_resp_out_ch2;
  ga::tlm_fifo<MemTypedReadReqType<AccDataType>, 2> rd_req_out_ch2;
  ga::tlm_fifo<MemTypedWriteDataType<AccDataType>, 2> wr_data_out_ch1;
  ga::tlm_fifo<MemTypedWriteReqType<AccDataType>, 2> wr_req_out_ch1;

  ga::tlm_fifo<AccMemReadRespType, 2> acc_rd_resp_out_ch1;
  ga::tlm_fifo<AccMemReadReqType, 2> acc_rd_req_out_ch1;
  ga::tlm_fifo<AccMemReadRespType, 2> acc_rd_resp_out_ch2;
  ga::tlm_fifo<AccMemReadReqType, 2> acc_rd_req_out_ch2;
  ga::tlm_fifo<AccMemWriteDataType, 2> acc_wr_data_out_ch1;
  ga::tlm_fifo<AccMemWriteReqType, 2> acc_wr_req_out_ch1;

  ga::tlm_fifo<SplMemWriteReqType, 2> spl_wr_req_out_ch;
  ga::tlm_fifo<SplMemWriteRespType, 2> spl_wr_resp_out_ch;
  ga::tlm_fifo<SplMemReadReqType, 2> spl_rd_req_out_ch;
  ga::tlm_fifo<SplMemReadRespType, 2> spl_rd_resp_out_ch;

  SC_HAS_PROCESS(AccTbTop);

  std::array<CacheLineType, tb_params::DRAM_SIZE_IN_CLS> dram; // 128KB
  typename AccTb::TbInType test_in;
  typename AccTb::TbOutType test_out;


  AccTbTop(sc_module_name modname = sc_gen_unique_name("TestMemArbiterTop")) :
    sc_module(modname), clkgen("clkgen"), acc_read_tb("acc_read_tb", dram,
        test_in, test_out),

#ifdef CTOS_MODEL //this is required for stand-alone test using CtoS generated files
        mem_ntw("mem_ntw", CTOS_TARGET_SUFFIX(CTOS_MODEL),NULL,false)
#else
  mem_ntw("mem_ntw"),
#endif
  acc_in1("acc_in1"), acc_in2("acc_in2"), acc_out1("acc_out1"), clk_ch("clk_ch"), rst_ch("rst_ch") {
    acc_read_tb.clk(clk_ch);
    clkgen.clk(clk_ch);
    acc_read_tb.rst(rst_ch);
    clkgen.rst(rst_ch);
    acc_in1.clk(clk_ch);
    acc_in1.rst(rst_ch);
    acc_in2.clk(clk_ch);
    acc_in2.rst(rst_ch);
    acc_out1.clk(clk_ch);
    acc_out1.rst(rst_ch);
    mem_ntw.clk(clk_ch);
    mem_ntw.rst(rst_ch);
    mem_ntw.rd_arbiter.idle(rd_arb_idle);
    mem_ntw.wr_arbiter.idle(wr_arb_idle);

    acc_in1.acc_req_in(rd_req_out_ch1);
    acc_in1.acc_resp_out(rd_resp_out_ch1);
    acc_in1.mem_req_out(acc_rd_req_out_ch1);
    acc_in1.mem_resp_in(acc_rd_resp_out_ch1);
    acc_in2.acc_req_in(rd_req_out_ch2);
    acc_in2.acc_resp_out(rd_resp_out_ch2);
    acc_in2.mem_req_out(acc_rd_req_out_ch2);
    acc_in2.mem_resp_in(acc_rd_resp_out_ch2);
    acc_out1.mem_req_out(acc_wr_req_out_ch1);
    acc_out1.acc_req_in(wr_req_out_ch1);
    acc_out1.mem_data_out(acc_wr_data_out_ch1);
    acc_out1.acc_data_in(wr_data_out_ch1);

    mem_ntw.acc_rd_req[0](acc_rd_req_out_ch1);
    mem_ntw.acc_rd_req[1](acc_rd_req_out_ch2);
    mem_ntw.acc_rd_resp[0](acc_rd_resp_out_ch1);
    mem_ntw.acc_rd_resp[1](acc_rd_resp_out_ch2);
    mem_ntw.acc_wr_req[0](acc_wr_req_out_ch1);
    mem_ntw.acc_wr_data[0](acc_wr_data_out_ch1);

    mem_ntw.spl_rd_resp(spl_rd_resp_out_ch);
    mem_ntw.spl_rd_req(spl_rd_req_out_ch);
    mem_ntw.spl_wr_resp(spl_wr_resp_out_ch);
    mem_ntw.spl_wr_req(spl_wr_req_out_ch);


    acc_read_tb.acc_rd_req_out1(rd_req_out_ch1);
    acc_read_tb.acc_rd_req_out2(rd_req_out_ch2);
    acc_read_tb.acc_rd_resp_in1(rd_resp_out_ch1);
    acc_read_tb.acc_rd_resp_in2(rd_resp_out_ch2);
    acc_read_tb.acc_wr_req_out1(wr_req_out_ch1);
    acc_read_tb.acc_wr_data_out1(wr_data_out_ch1);

    acc_read_tb.spl_rd_req_in(spl_rd_req_out_ch);
    acc_read_tb.spl_rd_resp_out(spl_rd_resp_out_ch);
    acc_read_tb.spl_wr_req_in(spl_wr_req_out_ch);
    acc_read_tb.spl_wr_resp_out(spl_wr_resp_out_ch);
  }

  void reset() {
    clkgen.reset_state = true;
  }
  void execute() {
    acc_read_tb.active = true;
  }
};
#endif /* ACCIO_ARBITER_TEST_H_ */
