// See LICENSE for license details.
/*
 * accio_simple_arbiter_test.h
 *
 *  Created on: Jun 21, 2016
 *      Author: aayupov
 */

#ifndef ACCIO_SIMPLE_ARBITER_TEST_H_
#define ACCIO_SIMPLE_ARBITER_TEST_H_

#include "systemc.h"
#include "ga_tlm_fifo.h"
#include "types.h"
#include "acc_mem_arbiter.h"

SC_MODULE(AccArbTb) {
  sc_in_clk clk;
  sc_in<bool> rst;

  ga::tlm_fifo_out<SplMemReadReqType> rd_req1;
  ga::tlm_fifo_out<SplMemReadReqType> rd_req2;
  ga::tlm_fifo_out<SplMemReadReqType> rd_req3;
  ga::tlm_fifo_out<SplMemReadReqType> rd_req4;

  ga::tlm_fifo_in<SplMemReadReqType> spl_req1;
  ga::tlm_fifo_in<SplMemReadReqType> spl_req2;

  ga::tlm_fifo_out<SplMemReadRespType> spl_rsp1;
  ga::tlm_fifo_out<SplMemReadRespType> spl_rsp2;

  ga::tlm_fifo_in<SplMemReadRespType> rd_rsp1;
  ga::tlm_fifo_in<SplMemReadRespType> rd_rsp2;
  ga::tlm_fifo_in<SplMemReadRespType> rd_rsp3;
  ga::tlm_fifo_in<SplMemReadRespType> rd_rsp4;

  struct TbInType {
    std::queue<SplMemReadReqType> req1;
    std::queue<SplMemReadReqType> req2;
    std::queue<SplMemReadReqType> req3;
    std::queue<SplMemReadReqType> req4;
  };
  struct TbOutType {
    std::queue<SplMemReadRespType> resp1;
    std::queue<SplMemReadRespType> resp2;
    std::queue<SplMemReadRespType> resp3;
    std::queue<SplMemReadRespType> resp4;
  };
  // comes from top
  TbInType &test_in;
  TbOutType &test_out;
  // comes from top ends
  bool active;

  SC_HAS_PROCESS(AccArbTb);

  AccArbTb(sc_module_name modname, std::array<CacheLineType, tb_params::DRAM_SIZE_IN_CLS> &dram, TbInType &test_in, TbOutType &test_out) :
    sc_module(modname), clk("clk"), rst("rst"), rd_req1("rd_req1"), rd_req2("rd_req2"), rd_req3("rd_req3"), rd_req4("rd_req4"), spl_req1("spl_req1"), spl_req2("spl_req2"), spl_rsp1("spl_rsp1"), spl_rsp2("spl_rsp2"), rd_rsp1("rd_rsp1"), rd_rsp2("rd_rsp2"), rd_rsp3("rd_rsp3"), rd_rsp4("rd_rsp4"),
    test_in(test_in), test_out(test_out), active(false)
  {
    SC_CTHREAD(acc_thread, clk.pos());
    async_reset_signal_is(rst, false);
    SC_CTHREAD(spl_thread, clk.pos());
    async_reset_signal_is(rst, false);
    spl_req1.clk_rst(clk, rst);
    spl_req2.clk_rst(clk, rst);
    spl_rsp1.clk_rst(clk, rst);
    spl_rsp2.clk_rst(clk, rst);
    rd_req1.clk_rst(clk, rst);
    rd_req2.clk_rst(clk, rst);
    rd_req3.clk_rst(clk, rst);
    rd_req4.clk_rst(clk, rst);
    rd_rsp1.clk_rst(clk, rst);
    rd_rsp2.clk_rst(clk, rst);
    rd_rsp3.clk_rst(clk, rst);
    rd_rsp4.clk_rst(clk, rst);
  }

  template <typename QUEUE, typename OUT>
  bool trySendDataToChannel(QUEUE& queue, OUT& out) {
    if (!queue.empty() && out.nb_can_put()) {
      out.nb_put(queue.front());
      queue.pop();
      return true;
    }
    return false;
  }

  template <typename QUEUE, typename IN>
  bool trySendDataToQueue(IN& in, QUEUE& queue) {
    if (in.nb_can_get()) {
      SplMemReadRespType data;
      bool success = in.nb_get(data);
      assert(success);
      queue.push(data);
      return true;
    }
    return false;
  }


  void acc_thread() {
    active = false;
    size_t wait_cycles_after_last_input = 0;
    rd_req1.reset_put();
    rd_req2.reset_put();
    rd_req3.reset_put();
    rd_req4.reset_put();
    rd_rsp1.reset_get();
    rd_rsp2.reset_get();
    rd_rsp3.reset_get();
    rd_rsp4.reset_get();
    {
    }
    wait();
    while (1) {
      {
        if (active) {
          trySendDataToChannel(test_in.req1, rd_req1);
          trySendDataToChannel(test_in.req2, rd_req2);
          trySendDataToChannel(test_in.req3, rd_req3);
          trySendDataToChannel(test_in.req4, rd_req4);


          trySendDataToQueue(rd_rsp1, test_out.resp1);
          trySendDataToQueue(rd_rsp2, test_out.resp2);
          trySendDataToQueue(rd_rsp3, test_out.resp3);
          trySendDataToQueue(rd_rsp4, test_out.resp4);

          if (test_in.req1.empty() && test_in.req2.empty() && test_in.req3.empty() && test_in.req4.empty()) {
            wait_cycles_after_last_input++;
            if (wait_cycles_after_last_input > 1000) {
              cout << "DONE <- " << "AccArbTb::acc_thread" << endl;
              active = false;
              sc_pause();
            }
          }
        }
      }
      wait();
    }
  }

  void spl_thread() {
    {
      spl_rsp1.reset_put();
      spl_rsp2.reset_put();
      spl_req1.reset_get();
      spl_req2.reset_get();
    }
    wait();
    while (1) {
      {
        if (spl_req1.nb_can_get() && spl_rsp1.nb_can_put()) {
          SplMemReadReqType req = spl_req1.get();
          SplMemReadRespType rsp(CacheLineType(0,0,0,0,0,0,0, req.addr*2), req.io_unit_id, req.tag);
          spl_rsp1.nb_put(rsp);
        }
        if (spl_req2.nb_can_get() && spl_rsp2.nb_can_put()) {
          SplMemReadReqType req = spl_req2.get();
          SplMemReadRespType rsp(CacheLineType(0,0,0,0,0,0,0, req.addr*2), req.io_unit_id, req.tag);
          spl_rsp2.nb_put(rsp);
        }
      }
      wait(1+rand()%10);

    }
  }

};

class AccArbTbTop : public sc_module
{
public:

  ClockGenerator clkgen;
  AccArbTb acc_arb_tb;

  MemArbiterN<4,2,SplMemReadReqType, SplMemReadRespType> arbiter;

  sc_signal<bool> clk_ch;
  sc_signal<bool> rst_ch;

  sc_signal<bool> rd_arb_idle;


  ga::tlm_fifo<SplMemReadReqType> rd_req1;
  ga::tlm_fifo<SplMemReadReqType> rd_req2;
  ga::tlm_fifo<SplMemReadReqType> rd_req3;
  ga::tlm_fifo<SplMemReadReqType> rd_req4;

  ga::tlm_fifo<SplMemReadReqType> spl_req1;
  ga::tlm_fifo<SplMemReadReqType> spl_req2;

  ga::tlm_fifo<SplMemReadRespType> spl_rsp1;
  ga::tlm_fifo<SplMemReadRespType> spl_rsp2;

  ga::tlm_fifo<SplMemReadRespType> rd_rsp1;
  ga::tlm_fifo<SplMemReadRespType> rd_rsp2;
  ga::tlm_fifo<SplMemReadRespType> rd_rsp3;
  ga::tlm_fifo<SplMemReadRespType> rd_rsp4;

  SC_HAS_PROCESS(AccTbTop);

  std::array<CacheLineType, tb_params::DRAM_SIZE_IN_CLS> dram; // 128KB
  typename AccArbTb::TbInType test_in;
  typename AccArbTb::TbOutType test_out;


  AccArbTbTop(sc_module_name modname = sc_gen_unique_name("TestMemArbiterTop")) :
    sc_module(modname), clkgen("clkgen"), acc_arb_tb("acc_arb_tb",dram,
        test_in, test_out), arbiter("arbiter"), clk_ch("clk_ch"), rst_ch(
            "rst_ch"), rd_arb_idle("rd_arb_idle"), rd_req1("rd_req1"), rd_req2(
                "rd_req2"), rd_req3("rd_req3"), rd_req4("rd_req4"), spl_req1(
                    "spl_req1"), spl_req2("spl_req2"), spl_rsp1("spl_rsp1"), spl_rsp2(
                        "spl_rsp2"), rd_rsp1("rd_rsp1"), rd_rsp2("rd_rsp2"), rd_rsp3(
                            "rd_rsp3"), rd_rsp4("rd_rsp4") {

    arbiter.clk(clk_ch);
    arbiter.rst(rst_ch);
    acc_arb_tb.clk(clk_ch);
    acc_arb_tb.rst(rst_ch);

    arbiter.in_req_fifo[0](rd_req1);
    arbiter.in_req_fifo[1](rd_req2);
    arbiter.in_req_fifo[2](rd_req3);
    arbiter.in_req_fifo[3](rd_req4);

    acc_arb_tb.rd_req1(rd_req1);
    acc_arb_tb.rd_req2(rd_req2);
    acc_arb_tb.rd_req3(rd_req3);
    acc_arb_tb.rd_req4(rd_req4);

    arbiter.out_resp_fifo[0](rd_rsp1);
    arbiter.out_resp_fifo[1](rd_rsp2);
    arbiter.out_resp_fifo[2](rd_rsp3);
    arbiter.out_resp_fifo[3](rd_rsp4);

    acc_arb_tb.rd_rsp1(rd_rsp1);
    acc_arb_tb.rd_rsp2(rd_rsp2);
    acc_arb_tb.rd_rsp3(rd_rsp3);
    acc_arb_tb.rd_rsp4(rd_rsp4);

    arbiter.out_req_fifo[0](spl_req1);
    arbiter.out_req_fifo[1](spl_req2);
    acc_arb_tb.spl_req1(spl_req1);
    acc_arb_tb.spl_req2(spl_req2);

    arbiter.in_resp_fifo[0](spl_rsp1);
    arbiter.in_resp_fifo[1](spl_rsp2);
    acc_arb_tb.spl_rsp1(spl_rsp1);
    acc_arb_tb.spl_rsp2(spl_rsp2);

    arbiter.idle(rd_arb_idle);
    clkgen.rst(rst_ch);
    clkgen.clk(clk_ch);
  }


  void reset() {
    clkgen.reset_state = true;
  }
  void execute() {
    acc_arb_tb.active = true;
  }
};


class AccArbSimpleTbTop : public sc_module
{
public:

  ClockGenerator clkgen;
  AccArbTb acc_arb_tb;

  MemArbiterNoTag<4,SplMemReadReqType, SplMemReadRespType, AU_ARBITER> arbiter;

  sc_signal<bool> clk_ch;
  sc_signal<bool> rst_ch;

  sc_signal<bool> rd_arb_idle;


  ga::tlm_fifo<SplMemReadReqType> rd_req1;
  ga::tlm_fifo<SplMemReadReqType> rd_req2;
  ga::tlm_fifo<SplMemReadReqType> rd_req3;
  ga::tlm_fifo<SplMemReadReqType> rd_req4;

  ga::tlm_fifo<SplMemReadReqType> spl_req1;
  ga::tlm_fifo<SplMemReadReqType> spl_req2;

  ga::tlm_fifo<SplMemReadRespType> spl_rsp1;
  ga::tlm_fifo<SplMemReadRespType> spl_rsp2;

  ga::tlm_fifo<SplMemReadRespType> rd_rsp1;
  ga::tlm_fifo<SplMemReadRespType> rd_rsp2;
  ga::tlm_fifo<SplMemReadRespType> rd_rsp3;
  ga::tlm_fifo<SplMemReadRespType> rd_rsp4;

  SC_HAS_PROCESS(AccArbSimpleTbTop);

  std::array<CacheLineType, tb_params::DRAM_SIZE_IN_CLS> dram; // 128KB
  typename AccArbTb::TbInType test_in;
  typename AccArbTb::TbOutType test_out;


  AccArbSimpleTbTop(sc_module_name modname = sc_gen_unique_name("TestMemArbiterTop")) :
    sc_module(modname), clkgen("clkgen"), acc_arb_tb("acc_arb_tb",dram,
        test_in, test_out), arbiter("arbiter"), clk_ch("clk_ch"), rst_ch(
            "rst_ch"), rd_arb_idle("rd_arb_idle"), rd_req1("rd_req1"), rd_req2(
                "rd_req2"), rd_req3("rd_req3"), rd_req4("rd_req4"), spl_req1(
                    "spl_req1"), spl_req2("spl_req2"), spl_rsp1("spl_rsp1"), spl_rsp2(
                        "spl_rsp2"), rd_rsp1("rd_rsp1"), rd_rsp2("rd_rsp2"), rd_rsp3(
                            "rd_rsp3"), rd_rsp4("rd_rsp4") {

    arbiter.clk(clk_ch);
    arbiter.rst(rst_ch);
    acc_arb_tb.clk(clk_ch);
    acc_arb_tb.rst(rst_ch);

    arbiter.in_req_fifo[0](rd_req1);
    arbiter.in_req_fifo[1](rd_req2);
    arbiter.in_req_fifo[2](rd_req3);
    arbiter.in_req_fifo[3](rd_req4);

    acc_arb_tb.rd_req1(rd_req1);
    acc_arb_tb.rd_req2(rd_req2);
    acc_arb_tb.rd_req3(rd_req3);
    acc_arb_tb.rd_req4(rd_req4);

    arbiter.out_resp_fifo[0](rd_rsp1);
    arbiter.out_resp_fifo[1](rd_rsp2);
    arbiter.out_resp_fifo[2](rd_rsp3);
    arbiter.out_resp_fifo[3](rd_rsp4);

    acc_arb_tb.rd_rsp1(rd_rsp1);
    acc_arb_tb.rd_rsp2(rd_rsp2);
    acc_arb_tb.rd_rsp3(rd_rsp3);
    acc_arb_tb.rd_rsp4(rd_rsp4);

    arbiter.out_req_fifo(spl_req1);
    acc_arb_tb.spl_req1(spl_req1);
    acc_arb_tb.spl_req2(spl_req2);

    arbiter.in_resp_fifo(spl_rsp1);
    acc_arb_tb.spl_rsp1(spl_rsp1);
    acc_arb_tb.spl_rsp2(spl_rsp2);

    arbiter.idle(rd_arb_idle);
    clkgen.rst(rst_ch);
    clkgen.clk(clk_ch);
  }


  void reset() {
    clkgen.reset_state = true;
  }
  void execute() {
    acc_arb_tb.active = true;
  }
};



#endif /* ACCIO_SIMPLE_ARBITER_TEST_H_ */
