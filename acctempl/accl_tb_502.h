// See LICENSE for license details.

#ifndef ACCL_TB_H_
#define ACCL_TB_H_

#include "stdio.h"
#include "systemc.h"
#include "clock_generator.h"
#include "types.h"
#include "acc_typed_io.h"
#include "spl_mem_network.h"
#include "reinterpret_utils.h"
//#include "dramsim2_mem_ctrl.h"

#include <stdlib.h>

template <typename CONFIG>
class accl_tb : public sc_module
{
  typedef CONFIG Config;
public:
	sc_in_clk clk;
	sc_in<bool> rst;

  //// functional ports////
  sc_out<Config> config;
  sc_out<bool> start;
  sc_in<bool> done;

  Config conf;

	SC_HAS_PROCESS(accl_tb);

	accl_tb(sc_module_name modname) :
		sc_module(modname)
		, clk("clk")
		, rst("rst")
      , config("config"), start("start"), done("done") {
    SC_CTHREAD(driver, clk.pos());
    async_reset_signal_is(rst, false);
  }

  void driver() {
    {
      start = true;
      config = conf;
    }
    wait();
    while (1) {
      {
        if (done) {
          cout << sc_time_stamp()<< " TB: DONE received" << endl;
          start = false;
          wait(500);
          sc_pause();
        }
      }
      wait();
    }
  }
};

#include "mock_memory.h"

template <typename CONFIG, size_t FREQ_IN_MHZ>
class acc_top_driver : public sc_module {
public:
  typedef CONFIG Config;

  ClockGenerator cgen;
  accl_tb<CONFIG> tb;
  // used for app memory allocation
  unsigned char      *m_JointVirt;     ///< Joint workspace pointer.
  size_t              m_JointSize;     ///< Joint workspace size in bytes.


  acc_top_driver (sc_module_name name=sc_gen_unique_name("acc_top")) : sc_module(name),
      cgen("cgen", 1000/float(FREQ_IN_MHZ)), tb("tb")
  {
  }

  virtual int alloc( unsigned long long size_in_bytes) { // Returns zero on success
    // Doesn't align the memory so you don't necessarily get has much usable memory as you exect
    //    m_JointVirt = (unsigned char *)malloc(size_in_bytes);
    // Works on virtualbox but not EC machines (probably due to gcc 4.7.0 being too old)
    //    m_JointVirt = (unsigned char *) aligned_alloc( 64, size_in_bytes);
    // Works on EC machines with gcc 4.7.0
    posix_memalign( (void **) (&m_JointVirt), 64, size_in_bytes);
    
    if (!m_JointVirt) return 1;
    m_JointSize = size_in_bytes;
    return 0;
  }
  virtual void  compute( Config * config_ptr, const unsigned int config_size) {
    tb.conf = *config_ptr;
    cgen.reset_state = true;
    // reset execution
    sc_start();
    // reset is set to false in the prev call of sc_start
    //main execution
    sc_start();
  }
  virtual void  join() {

  }
  virtual void  free() {
    ::free(m_JointVirt);
    m_JointVirt = NULL;
  }
};

template <typename AU, size_t FREQ_IN_MHZ=1000, size_t LATENCY_IN_NS=128, size_t RD_PORTS=1, size_t WR_PORTS=1>
class acc_top : public acc_top_driver<typename AU::Config, FREQ_IN_MHZ> {
public:
  typedef typename AU::Config Config;

  MockMemorySimulator<FREQ_IN_MHZ, LATENCY_IN_NS, RD_PORTS, WR_PORTS> mem;

  AU dut;

  sc_vector<ga::tlm_fifo<SplMemReadRespType> > spl_rd_resp_fifo;
  sc_vector<ga::tlm_fifo<SplMemReadReqType> >spl_rd_req_fifo;
  sc_vector<ga::tlm_fifo<SplMemWriteRespType> > spl_wr_resp_fifo;
  sc_vector<ga::tlm_fifo<SplMemWriteReqType> > spl_wr_req_fifo;

  // tb and dut
  sc_signal<Config> config;
  sc_signal<bool> start;
  sc_signal<bool> done;
  sc_signal<bool> clk_ch;
  sc_signal<bool> rst_ch;


  SC_HAS_PROCESS(acc_top);
  acc_top (sc_module_name name=sc_gen_unique_name("acc_top")) : acc_top_driver<Config, FREQ_IN_MHZ>(name),
      mem("mem"), dut("dut"),
      spl_rd_resp_fifo("spl_rd_resp_fifo", RD_PORTS), spl_rd_req_fifo("spl_rd_req_fifo", RD_PORTS),
      spl_wr_resp_fifo("spl_wr_resp_fifo", WR_PORTS), spl_wr_req_fifo("spl_wr_req_fifo", WR_PORTS),
      config(
          "config"), start("start"), done("done"), clk_ch("clk_ch"), rst_ch(
          "rst_ch") {
    for (unsigned i = 0; i < RD_PORTS; ++i) {
      mem.memReadReqIn[i](spl_rd_req_fifo[i]);
      dut.spl_rd_req[i](spl_rd_req_fifo[i]);
      mem.memReadRespOut[i](spl_rd_resp_fifo[i]);
      dut.spl_rd_resp[i](spl_rd_resp_fifo[i]);
    }
    for (unsigned i = 0; i < WR_PORTS; ++i) {
      mem.memWriteReqIn[i](spl_wr_req_fifo[i]);
      dut.spl_wr_req[i](spl_wr_req_fifo[i]);
      mem.memWriteRespOut[i](spl_wr_resp_fifo[i]);
      dut.spl_wr_resp[i](spl_wr_resp_fifo[i]);
    }

    this->cgen.clk(clk_ch);
    this->cgen.rst(rst_ch);
    this->tb.clk(clk_ch);
    this->tb.rst(rst_ch);
    mem.clk(clk_ch);
    mem.rst(rst_ch);
    dut.clk(clk_ch);
    dut.rst(rst_ch);

    this->tb.config(config);
    dut.config(config);
    this->tb.start(start);
    dut.start(start);
    this->tb.done(done);
    dut.done(done);
  }


};
// specialized for one read one write channel (the dut memory ports are not arrays in that case)
template <typename AU, size_t FREQ_IN_MHZ, size_t LATENCY_IN_NS>
class acc_top<AU, FREQ_IN_MHZ, LATENCY_IN_NS, 1, 1> : public acc_top_driver<typename AU::Config, FREQ_IN_MHZ> {
public:
  enum {RD_PORTS = 1, WR_PORTS = 1};

  typedef typename AU::Config Config;

  MockMemorySimulator<FREQ_IN_MHZ, LATENCY_IN_NS, RD_PORTS, WR_PORTS> mem;

  AU dut;

  sc_vector<ga::tlm_fifo<SplMemReadRespType> > spl_rd_resp_fifo;
  sc_vector<ga::tlm_fifo<SplMemReadReqType> >spl_rd_req_fifo;
  sc_vector<ga::tlm_fifo<SplMemWriteRespType> > spl_wr_resp_fifo;
  sc_vector<ga::tlm_fifo<SplMemWriteReqType> > spl_wr_req_fifo;

  // tb and dut
  sc_signal<Config> config;
  sc_signal<bool> start;
  sc_signal<bool> done;
  sc_signal<bool> clk_ch;
  sc_signal<bool> rst_ch;


  SC_HAS_PROCESS(acc_top);
  acc_top (sc_module_name name=sc_gen_unique_name("acc_top")) : acc_top_driver<Config, FREQ_IN_MHZ>(name),
      mem("mem"), dut("dut"),
      spl_rd_resp_fifo("spl_rd_resp_fifo", RD_PORTS), spl_rd_req_fifo("spl_rd_req_fifo", RD_PORTS),
      spl_wr_resp_fifo("spl_wr_resp_fifo", WR_PORTS), spl_wr_req_fifo("spl_wr_req_fifo", WR_PORTS),
      config(
          "config"), start("start"), done("done"), clk_ch("clk_ch"), rst_ch(
          "rst_ch") {
    for (unsigned i = 0; i < RD_PORTS; ++i) {
      mem.memReadReqIn[i](spl_rd_req_fifo[i]);
      dut.spl_rd_req(spl_rd_req_fifo[i]);
      mem.memReadRespOut[i](spl_rd_resp_fifo[i]);
      dut.spl_rd_resp(spl_rd_resp_fifo[i]);
    }
    for (unsigned i = 0; i < WR_PORTS; ++i) {
      mem.memWriteReqIn[i](spl_wr_req_fifo[i]);
      dut.spl_wr_req(spl_wr_req_fifo[i]);
      mem.memWriteRespOut[i](spl_wr_resp_fifo[i]);
      dut.spl_wr_resp(spl_wr_resp_fifo[i]);
    }

    this->cgen.clk(clk_ch);
    this->cgen.rst(rst_ch);
    this->tb.clk(clk_ch);
    this->tb.rst(rst_ch);
    mem.clk(clk_ch);
    mem.rst(rst_ch);
    dut.clk(clk_ch);
    dut.rst(rst_ch);

    this->tb.config(config);
    dut.config(config);
    this->tb.start(start);
    dut.start(start);
    this->tb.done(done);
    dut.done(done);
  }
};

#endif /* ACCL_TB_H_ */
