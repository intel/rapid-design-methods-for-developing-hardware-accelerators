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
#ifndef __VECTORADD_ACC_H__
#define __VECTORADD_ACC_H__
//[[[end]]] (checksum: 8ec233e0d814db3a58b2ebc79abd230a)

#include "accelerator_interface.h"
#include "accelerator_template.h"

/*[[[cog
     cog.outl("class %s_acc : public accelerator_interface<Config> {" % dut.nm)
  ]]]*/
class vectoradd_acc : public accelerator_interface<Config> {
//[[[end]]] (checksum: 5e4aa5aae2cc3002abe64bb89b5ee2ae)
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
  typedef LoadUnitParams< Blk, __inp_Slots__, 1 << 30, 1> inaLoadParams;
  typedef LoadUnitParams< Blk, __inp_Slots__, 1 << 30, 1> inbLoadParams;
  typedef StoreUnitParams< Blk> outStoreParams;
  AccIn<inaLoadParams> ina_mem_in;
  AccIn<inbLoadParams> inb_mem_in;
  AccOut<outStoreParams> out_mem_out;
  //[[[end]]] (checksum: 36ef663c624f2bd1f0971c0a2985742a)

  /*[[[cog
       cog.outl("MemArbiter<%d, SplMemWriteReqType, SplMemWriteRespType> wr_arbiter;" % (len(dut.outs),))
       cog.outl("MemArbiter<%d, SplMemReadReqType, SplMemReadRespType> rd_arbiter;" % (len(dut.inps),))
    ]]]*/
  MemArbiter<1, SplMemWriteReqType, SplMemWriteRespType> wr_arbiter;
  MemArbiter<2, SplMemReadReqType, SplMemReadRespType> rd_arbiter;
  //[[[end]]] (checksum: 6c96cc81bbeeacb910cd87f500c8c586)

  // main compute block
  /*[[[cog
       cog.outl("%s_hls dut;" % dut.nm)
    ]]]*/
  vectoradd_hls dut;
  //[[[end]]] (checksum: 05de23fe001795849a4f07de473340f6)

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
  AccIn<inaLoadParams>::ChannelToArbiter    ina_mem_in_arb_ch;
  AccIn<inbLoadParams>::ChannelToArbiter    inb_mem_in_arb_ch;
  AccOut<outStoreParams >::ChannelToArbiter out_mem_out_arb_ch;
  ga::tlm_fifo<MemTypedReadReqType<Blk> > inaReq;
  ga::tlm_fifo<MemTypedReadRespType<Blk> > inaResp;
  ga::tlm_fifo<MemTypedReadReqType<Blk> > inbReq;
  ga::tlm_fifo<MemTypedReadRespType<Blk> > inbResp;
  ga::tlm_fifo<MemTypedWriteReqType<Blk> > outReq;
  ga::tlm_fifo<MemTypedWriteDataType<Blk> > outData;
  //[[[end]]] (checksum: 9e7428430ef93153cc7ebaa0a137ff79)

  //
  SimpleGate<3, GATE_AND> and_gate;
  IdleMonitor<8> idle_monitor;

  /*[[[cog
       cog.outl("SC_HAS_PROCESS(%s_acc);" % dut.nm)
    ]]]*/
  SC_HAS_PROCESS(vectoradd_acc);
  //[[[end]]] (checksum: f13e7903cbd4ba3c59f4ea1d17b9d51b)

  /*[[[cog
       cog.outl("""%s_acc(sc_module_name name = sc_gen_unique_name("%s_acc")) :""" % (dut.nm,dut.nm))
    ]]]*/
  vectoradd_acc(sc_module_name name = sc_gen_unique_name("vectoradd_acc")) :
  //[[[end]]] (checksum: eb7d1e1bae27d677438e1ad8163613cd)
                accelerator_interface<Config>(name)
                /*[[[cog
                     for p in dut.inps:
                       cog.outl(""", %s_mem_in("%s_mem_in")""" % (p.nm,p.nm))
                     for p in dut.outs:
                       cog.outl(""", %s_mem_out("%s_mem_out")""" % (p.nm,p.nm))
                ]]]*/
                , ina_mem_in("ina_mem_in")
                , inb_mem_in("inb_mem_in")
                , out_mem_out("out_mem_out")
                //[[[end]]] (checksum: 15e1cdd5e5d0a21ae2576125bba861ec)
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
                , inaReq("inaReq")
                , inaResp("inaResp")
                , inbReq("inbReq")
                , inbResp("inbResp")
                , outReq("outReq")
                , outData("outData")
                //[[[end]]] (checksum: 3d8e149318d5d4920585bfe5c7968e39)
	            , idle_monitor("idle_monitor")		 
  {
    /*[[[cog
         for idx,p in enumerate(dut.inps):
           cog.outl("%s_mem_in_arb_ch.bindArbiter<%d>(rd_arbiter,%d,%s_mem_in);" % (p.nm,len(dut.inps),idx,p.nm))
         for idx,p in enumerate(dut.outs):
           cog.outl("%s_mem_out_arb_ch.bindArbiter<%d>(wr_arbiter,%d,%s_mem_out);" % (p.nm,len(dut.outs),idx,p.nm))
      ]]]*/
    ina_mem_in_arb_ch.bindArbiter<2>(rd_arbiter,0,ina_mem_in);
    inb_mem_in_arb_ch.bindArbiter<2>(rd_arbiter,1,inb_mem_in);
    out_mem_out_arb_ch.bindArbiter<1>(wr_arbiter,0,out_mem_out);
    //[[[end]]] (checksum: 929dca2fc76dc11e486400a108cf6db3)

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
    ina_mem_in.clk(clk);
    ina_mem_in.rst(rst);
    inb_mem_in.clk(clk);
    inb_mem_in.rst(rst);
    out_mem_out.clk(clk);
    out_mem_out.rst(rst);
    //[[[end]]] (checksum: 804ca7875110376257a797c4dc75679d)

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
    dut.inaReqOut(inaReq);
    ina_mem_in.acc_req_in(inaReq);

    dut.inaRespIn(inaResp);
    ina_mem_in.acc_resp_out(inaResp);

    dut.inbReqOut(inbReq);
    inb_mem_in.acc_req_in(inbReq);

    dut.inbRespIn(inbResp);
    inb_mem_in.acc_resp_out(inbResp);

    dut.outReqOut(outReq);
    out_mem_out.acc_req_in(outReq);

    dut.outDataOut(outData);
    out_mem_out.acc_data_in(outData);

    //[[[end]]] (checksum: bdf967387abc8d649955c17f93d9519d)
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

