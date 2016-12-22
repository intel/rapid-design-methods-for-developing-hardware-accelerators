// See LICENSE for license details.
/*[[[cog
     import cog
     from cog_acctempl import *
     from dut_params import *
  ]]]*/
//[[[end]]] (checksum: d41d8cd98f00b204e9800998ecf8427e)
/*[[[cog
     cog.outl("#ifndef __%s_HLS_H__" % dut.nm.upper())
     cog.outl("#define __%s_HLS_H__" % dut.nm.upper())
  ]]]*/
#ifndef __CYCLE_DETECTION_HLS_H__
#define __CYCLE_DETECTION_HLS_H__
//[[[end]]] (checksum: ec82331dffc61303ec0311557f0be957)

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
class cycle_detection_hls : public sc_module
//[[[end]]] (checksum: b86d4c33541c6e64e3555ccc62bd98a9)
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
#include "cycle_detection_declarations.h"
//[[[end]]] (checksum: 853e514a2b10ffd8aea8d21568ef8845)

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
  ga::tlm_fifo_out<MemTypedReadReqType<Node> > inpReqOut;
  ga::tlm_fifo_in<MemTypedReadRespType<Node> > inpRespIn;

  ga::tlm_fifo_out<MemTypedWriteReqType<CycleExistInfo> > outReqOut;
  ga::tlm_fifo_out<MemTypedWriteDataType<CycleExistInfo> > outDataOut;

  //[[[end]]] (checksum: 39cb1fbafd0a293010c108fe92bb8a76)
  // storage fifos
  /*[[[cog
       for f in dut.storage_fifos:
         cog.outl("ga::ga_storage_fifo<%s, %d> %s;" % (f.ty, f.capacity, f.nm))
    ]]]*/
  //[[[end]]] (checksum: d41d8cd98f00b204e9800998ecf8427e)

  /*[[[cog
       cog.outl("SC_HAS_PROCESS(%s_hls);" % (dut.nm,))
    ]]]*/
  SC_HAS_PROCESS(cycle_detection_hls);
  //[[[end]]] (checksum: 3b0ca55ee14d90fdf218abb17acc4010)


  Config configReg;

  long long unsigned slow_offset;
  long long unsigned fast_offset;

  bool started;


  /*[[[cog
       cog.outl("%s_hls(sc_module_name modname) :" % (dut.nm,))
    ]]]*/
  cycle_detection_hls(sc_module_name modname) :
  //[[[end]]] (checksum: 646d9160996330c87f5de6b8b20bb8cd)
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
       , inpReqOut("inpReqOut")
       , inpRespIn("inpRespIn")
       , outReqOut("outReqOut")
       , outDataOut("outDataOut")
       //[[[end]]] (checksum: 2881ddb8430a98cd05d5dae52488ecb0)
  {
    //Add SC_CTHREAD (and async_reset_signal_is) calls
    SC_CTHREAD(detection, clk.pos());
    async_reset_signal_is(rst, false);

    /*[[[cog
         for p in dut.inps:
           cog.outl("%s.clk_rst(clk, rst);" % (p.reqNmK(),))
           cog.outl("%s.clk_rst(clk, rst);" % (p.respNmK(),))
         for p in dut.outs:
           cog.outl("%s.clk_rst(clk, rst);" % (p.reqNmK(),))
           cog.outl("%s.clk_rst(clk, rst);" % (p.dataNmK(),))
      ]]]*/
    inpReqOut.clk_rst(clk, rst);
    inpRespIn.clk_rst(clk, rst);
    outReqOut.clk_rst(clk, rst);
    outDataOut.clk_rst(clk, rst);
    //[[[end]]] (checksum: 0436c52fea41c23b502c1f072eb76632)

  }

  void detection() {
    started = false;
    done = false;
    slow_offset = 0;
    fast_offset = 0;
    inpReqOut.reset_put();
    inpRespIn.reset_get();
    outReqOut.reset_put();
    outDataOut.reset_put();

    wait();

    while (1) {
      if (start && (!started))
      {
        configReg = config;
        slow_offset = configReg.getInpAddr(0);
        fast_offset = configReg.getInpAddr(0);
        started = true;
      } 
      else if(started &&(!done))
      {
        if(!fast_offset){ // reached the end, no cycle found
          outReqOut.put(MemTypedWriteReqType<CycleExistInfo>(configReg.getOutAddr(0),1));
          outDataOut.put(MemTypedWriteDataType<CycleExistInfo>(false));
          done = true;
        } else { // move fast pointer
            inpReqOut.put(MemTypedReadReqType<Node> (fast_offset,1)); 
            fast_offset = inpRespIn.get().data.next_offset;
            if(fast_offset == slow_offset){ // found cycle
                outReqOut.put(MemTypedWriteReqType<CycleExistInfo>(configReg.getOutAddr(0),1));
                outDataOut.put(MemTypedWriteDataType<CycleExistInfo>(true));
                done = true;
            }
            if (fast_offset && !done) { // move both pointers
            // LAB3: add code to move both fast and slow offset (pointers). see how the fast_offset is moved above
            // ...
            // fast_offset = ...
            // slow_offset = ...
            // END OF LAB3  
            }
         }
      }
      wait();
    }
  }  

};

/*[[[cog
     cog.outl("#include \"%s_acc.h\"" % (dut.nm,))
  ]]]*/
#include "cycle_detection_acc.h"
//[[[end]]] (checksum: d8578d602dfadfc7a250b76ffeda29fc)

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
typedef cycle_detection_acc dut_t;
//[[[end]]] (checksum: b8871e4f264a2179d98bd2850e73cf51)
#else
/*[[[cog
     cog.outl("#include \"%s_sched.h\"" % dut.nm)
     cog.outl("")
     cog.outl("typedef multi_acc_template_Np<NUM_AUS, %s_sched<NUM_AUS>, %s_acc, Config, RD_CHANNELS, WR_CHANNELS> dut_t;" % (dut.nm,dut.nm))
  ]]]*/
#include "cycle_detection_sched.h"

typedef multi_acc_template_Np<NUM_AUS, cycle_detection_sched<NUM_AUS>, cycle_detection_acc, Config, RD_CHANNELS, WR_CHANNELS> dut_t;
//[[[end]]] (checksum: efe935f82903130dd119c0237fa1a560)
#endif

#endif

