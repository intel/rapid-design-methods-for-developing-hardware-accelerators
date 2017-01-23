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
#ifndef __BWMATCH_HLS_H__
#define __BWMATCH_HLS_H__
//[[[end]]] (checksum: 21a1e45a9337ad55237de88087263bb9)

#ifndef __SYNTHESIS__
#include <type_traits>
#endif

#include "systemc.h"

#include "types.h"

#include "Config.h"

#include "ga_tlm_fifo.h"

#include "hls_utils.h"

#ifndef __BWPatternLoadParamsSlots__
#define __BWPatternLoadParamsSlots__ 16
#endif

#ifndef __BWCacheLineLoadParamsSlots__
#define __BWCacheLineLoadParamsSlots__ 96
#endif

#ifndef __BWResultLoadParamsSlots__
#define __BWResultLoadParamsSlots__  16
#endif

  typedef UInt32 BWIdx;

  struct BWEmpty {
    //    BWEmpty() : dummy(0) {}
    //    UInt8 dummy;

    friend std::ostream& operator<<( std::ostream& os, const BWEmpty& d) {
      os << "BWEmpty<" << ">" << std::endl;
      return os;
    }

    inline bool operator==( const BWEmpty& rhs) const {
      bool result = true;
      return result;
    }

  };

/*[[[cog
     for m in dut.modules.values():
       cog.outl("#include \"%s.h\"" % m.nm)
  ]]]*/
#include "bwmatch_retire.h"
#include "bwmatch_compute.h"
//[[[end]]] (checksum: 7670de4b8bfc763ec6d50a1c3852871b)

/*[[[cog
     cog.outl("class %s_hls : public sc_module" % dut.nm)
  ]]]*/
class bwmatch_hls : public sc_module
//[[[end]]] (checksum: 4cf2dfc38dc908ba066053b5b01144e5)
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
       for p in dut.inps:
         cog.outl("ga::tlm_fifo_%sout<%s > %s;" % (dut.isHier(RdReqPort(p.nm)),p.reqTy(),p.reqNmK()))
         cog.outl("ga::tlm_fifo_%sin<%s > %s;" % (dut.isHier(RdRespPort(p.nm)),p.respTy(),p.respNmK()))
         cog.outl("")
       for p in dut.outs:
         cog.outl("ga::tlm_fifo_%sout<%s > %s;" % (dut.isHier(WrReqPort(p.nm)),p.reqTy(),p.reqNmK()))
         cog.outl("ga::tlm_fifo_%sout<%s > %s;" % (dut.isHier(WrDataPort(p.nm)),p.dataTy(),p.dataNmK()))
         cog.outl("")
    ]]]*/
  ga::tlm_fifo_hier_out<MemTypedReadReqType<BWPattern> > patReqOut;
  ga::tlm_fifo_hier_in<MemTypedReadRespType<BWPattern> > patRespIn;

  ga::tlm_fifo_hier_out<MemSingleReadReqType<BWCacheLine,BWState> > clReqOut;
  ga::tlm_fifo_hier_in<MemSingleReadRespType<BWCacheLine,BWState> > clRespIn;

  ga::tlm_fifo_hier_out<MemTypedReadReqType<BWResult> > preReqOut;
  ga::tlm_fifo_hier_in<MemTypedReadRespType<BWResult> > preRespIn;

  ga::tlm_fifo_hier_out<MemTypedWriteReqType<BWResultLine> > resReqOut;
  ga::tlm_fifo_hier_out<MemTypedWriteDataType<BWResultLine> > resDataOut;

  //[[[end]]] (checksum: c021c8c5308f3d836bd41cddc225d7f3)
  // Instantiate modules
  /*[[[cog
       for m in dut.modules.values():
         cog.outl("%s %s_inst;" % (m.nm, m.nm))
    ]]]*/
  bwmatch_retire bwmatch_retire_inst;
  bwmatch_compute bwmatch_compute_inst;
  //[[[end]]] (checksum: f54c04b66f888ed36c88bb5d921b6668)

  // TLM fifos (between modules)
  /*[[[cog
       for f in dut.tlm_fifos:
         cog.outl("ga::tlm_fifo<%s, %d> %s;" % (f.ty, f.capacity, f.nm))
    ]]]*/
  ga::tlm_fifo<BWState, 4> patQ;
  ga::tlm_fifo<BWState, 4> finalResultQ;
  //[[[end]]] (checksum: f42668f937ed66dd1d76e372c1359212)

  // storage fifos (between threads in this module)
  /*[[[cog
       for f in dut.storage_fifos:
         if dut.find_parent(dut.put_tbl[f.nm]).nm == dut.module.nm:
            cog.outl("ga::ga_storage_fifo<%s, %d> %s;" % (f.ty, f.capacity, f.nm))
    ]]]*/
  //[[[end]]] (checksum: d41d8cd98f00b204e9800998ecf8427e)

  /*[[[cog
       cog.outl("SC_HAS_PROCESS(%s_hls);" % (dut.nm,))
    ]]]*/
  SC_HAS_PROCESS(bwmatch_hls);
  //[[[end]]] (checksum: adce1318a09b657ed9fcdf6767e6ec0b)

  /*[[[cog
       cog.outl("%s_hls(sc_module_name modname) :" % (dut.nm,))
    ]]]*/
  bwmatch_hls(sc_module_name modname) :
  //[[[end]]] (checksum: 78ea348c690e990ccb1123ab130d7302)
         sc_module(modname)
       , clk("clk"), rst("rst")
       , config("config"), start("start"), done("done")
       /*[[[cog
            for p in dut.inps:
              cog.outl(", {0}(\"{0}\")".format( p.reqNmK()))
              cog.outl(", {0}(\"{0}\")".format( p.respNmK()))
            for p in dut.outs:
              cog.outl(", {0}(\"{0}\")".format( p.reqNmK()))
              cog.outl(", {0}(\"{0}\")".format( p.dataNmK()))
            for m in dut.modules.values():
              cog.outl(", {0}(\"{0}\")".format( m.nm + "_inst"))
         ]]]*/
       , patReqOut("patReqOut")
       , patRespIn("patRespIn")
       , clReqOut("clReqOut")
       , clRespIn("clRespIn")
       , preReqOut("preReqOut")
       , preRespIn("preRespIn")
       , resReqOut("resReqOut")
       , resDataOut("resDataOut")
       , bwmatch_retire_inst("bwmatch_retire_inst")
       , bwmatch_compute_inst("bwmatch_compute_inst")
       //[[[end]]] (checksum: d74ab70bb0750647f326d14d53e7af61)
  {
    /*[[[cog
         for c in dut.cthreads.values():
           cog.outl("SC_CTHREAD(%s, clk.pos());" % (c.nm,))
           cog.outl("async_reset_signal_is(rst, false);")
           cog.outl("")
      ]]]*/
    //[[[end]]] (checksum: d41d8cd98f00b204e9800998ecf8427e)
    /*[[[cog
         for p in dut.inps:
           if "" == dut.isHier(RdReqPort(p.nm)):
              cog.outl("%s.clk_rst(clk, rst);" % (p.reqNmK(),))
           if "" == dut.isHier(RdRespPort(p.nm)):
              cog.outl("%s.clk_rst(clk, rst);" % (p.respNmK(),))
         for p in dut.outs:
           if "" == dut.isHier(WrReqPort(p.nm)):
              cog.outl("%s.clk_rst(clk, rst);" % (p.reqNmK(),))
           if "" == dut.isHier(WrDataPort(p.nm)):
              cog.outl("%s.clk_rst(clk, rst);" % (p.dataNmK(),))
      ]]]*/
    //[[[end]]] (checksum: d41d8cd98f00b204e9800998ecf8427e)

#ifndef USE_HLS
    /*[[[cog
         for f in dut.storage_fifos:
           if dut.find_parent(dut.put_tbl[f.nm]).nm == dut.module.nm:
             cog.outl("%s.clk_rst(clk, rst);" % f.nm)
      ]]]*/
    //[[[end]]] (checksum: d41d8cd98f00b204e9800998ecf8427e)
#endif

    /*[[[cog
         for m in dut.modules.values():
            for sig in ["clk","rst","start","config"]:
               cog.outl("{0}_inst.{1}({1});".format( m.nm, sig))
            if m.writes_to_done:
               cog.outl("{0}_inst.{1}({1});".format( m.nm, "done"))
            for p in dut.inps:
               if m.portOf( RdReqPort(p.nm)):
                 cog.outl("{0}_inst.{1}({1});".format( m.nm, p.reqNmK()))
               if m.portOf( RdRespPort(p.nm)):
                 cog.outl("{0}_inst.{1}({1});".format( m.nm, p.respNmK()))
            for p in dut.outs:
               if m.portOf( WrReqPort(p.nm)):
                 cog.outl("{0}_inst.{1}({1});".format( m.nm, p.reqNmK()))
               if m.portOf( WrDataPort(p.nm)):
                 cog.outl("{0}_inst.{1}({1});".format( m.nm, p.dataNmK()))
            for f in dut.tlm_fifos:
               cog.outl("{0}_inst.{1}({1});".format( m.nm, f.nm))
      ]]]*/
    bwmatch_retire_inst.clk(clk);
    bwmatch_retire_inst.rst(rst);
    bwmatch_retire_inst.start(start);
    bwmatch_retire_inst.config(config);
    bwmatch_retire_inst.patReqOut(patReqOut);
    bwmatch_retire_inst.patRespIn(patRespIn);
    bwmatch_retire_inst.preReqOut(preReqOut);
    bwmatch_retire_inst.preRespIn(preRespIn);
    bwmatch_retire_inst.resReqOut(resReqOut);
    bwmatch_retire_inst.resDataOut(resDataOut);
    bwmatch_retire_inst.patQ(patQ);
    bwmatch_retire_inst.finalResultQ(finalResultQ);
    bwmatch_compute_inst.clk(clk);
    bwmatch_compute_inst.rst(rst);
    bwmatch_compute_inst.start(start);
    bwmatch_compute_inst.config(config);
    bwmatch_compute_inst.done(done);
    bwmatch_compute_inst.clReqOut(clReqOut);
    bwmatch_compute_inst.clRespIn(clRespIn);
    bwmatch_compute_inst.patQ(patQ);
    bwmatch_compute_inst.finalResultQ(finalResultQ);
    //[[[end]]] (checksum: ee51ec8d811b26192e0edb72533df08e)

  }

/*[[[cog
     for c in dut.cthreads.values():
       cog.outl("#include \"%s-%s.h\"" % (dut.nm,c.nm))
  ]]]*/
//[[[end]]] (checksum: d41d8cd98f00b204e9800998ecf8427e)

};

/*[[[cog
     cog.outl("#include \"%s_acc.h\"" % (dut.nm,))
  ]]]*/
#include "bwmatch_acc.h"
//[[[end]]] (checksum: a010d68c95966fefd822df83f1dd6271)

#ifndef NUM_AUS
#define NUM_AUS 1
#endif

#ifndef RD_CHANNELS
#define RD_CHANNELS NUM_AUS
#endif

#ifndef WR_CHANNELS
#define WR_CHANNELS 1
#endif

#if NUM_AUS == 1
/*[[[cog
     cog.outl("typedef %s_acc dut_t;" % (dut.nm,))
  ]]]*/
typedef bwmatch_acc dut_t;
//[[[end]]] (checksum: 5277b98660a5909fc6dab75538e694c8)
#else
/*[[[cog
     cog.outl("#include \"%s_sched.h\"" % dut.nm)
     cog.outl("")
     cog.outl("typedef multi_acc_template_Np<NUM_AUS, %s_sched<NUM_AUS>, %s_acc, Config, RD_CHANNELS, WR_CHANNELS> dut_t;" % (dut.nm,dut.nm))
  ]]]*/
#include "bwmatch_sched.h"

typedef multi_acc_template_Np<NUM_AUS, bwmatch_sched<NUM_AUS>, bwmatch_acc, Config, RD_CHANNELS, WR_CHANNELS> dut_t;
//[[[end]]] (checksum: 79d5d406602f21d5b1558132bbf9fc6b)
#endif

#endif

