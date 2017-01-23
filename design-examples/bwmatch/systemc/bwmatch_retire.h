// See LICENSE for license details.
/*[[[cog
     import cog
     from cog_acctempl import *
     from dut_params import dut
     if "module_nm" not in globals():
       lst = cog.previous.lstrip('/').rstrip('\n').split('=')
       assert( lst[0]=="module_nm")
       assert( len(lst)==2)
       global module_nm
       module_nm = lst[1]
     cog.outl( "//module_nm=" + module_nm)
  ]]]*/
//module_nm=bwmatch_retire
//[[[end]]] (checksum: 092bb720acb4734ad4eb11cb031b3989)
/*[[[cog
     m = dut.modules[module_nm]
     cog.outl("class %s : public sc_module {" % m.nm)
  ]]]*/
class bwmatch_retire : public sc_module {
//[[[end]]] (checksum: ce5f431460d4b3eec5f12471e35996c2)
public:

  sc_in_clk clk;
  sc_in<bool> rst;

  // functional ports
  sc_in<Config> config;
  sc_in<bool> start;
  /*[[[cog
       if m.writes_to_done:
         cog.outl("sc_out<bool> done;")
    ]]]*/
  //[[[end]]] (checksum: d41d8cd98f00b204e9800998ecf8427e)

  /*[[[cog
       for p in dut.inps:
         if m.portOf( RdReqPort(p.nm)):
           cog.outl("ga::tlm_fifo_out<%s > %s;" % (p.reqTy(),p.reqNmK()))
         if m.portOf( RdRespPort(p.nm)):
           cog.outl("ga::tlm_fifo_in<%s > %s;" % (p.respTy(),p.respNmK()))
       for p in dut.outs:
         if m.portOf( WrReqPort(p.nm)):
           cog.outl("ga::tlm_fifo_out<%s > %s;" % (p.reqTy(),p.reqNmK()))
         if m.portOf( WrDataPort(p.nm)):
           cog.outl("ga::tlm_fifo_out<%s > %s;" % (p.dataTy(),p.dataNmK()))
    ]]]*/
  ga::tlm_fifo_out<MemTypedReadReqType<BWPattern> > patReqOut;
  ga::tlm_fifo_in<MemTypedReadRespType<BWPattern> > patRespIn;
  ga::tlm_fifo_out<MemTypedReadReqType<BWResult> > preReqOut;
  ga::tlm_fifo_in<MemTypedReadRespType<BWResult> > preRespIn;
  ga::tlm_fifo_out<MemTypedWriteReqType<BWResultLine> > resReqOut;
  ga::tlm_fifo_out<MemTypedWriteDataType<BWResultLine> > resDataOut;
  //[[[end]]] (checksum: 45540050d43e2c65226f6bb2caa3564c)
  // hierarchical fifo port declarations:
  /*[[[cog
       for f in dut.tlm_fifos:
         if m.portOf( DequeuePort(f.nm)):
           cog.outl("ga::tlm_fifo_in<%s > %s;" % (f.ty, f.nm))
         if m.portOf( EnqueuePort(f.nm)):
           cog.outl("ga::tlm_fifo_out<%s > %s;" % (f.ty, f.nm))
    ]]]*/
  ga::tlm_fifo_out<BWState > patQ;
  ga::tlm_fifo_in<BWState > finalResultQ;
  //[[[end]]] (checksum: b29d22ce490d4f60ef2c0d12531e9e1f)

  // storage fifos: need to determine which belong to module m
  /*[[[cog
       for f in dut.storage_fifos:
         cog.outl("ga::ga_storage_fifo<%s, %d> %s;" % (f.ty, f.capacity, f.nm))
    ]]]*/
  ga::ga_storage_fifo<BWState, 4> irowQ;
  ga::ga_storage_fifo<BWState, 4> partialResultQ;
  ga::ga_storage_fifo<BWIdx, 4> reserveQ;
  ga::ga_storage_fifo<BWEmpty, 4> reserveAckQ;
  //[[[end]]] (checksum: 5534893910d77e28f539bc1e4e8e9a82)

  //User defined shared variables
  sc_signal<bool> phase;

  // If you increase this check that loop variables are large enough
#ifndef MAX_PRECOMP_LEN 
#define MAX_PRECOMP_LEN 7
#endif  

  BWResult precomp_res[1<<(2*MAX_PRECOMP_LEN)];


  /*[[[cog
       cog.outl("SC_HAS_PROCESS(%s);" % m.nm)
       cog.outl("%s(sc_module_name modname) :" % m.nm)
    ]]]*/
  SC_HAS_PROCESS(bwmatch_retire);
  bwmatch_retire(sc_module_name modname) :
  //[[[end]]] (checksum: 10c86209203dd4df66461aced028dc52)
    sc_module(modname)
    , clk("clk"), rst("rst")
    /*[[[cog
         if m.writes_to_done:
           cog.outl( ", config(\"config\"), start(\"start\"), done(\"done\")")
         else:
           cog.outl( ", config(\"config\"), start(\"start\")")
      ]]]*/
    , config("config"), start("start")
    //[[[end]]] (checksum: 94112a28b4b3fd90b2500fe25f5baaae)

    /*[[[cog
         for p in dut.inps:
           if m.portOf( RdReqPort(p.nm)):
             cog.outl(", %s(\"%s\")" % (p.reqNmK(),p.reqNmK()))
           if m.portOf( RdRespPort(p.nm)):
             cog.outl(", %s(\"%s\")" % (p.respNmK(),p.respNmK()))
         for p in dut.outs:
           if m.portOf( WrReqPort(p.nm)):
             cog.outl(", %s(\"%s\")" % (p.reqNmK(),p.reqNmK()))
           if m.portOf( WrDataPort(p.nm)):
             cog.outl(", %s(\"%s\")" % (p.dataNmK(),p.dataNmK()))
         for f in dut.tlm_fifos:
           if m.portOf( DequeuePort(f.nm)) or m.portOf( EnqueuePort(f.nm)):
             cog.outl(", %s(\"%s\")" % (f.nm, f.nm))
      ]]]*/
    , patReqOut("patReqOut")
    , patRespIn("patRespIn")
    , preReqOut("preReqOut")
    , preRespIn("preRespIn")
    , resReqOut("resReqOut")
    , resDataOut("resDataOut")
    , patQ("patQ")
    , finalResultQ("finalResultQ")
    //[[[end]]] (checksum: 2c7512050615b3673171a4b1edf46249)
  {
    /*[[[cog
         for c in m.cthreads.values():
           cog.outl("SC_CTHREAD(%s, clk.pos());" % (c.nm,))
           cog.outl("async_reset_signal_is(rst, false);")
           cog.outl("")
      ]]]*/
    SC_CTHREAD(res_reorder, clk.pos());
    async_reset_signal_is(rst, false);

    SC_CTHREAD(pat_gadget, clk.pos());
    async_reset_signal_is(rst, false);

    SC_CTHREAD(precomp_addr_gen, clk.pos());
    async_reset_signal_is(rst, false);

    SC_CTHREAD(precomp_fill, clk.pos());
    async_reset_signal_is(rst, false);

    SC_CTHREAD(pat_gadget2, clk.pos());
    async_reset_signal_is(rst, false);

    SC_CTHREAD(pat_addr_gen, clk.pos());
    async_reset_signal_is(rst, false);

    //[[[end]]] (checksum: f5dbfe35b601577916b4f9de3fdb9cf5)
    // TLM input output port reset: need to remove some and add tlm_fifo ports
    /*[[[cog
         for p in dut.inps:
           if m.portOf( RdReqPort(p.nm)):
             cog.outl("%s.clk_rst(clk, rst);" % (p.reqNmK(),))
           if m.portOf( RdRespPort(p.nm)):
             cog.outl("%s.clk_rst(clk, rst);" % (p.respNmK(),))
         for p in dut.outs:
           if m.portOf( WrReqPort(p.nm)):
             cog.outl("%s.clk_rst(clk, rst);" % (p.reqNmK(),))
           if m.portOf( WrDataPort(p.nm)):
             cog.outl("%s.clk_rst(clk, rst);" % (p.dataNmK(),))
         for f in dut.tlm_fifos:
           if m.portOf( DequeuePort(f.nm)) or m.portOf( EnqueuePort(f.nm)):
             cog.outl("%s.clk_rst(clk, rst);" % (f.nm,))
      ]]]*/
    patReqOut.clk_rst(clk, rst);
    patRespIn.clk_rst(clk, rst);
    preReqOut.clk_rst(clk, rst);
    preRespIn.clk_rst(clk, rst);
    resReqOut.clk_rst(clk, rst);
    resDataOut.clk_rst(clk, rst);
    patQ.clk_rst(clk, rst);
    finalResultQ.clk_rst(clk, rst);
    //[[[end]]] (checksum: 4dcba0847bea45d0e15855584bbb9769)

#ifndef USE_HLS
  /*[[[cog
       for f in dut.storage_fifos:
         cog.outl("  %s.clk_rst(clk, rst);" % f.nm)
    ]]]*/
  irowQ.clk_rst(clk, rst);
  partialResultQ.clk_rst(clk, rst);
  reserveQ.clk_rst(clk, rst);
  reserveAckQ.clk_rst(clk, rst);
  //[[[end]]] (checksum: d0a7779edcf76f4b967acd05eee81915)
#endif
  }

/*[[[cog
     for c in m.cthreads.values():
       cog.outl("#include \"%s-%s.h\"" % (dut.nm,c.nm))
  ]]]*/
#include "bwmatch-res_reorder.h"
#include "bwmatch-pat_gadget.h"
#include "bwmatch-precomp_addr_gen.h"
#include "bwmatch-precomp_fill.h"
#include "bwmatch-pat_gadget2.h"
#include "bwmatch-pat_addr_gen.h"
//[[[end]]] (checksum: f27a1537398768073adf688b5c262181)

};
