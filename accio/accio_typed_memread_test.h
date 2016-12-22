// See LICENSE for license details.
/*
 * accio_typed_memread_test.h
 *
 *  Created on: Jan 26, 2016
 *      Author: aayupov
 */

#ifndef ACCIO_TYPED_MEMREAD_TEST_H_
#define ACCIO_TYPED_MEMREAD_TEST_H_

#include "systemc.h"
#include "defines.h"
#include "types.h"
#include "ga_tlm_fifo.h"
#include "clock_generator.h"
#include "accio.h"
#include "acc_typed_io.h"
#include <array>
#include <queue>


template<typename T>
class TypedMemReadTestbench : public sc_module
{
public:
  sc_in_clk clk;
  sc_in<bool> rst;

  struct TbInType {
    std::queue<MemTypedReadReqType<T> > stream;
  };
  struct TbOutType {
    std::queue<MemTypedReadRespType<T> > stream;
  };
  // comes from top
  std::array<CacheLineType, tb_params::DRAM_SIZE_IN_CLS> &dram; // 128KB
  TbInType &test_in;
  TbOutType &test_out;
  // comes from top ends
  bool active;

  // functional IOs
  ga::tlm_fifo_out<MemTypedReadReqType<T> > acc_req_out;
  ga::tlm_fifo_in<MemTypedReadRespType<T> > acc_resp_in;
  ga::tlm_fifo_in<SplMemReadReqType> spl_req_in;
  ga::tlm_fifo_out<SplMemReadRespType> spl_resp_out;


  SC_HAS_PROCESS(TypedMemReadTestbench);

  // limit number of responses
//  template <size_t SIZE>
//  void fill_randomly(std::array<CacheLineType, SIZE> &array) {
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
            MemTypedReadReqType<T> accReq = test_in.stream.front();
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
          MemTypedReadRespType<T> rsp = acc_resp_in.get();
          cout << sc_time_stamp().to_string() << " TB:acc_thread response came back with data " << rsp.data << endl;
          wait(rand()%1000);
          test_out.stream.push(rsp);

        }
      }
      wait();
    }
  }

  void spl_thread() {
    {
      spl_resp_out.reset_put();
      spl_req_in.reset_get();
    }
    wait();
    while (1) {
      {
        if(spl_req_in.nb_can_get()) {
          SplMemReadReqType splReq = spl_req_in.get();
          cout << sc_time_stamp() << " TB:spl_thread SPL request received with address " << splReq.addr <<  endl;

          assert(spl_resp_out.nb_can_put());

          SplMemReadRespType splResp;
          splResp.io_unit_id = splReq.io_unit_id;
          splResp.tag = splReq.tag;
          splResp.data = dram.at(splReq.addr/64);

          spl_resp_out.put(splResp);
          cout << sc_time_stamp() << " TB:spl_thread SPL response generated and sent with data "<< splResp.data << endl;

        }
      }
      wait();
    }
  }


  TypedMemReadTestbench(sc_module_name modname, std::array<CacheLineType, tb_params::DRAM_SIZE_IN_CLS> &dram, TbInType &test_in, TbOutType &test_out) :
      sc_module(modname), clk("clk"), rst("rst"), dram(dram), test_in(test_in), test_out(test_out), active(false), acc_req_out("acc_req_out"), acc_resp_in(
          "acc_resp_in"), spl_req_in("spl_req_in"), spl_resp_out("spl_resp_out")  {
    SC_CTHREAD(acc_thread, clk.pos());
    async_reset_signal_is(rst, false);
    SC_CTHREAD(spl_thread, clk.pos());
    async_reset_signal_is(rst, false);
    spl_req_in.clk_rst(clk, rst);
    spl_resp_out.clk_rst(clk, rst);
    acc_resp_in.clk_rst(clk, rst);
    acc_req_out.clk_rst(clk, rst);
  }

};

template <typename T>
class TypedTestMemReadTop : public sc_module
{
public:

  ClockGenerator clkgen;
  TypedMemReadTestbench<T> acc_read_tb;
  AccMemIn<> acc_cl_in;
  AccMemTypedIn<T> acc_mem_typed_in;
  sc_signal<bool> clk_ch;
  sc_signal<bool> rst_ch;

  ga::tlm_fifo<MemTypedReadRespType<T>, 2> typed_resp_out_ch;
  ga::tlm_fifo<MemTypedReadReqType<T>, 2> typed_req_out_ch;

  ga::tlm_fifo<AccMemReadRespType, 2> acc_resp_out_ch;
  ga::tlm_fifo<SplMemReadReqType, 2> spl_req_out_ch;
  ga::tlm_fifo<AccMemReadReqType, 2> acc_req_out_ch;
  ga::tlm_fifo<SplMemReadRespType, 2> spl_resp_out_ch;

  SC_HAS_PROCESS(TypedTestMemReadTop);

  std::array<CacheLineType, tb_params::DRAM_SIZE_IN_CLS> dram; // 128KB
  typename TypedMemReadTestbench<T>::TbInType test_in;
  typename TypedMemReadTestbench<T>::TbOutType test_out;


  TypedTestMemReadTop(sc_module_name modname = sc_gen_unique_name("TypedTestMemReadTop")) :
      sc_module(modname), clkgen("clkgen"), acc_read_tb("acc_read_tb", dram,
          test_in,
          test_out), acc_cl_in("acc_cl_in"), acc_mem_typed_in(
          "acc_mem_typed_in"), clk_ch("clk_ch"), rst_ch("rst_ch") {
    acc_read_tb.clk(clk_ch);
    clkgen.clk(clk_ch);
    acc_read_tb.rst(rst_ch);
    clkgen.rst(rst_ch);
    acc_cl_in.clk(clk_ch);
    acc_cl_in.rst(rst_ch);
    acc_mem_typed_in.clk(clk_ch);
    acc_mem_typed_in.rst(rst_ch);
    acc_mem_typed_in.acc_resp_out(typed_resp_out_ch);
    acc_mem_typed_in.mem_req_out(acc_req_out_ch);
    acc_mem_typed_in.acc_req_in(typed_req_out_ch);
    acc_mem_typed_in.mem_resp_in(acc_resp_out_ch);

    acc_cl_in.acc_req_in(acc_req_out_ch);
    acc_cl_in.acc_resp_out(acc_resp_out_ch);
    acc_cl_in.spl_req_out(spl_req_out_ch);
    acc_cl_in.spl_resp_in(spl_resp_out_ch);

    acc_read_tb.acc_req_out(typed_req_out_ch);
    acc_read_tb.acc_resp_in(typed_resp_out_ch);
    acc_read_tb.spl_req_in(spl_req_out_ch);
    acc_read_tb.spl_resp_out(spl_resp_out_ch);
  }

  void reset() {
    clkgen.reset_state = true;
  }
  void execute() {
    acc_read_tb.active = true;
  }

};




#endif /* ACCIO_TYPED_MEMREAD_TEST_H_ */
