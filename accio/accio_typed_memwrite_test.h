// See LICENSE for license details.
/*
 * accio_typed_memwrite_test.h
 *
 *  Created on: Jan 27, 2016
 *      Author: aayupov
 */

#ifndef ACCIO_TYPED_MEMWRITE_TEST_H_
#define ACCIO_TYPED_MEMWRITE_TEST_H_


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
class TypedMemWriteTestbench : public sc_module
{
public:
  sc_in_clk clk;
  sc_in<bool> rst;

  struct TbInType {
    std::queue<MemTypedWriteReqType<T> > stream_req;
    std::queue<MemTypedWriteDataType<T> > stream_data;
  };
  struct TbOutType {
  };
  // comes from top
  std::array<CacheLineType, tb_params::DRAM_SIZE_IN_CLS> &dram; // 128KB
  TbInType &test_in;
  TbOutType &test_out;
  // comes from top ends
  bool active;

  // functional IOs
  ga::tlm_fifo_out<MemTypedWriteReqType<T> > acc_req_out;
  ga::tlm_fifo_out<MemTypedWriteDataType<T> > acc_data_out;
  ga::tlm_fifo_in<SplMemWriteReqType> spl_req_in;
  ga::tlm_fifo_out<SplMemWriteRespType> spl_resp_out;


  SC_HAS_PROCESS(TypedMemWriteTestbench);

  void acc_thread() {
    active = false;
    size_t wait_cycles_after_last_input = 0;
    {
      acc_req_out.reset_put();
      acc_data_out.reset_put();
    }
    wait();
    while (1) {
      if (active) {
        // limit the number of responses we expect to receive
        if (!test_in.stream_req.empty()) {
          // send a random data request
          if (acc_req_out.nb_can_put()) {
            MemTypedWriteReqType<T> accReq = test_in.stream_req.front();
            test_in.stream_req.pop();
            acc_req_out.put(accReq);
            cout << sc_time_stamp() << " TB:acc_thread send request with size " << (unsigned int)accReq.size << " and address "<< accReq.addr << endl;

            for (unsigned int i = 0; i < accReq.size; ++i) {
              assert(!test_in.stream_data.empty());
              MemTypedWriteDataType<T> data = test_in.stream_data.front();
              test_in.stream_data.pop();
              acc_data_out.put(data);
              cout << sc_time_stamp() << " TB:acc_thread send request with data " << data.data << endl;
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

          dram.at(splReq.addr/64) = splReq.data;
        }
      }
      wait();
    }
  }


  TypedMemWriteTestbench(sc_module_name modname, std::array<CacheLineType, tb_params::DRAM_SIZE_IN_CLS> &dram, TbInType &test_in, TbOutType &test_out) :
      sc_module(modname), clk("clk"), rst("rst"), dram(dram), test_in(test_in), test_out(test_out), active(false), acc_req_out("acc_req_out"), acc_data_out("acc_data_out"), spl_req_in("spl_req_in") {
    SC_CTHREAD(acc_thread, clk.pos());
    async_reset_signal_is(rst, false);
    SC_CTHREAD(spl_thread, clk.pos());
    async_reset_signal_is(rst, false);
    spl_req_in.clk_rst(clk, rst);
    spl_resp_out.clk_rst(clk, rst);
    acc_req_out.clk_rst(clk, rst);
    acc_data_out.clk_rst(clk, rst);
  }

};

template <typename T>
class TypedTestMemWriteTop : public sc_module
{
public:

  ClockGenerator clkgen;
  TypedMemWriteTestbench<T> acc_write_tb;
  AccMemOut acc_cl_out;
  AccMemTypedOut<T> acc_mem_typed_in;
  sc_signal<bool> clk_ch;
  sc_signal<bool> rst_ch;

  ga::tlm_fifo<MemTypedWriteReqType<T>, 2> typed_req_out_ch;
  ga::tlm_fifo<MemTypedWriteDataType<T>, 2> typed_data_out_ch;

  ga::tlm_fifo<AccMemWriteReqType, 2> acc_req_out_ch;
  ga::tlm_fifo<AccMemWriteDataType, 2> acc_data_out_ch;
  ga::tlm_fifo<SplMemWriteReqType, 2> spl_req_out_ch;
  ga::tlm_fifo<SplMemWriteRespType, 2> spl_resp_out_ch;

  SC_HAS_PROCESS(TypedTestMemWriteTop);

  std::array<CacheLineType, tb_params::DRAM_SIZE_IN_CLS> dram; // 128KB
  typename TypedMemWriteTestbench<T>::TbInType test_in;
  typename TypedMemWriteTestbench<T>::TbOutType test_out;


  TypedTestMemWriteTop(sc_module_name modname = sc_gen_unique_name("TypedTestMemWriteTop")) :
      sc_module(modname), clkgen("clkgen"), acc_write_tb("acc_write_tb", dram,
          test_in,
          test_out), acc_cl_out("acc_cl_out"), acc_mem_typed_in(
          "acc_mem_typed_in"), clk_ch("clk_ch"), rst_ch("rst_ch") {
    acc_write_tb.clk(clk_ch);
    clkgen.clk(clk_ch);
    acc_write_tb.rst(rst_ch);
    clkgen.rst(rst_ch);
    acc_cl_out.clk(clk_ch);
    acc_cl_out.rst(rst_ch);
    acc_mem_typed_in.clk(clk_ch);
    acc_mem_typed_in.rst(rst_ch);


    acc_mem_typed_in.acc_req_in(typed_req_out_ch);
    acc_mem_typed_in.acc_data_in(typed_data_out_ch);
    acc_mem_typed_in.mem_req_out(acc_req_out_ch);
    acc_mem_typed_in.mem_data_out(acc_data_out_ch);

    acc_cl_out.acc_req_in(acc_req_out_ch);
    acc_cl_out.acc_data_in(acc_data_out_ch);
    acc_cl_out.spl_req_out(spl_req_out_ch);
    acc_cl_out.spl_resp_out(spl_resp_out_ch);

    acc_write_tb.acc_req_out(typed_req_out_ch);
    acc_write_tb.acc_data_out(typed_data_out_ch);
    acc_write_tb.spl_req_in(spl_req_out_ch);
    acc_write_tb.spl_resp_out(spl_resp_out_ch);
  }

  void reset() {
    clkgen.reset_state = true;
  }
  void execute() {
    acc_write_tb.active = true;
  }

};



#endif /* ACCIO_TYPED_MEMWRITE_TEST_H_ */
