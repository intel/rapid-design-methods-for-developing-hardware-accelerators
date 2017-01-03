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
#ifndef __LINKEDLIST_ACC_H__
#define __LINKEDLIST_ACC_H__
//[[[end]]] (checksum: 07e1d7b7e191c0ff652d929eee77b353)

#include "accelerator_interface.h"
#include "accelerator_template.h"

/*[[[cog
     cog.outl("class %s_acc : public accelerator_interface<Config> {" % dut.nm)
  ]]]*/
class linkedlist_acc : public accelerator_interface<Config> {
//[[[end]]] (checksum: 37aaa94441f2bc27b19aa73b82216fa7)
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
  typedef LoadUnitSingleReqParams< Node, State, __lst_Slots__> lstLoadParams;
  typedef LoadUnitParams< HeadPtr, __inp_Slots__, 1 << 30, 1> inpLoadParams;
  typedef StoreUnitParams< HeadPtr> outStoreParams;
  AccIn<lstLoadParams> lst_mem_in;
  AccIn<inpLoadParams> inp_mem_in;
  AccOut<outStoreParams> out_mem_out;
  //[[[end]]] (checksum: 8df0c181d4ce2288968388f724c564b9)

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
  linkedlist_hls dut;
  //[[[end]]] (checksum: bab41e6b20a68afc85cfb72b7ec90132)

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
  AccIn<lstLoadParams>::ChannelToArbiter    lst_mem_in_arb_ch;
  AccIn<inpLoadParams>::ChannelToArbiter    inp_mem_in_arb_ch;
  AccOut<outStoreParams >::ChannelToArbiter out_mem_out_arb_ch;
  ga::tlm_fifo<MemSingleReadReqType<Node,State> > lstReq;
  ga::tlm_fifo<MemSingleReadRespType<Node,State> > lstResp;
  ga::tlm_fifo<MemTypedReadReqType<HeadPtr> > inpReq;
  ga::tlm_fifo<MemTypedReadRespType<HeadPtr> > inpResp;
  ga::tlm_fifo<MemTypedWriteReqType<HeadPtr> > outReq;
  ga::tlm_fifo<MemTypedWriteDataType<HeadPtr> > outData;
  //[[[end]]] (checksum: ce399db882afdb0abec5b69627fe3450)

  //
  SimpleGate<3, GATE_AND> and_gate;
  IdleMonitor<8> idle_monitor;

  /*[[[cog
       cog.outl("SC_HAS_PROCESS(%s_acc);" % dut.nm)
    ]]]*/
  SC_HAS_PROCESS(linkedlist_acc);
  //[[[end]]] (checksum: bad2e8ff54422b6172f898c2f390bfb2)

  /*[[[cog
       cog.outl("""%s_acc(sc_module_name name = sc_gen_unique_name("%s_acc")) :""" % (dut.nm,dut.nm))
    ]]]*/
  linkedlist_acc(sc_module_name name = sc_gen_unique_name("linkedlist_acc")) :
  //[[[end]]] (checksum: 1d52c776f3a0d4b64a4078f1286ec413)
                accelerator_interface<Config>(name)
                /*[[[cog
                     for p in dut.inps:
                       cog.outl(""", %s_mem_in("%s_mem_in")""" % (p.nm,p.nm))
                     for p in dut.outs:
                       cog.outl(""", %s_mem_out("%s_mem_out")""" % (p.nm,p.nm))
                ]]]*/
                , lst_mem_in("lst_mem_in")
                , inp_mem_in("inp_mem_in")
                , out_mem_out("out_mem_out")
                //[[[end]]] (checksum: d78ff98af735d3c6f507a1115fe6e727)
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
                , lstReq("lstReq")
                , lstResp("lstResp")
                , inpReq("inpReq")
                , inpResp("inpResp")
                , outReq("outReq")
                , outData("outData")
                //[[[end]]] (checksum: 5617e749ec6107095053bfd1f7316050)
	            , idle_monitor("idle_monitor")		 
  {
    /*[[[cog
         for idx,p in enumerate(dut.inps):
           cog.outl("%s_mem_in_arb_ch.bindArbiter<%d>(rd_arbiter,%d,%s_mem_in);" % (p.nm,len(dut.inps),idx,p.nm))
         for idx,p in enumerate(dut.outs):
           cog.outl("%s_mem_out_arb_ch.bindArbiter<%d>(wr_arbiter,%d,%s_mem_out);" % (p.nm,len(dut.outs),idx,p.nm))
      ]]]*/
    lst_mem_in_arb_ch.bindArbiter<2>(rd_arbiter,0,lst_mem_in);
    inp_mem_in_arb_ch.bindArbiter<2>(rd_arbiter,1,inp_mem_in);
    out_mem_out_arb_ch.bindArbiter<1>(wr_arbiter,0,out_mem_out);
    //[[[end]]] (checksum: d5d77f6bd47687f7c3a7c866e03e39d8)

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
    lst_mem_in.clk(clk);
    lst_mem_in.rst(rst);
    inp_mem_in.clk(clk);
    inp_mem_in.rst(rst);
    out_mem_out.clk(clk);
    out_mem_out.rst(rst);
    //[[[end]]] (checksum: 5f24de23b0ef7014032bf1e179a79d17)

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
    dut.lstReqOut(lstReq);
    lst_mem_in.acc_req_in(lstReq);

    dut.lstRespIn(lstResp);
    lst_mem_in.acc_resp_out(lstResp);

    dut.inpReqOut(inpReq);
    inp_mem_in.acc_req_in(inpReq);

    dut.inpRespIn(inpResp);
    inp_mem_in.acc_resp_out(inpResp);

    dut.outReqOut(outReq);
    out_mem_out.acc_req_in(outReq);

    dut.outDataOut(outData);
    out_mem_out.acc_data_in(outData);

    //[[[end]]] (checksum: 5ef7ec3496d55bcf9917efb39a5e5184)
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

