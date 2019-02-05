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
//module_nm=trws_loaf
//[[[end]]] (checksum: a3240da6b2f56dadf47e282286ef1879)
/*[[[cog
     m = dut.modules[module_nm]
     cog.outl("class %s : public sc_module {" % m.nm)
  ]]]*/
class trws_loaf : public sc_module {
//[[[end]]] (checksum: 98855fe66a05038407fd0b77b4a23271)
public:

  const unsigned int maxCLperRow = 4;
  const unsigned int log2ElementsPerCL = 5;
  const unsigned int elementsPerCL = 1<<log2ElementsPerCL;
  const unsigned int maxN = maxCLperRow<<log2ElementsPerCL;
  const unsigned int log2RowsPerClock = 5;
  const unsigned int rowsPerClock = 1<<log2RowsPerClock;
  // same as rowsPerClock
#define WAYS 32
  const unsigned int log2ColumnsPerClock = 5;
  const unsigned int columnsPerClock = 1<<log2ColumnsPerClock;

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
  ga::tlm_fifo_out<MemTypedReadReqType<Pair> > inpReqOut;
  ga::tlm_fifo_in<MemTypedReadRespType<Pair> > inpRespIn;
  //[[[end]]] (checksum: 9a013acc32ca54657b0d66dd3a19d057)
  // hierarchical fifo port declarations:
  /*[[[cog
       for f in dut.tlm_fifos:
         if m.portOf( DequeuePort(f.nm)):
           cog.outl("ga::tlm_fifo_in<%s > %s;" % (f.ty, f.nm))
         if m.portOf( EnqueuePort(f.nm)):
           cog.outl("ga::tlm_fifo_out<%s > %s;" % (f.ty, f.nm))
    ]]]*/
  ga::tlm_fifo_in<CacheLine > off;
  ga::tlm_fifo_out<CacheLine > out;
  //[[[end]]] (checksum: aa215eceadbec44953e79ca85e1ee12f)

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
  SC_HAS_PROCESS(trws_loaf);
  trws_loaf(sc_module_name modname) :
  //[[[end]]] (checksum: f6a4e017bc2fa1196c3b884276c5dd4e)
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
    , inpReqOut("inpReqOut")
    , inpRespIn("inpRespIn")
    , off("off")
    , out("out")
    //[[[end]]] (checksum: 6d0c031c2fc4f60f8432eb2b86652e70)
  {
    /*[[[cog
         for c in m.cthreads.values():
           cog.outl("SC_CTHREAD(%s, clk.pos());" % (c.nm,))
           cog.outl("async_reset_signal_is(rst, false);")
           cog.outl("")
      ]]]*/
    SC_CTHREAD(inp_fetcher, clk.pos());
    async_reset_signal_is(rst, false);

    SC_CTHREAD(inp_addr_gen, clk.pos());
    async_reset_signal_is(rst, false);

    //[[[end]]] (checksum: 51f4777338016fe2d3f3b51f9c0c2156)
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
    inpReqOut.clk_rst(clk, rst);
    inpRespIn.clk_rst(clk, rst);
    off.clk_rst(clk, rst);
    out.clk_rst(clk, rst);
    //[[[end]]] (checksum: 445766f431bb5c9aac969b0d6c25f073)

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
#include "trws-inp_fetcher.h"
#include "trws-inp_addr_gen.h"
//[[[end]]] (checksum: 0c17edffc91fda12eb109ac2225e35b3)

};
