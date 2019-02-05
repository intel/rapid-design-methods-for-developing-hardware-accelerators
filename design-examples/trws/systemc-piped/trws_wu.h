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
//module_nm=trws_wu
//[[[end]]] (checksum: 7f4d03c3f1594d20275d7571d468e9e0)
/*[[[cog
     m = dut.modules[module_nm]
     cog.outl("class %s : public sc_module {" % m.nm)
  ]]]*/
class trws_wu : public sc_module {
//[[[end]]] (checksum: f21f03a74ca750cbaad46e46c7b8e303)
public:
  static const unsigned int log2ElementsPerCL = 5;
  static const unsigned int elementsPerCL = 1<<log2ElementsPerCL;
  static const unsigned int maxCLperRow = 4;
  static const unsigned int maxN = maxCLperRow<<log2ElementsPerCL;

  unsigned short gamma[maxN];
  CacheLine weights[maxN][maxCLperRow];
  CacheLine weightsCopy[maxN][maxCLperRow];
  CacheLine weights2[maxN][maxCLperRow];
  CacheLine weights2Copy[maxN][maxCLperRow];
  UCacheLine q[maxN][maxCLperRow];

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
  ga::tlm_fifo_out<MemTypedReadReqType<UCacheLine> > giReqOut;
  ga::tlm_fifo_in<MemTypedReadRespType<UCacheLine> > giRespIn;
  ga::tlm_fifo_out<MemTypedReadReqType<CacheLine> > wiReqOut;
  ga::tlm_fifo_in<MemTypedReadRespType<CacheLine> > wiRespIn;
  ga::tlm_fifo_out<MemTypedReadReqType<UCacheLine> > miReqOut;
  ga::tlm_fifo_in<MemTypedReadRespType<UCacheLine> > miRespIn;
  ga::tlm_fifo_out<MemTypedWriteReqType<UCacheLine> > moReqOut;
  ga::tlm_fifo_out<MemTypedWriteDataType<UCacheLine> > moDataOut;
  //[[[end]]] (checksum: 52196bc101ee73368802c2ef5d4f2cdd)
  // hierarchical fifo port declarations:
  /*[[[cog
       for f in dut.tlm_fifos:
         if m.portOf( DequeuePort(f.nm)):
           cog.outl("ga::tlm_fifo_in<%s > %s;" % (f.ty, f.nm))
         if m.portOf( EnqueuePort(f.nm)):
           cog.outl("ga::tlm_fifo_out<%s > %s;" % (f.ty, f.nm))
    ]]]*/
  ga::tlm_fifo_out<CacheLine > off;
  ga::tlm_fifo_in<UCacheLine > res;
  //[[[end]]] (checksum: fa13ced726abb5c1700a3cb76e4dc122)

  // storage fifos: need to determine which belong to module m
  /*[[[cog
       for f in dut.storage_fifos:
         cog.outl("ga::ga_storage_fifo<%s, %d> %s;" % (f.ty, f.capacity, f.nm))
    ]]]*/
  ga::ga_storage_fifo<CacheLine, 2> out0;
  ga::ga_storage_fifo<CacheLine, 6> out1;
  ga::ga_storage_fifo<WrappedShort, 2> obest;
  //[[[end]]] (checksum: 668eccab1641937199762985c171e692)

  /*[[[cog
       cog.outl("SC_HAS_PROCESS(%s);" % m.nm)
       cog.outl("%s(sc_module_name modname) :" % m.nm)
    ]]]*/
  SC_HAS_PROCESS(trws_wu);
  trws_wu(sc_module_name modname) :
  //[[[end]]] (checksum: 0248635e4e290d0ed6e119372247e129)
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
    , giReqOut("giReqOut")
    , giRespIn("giRespIn")
    , wiReqOut("wiReqOut")
    , wiRespIn("wiRespIn")
    , miReqOut("miReqOut")
    , miRespIn("miRespIn")
    , moReqOut("moReqOut")
    , moDataOut("moDataOut")
    , off("off")
    , res("res")
    //[[[end]]] (checksum: 49f4834822f5aba785ecc1cc97f8d836)
  {
    /*[[[cog
         for c in m.cthreads.values():
           cog.outl("SC_CTHREAD(%s, clk.pos());" % (c.nm,))
           cog.outl("async_reset_signal_is(rst, false);")
           cog.outl("")
      ]]]*/
    SC_CTHREAD(updater, clk.pos());
    async_reset_signal_is(rst, false);

    SC_CTHREAD(gi_addr_gen, clk.pos());
    async_reset_signal_is(rst, false);

    SC_CTHREAD(wi_addr_gen, clk.pos());
    async_reset_signal_is(rst, false);

    SC_CTHREAD(mi_addr_gen, clk.pos());
    async_reset_signal_is(rst, false);

    SC_CTHREAD(mo_addr_gen, clk.pos());
    async_reset_signal_is(rst, false);

    //[[[end]]] (checksum: 79bb6f93950a5cb099e07e2c87ad0161)
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
    giReqOut.clk_rst(clk, rst);
    giRespIn.clk_rst(clk, rst);
    wiReqOut.clk_rst(clk, rst);
    wiRespIn.clk_rst(clk, rst);
    miReqOut.clk_rst(clk, rst);
    miRespIn.clk_rst(clk, rst);
    moReqOut.clk_rst(clk, rst);
    moDataOut.clk_rst(clk, rst);
    off.clk_rst(clk, rst);
    res.clk_rst(clk, rst);
    //[[[end]]] (checksum: 4e0dc5299f6c7da4584b2e089239342c)

#ifndef USE_HLS
  /*[[[cog
       for f in dut.storage_fifos:
         cog.outl("  %s.clk_rst(clk, rst);" % f.nm)
    ]]]*/
  out0.clk_rst(clk, rst);
  out1.clk_rst(clk, rst);
  obest.clk_rst(clk, rst);
  //[[[end]]] (checksum: 9a22c6bebed9798c31c0b28cde43193a)
#endif
  }

/*[[[cog
     for c in m.cthreads.values():
       cog.outl("#include \"%s-%s.h\"" % (dut.nm,c.nm))
  ]]]*/
#include "trws-updater.h"
#include "trws-gi_addr_gen.h"
#include "trws-wi_addr_gen.h"
#include "trws-mi_addr_gen.h"
#include "trws-mo_addr_gen.h"
//[[[end]]] (checksum: 6c13b551de057e3ae832f3d16be56143)

};
