// See LICENSE for license details.


#ifndef __ACCIO_TB_READ_H__
#define __ACCIO_TB_READ_H__

#include "gtest/gtest.h"
#include "systemc.h"
#include "defines.h"
#include "types.h"
#include "ga_tlm_fifo.h"
#include "clock_generator.h"
#include "accio.h"
#include <array>
#include <queue>


class AccMemReadTestbench : public sc_module
{
public:
	sc_in_clk clk;
	sc_in<bool> rst;

  struct TbInType {
    std::queue<AccMemReadReqType> stream;
  };
  struct TbOutType {
    std::queue<AccMemReadRespType> stream;
  };
  // comes from top
  std::array<CacheLineType, tb_params::DRAM_SIZE_IN_CLS> &dram; // 128KB
  TbInType &test_in;
  TbOutType &test_out;
  // comes from top ends
  bool active;

  // functional IOs
	ga::tlm_fifo_out<AccMemReadReqType> acc_req_out;
	ga::tlm_fifo_in<AccMemReadRespType> acc_resp_in;
  ga::tlm_fifo_in<SplMemReadReqType> spl_req_in;
  ga::tlm_fifo_out<SplMemReadRespType> spl_resp_out;

  ga::ga_storage_fifo<SplMemReadRespType, 10> fast_response_fifo;
  ga::ga_storage_fifo<SplMemReadRespType, 10> slow_response_fifo;

	SC_HAS_PROCESS(AccMemReadTestbench);

	// limit number of responses
//  template <size_t SIZE>
//	void fill_randomly(std::array<CacheLineType, SIZE> &array) {
//    for (int i = 0; i < SIZE; ++i) {
//      CacheLineType cl = CacheLineType::generate_random();
//      array[i] = cl;
//    }
//  }

  void acc_thread() {
    active = false;
    acc_req_out.reset_put();
    acc_resp_in.reset_get();
    size_t wait_cycles_after_last_input = 0;
    {
    }
    wait();
    while (1) {
      if (active) {
        // limit the number of responses we expect to receive
        if (!test_in.stream.empty()) {
          // send a random data request
          if (acc_req_out.nb_can_put()) {
            AccMemReadReqType accReq = test_in.stream.front();
            test_in.stream.pop();
            acc_req_out.put(accReq);
            cout << sc_time_stamp() << " TB:acc_thread send request with size " << (unsigned int)accReq.size << " and address "<< accReq.addr << endl;
          }
        } else {
          wait_cycles_after_last_input++;
          if (wait_cycles_after_last_input > 100) {
            active = false;
            sc_pause();
          }
        }
        // data is back
        if (acc_resp_in.nb_can_get()) {
          // introduce a delay on accelerator receiving side
          AccMemReadRespType rsp = acc_resp_in.get();
          cout << sc_time_stamp().to_string() << " TB:acc_thread response came back with data " << rsp.data.words[0] << endl;
          wait(rand()%1000);
          test_out.stream.push(rsp);

        }
      }
      wait();
    }
  }

  void spl_thread() {
    {
      //spl_resp_out.reset_put();
      fast_response_fifo.reset_put();
      slow_response_fifo.reset_put();
      spl_req_in.reset_get();
    }
    wait();
    while (1) {
      {
        if(spl_req_in.nb_can_get()) {
          SplMemReadReqType splReq = spl_req_in.get();
          cout << sc_time_stamp() << " TB:spl_thread SPL request received with address " << splReq.addr <<  endl;

          //assert(spl_resp_out.nb_can_put());

          SplMemReadRespType splResp;
          splResp.io_unit_id = splReq.io_unit_id;
          splResp.tag = splReq.tag;
          splResp.data = dram.at(splReq.addr/64);
          // push to either fast or slow queue
          if (splReq.tag%2) {
            fast_response_fifo.put(splResp);
          } else {
            slow_response_fifo.put(splResp);
          }

        }
      }
      wait();
    }
  }


  void spl_response_thread() {
    const int fast_slow_ratio = 3;
    int fast_slow_ratio_cnt = 0;
    {
      spl_resp_out.reset_put();
      fast_response_fifo.reset_get();
      slow_response_fifo.reset_get();
    }
    wait();
    while (1) {
      {
        if (fast_slow_ratio_cnt !=  0 && fast_response_fifo.nb_can_get()) {
          SplMemReadRespType splResp = fast_response_fifo.get();
          spl_resp_out.put(splResp);
          cout << sc_time_stamp() << " TB:spl_thread SPL response generated and sent with data "<< splResp.data.words[0] << endl;
        } else if (slow_response_fifo.nb_can_get()) {
          SplMemReadRespType splResp = slow_response_fifo.get();
          spl_resp_out.put(splResp);
          cout << sc_time_stamp() << " TB:spl_thread SPL response generated and sent with data "<< splResp.data.words[0] << endl;
        }

        fast_slow_ratio_cnt = (fast_slow_ratio_cnt + 1) % fast_slow_ratio;
      }
      wait();
    }
  }

  AccMemReadTestbench(sc_module_name modname, std::array<CacheLineType, tb_params::DRAM_SIZE_IN_CLS> &dram, TbInType &test_in, TbOutType &test_out) :
      sc_module(modname), clk("clk"), rst("rst"), dram(dram), test_in(test_in), test_out(test_out), active(false), acc_req_out("acc_req_out"), acc_resp_in(
          "acc_resp_in"), spl_req_in("spl_req_in"), spl_resp_out("spl_resp_out")  {
    SC_CTHREAD(acc_thread, clk.pos());
    async_reset_signal_is(rst, false);
    SC_CTHREAD(spl_thread, clk.pos());
    async_reset_signal_is(rst, false);
    SC_CTHREAD(spl_response_thread, clk.pos());
    async_reset_signal_is(rst, false);
    acc_resp_in.clk_rst(clk, rst);
    spl_req_in.clk_rst(clk, rst);
    spl_resp_out.clk_rst(clk, rst);
    acc_req_out.clk_rst(clk, rst);
  }

};

class TestMemReadTop : public sc_module
{
public:

  ClockGenerator clkgen;
  AccMemReadTestbench acc_read_tb;
  AccMemIn<> acc_mem_in;
  sc_signal<bool> clk_ch;
  sc_signal<bool> rst_ch;
  ga::tlm_fifo<AccMemReadRespType, 2> acc_resp_out_ch;
  ga::tlm_fifo<SplMemReadReqType, 2> spl_req_out_ch;
  ga::tlm_fifo<AccMemReadReqType, 2> acc_req_out_ch;
  ga::tlm_fifo<SplMemReadRespType, 2> spl_resp_out_ch;
	SC_HAS_PROCESS(TestMemReadTop);

	std::array<CacheLineType, tb_params::DRAM_SIZE_IN_CLS> dram; // 128KB
	AccMemReadTestbench::TbInType test_in;
	AccMemReadTestbench::TbOutType test_out;


  TestMemReadTop(sc_module_name modname = sc_gen_unique_name("TestMemReadTop")) :
      sc_module(modname), clkgen("clkgen_"), acc_read_tb("tb_", dram, test_in,
          test_out), acc_mem_in("accio_"), clk_ch("clk_ch"), rst_ch("rst_ch"), acc_resp_out_ch(
          "acc_resp_out_ch"), spl_req_out_ch("spl_req_out_ch"), acc_req_out_ch(
          "acc_req_out_ch"), spl_resp_out_ch("spl_resp_out_ch") {
    acc_read_tb.clk(clk_ch);
    clkgen.clk(clk_ch);
    acc_read_tb.rst(rst_ch);
    clkgen.rst(rst_ch);
    acc_mem_in.clk(clk_ch);
    acc_mem_in.rst(rst_ch);
    acc_read_tb.acc_resp_in(acc_resp_out_ch);
    acc_mem_in.acc_resp_out(acc_resp_out_ch);
    acc_read_tb.spl_req_in(spl_req_out_ch);
    acc_mem_in.spl_req_out(spl_req_out_ch);
    acc_read_tb.acc_req_out(acc_req_out_ch);
    acc_mem_in.acc_req_in(acc_req_out_ch);
    acc_read_tb.spl_resp_out(spl_resp_out_ch);
    acc_mem_in.spl_resp_in(spl_resp_out_ch);
  }

  void reset() {
    clkgen.reset_state = true;
  }
  void execute() {
    acc_read_tb.active = true;
  }

};







#endif //__ACCIO_TB_H__

