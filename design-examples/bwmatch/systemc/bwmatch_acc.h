// See LICENSE for license details.
/*[[[cog
     import cog
     from cog_acctempl import *
     from dut_params import *
  ]]]*/
//[[[end]]] (checksum: d41d8cd98f00b204e9800998ecf8427e)
/*[[[cog
     cog.outl("#ifndef __%s_ACC_H__" % dut.nm.upper())
     cog.outl("#define __%s_ACC_H__" % dut.nm.upper())
  ]]]*/
#ifndef __BWMATCH_ACC_H__
#define __BWMATCH_ACC_H__
//[[[end]]] (checksum: 4c08005c0200ca3e99ba4a85dc807d94)

#include "accelerator_interface.h"
#include "accelerator_template.h"

/*[[[cog
     cog.outl("class %s_acc : public accelerator_interface<Config> {" % dut.nm)
  ]]]*/
class bwmatch_acc : public accelerator_interface<Config> {
//[[[end]]] (checksum: b3c262c27399caac1101bc9992509fde)
public:
  // load/store units
  /*[[[cog
       for p in dut.inps:
         cog.outl("typedef %s %sLoadParams;" % (p.loadUnitType(), p.nm))

       for p in dut.outs:
         cog.outl("typedef %s %sStoreParams;" % (p.storeUnitType(),p.nm))

       for p in dut.inps:
          cog.outl("AccIn<%sLoadParams> %s_mem_in;" % (p.nm,p.nm))
       for p in dut.outs:
          cog.outl("AccOut<%sStoreParams> %s_mem_out;" % (p.nm,p.nm))
    ]]]*/
  typedef LoadUnitParams< BWPattern, __BWPatternLoadParamsSlots__, 1<<24, 1> patLoadParams;
  typedef LoadUnitSingleReqParams< BWCacheLine, BWState, __BWCacheLineLoadParamsSlots__> clLoadParams;
  typedef LoadUnitParams< BWResult, __BWResultLoadParamsSlots__, 1<<24, 1> preLoadParams;
  typedef StoreUnitParams< BWResultLine> resStoreParams;
  AccIn<patLoadParams> pat_mem_in;
  AccIn<clLoadParams> cl_mem_in;
  AccIn<preLoadParams> pre_mem_in;
  AccOut<resStoreParams> res_mem_out;
  //[[[end]]] (checksum: 5afa9d34d68b87ad62ee827afa69da7b)

  /*[[[cog
       cog.outl("MemArbiter<%d, SplMemWriteReqType, SplMemWriteRespType> wr_arbiter;" % (len(dut.outs),))
       cog.outl("MemArbiter<%d, SplMemReadReqType, SplMemReadRespType> rd_arbiter;" % (len(dut.inps),))
    ]]]*/
  MemArbiter<1, SplMemWriteReqType, SplMemWriteRespType> wr_arbiter;
  MemArbiter<3, SplMemReadReqType, SplMemReadRespType> rd_arbiter;
  //[[[end]]] (checksum: 986f5fd25d954eebddb71640549c53d0)

  // main compute block
  /*[[[cog
       cog.outl("%s_hls dut;" % dut.nm)
    ]]]*/
  bwmatch_hls dut;
  //[[[end]]] (checksum: a523e41dd12d4fe8afe2441882b6f230)

  // channels to connect components above
  sc_signal<bool> wr_arb_idle, rd_arb_idle, acc_done, overall_done;

  /*[[[cog
       for p in dut.inps:
          cog.outl("AccIn<%sLoadParams>::ChannelToArbiter    %s_mem_in_arb_ch;" % (p.nm,p.nm))
       for p in dut.outs:
          cog.outl("AccOut<%sStoreParams >::ChannelToArbiter %s_mem_out_arb_ch;" % (p.nm,p.nm))

       for p in dut.inps:
          cog.outl("ga::tlm_fifo<%s > %s;" % (p.reqTy(),p.reqNm()))
          cog.outl("ga::tlm_fifo<%s > %s;" % (p.respTy(),p.respNm()))
       for p in dut.outs:
          cog.outl("ga::tlm_fifo<%s > %s;" % (p.reqTy(),p.reqNm()))
          cog.outl("ga::tlm_fifo<%s > %s;" % (p.dataTy(),p.dataNm()))
    ]]]*/
  AccIn<patLoadParams>::ChannelToArbiter    pat_mem_in_arb_ch;
  AccIn<clLoadParams>::ChannelToArbiter    cl_mem_in_arb_ch;
  AccIn<preLoadParams>::ChannelToArbiter    pre_mem_in_arb_ch;
  AccOut<resStoreParams >::ChannelToArbiter res_mem_out_arb_ch;
  ga::tlm_fifo<MemTypedReadReqType<BWPattern> > patReq;
  ga::tlm_fifo<MemTypedReadRespType<BWPattern> > patResp;
  ga::tlm_fifo<MemSingleReadReqType<BWCacheLine,BWState> > clReq;
  ga::tlm_fifo<MemSingleReadRespType<BWCacheLine,BWState> > clResp;
  ga::tlm_fifo<MemTypedReadReqType<BWResult> > preReq;
  ga::tlm_fifo<MemTypedReadRespType<BWResult> > preResp;
  ga::tlm_fifo<MemTypedWriteReqType<BWResultLine> > resReq;
  ga::tlm_fifo<MemTypedWriteDataType<BWResultLine> > resData;
  //[[[end]]] (checksum: 035fcbeaa3646fa871ad768e761061cf)

  //
  SimpleGate<3, GATE_AND> and_gate;
  IdleMonitor<8> idle_monitor;

  /*[[[cog
       cog.outl("SC_HAS_PROCESS(%s_acc);" % dut.nm)
    ]]]*/
  SC_HAS_PROCESS(bwmatch_acc);
  //[[[end]]] (checksum: 4cd243caf0c240e8b2647069a35c6ae7)

  /*[[[cog
       cog.outl("""%s_acc(sc_module_name name = sc_gen_unique_name("%s_acc")) :""" % (dut.nm,dut.nm))
    ]]]*/
  bwmatch_acc(sc_module_name name = sc_gen_unique_name("bwmatch_acc")) :
  //[[[end]]] (checksum: 38fa92b462337857ed2abf3740f351e2)
                accelerator_interface<Config>(name)
                /*[[[cog
                     for p in dut.inps:
                       cog.outl(""", %s_mem_in("%s_mem_in")""" % (p.nm,p.nm))
                     for p in dut.outs:
                       cog.outl(""", %s_mem_out("%s_mem_out")""" % (p.nm,p.nm))
                ]]]*/
                , pat_mem_in("pat_mem_in")
                , cl_mem_in("cl_mem_in")
                , pre_mem_in("pre_mem_in")
                , res_mem_out("res_mem_out")
                //[[[end]]] (checksum: 64204f3656a76c830938000fb5382132)
                , dut("dut")
                , wr_arb_idle("wr_arb_idle")
                , rd_arb_idle("rd_arb_idle")
                , acc_done("acc_done")
                , overall_done("overall_done")
                /*[[[cog
                     for p in dut.inps:
                       cog.outl(""", %s("%s")""" % (p.reqNm(),p.reqNm()))
                       cog.outl(""", %s("%s")""" % (p.respNm(),p.respNm()))
                     for p in dut.outs:
                       cog.outl(""", %s("%s")""" % (p.reqNm(),p.reqNm()))
                       cog.outl(""", %s("%s")""" % (p.dataNm(),p.dataNm()))
                ]]]*/
                , patReq("patReq")
                , patResp("patResp")
                , clReq("clReq")
                , clResp("clResp")
                , preReq("preReq")
                , preResp("preResp")
                , resReq("resReq")
                , resData("resData")
                //[[[end]]] (checksum: 3eb73cdd478afc5e9ec2bc6509b3608a)
	            , idle_monitor("idle_monitor")		 
  {
    /*[[[cog
         for idx,p in enumerate(dut.inps):
           cog.outl("%s_mem_in_arb_ch.bindArbiter<%d>(rd_arbiter,%d,%s_mem_in);" % (p.nm,len(dut.inps),idx,p.nm))
         for idx,p in enumerate(dut.outs):
           cog.outl("%s_mem_out_arb_ch.bindArbiter<%d>(wr_arbiter,%d,%s_mem_out);" % (p.nm,len(dut.outs),idx,p.nm))
      ]]]*/
    pat_mem_in_arb_ch.bindArbiter<3>(rd_arbiter,0,pat_mem_in);
    cl_mem_in_arb_ch.bindArbiter<3>(rd_arbiter,1,cl_mem_in);
    pre_mem_in_arb_ch.bindArbiter<3>(rd_arbiter,2,pre_mem_in);
    res_mem_out_arb_ch.bindArbiter<1>(wr_arbiter,0,res_mem_out);
    //[[[end]]] (checksum: bddafd81d1f0b59fdd9bd5b24ad77f01)

    rd_arbiter.clk(clk);
    rd_arbiter.rst(rst);
    wr_arbiter.clk(clk);
    wr_arbiter.rst(rst);

    /*[[[cog
         for p in dut.inps:
           cog.outl("%s_mem_in.clk(clk);" % (p.nm,))
           cog.outl("%s_mem_in.rst(rst);" % (p.nm,))
         for p in dut.outs:
           cog.outl("%s_mem_out.clk(clk);" % (p.nm,))
           cog.outl("%s_mem_out.rst(rst);" % (p.nm,))
      ]]]*/
    pat_mem_in.clk(clk);
    pat_mem_in.rst(rst);
    cl_mem_in.clk(clk);
    cl_mem_in.rst(rst);
    pre_mem_in.clk(clk);
    pre_mem_in.rst(rst);
    res_mem_out.clk(clk);
    res_mem_out.rst(rst);
    //[[[end]]] (checksum: 46411aa77475f12c5c9653fe1a995592)

    idle_monitor.clk(clk);
    idle_monitor.rst(rst);

    dut.clk(clk);
    dut.rst(rst);
    dut.start(start);
    dut.config(config);
    /*[[[cog
         for p in dut.inps:
           cog.outl("dut.%s(%s);" % (p.reqNmK(),p.reqNm()))
           cog.outl("%s_mem_in.acc_req_in(%s);" % (p.nm,p.reqNm()))
           cog.outl("")
           cog.outl("dut.%s(%s);" % (p.respNmK(),p.respNm()))
           cog.outl("%s_mem_in.acc_resp_out(%s);" % (p.nm,p.respNm()))
           cog.outl("")
         for p in dut.outs:
           cog.outl("dut.%s(%s);" % (p.reqNmK(),p.reqNm()))
           cog.outl("%s_mem_out.acc_req_in(%s);" % (p.nm,p.reqNm()))
           cog.outl("")
           cog.outl("dut.%s(%s);" % (p.dataNmK(),p.dataNm()))
           cog.outl("%s_mem_out.acc_data_in(%s);" % (p.nm,p.dataNm()))
           cog.outl("")
      ]]]*/
    dut.patReqOut(patReq);
    pat_mem_in.acc_req_in(patReq);

    dut.patRespIn(patResp);
    pat_mem_in.acc_resp_out(patResp);

    dut.clReqOut(clReq);
    cl_mem_in.acc_req_in(clReq);

    dut.clRespIn(clResp);
    cl_mem_in.acc_resp_out(clResp);

    dut.preReqOut(preReq);
    pre_mem_in.acc_req_in(preReq);

    dut.preRespIn(preResp);
    pre_mem_in.acc_resp_out(preResp);

    dut.resReqOut(resReq);
    res_mem_out.acc_req_in(resReq);

    dut.resDataOut(resData);
    res_mem_out.acc_data_in(resData);

    //[[[end]]] (checksum: ade0fcf9194636cbc0b990a84c872b2d)
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

