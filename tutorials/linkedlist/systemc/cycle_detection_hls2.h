// See LICENSE for license details.
#include "acc_typed_io.h"
#include "cycle_detection.h"




//SC_MODULE (cycle_detection){
class cycle_detection : public sc_module
{
public:
  sc_in_clk clock;
  sc_in<bool> rst;
  sc_in<bool> start;

  //from config
  sc_in<Config> configInp;

  Config configReg;

  sc_out<bool> done;

  long long unsigned slow_offset:ADDR_W;
  long long unsigned fast_offset:ADDR_W;

  bool started;

  // memory ports
  ga::tlm_fifo_out<MemTypedReadReqType<Node> > inpReqOut;
  ga::tlm_fifo_in<MemTypedReadRespType<Node> > inpRespIn;

  ga::tlm_fifo_out<MemTypedWriteReqType<CycleExistInfo> > outReqOut;
  ga::tlm_fifo_out<MemTypedWriteDataType<CycleExistInfo> > outDataOut;

  //SC_HAS_PROCESS(cycle_detection);

  SC_CTOR(cycle_detection){
    SC_CTHREAD(detection, clock.pos());
    async_reset_signal_is(rst, false);
    inpReqOut.clk_rst(clock, rst);
    inpRespIn.clk_rst(clock, rst);
    outReqOut.clk_rst(clock, rst);
    outDataOut.clk_rst(clock, rst);
  }
    
  void detection() {
    started = false;
    doneReg = false;
    done = false;
        slow_offset = 0;
        fast_offset = 0;
    inpReqOut.reset_put();
    inpRespIn.reset_get();
    outReqOut.reset_put();
    outDataOut.reset_put();

    wait();
    while(1) {
      if (start && (!started))
      {
        configReg = configInp;
        slow_offset = configReg.head_offset;
        fast_offset = configReg.head_offset;
        started = true;
      } 
      else if(started&&(!done))
      {
        if(!fast_offset){ // reached the end, no cycle found
          outReqOut.put(MemTypedWriteReqType<CycleExistInfo>(configReg.output_offset,1));
          outDataOut.put(MemTypedWriteDataType<CycleExistInfo>(false));
          done = true;
        } else { // move fast pointer
            inpReqOut.put(MemTypedReadReqType<Node> (fast_offset,1)); 
            fast_offset = inpRespIn.get().data.next_offset;
            if(fast_offset == slow_offset){ // found cycle
                outReqOut.put(MemTypedWriteReqType<CycleExistInfo>(configReg.output_offset,1));
                outDataOut.put(MemTypedWriteDataType<CycleExistInfo>(true));
                done = true;
            }
            if (fast_offset && !done) { // move both pointers
               inpReqOut.put(MemTypedReadReqType<Node> (fast_offset,1));
               wait();
               inpReqOut.put(MemTypedReadReqType<Node> (slow_offset,1));
               fast_offset = inpRespIn.get().data.next_offset;
               wait();
               slow_offset = inpRespIn.get().data.next_offset; 
            }
         }
      }
      wait();
    }

  }

};


#include "accelerator_interface.h"
#include "accelerator_template.h"

class cycledetection_acc : public accelerator_interface<Config> {
public:
  // load/store units
  typedef LoadUnitParams< Node, 2, 1, 1> NodeLoadParams;
  typedef StoreUnitParams<CycleExistInfo> CycleExistInfoParams;

  AccIn<NodeLoadParams> inp_mem_in;
  AccOut<CycleExistInfoParams> out_mem_out;

  MemArbiter<1, SplMemWriteReqType, SplMemWriteRespType> wr_arbiter;
  MemArbiter<1, SplMemReadReqType, SplMemReadRespType> rd_arbiter;

  // main compute block
  cycle_detection dut;

  // channels to connect components above
  sc_signal<bool> wr_arb_idle, rd_arb_idle, acc_done, overall_done;

  AccIn<NodeLoadParams>::ChannelToArbiter    inp_mem_in_arb_ch;
  AccOut<CycleExistInfoParams >::ChannelToArbiter out_mem_out_arb_ch;

  ga::tlm_fifo<MemTypedReadReqType<Node> > inpReq;
  ga::tlm_fifo<MemTypedReadRespType<Node> > inpResp;

  ga::tlm_fifo<MemTypedWriteReqType<CycleExistInfo> > outReq;
  ga::tlm_fifo<MemTypedWriteDataType<CycleExistInfo> > outData;

  //
  SimpleGate<3, GATE_AND> and_gate;
  IdleMonitor<8> idle_monitor;


  SC_HAS_PROCESS(cycledetection_acc);

  cycledetection_acc(sc_module_name name = sc_gen_unique_name("cycledetection_acc")) :
                accelerator_interface<Config>(name)
        , inp_mem_in("inp_mem_in")
        , out_mem_out("out_mem_out")
        , dut("dut")
              , wr_arb_idle("wr_arb_idle")
              , rd_arb_idle("rd_arb_idle")
              , acc_done("acc_done")
              , overall_done("overall_done")
         , inpReq("inpReq")     
         , inpResp("inpResp")     
              , outReq("inpReq")
         , outData("inpData")
        , idle_monitor("idle_monitor")     
  {
    inp_mem_in_arb_ch.bindArbiter<1>(rd_arbiter,0,inp_mem_in);
    out_mem_out_arb_ch.bindArbiter<1>(wr_arbiter,0,out_mem_out);

    rd_arbiter.clk(clk);
    rd_arbiter.rst(rst);
    wr_arbiter.clk(clk);
    wr_arbiter.rst(rst);

    inp_mem_in.clk(clk);
    inp_mem_in.rst(rst);
    out_mem_out.clk(clk);
    out_mem_out.rst(rst);

    idle_monitor.clk(clk);
    idle_monitor.rst(rst);

    dut.clock(clk);
    dut.rst(rst);
    dut.start(start);
    dut.configInp(config);

    dut.inpReqOut(inpReq);
    inp_mem_in.acc_req_in(inpReq);

    dut.inpRespIn(inpResp);
    inp_mem_in.acc_resp_out(inpResp);

    dut.outReqOut(outReq);
    out_mem_out.acc_req_in(outReq);

    dut.outDataOut(outData);
    out_mem_out.acc_data_in(outData);

    rd_arbiter.out_req_fifo(spl_rd_req);
    rd_arbiter.in_resp_fifo(spl_rd_resp);

    wr_arbiter.out_req_fifo(spl_wr_req);
    wr_arbiter.in_resp_fifo(spl_wr_resp);

    // idle/done ANDing
    wr_arbiter.idle(wr_arb_idle);
    and_gate.ins[0](wr_arb_idle);
    rd_arbiter.idle(rd_arb_idle);
    and_gate.ins[1](rd_arb_idle);
    dut.done(acc_done);
    and_gate.ins[2](acc_done);
    and_gate.out(overall_done);
    idle_monitor.in_idle(overall_done);
    idle_monitor.out_idle(done);
  }
};

typedef cycledetection_acc dut_t;

#if defined (USE_CTOS) && (__SYNTHESIS__)
SC_MODULE_EXPORT(cycledetection_acc);
#endif
