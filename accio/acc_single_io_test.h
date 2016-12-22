// See LICENSE for license details.
/*
 * acc_single_io_test.h
 *
 *  Created on: May 31, 2016
 *      Author: aayupov
 */

#ifndef ACC_SINGLE_IO_TEST_H_
#define ACC_SINGLE_IO_TEST_H_

#include "systemc.h"
#include "defines.h"
#include "types.h"
#include "ga_tlm_fifo.h"
#include "clock_generator.h"
#include "spl_mem_network.h"
#include <array>
#include <queue>


template<typename T, typename UTAG>
class AccSingleIOTestbench : public sc_module
{
public:
  sc_in_clk clk;
  sc_in<bool> rst;
  typedef enum {RO_READ, RO_WRITE} ReqOrderEnum;
  struct TbInType {
    std::queue<MemSingleWriteReqType<T, UTAG> > stream_wr_req;
    std::queue<MemSingleReadReqType<T, UTAG> > stream_rd_req;
    std::queue<ReqOrderEnum> stream_order;
  };
  struct TbOutType {
    std::queue<MemSingleReadRespType<T, UTAG> > stream_rd_resp;
  };
  // comes from top
  std::array<CacheLineType, tb_params::DRAM_SIZE_IN_CLS> &dram; // 128KB
  TbInType &test_in;
  TbOutType &test_out;
  // comes from top ends
  bool active;


  // functional IOs
  ga::tlm_fifo_out<MemSingleWriteReqType<T, UTAG> > acc_wr_req_out;
  ga::tlm_fifo_in<MemSingleWriteRespType<UTAG> > acc_wr_resp_in;
  ga::tlm_fifo_out<MemSingleReadReqType<T, UTAG> > acc_rd_req_out;
  ga::tlm_fifo_in<MemSingleReadRespType<T, UTAG> > acc_rd_resp_in;

  ga::tlm_fifo_in<SplMemWriteReqType> spl_wr_req_in;
  ga::tlm_fifo_out<SplMemWriteRespType> spl_wr_resp_out;
  ga::tlm_fifo_in<SplMemReadReqType> spl_rd_req_in;
  ga::tlm_fifo_out<SplMemReadRespType> spl_rd_resp_out;

  SC_HAS_PROCESS(AccSingleIOTestbench);
  sc_signal<bool> read_req_made;
  sc_signal<bool> read_resp_rcvd;
  size_t read_time;
  size_t min_read_time;
  size_t ave_read_time;
  size_t num_of_reads;
  void acc_thread() {
    active = false;
    acc_wr_req_out.reset_put();
    acc_rd_req_out.reset_put();
    acc_wr_resp_in.reset_get();
    size_t wait_cycles_after_last_input = 0;
    {
      read_req_made = false;
      read_time = 0;
      num_of_reads = 0;
    }
    wait();
    while (1) {
      if (active) {
        //cout << "WHAT"<< endl;
        // limit the number of responses we expect to receive
        if (!test_in.stream_order.empty()) {
          //if (!read_req_made) {
             ReqOrderEnum next_req = test_in.stream_order.front(); test_in.stream_order.pop();
            switch (next_req) {
            case RO_READ: {
              MemSingleReadReqType<T, UTAG> rd_req = test_in.stream_rd_req.front(); test_in.stream_rd_req.pop();
              //cout << "TB - RD REQ " << rd_req.utag << endl;

              acc_rd_req_out.put(rd_req);
              read_req_made = true;
              num_of_reads++;
              read_time = sc_time_stamp().to_default_time_units();
              break;
            }
            case RO_WRITE: {
              MemSingleWriteReqType<T, UTAG> wr_req = test_in.stream_wr_req.front(); test_in.stream_wr_req.pop();
              acc_wr_req_out.put(wr_req);
              acc_wr_resp_in.get();
              break;
            }
            default: assert(0);
            }
//          }
//          if (read_resp_rcvd) {
//            read_req_made = false;
//            read_resp_rcvd = false;
//          }
        } else {
          wait_cycles_after_last_input++;
          if (wait_cycles_after_last_input > 1000) {
            active = false;
            sc_pause();
          }
        }
      }
      wait();
    }
  }

  void resp_thread() {
    {
      acc_rd_resp_in.reset_get();
      read_resp_rcvd = false;
      ave_read_time = 0;
      min_read_time = 10000;
    }
    wait();
    while (1) {
      {
//        if (!read_resp_rcvd) {
          MemSingleReadRespType<T, UTAG> rd_resp = acc_rd_resp_in.get();
          //cout << "TB - RD RESP " << rd_resp.utag << endl;
          test_out.stream_rd_resp.push(rd_resp);
//          read_resp_rcvd = true;
//          ave_read_time += (sc_time_stamp().to_default_time_units() - read_time);
//          min_read_time = std::min(min_read_time, (size_t)(sc_time_stamp().to_default_time_units() - read_time));
//          cout << "read time service time " << (double)ave_read_time/num_of_reads << " min service time = " << min_read_time << endl;;
//        }
      }
      wait();
    }
  }

  void spl_thread() {
    {
      spl_rd_resp_out.reset_put();
      spl_wr_req_in.reset_get();
      spl_rd_req_in.reset_get();
      spl_wr_resp_out.reset_put();
    }
    wait();
    while (1) {
      {

        if(spl_wr_req_in.nb_can_get() && spl_wr_resp_out.nb_can_put()) {
          SplMemWriteReqType splReq = spl_wr_req_in.get();
          cout << sc_time_stamp() << " TB:spl_thread SPL write request received with address " << splReq.addr << " data " << splReq.data << endl;

          dram.at(splReq.addr/64) = splReq.data;
          spl_wr_resp_out.nb_put(SplMemWriteRespType(splReq.io_unit_id, splReq.tag));
        }
        if(spl_rd_req_in.nb_can_get()) {
          SplMemReadReqType splReq = spl_rd_req_in.get();
          cout << sc_time_stamp() << " TB:spl_thread SPL request received with address " << splReq.addr <<  endl;

          assert(spl_rd_resp_out.nb_can_put());

          SplMemReadRespType splResp;
          splResp.io_unit_id = splReq.io_unit_id;
          splResp.tag = splReq.tag;
          splResp.data = dram.at(splReq.addr/64);
          wait(10);
          spl_rd_resp_out.put(splResp);
          cout << sc_time_stamp() << " TB:spl_thread SPL response generated and sent with data "<< splResp.data << endl;

        }
      }
      wait();
    }
  }


  AccSingleIOTestbench(sc_module_name modname, std::array<CacheLineType, tb_params::DRAM_SIZE_IN_CLS> &dram, TbInType &test_in, TbOutType &test_out) :
      sc_module(modname), clk("clk"), rst("rst"), dram(dram), test_in(test_in), test_out(
          test_out), active(false), acc_wr_req_out(
					"acc_wr_req_out"), acc_wr_resp_in("acc_wr_resp_in"), acc_rd_req_out(
          "acc_rd_req_out"), acc_rd_resp_in("acc_rd_resp_in"), spl_wr_req_in(
					"spl_wr_req_in"), spl_wr_resp_out("spl_wr_resp_out"), spl_rd_req_in(
					"spl_rd_req_in"), spl_rd_resp_out(
          "spl_rd_resp_out"), read_req_made(
					"read_req_made"), read_resp_rcvd("read_resp_rcvd") {
    SC_CTHREAD(acc_thread, clk.pos());
    async_reset_signal_is(rst, false);
    SC_CTHREAD(spl_thread, clk.pos());
    async_reset_signal_is(rst, false);
    spl_wr_req_in.clk_rst(clk, rst);
    spl_rd_req_in.clk_rst(clk, rst);
    spl_rd_resp_out.clk_rst(clk, rst);
    acc_wr_req_out.clk_rst(clk,rst);
    acc_rd_req_out.clk_rst(clk,rst);
    acc_rd_resp_in.clk_rst(clk,rst);
    acc_wr_resp_in.clk_rst(clk,rst);

    SC_CTHREAD(resp_thread, clk.pos());
    async_reset_signal_is(rst, false);
  }

};

template <typename T, typename UTAG>
class TestSingleIOTop : public sc_module
{
public:

  ClockGenerator clkgen;
  AccSingleIOTestbench<T, UTAG> acc_tb;
  AccInOut<LoadStoreUnitSingleReqParams<T, UTAG, 2, 1, 32, 8> > acc_io_unit;
  sc_signal<bool> clk_ch;
  sc_signal<bool> rst_ch;



  ga::tlm_fifo<MemSingleWriteRespType<UTAG> > acc_wr_resp;
  ga::tlm_fifo<MemSingleWriteReqType<T, UTAG> > acc_wr_req;
  ga::tlm_fifo<MemSingleReadReqType<T, UTAG> > acc_rd_req;
  ga::tlm_fifo<MemSingleReadRespType<T, UTAG> > acc_rd_resp;

  ga::tlm_fifo<SplMemWriteReqType, 128> spl_wr_req;
  ga::tlm_fifo<SplMemWriteRespType, 128> spl_wr_resp;
  ga::tlm_fifo<SplMemReadReqType, 128> spl_rd_req;
  ga::tlm_fifo<SplMemReadRespType, 128> spl_rd_resp;

  SC_HAS_PROCESS(TestSingleIOTop);

  std::array<CacheLineType, tb_params::DRAM_SIZE_IN_CLS> dram; // 128KB
  typename AccSingleIOTestbench<T, UTAG>::TbInType test_in;
  typename AccSingleIOTestbench<T, UTAG>::TbOutType test_out;


  TestSingleIOTop(sc_module_name modname = sc_gen_unique_name("TypedTestMemWriteTop")) :
      sc_module(modname), clkgen("clkgen"), acc_tb("acc_tb", dram,
          test_in,
          test_out), acc_io_unit("acc_io_unit"), clk_ch("clk_ch"), rst_ch("rst_ch") {
    acc_tb.clk(clk_ch);
    clkgen.clk(clk_ch);
    acc_tb.rst(rst_ch);
    clkgen.rst(rst_ch);
    acc_io_unit.clk(clk_ch);
    acc_io_unit.rst(rst_ch);



    acc_io_unit.acc_rd_req_in(acc_rd_req);
    acc_io_unit.acc_wr_req_in(acc_wr_req);
    acc_io_unit.acc_rd_resp_out(acc_rd_resp);
    acc_io_unit.acc_wr_resp_out(acc_wr_resp);
    acc_io_unit.spl_wr_req_out(spl_wr_req);
    acc_io_unit.spl_wr_resp_in(spl_wr_resp);
    acc_io_unit.spl_rd_req_out(spl_rd_req);
    acc_io_unit.spl_rd_resp_in(spl_rd_resp);

	acc_tb.acc_wr_resp_in(acc_wr_resp);
	acc_tb.acc_wr_req_out(acc_wr_req);
    acc_tb.acc_rd_req_out(acc_rd_req);
    acc_tb.acc_rd_resp_in(acc_rd_resp);
    acc_tb.spl_wr_req_in(spl_wr_req);
    acc_tb.spl_wr_resp_out(spl_wr_resp);
    acc_tb.spl_rd_req_in(spl_rd_req);
    acc_tb.spl_rd_resp_out(spl_rd_resp);






  }

  void reset() {
    clkgen.reset_state = true;
  }
  void execute() {
    acc_tb.active = true;
  }

};


#endif /* ACC_SINGLE_IO_TEST_H_ */
