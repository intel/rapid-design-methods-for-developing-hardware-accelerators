// See LICENSE for license details.
/*[[[cog
     import cog
     from cog_acctempl import *
     from dut_params import *
  ]]]*/
//[[[end]]]
/*[[[cog
     cog.outl("#ifndef __%s_ACC_H__" % dut_name.upper())
     cog.outl("#define __%s_ACC_H__" % dut_name.upper())
  ]]]*/
#ifndef __CYCLE_DETECTION_ACC_H__
#define __CYCLE_DETECTION_ACC_H__
//[[[end]]]

#include "accelerator_interface.h"
#include "accelerator_template.h"

/*[[[cog
     cog.outl("class %s_acc : public accelerator_interface<Config> {" % dut_name)
  ]]]*/
class cycle_detection_acc : public accelerator_interface<Config> {
//[[[end]]]
public:
  // load/store units
  /*[[[cog
       for p in inps:
          if type(p) is SingleRead:
            cog.outl("typedef LoadUnitSingleReqParams< %s, %s, %s> %sLoadParams;" % (p.ty,p.tag_ty,p.buf_size_in_cl, p.nm))
          else:
            cog.outl("typedef LoadUnitParams< %s, %s, %s, %s> %sLoadParams;" % (p.ty,p.buf_size_in_cl, p.max_burst_count, p.buf_size_in_burstd_reqs, p.nm))
       for p in outs:
          cog.outl("typedef StoreUnitParams<%s              > %sStoreParams;" % (p.ty,p.nm))

       for p in inps:
          cog.outl("AccIn<%sLoadParams> %s_mem_in;" % (p.nm,p.nm))
       for p in outs:
          cog.outl("AccOut<%sStoreParams> %s_mem_out;" % (p.nm,p.nm))
    ]]]*/
  typedef LoadUnitParams< Node, 64, 1<<24, 64> inpLoadParams;
  typedef StoreUnitParams<CycleExistInfo              > outStoreParams;
  AccIn<inpLoadParams> inp_mem_in;
  AccOut<outStoreParams> out_mem_out;
  //[[[end]]]

  /*[[[cog
       cog.outl("MemArbiter<%d, SplMemWriteReqType, SplMemWriteRespType> wr_arbiter;" % (len(outs),))
       cog.outl("MemArbiter<%d, SplMemReadReqType, SplMemReadRespType> rd_arbiter;" % (len(inps),))
    ]]]*/
  MemArbiter<1, SplMemWriteReqType, SplMemWriteRespType> wr_arbiter;
  MemArbiter<1, SplMemReadReqType, SplMemReadRespType> rd_arbiter;
  //[[[end]]]

  // main compute block
  /*[[[cog
       cog.outl("%s dut;" % module_name)
    ]]]*/
  cycle_detection_hls dut;
  //[[[end]]]

  // channels to connect components above
  sc_signal<bool> wr_arb_idle, rd_arb_idle, acc_done, overall_done;

  /*[[[cog
       for p in inps:
          cog.outl("AccIn<%sLoadParams>::ChannelToArbiter    %s_mem_in_arb_ch;" % (p.nm,p.nm))
       for p in outs:
          cog.outl("AccOut<%sStoreParams >::ChannelToArbiter %s_mem_out_arb_ch;" % (p.nm,p.nm))

       for p in inps:
          cog.outl("ga::tlm_fifo<%s > %s;" % (p.reqTy(),p.reqNm()))
          cog.outl("ga::tlm_fifo<%s > %s;" % (p.respTy(),p.respNm()))
       for p in outs:
          cog.outl("ga::tlm_fifo<%s > %s;" % (p.reqTy(),p.reqNm()))
          cog.outl("ga::tlm_fifo<%s > %s;" % (p.dataTy(),p.dataNm()))
    ]]]*/
  AccIn<inpLoadParams>::ChannelToArbiter    inp_mem_in_arb_ch;
  AccOut<outStoreParams >::ChannelToArbiter out_mem_out_arb_ch;
  ga::tlm_fifo<MemTypedReadReqType<Node> > inpReq;
  ga::tlm_fifo<MemTypedReadRespType<Node> > inpResp;
  ga::tlm_fifo<MemTypedWriteReqType<CycleExistInfo> > outReq;
  ga::tlm_fifo<MemTypedWriteDataType<CycleExistInfo> > outData;
  //[[[end]]]

  //
  SimpleGate<3, GATE_AND> and_gate;
  IdleMonitor<8> idle_monitor;

  /*[[[cog
       cog.outl("SC_HAS_PROCESS(%s_acc);" % dut_name)
    ]]]*/
  SC_HAS_PROCESS(cycle_detection_acc);
  //[[[end]]]

  /*[[[cog
       cog.outl("""%s_acc(sc_module_name name = sc_gen_unique_name("%s_acc")) :""" % (dut_name,dut_name))
    ]]]*/
  cycle_detection_acc(sc_module_name name = sc_gen_unique_name("cycle_detection_acc")) :
  //[[[end]]]
                accelerator_interface<Config>(name)
                /*[[[cog
                     for p in inps:
                       cog.outl(""", %s_mem_in("%s_mem_in")""" % (p.nm,p.nm))
                     for p in outs:
                       cog.outl(""", %s_mem_out("%s_mem_out")""" % (p.nm,p.nm))
                ]]]*/
                , inp_mem_in("inp_mem_in")
                , out_mem_out("out_mem_out")
                //[[[end]]]
                , dut("dut")
                , wr_arb_idle("wr_arb_idle")
                , rd_arb_idle("rd_arb_idle")
                , acc_done("acc_done")
                , overall_done("overall_done")
                /*[[[cog
                     for p in inps:
                       cog.outl(""", %s("%s")""" % (p.reqNm(),p.reqNm()))
                       cog.outl(""", %s("%s")""" % (p.respNm(),p.respNm()))
                     for p in outs:
                       cog.outl(""", %s("%s")""" % (p.reqNm(),p.reqNm()))
                       cog.outl(""", %s("%s")""" % (p.dataNm(),p.dataNm()))
                ]]]*/
                , inpReq("inpReq")
                , inpResp("inpResp")
                , outReq("outReq")
                , outData("outData")
                //[[[end]]]
	            , idle_monitor("idle_monitor")		 
  {
    /*[[[cog
         for idx,p in enumerate(inps):
           cog.outl("%s_mem_in_arb_ch.bindArbiter<%d>(rd_arbiter,%d,%s_mem_in);" % (p.nm,len(inps),idx,p.nm))
         for idx,p in enumerate(outs):
           cog.outl("%s_mem_out_arb_ch.bindArbiter<%d>(wr_arbiter,%d,%s_mem_out);" % (p.nm,len(outs),idx,p.nm))
      ]]]*/
    inp_mem_in_arb_ch.bindArbiter<1>(rd_arbiter,0,inp_mem_in);
    out_mem_out_arb_ch.bindArbiter<1>(wr_arbiter,0,out_mem_out);
    //[[[end]]]

    rd_arbiter.clk(clk);
    rd_arbiter.rst(rst);
    wr_arbiter.clk(clk);
    wr_arbiter.rst(rst);

    /*[[[cog
         for p in inps:
           cog.outl("%s_mem_in.clk(clk);" % (p.nm,))
           cog.outl("%s_mem_in.rst(rst);" % (p.nm,))
         for p in outs:
           cog.outl("%s_mem_out.clk(clk);" % (p.nm,))
           cog.outl("%s_mem_out.rst(rst);" % (p.nm,))
      ]]]*/
    inp_mem_in.clk(clk);
    inp_mem_in.rst(rst);
    out_mem_out.clk(clk);
    out_mem_out.rst(rst);
    //[[[end]]]

    idle_monitor.clk(clk);
    idle_monitor.rst(rst);

    dut.clk(clk);
    dut.rst(rst);
    dut.start(start);
    /*[[[cog
    cog.outl("dut.%s(config);" % kernel_config_name)
    ]]]*/
    dut.config(config);
    //[[[end]]] 
    /*[[[cog
         for p in inps:
           cog.outl("dut.%s(%s);" % (p.reqNmK(),p.reqNm()))
           cog.outl("%s_mem_in.acc_req_in(%s);" % (p.nm,p.reqNm()))
           cog.outl("")
           cog.outl("dut.%s(%s);" % (p.respNmK(),p.respNm()))
           cog.outl("%s_mem_in.acc_resp_out(%s);" % (p.nm,p.respNm()))
           cog.outl("")
         for p in outs:
           cog.outl("dut.%s(%s);" % (p.reqNmK(),p.reqNm()))
           cog.outl("%s_mem_out.acc_req_in(%s);" % (p.nm,p.reqNm()))
           cog.outl("")
           cog.outl("dut.%s(%s);" % (p.dataNmK(),p.dataNm()))
           cog.outl("%s_mem_out.acc_data_in(%s);" % (p.nm,p.dataNm()))
           cog.outl("")
      ]]]*/
    dut.inpReqOut(inpReq);
    inp_mem_in.acc_req_in(inpReq);

    dut.inpRespIn(inpResp);
    inp_mem_in.acc_resp_out(inpResp);

    dut.outReqOut(outReq);
    out_mem_out.acc_req_in(outReq);

    dut.outDataOut(outData);
    out_mem_out.acc_data_in(outData);

    //[[[end]]]
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

#endif

