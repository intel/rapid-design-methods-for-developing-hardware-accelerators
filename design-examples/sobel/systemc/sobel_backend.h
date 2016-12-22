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
//module_nm=sobel_backend
//[[[end]]] (checksum: 15afbf59f1c8fcf7934f32f5a37fdea7)
/*[[[cog
     m = dut.modules[module_nm]
     cog.outl("class %s : public sc_module {" % m.nm)
  ]]]*/
class sobel_backend : public sc_module {
//[[[end]]] (checksum: 86aad966e1238e6df098d6279c181376)
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
  //[[[end]]] (checksum: d41d8cd98f00b204e9800998ecf8427e)
  // hierarchical fifo port declarations:
  /*[[[cog
       for f in dut.tlm_fifos:
         if m.portOf( DequeuePort(f.nm)):
           cog.outl("ga::tlm_fifo_in<%s > %s;" % (f.ty, f.nm))
         if m.portOf( EnqueuePort(f.nm)):
           cog.outl("ga::tlm_fifo_out<%s > %s;" % (f.ty, f.nm))
    ]]]*/
  ga::tlm_fifo_in<BlkMid > mid0;
  ga::tlm_fifo_out<BlkMid > mid1;
  //[[[end]]] (checksum: aadc84df435dce7f4be6780560591398)

  // storage fifos: need to determine which belong to module m
  /*[[[cog
       for f in dut.storage_fifos:
         cog.outl("ga::ga_storage_fifo<%s, %d> %s;" % (f.ty, f.capacity, f.nm))
    ]]]*/
  //[[[end]]] (checksum: d41d8cd98f00b204e9800998ecf8427e)

  /*[[[cog
       cog.outl("SC_HAS_PROCESS(%s);" % m.nm)
       cog.outl("%s(sc_module_name modname) :" % m.nm)
    ]]]*/
  SC_HAS_PROCESS(sobel_backend);
  sobel_backend(sc_module_name modname) :
  //[[[end]]] (checksum: 94bd8c29b05673b07eedd20223e8eeaa)
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
    , mid0("mid0")
    , mid1("mid1")
    //[[[end]]] (checksum: 1ea5888106b090227efb6758e81c6c5b)
  {
    /*[[[cog
         for c in m.cthreads.values():
           cog.outl("SC_CTHREAD(%s, clk.pos());" % (c.nm,))
           cog.outl("async_reset_signal_is(rst, false);")
           cog.outl("")
      ]]]*/
    SC_CTHREAD(deltay, clk.pos());
    async_reset_signal_is(rst, false);

    //[[[end]]] (checksum: ffbb1c8137004ff6d5c57f92b59a4c46)
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
    mid0.clk_rst(clk, rst);
    mid1.clk_rst(clk, rst);
    //[[[end]]] (checksum: 83615bd3117ebcf8a18c05e8f069b47c)

#ifndef USE_HLS
  /*[[[cog
       for f in dut.storage_fifos:
         cog.outl("  %s.clk_rst(clk, rst);" % f.nm)
    ]]]*/
  //[[[end]]] (checksum: d41d8cd98f00b204e9800998ecf8427e)
#endif
  }

/*[[[cog
     for c in m.cthreads.values():
       cog.outl("#include \"%s-%s.h\"" % (dut.nm,c.nm))
  ]]]*/
#include "sobel-deltay.h"
//[[[end]]] (checksum: f7a4bedd23ceed63389357d960345a57)

};
