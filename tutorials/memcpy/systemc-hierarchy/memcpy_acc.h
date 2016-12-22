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
#ifndef __MEMCPY_ACC_H__
#define __MEMCPY_ACC_H__
//[[[end]]] (checksum: d6a29a87dfa13e62c1b657fa15136a86)

#include "accelerator_interface.h"
#include "accelerator_template.h"

/*[[[cog
     cog.outl("class %s_acc : public accelerator_interface<Config> {" % dut.nm)
  ]]]*/
class memcpy_acc : public accelerator_interface<Config> {
//[[[end]]] (checksum: 1101e39fcaad191f3083a7a37d5f7a10)
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
  typedef LoadUnitParams< CacheLine, __inp_Slots__, 1 << 24, 1> inpLoadParams;
  typedef StoreUnitParams< CacheLine> outStoreParams;
  AccIn<inpLoadParams> inp_mem_in;
  AccOut<outStoreParams> out_mem_out;
  //[[[end]]] (checksum: 2560389de18bee5709bcaf76e91edecf)

  /*[[[cog
       cog.outl("MemArbiter<%d, SplMemWriteReqType, SplMemWriteRespType> wr_arbiter;" % (len(dut.outs),))
       cog.outl("MemArbiter<%d, SplMemReadReqType, SplMemReadRespType> rd_arbiter;" % (len(dut.inps),))
    ]]]*/
  MemArbiter<1, SplMemWriteReqType, SplMemWriteRespType> wr_arbiter;
  MemArbiter<1, SplMemReadReqType, SplMemReadRespType> rd_arbiter;
  //[[[end]]] (checksum: 62f8ec1cdc81a5651084ebbd4eba1b51)

  // main compute block
  /*[[[cog
       cog.outl("%s_hls dut;" % dut.nm)
    ]]]*/
  memcpy_hls dut;
  //[[[end]]] (checksum: 6347c862a70780289500a291f15bda18)

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
  AccIn<inpLoadParams>::ChannelToArbiter    inp_mem_in_arb_ch;
  AccOut<outStoreParams >::ChannelToArbiter out_mem_out_arb_ch;
  ga::tlm_fifo<MemTypedReadReqType<CacheLine> > inpReq;
  ga::tlm_fifo<MemTypedReadRespType<CacheLine> > inpResp;
  ga::tlm_fifo<MemTypedWriteReqType<CacheLine> > outReq;
  ga::tlm_fifo<MemTypedWriteDataType<CacheLine> > outData;
  //[[[end]]] (checksum: 2e0064c2dd5ce128c063a000b1fa38e3)

  //
  SimpleGate<3, GATE_AND> and_gate;
  IdleMonitor<8> idle_monitor;

  /*[[[cog
       cog.outl("SC_HAS_PROCESS(%s_acc);" % dut.nm)
    ]]]*/
  SC_HAS_PROCESS(memcpy_acc);
  //[[[end]]] (checksum: cf4fbc59673e305e67ea0470c6f985a2)

  /*[[[cog
       cog.outl("""%s_acc(sc_module_name name = sc_gen_unique_name("%s_acc")) :""" % (dut.nm,dut.nm))
    ]]]*/
  memcpy_acc(sc_module_name name = sc_gen_unique_name("memcpy_acc")) :
  //[[[end]]] (checksum: e61886312eb9c86e90ecee9b53ddf0eb)
                accelerator_interface<Config>(name)
                /*[[[cog
                     for p in dut.inps:
                       cog.outl(""", %s_mem_in("%s_mem_in")""" % (p.nm,p.nm))
                     for p in dut.outs:
                       cog.outl(""", %s_mem_out("%s_mem_out")""" % (p.nm,p.nm))
                ]]]*/
                , inp_mem_in("inp_mem_in")
                , out_mem_out("out_mem_out")
                //[[[end]]] (checksum: 459e5320897585921590d2fc7a0f7111)
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
                , inpReq("inpReq")
                , inpResp("inpResp")
                , outReq("outReq")
                , outData("outData")
                //[[[end]]] (checksum: 25614efd0ed70c41415dd77a54cfb47c)
	            , idle_monitor("idle_monitor")		 
  {
    /*[[[cog
         for idx,p in enumerate(dut.inps):
           cog.outl("%s_mem_in_arb_ch.bindArbiter<%d>(rd_arbiter,%d,%s_mem_in);" % (p.nm,len(dut.inps),idx,p.nm))
         for idx,p in enumerate(dut.outs):
           cog.outl("%s_mem_out_arb_ch.bindArbiter<%d>(wr_arbiter,%d,%s_mem_out);" % (p.nm,len(dut.outs),idx,p.nm))
      ]]]*/
    inp_mem_in_arb_ch.bindArbiter<1>(rd_arbiter,0,inp_mem_in);
    out_mem_out_arb_ch.bindArbiter<1>(wr_arbiter,0,out_mem_out);
    //[[[end]]] (checksum: 1bf707f86473c2c7622816c2881ecaa5)

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
    inp_mem_in.clk(clk);
    inp_mem_in.rst(rst);
    out_mem_out.clk(clk);
    out_mem_out.rst(rst);
    //[[[end]]] (checksum: 5a74f4ee530b04ed6b03897867017dae)

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
    dut.inpReqOut(inpReq);
    inp_mem_in.acc_req_in(inpReq);

    dut.inpRespIn(inpResp);
    inp_mem_in.acc_resp_out(inpResp);

    dut.outReqOut(outReq);
    out_mem_out.acc_req_in(outReq);

    dut.outDataOut(outData);
    out_mem_out.acc_data_in(outData);

    //[[[end]]] (checksum: 5780039e533a1781a4c1b157f508069f)
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

