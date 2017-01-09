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
#ifndef __SOBEL_HLS_H__
#define __SOBEL_HLS_H__
//[[[end]]] (checksum: 8fe8e4a565da4ddfe21fc1c7e1baff0b)

#ifndef __SYNTHESIS__
#include <type_traits>
#endif

#include <limits>

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
#include "sobel_frontend.h"
#include "sobel_backend.h"
#include "sobel_restrict.h"
//[[[end]]] (checksum: 2fe5745996b3630fce1e2833407306fa)

/*[[[cog
     cog.outl("class %s_hls : public sc_module" % dut.nm)
  ]]]*/
class sobel_hls : public sc_module
//[[[end]]] (checksum: 610ef54ead36409088d6a8e7fea1880b)
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
  ga::tlm_fifo_out<MemTypedReadReqType<BlkInp> > inpReqOut;
  ga::tlm_fifo_hier_in<MemTypedReadRespType<BlkInp> > inpRespIn;

  ga::tlm_fifo_out<MemTypedWriteReqType<BlkOut> > outReqOut;
  ga::tlm_fifo_hier_out<MemTypedWriteDataType<BlkOut> > outDataOut;

  //[[[end]]] (checksum: ddca4b0c3c4f43ee877afdd5d5e21402)
  // Instantiate modules
  /*[[[cog
       for m in dut.modules.values():
         cog.outl("%s %s_inst;" % (m.nm, m.nm))
    ]]]*/
  sobel_frontend sobel_frontend_inst;
  sobel_backend sobel_backend_inst;
  sobel_restrict sobel_restrict_inst;
  //[[[end]]] (checksum: b498a5537bd9dfdbde478060642c3d4c)

  // TLM fifos (between modules)
  /*[[[cog
       for f in dut.tlm_fifos:
         cog.outl("ga::tlm_fifo<%s, %d> %s;" % (f.ty, f.capacity, f.nm))
    ]]]*/
  ga::tlm_fifo<BlkMid, 2> mid0;
  ga::tlm_fifo<BlkMid, 2> mid1;
  //[[[end]]] (checksum: 41fd2c5a1610f41e896614f508aae378)

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
  SC_HAS_PROCESS(sobel_hls);
  //[[[end]]] (checksum: c30e5e1f40476e8f9f91f7630bd825c7)

  /*[[[cog
       cog.outl("%s_hls(sc_module_name modname) :" % (dut.nm,))
    ]]]*/
  sobel_hls(sc_module_name modname) :
  //[[[end]]] (checksum: b43b02d97db929fb425e02344ef96541)
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
       , sobel_frontend_inst("sobel_frontend_inst")
       , sobel_backend_inst("sobel_backend_inst")
       , sobel_restrict_inst("sobel_restrict_inst")
       //[[[end]]] (checksum: 6ee056c1d968012f061e8265bcac9391)
  {
    /*[[[cog
         for c in dut.cthreads.values():
           cog.outl("SC_CTHREAD(%s, clk.pos());" % (c.nm,))
           cog.outl("async_reset_signal_is(rst, false);")
           cog.outl("")
      ]]]*/
    SC_CTHREAD(out_addr_gen, clk.pos());
    async_reset_signal_is(rst, false);

    SC_CTHREAD(inp_addr_gen, clk.pos());
    async_reset_signal_is(rst, false);

    //[[[end]]] (checksum: 0aed583cd633cbae002b286d2644e73b)
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
    outReqOut.clk_rst(clk, rst);
    //[[[end]]] (checksum: 236c4ebdafb50ae11d8df53ae95de399)

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
    sobel_frontend_inst.clk(clk);
    sobel_frontend_inst.rst(rst);
    sobel_frontend_inst.start(start);
    sobel_frontend_inst.config(config);
    sobel_frontend_inst.inpRespIn(inpRespIn);
    sobel_frontend_inst.mid0(mid0);
    sobel_backend_inst.clk(clk);
    sobel_backend_inst.rst(rst);
    sobel_backend_inst.start(start);
    sobel_backend_inst.config(config);
    sobel_backend_inst.mid0(mid0);
    sobel_backend_inst.mid1(mid1);
    sobel_restrict_inst.clk(clk);
    sobel_restrict_inst.rst(rst);
    sobel_restrict_inst.start(start);
    sobel_restrict_inst.config(config);
    sobel_restrict_inst.done(done);
    sobel_restrict_inst.outDataOut(outDataOut);
    sobel_restrict_inst.mid1(mid1);
    //[[[end]]] (checksum: ce9f46078d9b623d1d2542bf0ce1d435)

  }

/*[[[cog
     for c in dut.cthreads.values():
       cog.outl("#include \"%s-%s.h\"" % (dut.nm,c.nm))
  ]]]*/
#include "sobel-out_addr_gen.h"
#include "sobel-inp_addr_gen.h"
//[[[end]]] (checksum: 9f07a2053976e67d682ebdb0dbb957b5)

};

/*[[[cog
     cog.outl("#include \"%s_acc.h\"" % (dut.nm,))
  ]]]*/
#include "sobel_acc.h"
//[[[end]]] (checksum: 36a8f386a8f71e8bb83924bc3c439c54)

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
typedef sobel_acc dut_t;
//[[[end]]] (checksum: ffaecdbe07e8fd348e2809fe92e2862c)
#else
/*[[[cog
     cog.outl("#include \"%s_sched.h\"" % dut.nm)
     cog.outl("")
     cog.outl("typedef multi_acc_template_Np<NUM_AUS, %s_sched<NUM_AUS>, %s_acc, Config, RD_CHANNELS, WR_CHANNELS> dut_t;" % (dut.nm,dut.nm))
  ]]]*/
#include "sobel_sched.h"

typedef multi_acc_template_Np<NUM_AUS, sobel_sched<NUM_AUS>, sobel_acc, Config, RD_CHANNELS, WR_CHANNELS> dut_t;
//[[[end]]] (checksum: 8785390f944885a5be200b95679f5168)
#endif

#endif

