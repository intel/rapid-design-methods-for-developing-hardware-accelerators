// See LICENSE for license details.


#ifndef __ACCELERATOR_MODULE_H__
#define __ACCELERATOR_MODULE_H__

#include "systemc.h"
#include "types.h"
#include "spl_mem_network.h"
#include "acc_typed_io.h"
#include "command_queue_handler.h"

template<typename ACCELERATOR, typename CONFIG, size_t NUM_OF_READERS, size_t NUM_OF_WRITERS, typename T1=AccNoType, typename T2=AccNoType, typename T3=AccNoType, typename T4=AccNoType>
class AcceleratorTemplate {
  AcceleratorTemplate(sc_module_name modname) {
    assert(0);
  }
};

template <typename ACCELERATOR, typename CONFIG, typename T1, typename T2>
class AcceleratorTemplate<ACCELERATOR,CONFIG,1,1,T1,T2> : public sc_module
{
public:
  // const modeled through enums. const fields would have to be initialized elsewhere
  enum {NUM_OF_READERS = 1};
  enum {NUM_OF_WRITERS = 1};

  typedef typename T1::type AccInDataType;
  typedef typename T2::type AccOutDataType;

  sc_in_clk clk;
	sc_in<bool> rst;
	sc_in<CONFIG> config;
  sc_in<bool> start;
	sc_out<bool> done;
  //top level ports
	//ga::tlm_fifo_hier_in<AccCommandType> cmd_in;
  ga::tlm_fifo_hier_in<SplMemReadRespType> spl_rd_resp;
  ga::tlm_fifo_hier_out<SplMemReadReqType> spl_rd_req;
  ga::tlm_fifo_hier_in<SplMemWriteRespType> spl_wr_resp;
  ga::tlm_fifo_hier_out<SplMemWriteReqType> spl_wr_req;
  // end of top level ports


	CommandQueueHandler<CONFIG, NUM_OF_READERS, NUM_OF_WRITERS, AccInDataType, AccOutDataType> cmd_queue;
	//SplMemNetwork<NUM_OF_READERS, NUM_OF_WRITERS> mem_netw;
	AccIn<T1> mem_in;
  AccOut<T2> mem_out;

  MemArbiter<NUM_OF_WRITERS, SplMemWriteReqType, SplMemWriteRespType> wr_arbiter;
  MemArbiter<NUM_OF_READERS, SplMemReadReqType, SplMemReadRespType> rd_arbiter;

  SimpleGate<3, GATE_AND> and_gate;
  IdleMonitor<8> idle_monitor;

  ACCELERATOR acc;

  //intermediate channels
  // accelerator to AccIn/AccOut
  ga::tlm_fifo<MemTypedReadReqType<AccInDataType> > t1_req;
  ga::tlm_fifo<MemTypedReadRespType<AccInDataType> > t1_resp;
  ga::tlm_fifo<MemTypedWriteReqType<AccOutDataType> > t2_req;
  ga::tlm_fifo<MemTypedWriteDataType<AccOutDataType> > t2_data;
  // arbiter to AccIn/AccOut
  typename AccIn<T1>::ChannelToArbiter acc_in_arbiter_ch;
  typename AccOut<T2>::ChannelToArbiter acc_out_arbiter_ch;

  sc_signal<bool> wr_arb_idle, rd_arb_idle, acc_done, overall_done;

	SC_HAS_PROCESS(AcceleratorTemplate);

	AcceleratorTemplate (sc_module_name modname) :
        sc_module(modname), clk("clk"), rst("rst"), config("config"), start("start"), done("done"), spl_rd_resp("spl_rd_resp"), spl_rd_req("spl_rd_req"), spl_wr_resp("spl_wr_resp"), spl_wr_req("spl_wr_req"),
		cmd_queue("cmd_queue"), mem_in("mem_in"), mem_out("mem_out"), wr_arbiter("wr_arbiter"), rd_arbiter("rd_arbiter"), idle_monitor("idle_monitor"), acc("acc"), t1_req("t1_req"), t1_resp("t1_resp"), t2_req("t2_req"), t2_data("t2_data")
    {

	  cmd_queue.clk(clk);
    cmd_queue.rst(rst);
    acc.clk(clk);
    acc.rst(rst);
    rd_arbiter.clk(clk);
    rd_arbiter.rst(rst);
    wr_arbiter.clk(clk);
    wr_arbiter.rst(rst);
    mem_in.clk(clk);
    mem_in.rst(rst);
    mem_out.clk(clk);
    mem_out.rst(rst);
    idle_monitor.clk(clk);
    idle_monitor.rst(rst);

    // cmd connection to the primary in port
    cmd_queue.config(config);
    cmd_queue.start(start);

    // accelerator data to AccIn/Out
    acc.bind_mem_ports(t1_resp, t2_data);
    acc.config(config);
    //acc.done(done);
    mem_out.acc_data_in(t2_data);
    mem_in.acc_resp_out(t1_resp);

    // command queue requests to AccIn/Out
    cmd_queue.rd_req_out(t1_req);
    mem_in.acc_req_in(t1_req);
    cmd_queue.wr_req_out(t2_req);
    mem_out.acc_req_in(t2_req);

    // arbiter connections to AccIn/Out
    acc_in_arbiter_ch.template bindArbiter<NUM_OF_READERS>(rd_arbiter, 0, mem_in);
    acc_out_arbiter_ch.template bindArbiter<NUM_OF_WRITERS>(wr_arbiter, 0, mem_out);
    // arbiter connections to spl interface (primary output)
    wr_arbiter.out_req_fifo(spl_wr_req);
    wr_arbiter.in_resp_fifo(spl_wr_resp);
    rd_arbiter.out_req_fifo(spl_rd_req);
    rd_arbiter.in_resp_fifo(spl_rd_resp);

    // AND idles for the main done signal
    wr_arbiter.idle(wr_arb_idle);
    and_gate.ins[0](wr_arb_idle);
    rd_arbiter.idle(rd_arb_idle);
    and_gate.ins[1](rd_arb_idle);
    acc.done(acc_done);
    and_gate.ins[2](acc_done);
    and_gate.out(overall_done);
    idle_monitor.in_idle(overall_done);
    idle_monitor.out_idle(done);
	}


};

#endif //__ACCELERATOR_MODULE_H__

