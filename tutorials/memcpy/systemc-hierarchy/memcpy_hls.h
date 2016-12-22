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
#ifndef __MEMCPY_HLS_H__
#define __MEMCPY_HLS_H__
//[[[end]]] (checksum: f5d0a4d0e706ab8b40e85b21f0328629)

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
class memcpy_hls : public sc_module
//[[[end]]] (checksum: fa2560ce13aa08d8a581d079b39bf215)
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
  ga::tlm_fifo_out<MemTypedReadReqType<CacheLine> > inpReqOut;
  ga::tlm_fifo_in<MemTypedReadRespType<CacheLine> > inpRespIn;

  ga::tlm_fifo_out<MemTypedWriteReqType<CacheLine> > outReqOut;
  ga::tlm_fifo_out<MemTypedWriteDataType<CacheLine> > outDataOut;

  //[[[end]]] (checksum: fd9fde2da603878fca9336358740669c)
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
  SC_HAS_PROCESS(memcpy_hls);
  //[[[end]]] (checksum: 1639f171cef6dba259fd16954ea97228)

  /*[[[cog
       cog.outl("%s_hls(sc_module_name modname) :" % (dut.nm,))
    ]]]*/
  memcpy_hls(sc_module_name modname) :
  //[[[end]]] (checksum: 18fc9bafe7cadd2e6ff44e18aab1421e)
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
       , inpReqOut("inpReqOut")
       , inpRespIn("inpRespIn")
       , outReqOut("outReqOut")
       , outDataOut("outDataOut")
       //[[[end]]] (checksum: 2881ddb8430a98cd05d5dae52488ecb0)
  {
    /*[[[cog
         for c in dut.cthreads.values():
           cog.outl("SC_CTHREAD(%s, clk.pos());" % (c.nm,))
           cog.outl("async_reset_signal_is(rst, false);")
           cog.outl("")
      ]]]*/
    SC_CTHREAD(inp_fetcher, clk.pos());
    async_reset_signal_is(rst, false);

    SC_CTHREAD(inp_addr_gen, clk.pos());
    async_reset_signal_is(rst, false);

    SC_CTHREAD(out_addr_gen, clk.pos());
    async_reset_signal_is(rst, false);

    //[[[end]]] (checksum: 773ff4522c18e0df27ce11fb3e624133)
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
    inpReqOut.clk_rst(clk, rst);
    inpRespIn.clk_rst(clk, rst);
    outReqOut.clk_rst(clk, rst);
    outDataOut.clk_rst(clk, rst);
    //[[[end]]] (checksum: 0436c52fea41c23b502c1f072eb76632)

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
    //[[[end]]] (checksum: d41d8cd98f00b204e9800998ecf8427e)

  }

/*[[[cog
     for c in dut.cthreads.values():
       cog.outl("#include \"%s-%s.h\"" % (dut.nm,c.nm))
  ]]]*/
#include "memcpy-inp_fetcher.h"
#include "memcpy-inp_addr_gen.h"
#include "memcpy-out_addr_gen.h"
//[[[end]]] (checksum: 484e6e29ad0d99d170f7f9de8f0c3cb6)

};

/*[[[cog
     cog.outl("#include \"%s_acc.h\"" % (dut.nm,))
  ]]]*/
#include "memcpy_acc.h"
//[[[end]]] (checksum: e868954c0cd9f92182731fd98ad0951c)

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
typedef memcpy_acc dut_t;
//[[[end]]] (checksum: 0ce887778cbdeebc316aa28c2a590876)
#else
/*[[[cog
     cog.outl("#include \"%s_sched.h\"" % dut.nm)
     cog.outl("")
     cog.outl("typedef multi_acc_template_Np<NUM_AUS, %s_sched<NUM_AUS>, %s_acc, Config, RD_CHANNELS, WR_CHANNELS> dut_t;" % (dut.nm,dut.nm))
  ]]]*/
#include "memcpy_sched.h"

typedef multi_acc_template_Np<NUM_AUS, memcpy_sched<NUM_AUS>, memcpy_acc, Config, RD_CHANNELS, WR_CHANNELS> dut_t;
//[[[end]]] (checksum: a810fe1d315e268751e5f964942c0ebb)
#endif

#endif

