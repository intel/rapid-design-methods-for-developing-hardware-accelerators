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
//[[[end]]]
/*[[[cog
     m = dut.modules[module_nm]
     cog.outl("class %s : public sc_module {" % m.nm)
  ]]]*/
//[[[end]]]
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
  //[[[end]]]

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
  //[[[end]]]
  // hierarchical fifo port declarations:
  /*[[[cog
       for f in dut.tlm_fifos:
         if m.portOf( DequeuePort(f.nm)):
           cog.outl("ga::tlm_fifo_in<%s > %s;" % (f.ty, f.nm))
         if m.portOf( EnqueuePort(f.nm)):
           cog.outl("ga::tlm_fifo_out<%s > %s;" % (f.ty, f.nm))
    ]]]*/
  //[[[end]]]

  // storage fifos: need to determine which belong to module m
  /*[[[cog
       for f in dut.storage_fifos:
         cog.outl("ga::ga_storage_fifo<%s, %d> %s;" % (f.ty, f.capacity, f.nm))
    ]]]*/
  //[[[end]]]

  /*[[[cog
       cog.outl("SC_HAS_PROCESS(%s);" % m.nm)
       cog.outl("%s(sc_module_name modname) :" % m.nm)
    ]]]*/
  //[[[end]]]
    sc_module(modname)
    , clk("clk"), rst("rst")
    /*[[[cog
         if m.writes_to_done:
           cog.outl( ", config(\"config\"), start(\"start\"), done(\"done\")")
         else:
           cog.outl( ", config(\"config\"), start(\"start\")")
      ]]]*/
    //[[[end]]]

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
    //[[[end]]]
  {
    /*[[[cog
         for c in m.cthreads.values():
           cog.outl("SC_CTHREAD(%s, clk.pos());" % (c.nm,))
           cog.outl("async_reset_signal_is(rst, false);")
           cog.outl("")
      ]]]*/
    //[[[end]]]
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
    //[[[end]]]

#ifndef USE_HLS
  /*[[[cog
       for f in dut.storage_fifos:
         cog.outl("  %s.clk_rst(clk, rst);" % f.nm)
    ]]]*/
  //[[[end]]]
#endif
  }

/*[[[cog
     for c in m.cthreads.values():
       cog.outl("#include \"%s-%s.h\"" % (dut.nm,c.nm))
  ]]]*/
//[[[end]]]

};
