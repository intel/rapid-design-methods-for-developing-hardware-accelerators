// See LICENSE for license details.
/*[[[cog
     import cog
     from cog_acctempl import *
     from dut_params import *
  ]]]*/
//[[[end]]]
/*[[[cog
     cog.outl("#ifndef __%s_HLS_H__" % dut.nm.upper())
     cog.outl("#define __%s_HLS_H__" % dut.nm.upper())
  ]]]*/
//[[[end]]]

#ifndef __SYNTHESIS__
#include <type_traits>
#endif

#include "systemc.h"

#include "types.h"

#include "Config.h"

#include "ga_tlm_fifo.h"

#include "hls_utils.h"

/*[[[cog
     cog.outl("class %s_hls : public sc_module" % dut.nm)
  ]]]*/
//[[[end]]]
{

public:
  sc_in_clk clk;
  sc_in<bool> rst;

  // functional ports
  sc_in<Config> config;
  sc_in<bool> start;
  sc_out<bool> done;

/*[[[cog
     cog.outl( "#include \"%s_declarations.h\"" % dut.nm)
  ]]]*/
//[[[end]]]

  // memory ports
  /*[[[cog
       for p in dut.inps:
         cog.outl("ga::tlm_fifo_out<%s > %s;" % (p.reqTy(),p.reqNmK()))
         cog.outl("ga::tlm_fifo_in<%s > %s;" % (p.respTy(),p.respNmK()))
         cog.outl("")
       for p in dut.outs:
         cog.outl("ga::tlm_fifo_out<%s > %s;" % (p.reqTy(),p.reqNmK()))
         cog.outl("ga::tlm_fifo_out<%s > %s;" % (p.dataTy(),p.dataNmK()))
         cog.outl("")
    ]]]*/
  //[[[end]]]
  // storage fifos
  /*[[[cog
       for f in dut.storage_fifos:
         cog.outl("ga::ga_storage_fifo<%s, %d> %s;" % (f.ty, f.capacity, f.nm))
    ]]]*/
  //[[[end]]]

  /*[[[cog
       cog.outl("SC_HAS_PROCESS(%s_hls);" % (dut.nm,))
    ]]]*/
  //[[[end]]]

  /*[[[cog
       cog.outl("%s_hls(sc_module_name modname) :" % (dut.nm,))
    ]]]*/
  //[[[end]]]
         sc_module(modname)
       , clk("clk"), rst("rst")
       , config("config"), start("start"), done("done")
       /*[[[cog
            for p in dut.inps:
              cog.outl(""", %s("%s")""" % (p.reqNmK(),p.reqNmK()))
              cog.outl(""", %s("%s")""" % (p.respNmK(),p.respNmK()))
            for p in dut.outs:
              cog.outl(""", %s("%s")""" % (p.reqNmK(),p.reqNmK()))
              cog.outl(""", %s("%s")""" % (p.dataNmK(),p.dataNmK()))
         ]]]*/
       //[[[end]]]
  {
    /*[[[cog
         for c in dut.cthreads.values():
           cog.outl("SC_CTHREAD(%s, clk.pos());" % (c.nm,))
           cog.outl("async_reset_signal_is(rst, false);")
           cog.outl("")
      ]]]*/
    //[[[end]]]
    /*[[[cog
         for p in dut.inps:
           cog.outl("%s.clk_rst(clk, rst);" % (p.reqNmK(),))
           cog.outl("%s.clk_rst(clk, rst);" % (p.respNmK(),))
         for p in dut.outs:
           cog.outl("%s.clk_rst(clk, rst);" % (p.reqNmK(),))
           cog.outl("%s.clk_rst(clk, rst);" % (p.dataNmK(),))
      ]]]*/
    //[[[end]]]

#ifndef USE_HLS
    /*[[[cog
         for f in dut.storage_fifos:
           cog.outl("%s.clk_rst(clk, rst);" % f.nm)
      ]]]*/
    //[[[end]]]
#endif

  }

/*[[[cog
     for c in dut.cthreads.values():
       cog.outl("#include \"%s-%s.h\"" % (dut.nm,c.nm))
  ]]]*/
//[[[end]]]

};

/*[[[cog
     cog.outl("#include \"%s_acc.h\"" % (dut.nm,))
  ]]]*/
//[[[end]]]

#ifndef NUM_AUS
#define NUM_AUS 1
#endif

#ifndef RD_CHANNELS
#define RD_CHANNELS NUM_AUS
#endif

#ifndef WR_CHANNELS
#define WR_CHANNELS NUM_AUS
#endif

#if NUM_AUS == 1
/*[[[cog
     cog.outl("typedef %s_acc dut_t;" % (dut.nm,))
  ]]]*/
//[[[end]]]
#else
/*[[[cog
     cog.outl("#include \"%s_sched.h\"" % dut.nm)
     cog.outl("")
     cog.outl("typedef multi_acc_template_Np<NUM_AUS, %s_sched<NUM_AUS>, %s_acc, Config, RD_CHANNELS, WR_CHANNELS> dut_t;" % (dut.nm,dut.nm))
  ]]]*/
//[[[end]]]
#endif

#endif

