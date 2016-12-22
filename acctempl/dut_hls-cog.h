// See LICENSE for license details.
/*[[[cog
     import cog
     from cog_acctempl import *
     from dut_params import *
  ]]]*/
//[[[end]]]
/*[[[cog
     cog.outl("#ifndef __%s_HLS_H__" % dut_name.upper())
     cog.outl("#define __%s_HLS_H__" % dut_name.upper())
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
     cog.outl("class %s_hls : public sc_module" % dut_name)
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

  // memory ports
  /*[[[cog
       for p in inps:
         cog.outl("ga::tlm_fifo_out<%s > %s;" % (p.reqTy(),p.reqNmK()))
         cog.outl("ga::tlm_fifo_in<%s > %s;" % (p.respTy(),p.respNmK()))
         cog.outl("")
       for p in outs:
         cog.outl("ga::tlm_fifo_out<%s > %s;" % (p.reqTy(),p.reqNmK()))
         cog.outl("ga::tlm_fifo_out<%s > %s;" % (p.dataTy(),p.dataNmK()))
         cog.outl("")
    ]]]*/
  //[[[end]]]
  /*[[[cog
       cog.outl("SC_HAS_PROCESS(%s_hls);" % (dut_name,))
    ]]]*/
  //[[[end]]]

  /*[[[cog
       cog.outl("%s_hls(sc_module_name modname) :" % (dut_name,))
    ]]]*/
  //[[[end]]]
         sc_module(modname)
       , clk("clk"), rst("rst")
       , config("config"), start("start"), done("done")
       /*[[[cog
            for p in inps:
              cog.outl(""", %s("%s")""" % (p.reqNmK(),p.reqNmK()))
              cog.outl(""", %s("%s")""" % (p.respNmK(),p.respNmK()))
            for p in outs:
              cog.outl(""", %s("%s")""" % (p.reqNmK(),p.reqNmK()))
              cog.outl(""", %s("%s")""" % (p.dataNmK(),p.dataNmK()))
         ]]]*/
       //[[[end]]]
  {
    //Add SC_CTHREAD (and async_reset_signal_is) calls
    /*Example:
    SC_CTHREAD( thread_function_name, clk.pos());
    async_reset_signal_is(rst, false);
    */

    /*[[[cog
         for p in inps:
           cog.outl("%s.clk_rst(clk, rst);" % (p.reqNmK(),))
           cog.outl("%s.clk_rst(clk, rst);" % (p.respNmK(),))
         for p in outs:
           cog.outl("%s.clk_rst(clk, rst);" % (p.reqNmK(),))
           cog.outl("%s.clk_rst(clk, rst);" % (p.dataNmK(),))
      ]]]*/
    //[[[end]]]

  }

  //Add C-thread definitions
  // Remember to ".reset_put()" or ".reset_get()" the interface fifos you use in the thread

  /*Outline:
  void thread_function_name() {

    // Reset get and put ports
    // Declare local variables
    // Initialize variables

    done = false;

    wait();

    while (1) {
      if ( start) {

        // Perform get
        // Modify data
        // Perform put

        // set done to true

      }
      wait();
    }
  }  
  */

};

/*[[[cog
     cog.outl("#include \"%s_acc.h\"" % (dut_name,))
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
     cog.outl("typedef %s_acc dut_t;" % (dut_name,))
  ]]]*/
//[[[end]]]
#else
/*[[[cog
     cog.outl("#include \"%s_sched.h\"" % dut_name)
     cog.outl("")
     cog.outl("typedef multi_acc_template_Np<NUM_AUS, %s_sched<NUM_AUS>, %s_acc, Config, RD_CHANNELS, WR_CHANNELS> dut_t;" % (dut_name,dut_name))
  ]]]*/
//[[[end]]]
#endif

#endif

