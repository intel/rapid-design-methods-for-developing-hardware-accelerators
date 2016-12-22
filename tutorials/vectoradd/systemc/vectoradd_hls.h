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
#ifndef __VECTORADD_HLS_H__
#define __VECTORADD_HLS_H__
//[[[end]]] (checksum: 68976cbf3fc3a7a88093ee01b4640ba2)

#ifndef __SYNTHESIS__
#include <type_traits>
#endif

#include "systemc.h"

#include "types.h"

#ifndef __inp_Slots__
#define __inp_Slots__ 128
#endif

#include "Config.h"

#include "ga_tlm_fifo.h"

#include "hls_utils.h"

/*[[[cog
     for m in dut.modules.values():
       cog.outl("#include \"%s.h\"" % m.nm)
  ]]]*/
//[[[end]]] (checksum: d41d8cd98f00b204e9800998ecf8427e)

/*[[[cog
     cog.outl("class %s_hls : public sc_module" % dut.nm)
  ]]]*/
class vectoradd_hls : public sc_module
//[[[end]]] (checksum: a522b9d819cfa32ba2a7b5a13ba6b13b)
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
  ga::tlm_fifo_out<MemTypedReadReqType<Blk> > inaReqOut;
  ga::tlm_fifo_in<MemTypedReadRespType<Blk> > inaRespIn;

  ga::tlm_fifo_out<MemTypedReadReqType<Blk> > inbReqOut;
  ga::tlm_fifo_in<MemTypedReadRespType<Blk> > inbRespIn;

  ga::tlm_fifo_out<MemTypedWriteReqType<Blk> > outReqOut;
  ga::tlm_fifo_out<MemTypedWriteDataType<Blk> > outDataOut;

  //[[[end]]] (checksum: c37bbe7561019931de10d28fe97f100b)
  // Instantiate modules
  /*[[[cog
       for m in dut.modules.values():
         cog.outl("%s %s_inst;" % (m.nm, m.nm))
    ]]]*/
  //[[[end]]] (checksum: d41d8cd98f00b204e9800998ecf8427e)

  // TLM fifos (between modules)
  /*[[[cog
       for f in dut.tlm_fifos:
         cog.outl("ga::tlm_fifo<%s, %d> %s;" % (f.ty, f.capacity, f.nm))
    ]]]*/
  //[[[end]]] (checksum: d41d8cd98f00b204e9800998ecf8427e)

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
  SC_HAS_PROCESS(vectoradd_hls);
  //[[[end]]] (checksum: 8a33054eda3c34abd00ab9c12f85f36d)

  /*[[[cog
       cog.outl("%s_hls(sc_module_name modname) :" % (dut.nm,))
    ]]]*/
  vectoradd_hls(sc_module_name modname) :
  //[[[end]]] (checksum: 556d5312d27b1169e2b17f2f9bed19b8)
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
       , inaReqOut("inaReqOut")
       , inaRespIn("inaRespIn")
       , inbReqOut("inbReqOut")
       , inbRespIn("inbRespIn")
       , outReqOut("outReqOut")
       , outDataOut("outDataOut")
       //[[[end]]] (checksum: f885490aa97b874c77060c34d7b26b4a)
  {
    /*[[[cog
         for c in dut.cthreads.values():
           cog.outl("SC_CTHREAD(%s, clk.pos());" % (c.nm,))
           cog.outl("async_reset_signal_is(rst, false);")
           cog.outl("")
      ]]]*/
    SC_CTHREAD(fetcher, clk.pos());
    async_reset_signal_is(rst, false);

    SC_CTHREAD(ina_addr_gen, clk.pos());
    async_reset_signal_is(rst, false);

    SC_CTHREAD(inb_addr_gen, clk.pos());
    async_reset_signal_is(rst, false);

    SC_CTHREAD(out_addr_gen, clk.pos());
    async_reset_signal_is(rst, false);

    //[[[end]]] (checksum: e80ffb6161b62dd81e80181f0c3867df)
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
    inaReqOut.clk_rst(clk, rst);
    inaRespIn.clk_rst(clk, rst);
    inbReqOut.clk_rst(clk, rst);
    inbRespIn.clk_rst(clk, rst);
    outReqOut.clk_rst(clk, rst);
    outDataOut.clk_rst(clk, rst);
    //[[[end]]] (checksum: ac5ad5b4d9749d15e11be56f60fe774a)

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
               if m.portOf( EnqueuePort(f.nm)) or m.portOf( DequeuePort(f.nm)):
                 cog.outl("{0}_inst.{1}({1});".format( m.nm, f.nm))
      ]]]*/
    //[[[end]]] (checksum: d41d8cd98f00b204e9800998ecf8427e)

  }

/*[[[cog
     for c in dut.cthreads.values():
       cog.outl("#include \"%s-%s.h\"" % (dut.nm,c.nm))
  ]]]*/
#include "vectoradd-fetcher.h"
#include "vectoradd-ina_addr_gen.h"
#include "vectoradd-inb_addr_gen.h"
#include "vectoradd-out_addr_gen.h"
//[[[end]]] (checksum: 0f87c475b0dc486318dd24a7681857e6)

};

/*[[[cog
     cog.outl("#include \"%s_acc.h\"" % (dut.nm,))
  ]]]*/
#include "vectoradd_acc.h"
//[[[end]]] (checksum: e252b2d9f214a2e22064d8bfb1dec5d1)

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
typedef vectoradd_acc dut_t;
//[[[end]]] (checksum: 8dff3b4bc9f40180e8a1f4385e3ad540)
#else
/*[[[cog
     cog.outl("#include \"%s_sched.h\"" % dut.nm)
     cog.outl("")
     cog.outl("typedef multi_acc_template_Np<NUM_AUS, %s_sched<NUM_AUS>, %s_acc, Config, RD_CHANNELS, WR_CHANNELS> dut_t;" % (dut.nm,dut.nm))
  ]]]*/
#include "vectoradd_sched.h"

typedef multi_acc_template_Np<NUM_AUS, vectoradd_sched<NUM_AUS>, vectoradd_acc, Config, RD_CHANNELS, WR_CHANNELS> dut_t;
//[[[end]]] (checksum: 894021e16470caf9333017898eb77923)
#endif

#endif

