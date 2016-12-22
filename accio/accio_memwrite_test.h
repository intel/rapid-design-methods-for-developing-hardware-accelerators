// See LICENSE for license details.


#ifndef __ACCIO_TB_WRITE_H__
#define __ACCIO_TB_WRITE_H__

#include "systemc.h"
#include "types.h"
#include "ga_tlm_fifo.h"
#include "clock_generator.h"
#include "accio.h"
#include <array>


class AccMemWriteTestbench : public sc_module
{
public:
  sc_in_clk clk;
  sc_in<bool> rst;

  struct TbInType {
    std::queue<AccMemWriteReqType> stream_req;
    std::queue<AccMemWriteDataType> stream_data;
  };
  struct TbOutType {
  };
  // comes from top
  std::array<CacheLineType, tb_params::DRAM_SIZE_IN_CLS> &dram; // 128KB
  TbInType &test_in;
  TbOutType &test_out;
  // comes from top ends
  bool active;

  ga::tlm_fifo_out<AccMemWriteReqType> acc_req_out;
  ga::tlm_fifo_out<AccMemWriteDataType> acc_data_out;
  ga::tlm_fifo_in<SplMemWriteReqType> spl_req_in;
  ga::tlm_fifo_in<SplMemWriteRespType> spl_resp_in;


  SC_HAS_PROCESS(AccMemWriteTestbench);

  // limit number of responses
  static const size_t TEST_REQ_NUM = 100;
  size_t req_counter;
  size_t resp_counter;


  void acc_thread() {
    active = false;
    acc_req_out.reset_put();
    acc_data_out.reset_put();
    size_t wait_cycles_after_last_input = 0;
    {
      resp_counter = 0;
      req_counter = 0;
    }
    wait();
    while (1) {
      if (active) {
        // limit the number of responses we expect to receive
        if (!test_in.stream_req.empty()) {
          // send a random data request
          if (acc_req_out.nb_can_put()) {
            AccMemWriteReqType accReq = test_in.stream_req.front();
            test_in.stream_req.pop();
            acc_req_out.put(accReq);
            cout << sc_time_stamp() << " TB:acc_thread send write request with size " << (unsigned int)accReq.size << " and address "<< accReq.addr << endl;

            for (unsigned i = 0; i < accReq.size; ++i) {
              assert(!test_in.stream_data.empty());
              AccMemWriteDataType data = test_in.stream_data.front();
              test_in.stream_data.pop();
              acc_data_out.put(data);
              wait();
            }
          }
        } else {
          wait_cycles_after_last_input++;
          if (wait_cycles_after_last_input > 100) {
            active = false;
            sc_pause();
          }
        }

      }
      wait();
    }
  }

  void spl_thread() {
    {
      spl_req_in.reset_get();
    }
    wait();
    while (1) {
      {
        if(spl_req_in.nb_can_get()) {
          SplMemWriteReqType splReq = spl_req_in.get();
          cout << sc_time_stamp() << " TB:spl_thread SPL write request received with address " << splReq.addr << " data " << splReq.data << endl;
          // we do read modify write
          CacheLineType read_cl =dram.at(splReq.addr/64);
          memcpy(&read_cl, &splReq.data+splReq.offset, splReq.width);
          dram.at(splReq.addr/64) = read_cl;
        }
      }
      wait();
    }
  }


  AccMemWriteTestbench(sc_module_name modname, std::array<CacheLineType, tb_params::DRAM_SIZE_IN_CLS> &dram, TbInType &test_in, TbOutType &test_out) :
    sc_module(modname), clk("clk"), rst("rst"),   dram(dram), test_in(test_in), test_out(test_out), active(false), acc_req_out("acc_req_out"), acc_data_out("acc_data_out"), spl_req_in("spl_req_in") {
    SC_CTHREAD(acc_thread, clk.pos());
    async_reset_signal_is(rst, false);
    SC_CTHREAD(spl_thread, clk.pos());
    async_reset_signal_is(rst, false);
    spl_resp_in.clk_rst(clk, rst);
    spl_req_in.clk_rst(clk, rst);
    acc_data_out.clk_rst(clk, rst);
    acc_req_out.clk_rst(clk, rst);
  }

};

class TestMemWriteTop : public sc_module
{
public:

  ClockGenerator clkgen;
  AccMemWriteTestbench acc_write_tb;
  AccMemOut acc_mem_out;
  sc_signal<bool> clk_ch;
  sc_signal<bool> rst_ch;
  ga::tlm_fifo<SplMemWriteRespType, 2> spl_resp_out_ch;
  ga::tlm_fifo<SplMemWriteReqType, 2> spl_req_out_ch;
  ga::tlm_fifo<AccMemWriteReqType, 2> acc_req_out_ch;
  ga::tlm_fifo<AccMemWriteDataType, 2> acc_data_out_ch;
  SC_HAS_PROCESS(TestMemWriteTop);

  std::array<CacheLineType, tb_params::DRAM_SIZE_IN_CLS> dram; // 128KB
  AccMemWriteTestbench::TbInType test_in;
  AccMemWriteTestbench::TbOutType test_out;


  TestMemWriteTop(sc_module_name modname) :
      sc_module(modname), clkgen("clkgen"),
      acc_write_tb("acc_write_tb", dram, test_in, test_out),
      acc_mem_out("acc_mem_out"), clk_ch("clk_ch"), rst_ch("rst_ch"),
      spl_resp_out_ch("spl_resp_out_ch"), spl_req_out_ch("spl_req_out_ch"),
      acc_req_out_ch("acc_req_out_ch"), acc_data_out_ch("acc_data_out_ch") {
    acc_write_tb.clk(clk_ch);
    clkgen.clk(clk_ch);
    acc_write_tb.rst(rst_ch);
    clkgen.rst(rst_ch);
    acc_mem_out.clk(clk_ch);
    acc_mem_out.rst(rst_ch);
    acc_write_tb.spl_req_in(spl_req_out_ch);
    acc_mem_out.spl_req_out(spl_req_out_ch);
    acc_write_tb.acc_req_out(acc_req_out_ch);
    acc_mem_out.acc_req_in(acc_req_out_ch);
    acc_write_tb.acc_data_out(acc_data_out_ch);
    acc_mem_out.acc_data_in(acc_data_out_ch);
    acc_write_tb.spl_resp_in(spl_resp_out_ch);
    acc_mem_out.spl_resp_out(spl_resp_out_ch);


  }

  void reset() {
    clkgen.reset_state = true;
  }
  void execute() {
    acc_write_tb.active = true;
  }

};







#endif //__ACCIO_TB_H__

