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
//module_nm=bwmatch_compute
//[[[end]]] (checksum: 127b96939aafab45dbb486f2a62c29f4)
/*[[[cog
     m = dut.modules[module_nm]
     cog.outl("class %s : public sc_module {" % m.nm)
  ]]]*/
class bwmatch_compute : public sc_module {
//[[[end]]] (checksum: 3759c4cb8662093d38503011336c3348)
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
  sc_out<bool> done;
  //[[[end]]] (checksum: 1815417e5fd0e4cdf393319dca5a159b)

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
  ga::tlm_fifo_out<MemSingleReadReqType<BWCacheLine,BWState> > clReqOut;
  ga::tlm_fifo_in<MemSingleReadRespType<BWCacheLine,BWState> > clRespIn;
  //[[[end]]] (checksum: 72fe38711e2d738bf9ce5ad21bc12451)
  // hierarchical fifo port declarations:
  /*[[[cog
       for f in dut.tlm_fifos:
         if m.portOf( DequeuePort(f.nm)):
           cog.outl("ga::tlm_fifo_in<%s > %s;" % (f.ty, f.nm))
         if m.portOf( EnqueuePort(f.nm)):
           cog.outl("ga::tlm_fifo_out<%s > %s;" % (f.ty, f.nm))
    ]]]*/
  ga::tlm_fifo_in<BWState > patQ;
  ga::tlm_fifo_out<BWState > finalResultQ;
  //[[[end]]] (checksum: 88c1af5727b4183e5cd6e686ff90d113)

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

  /*[[[cog
       cog.outl("SC_HAS_PROCESS(%s);" % m.nm)
       cog.outl("%s(sc_module_name modname) :" % m.nm)
    ]]]*/
  SC_HAS_PROCESS(bwmatch_compute);
  bwmatch_compute(sc_module_name modname) :
  //[[[end]]] (checksum: 8c337a8afe6df56825c1d3f2830c4cd7)
    sc_module(modname)
    , clk("clk"), rst("rst")
    /*[[[cog
         if m.writes_to_done:
           cog.outl( ", config(\"config\"), start(\"start\"), done(\"done\")")
         else:
           cog.outl( ", config(\"config\"), start(\"start\")")
      ]]]*/
    , config("config"), start("start"), done("done")
    //[[[end]]] (checksum: 875e1f060b92d3b43a9944e102fe22fe)

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
    , clReqOut("clReqOut")
    , clRespIn("clRespIn")
    , patQ("patQ")
    , finalResultQ("finalResultQ")
    //[[[end]]] (checksum: 661c5ca82b9bea371c571f0d0a0bcb55)
  {
    /*[[[cog
         for c in m.cthreads.values():
           cog.outl("SC_CTHREAD(%s, clk.pos());" % (c.nm,))
           cog.outl("async_reset_signal_is(rst, false);")
           cog.outl("")
      ]]]*/
    SC_CTHREAD(pat_fetcher, clk.pos());
    async_reset_signal_is(rst, false);

    SC_CTHREAD(occurrence_wr_loop, clk.pos());
    async_reset_signal_is(rst, false);

    SC_CTHREAD(occurrence_rd_loop, clk.pos());
    async_reset_signal_is(rst, false);

    //[[[end]]] (checksum: 19d9a5f9579cdd4afcf1081fe9333785)
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
    clReqOut.clk_rst(clk, rst);
    clRespIn.clk_rst(clk, rst);
    patQ.clk_rst(clk, rst);
    finalResultQ.clk_rst(clk, rst);
    //[[[end]]] (checksum: bd4f2dec0ad462cdcd1c8ac26f6105fd)

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
#include "bwmatch-pat_fetcher.h"
#include "bwmatch-occurrence_wr_loop.h"
#include "bwmatch-occurrence_rd_loop.h"
//[[[end]]] (checksum: ae5a64480fd3550795726680e0994921)

};
