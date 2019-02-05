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
#ifndef __TRWS_ACC_H__
#define __TRWS_ACC_H__
//[[[end]]] (checksum: 1df28a537af2a31b305bbd09860812ef)

#include "accelerator_interface.h"
#include "accelerator_template.h"

/*[[[cog
     cog.outl("class %s_acc : public accelerator_interface<Config> {" % dut.nm)
  ]]]*/
class trws_acc : public accelerator_interface<Config> {
//[[[end]]] (checksum: 12f2cfddf8abcddf35e1591211255dab)
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
  typedef LoadUnitParams< UCacheLine, __gi_Slots__, 1 << 28, 1> giLoadParams;
  typedef LoadUnitParams< CacheLine, __wi_Slots__, 1 << 28, 1> wiLoadParams;
  typedef LoadUnitParams< UCacheLine, __mi_Slots__, 1 << 28, 1> miLoadParams;
  typedef LoadUnitParams< Pair, __inp_Slots__, 1 << 28, 1> inpLoadParams;
  typedef StoreUnitParams< UCacheLine> moStoreParams;
  AccIn<giLoadParams> gi_mem_in;
  AccIn<wiLoadParams> wi_mem_in;
  AccIn<miLoadParams> mi_mem_in;
  AccIn<inpLoadParams> inp_mem_in;
  AccOut<moStoreParams> mo_mem_out;
  //[[[end]]] (checksum: 34cb1a117ceb1c75900177faa5b8ef37)

  /*[[[cog
       cog.outl("MemArbiter<%d, SplMemWriteReqType, SplMemWriteRespType> wr_arbiter;" % (len(dut.outs),))
       cog.outl("MemArbiter<%d, SplMemReadReqType, SplMemReadRespType> rd_arbiter;" % (len(dut.inps),))
    ]]]*/
  MemArbiter<1, SplMemWriteReqType, SplMemWriteRespType> wr_arbiter;
  MemArbiter<4, SplMemReadReqType, SplMemReadRespType> rd_arbiter;
  //[[[end]]] (checksum: 1545520f9420c3ef512dca31ff5d4477)

  // main compute block
  /*[[[cog
       cog.outl("%s_hls dut;" % dut.nm)
    ]]]*/
  trws_hls dut;
  //[[[end]]] (checksum: c43ec93c8afec58733eed673805b151d)

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
  AccIn<giLoadParams>::ChannelToArbiter    gi_mem_in_arb_ch;
  AccIn<wiLoadParams>::ChannelToArbiter    wi_mem_in_arb_ch;
  AccIn<miLoadParams>::ChannelToArbiter    mi_mem_in_arb_ch;
  AccIn<inpLoadParams>::ChannelToArbiter    inp_mem_in_arb_ch;
  AccOut<moStoreParams >::ChannelToArbiter mo_mem_out_arb_ch;
  ga::tlm_fifo<MemTypedReadReqType<UCacheLine> > giReq;
  ga::tlm_fifo<MemTypedReadRespType<UCacheLine> > giResp;
  ga::tlm_fifo<MemTypedReadReqType<CacheLine> > wiReq;
  ga::tlm_fifo<MemTypedReadRespType<CacheLine> > wiResp;
  ga::tlm_fifo<MemTypedReadReqType<UCacheLine> > miReq;
  ga::tlm_fifo<MemTypedReadRespType<UCacheLine> > miResp;
  ga::tlm_fifo<MemTypedReadReqType<Pair> > inpReq;
  ga::tlm_fifo<MemTypedReadRespType<Pair> > inpResp;
  ga::tlm_fifo<MemTypedWriteReqType<UCacheLine> > moReq;
  ga::tlm_fifo<MemTypedWriteDataType<UCacheLine> > moData;
  //[[[end]]] (checksum: 58cf76df2eeb598b0c46f08ae8c023ee)

  //
  SimpleGate<3, GATE_AND> and_gate;
  IdleMonitor<8> idle_monitor;

  /*[[[cog
       cog.outl("SC_HAS_PROCESS(%s_acc);" % dut.nm)
    ]]]*/
  SC_HAS_PROCESS(trws_acc);
  //[[[end]]] (checksum: 06e71829e12da9700609157456e357f0)

  /*[[[cog
       cog.outl("""%s_acc(sc_module_name name = sc_gen_unique_name("%s_acc")) :""" % (dut.nm,dut.nm))
    ]]]*/
  trws_acc(sc_module_name name = sc_gen_unique_name("trws_acc")) :
  //[[[end]]] (checksum: ca1b44b1ddeafec1120d09c16dcce335)
                accelerator_interface<Config>(name)
                /*[[[cog
                     for p in dut.inps:
                       cog.outl(""", %s_mem_in("%s_mem_in")""" % (p.nm,p.nm))
                     for p in dut.outs:
                       cog.outl(""", %s_mem_out("%s_mem_out")""" % (p.nm,p.nm))
                ]]]*/
                , gi_mem_in("gi_mem_in")
                , wi_mem_in("wi_mem_in")
                , mi_mem_in("mi_mem_in")
                , inp_mem_in("inp_mem_in")
                , mo_mem_out("mo_mem_out")
                //[[[end]]] (checksum: 4dc4b12c7cc9bbbf3f772643d5db7d01)
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
                , giReq("giReq")
                , giResp("giResp")
                , wiReq("wiReq")
                , wiResp("wiResp")
                , miReq("miReq")
                , miResp("miResp")
                , inpReq("inpReq")
                , inpResp("inpResp")
                , moReq("moReq")
                , moData("moData")
                //[[[end]]] (checksum: c11a3b1f02f5035cd5f42644e0d80a41)
	            , idle_monitor("idle_monitor")		 
  {
    /*[[[cog
         for idx,p in enumerate(dut.inps):
           cog.outl("%s_mem_in_arb_ch.bindArbiter<%d>(rd_arbiter,%d,%s_mem_in);" % (p.nm,len(dut.inps),idx,p.nm))
         for idx,p in enumerate(dut.outs):
           cog.outl("%s_mem_out_arb_ch.bindArbiter<%d>(wr_arbiter,%d,%s_mem_out);" % (p.nm,len(dut.outs),idx,p.nm))
      ]]]*/
    gi_mem_in_arb_ch.bindArbiter<4>(rd_arbiter,0,gi_mem_in);
    wi_mem_in_arb_ch.bindArbiter<4>(rd_arbiter,1,wi_mem_in);
    mi_mem_in_arb_ch.bindArbiter<4>(rd_arbiter,2,mi_mem_in);
    inp_mem_in_arb_ch.bindArbiter<4>(rd_arbiter,3,inp_mem_in);
    mo_mem_out_arb_ch.bindArbiter<1>(wr_arbiter,0,mo_mem_out);
    //[[[end]]] (checksum: 9ce01dc5c66951b8ee2bb26d33b4b523)

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
    gi_mem_in.clk(clk);
    gi_mem_in.rst(rst);
    wi_mem_in.clk(clk);
    wi_mem_in.rst(rst);
    mi_mem_in.clk(clk);
    mi_mem_in.rst(rst);
    inp_mem_in.clk(clk);
    inp_mem_in.rst(rst);
    mo_mem_out.clk(clk);
    mo_mem_out.rst(rst);
    //[[[end]]] (checksum: 43c2094e1752505c8d0e9e774628216a)

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
    dut.giReqOut(giReq);
    gi_mem_in.acc_req_in(giReq);

    dut.giRespIn(giResp);
    gi_mem_in.acc_resp_out(giResp);

    dut.wiReqOut(wiReq);
    wi_mem_in.acc_req_in(wiReq);

    dut.wiRespIn(wiResp);
    wi_mem_in.acc_resp_out(wiResp);

    dut.miReqOut(miReq);
    mi_mem_in.acc_req_in(miReq);

    dut.miRespIn(miResp);
    mi_mem_in.acc_resp_out(miResp);

    dut.inpReqOut(inpReq);
    inp_mem_in.acc_req_in(inpReq);

    dut.inpRespIn(inpResp);
    inp_mem_in.acc_resp_out(inpResp);

    dut.moReqOut(moReq);
    mo_mem_out.acc_req_in(moReq);

    dut.moDataOut(moData);
    mo_mem_out.acc_data_in(moData);

    //[[[end]]] (checksum: 1bf71aedcdd2c57919c523212d62bcf7)
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

