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
#ifndef __LINKEDLIST_HLS_H__
#define __LINKEDLIST_HLS_H__
//[[[end]]] (checksum: 931e9ca5b2c8de4658bf09dc8e5ac634)

#ifndef __SYNTHESIS__
#include <type_traits>
#endif

#include "systemc.h"

#include "types.h"

#ifndef __inp_Slots__
#define __inp_Slots__ 128
#endif

#ifndef __lst_Slots__
#define __lst_Slots__ 128
#endif

#include "Config.h"

class State {
public:
  HeadPtr head;
  HeadPtr fast;
  HeadPtr slow;
  unsigned char state;
  State() {
    head.head = fast.head = slow.head = state = 0;
  }
  friend std::ostream& operator<<( std::ostream& os, const State& s) {
    return os << "State(" << s.head.head << "," << s.fast.head << "," << s.slow.head << "," << s.state;
  }

  bool operator==( const State& s) const {
    return head == s.head && fast == s.fast && slow == s.slow && state == s.state;
  }

};

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
class linkedlist_hls : public sc_module
//[[[end]]] (checksum: fd997b65bbca8f8df62f08993c29ec32)
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
  ga::tlm_fifo_out<MemSingleReadReqType<Node,State> > lstReqOut;
  ga::tlm_fifo_in<MemSingleReadRespType<Node,State> > lstRespIn;

  ga::tlm_fifo_out<MemTypedReadReqType<HeadPtr> > inpReqOut;
  ga::tlm_fifo_in<MemTypedReadRespType<HeadPtr> > inpRespIn;

  ga::tlm_fifo_out<MemTypedWriteReqType<HeadPtr> > outReqOut;
  ga::tlm_fifo_out<MemTypedWriteDataType<HeadPtr> > outDataOut;

  //[[[end]]] (checksum: 027573f0e465c58afe6415ef574e66f5)
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
  SC_HAS_PROCESS(linkedlist_hls);
  //[[[end]]] (checksum: 43571c7484ebf61db33a6f0b6e96f014)

  /*[[[cog
       cog.outl("%s_hls(sc_module_name modname) :" % (dut.nm,))
    ]]]*/
  linkedlist_hls(sc_module_name modname) :
  //[[[end]]] (checksum: 3b218ead9ed2443388c100aca6f5f903)
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
       , lstReqOut("lstReqOut")
       , lstRespIn("lstRespIn")
       , inpReqOut("inpReqOut")
       , inpRespIn("inpRespIn")
       , outReqOut("outReqOut")
       , outDataOut("outDataOut")
       //[[[end]]] (checksum: bfde632338beadf7a63468848c3c4e24)
  {
    /*[[[cog
         for c in dut.cthreads.values():
           cog.outl("SC_CTHREAD(%s, clk.pos());" % (c.nm,))
           cog.outl("async_reset_signal_is(rst, false);")
           cog.outl("")
      ]]]*/
    SC_CTHREAD(fetcher, clk.pos());
    async_reset_signal_is(rst, false);

    SC_CTHREAD(inp_addr_gen, clk.pos());
    async_reset_signal_is(rst, false);

    SC_CTHREAD(out_addr_gen, clk.pos());
    async_reset_signal_is(rst, false);

    //[[[end]]] (checksum: cc1796c05bc15d135efd5ed10a2724d8)
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
    lstReqOut.clk_rst(clk, rst);
    lstRespIn.clk_rst(clk, rst);
    inpReqOut.clk_rst(clk, rst);
    inpRespIn.clk_rst(clk, rst);
    outReqOut.clk_rst(clk, rst);
    outDataOut.clk_rst(clk, rst);
    //[[[end]]] (checksum: 4ada1e3077f0ab1e3cb6f414a7a95ad1)

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
#include "linkedlist-fetcher.h"
#include "linkedlist-inp_addr_gen.h"
#include "linkedlist-out_addr_gen.h"
//[[[end]]] (checksum: 1d6b01b12c8c9665312958976e84aec2)

};

/*[[[cog
     cog.outl("#include \"%s_acc.h\"" % (dut.nm,))
  ]]]*/
#include "linkedlist_acc.h"
//[[[end]]] (checksum: 5af65e7d3ac8202d2a2557d93ffe5565)

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
typedef linkedlist_acc dut_t;
//[[[end]]] (checksum: 94eb21fb88262a650b9722ff5e2990e9)
#else
/*[[[cog
     cog.outl("#include \"%s_sched.h\"" % dut.nm)
     cog.outl("")
     cog.outl("typedef multi_acc_template_Np<NUM_AUS, %s_sched<NUM_AUS>, %s_acc, Config, RD_CHANNELS, WR_CHANNELS> dut_t;" % (dut.nm,dut.nm))
  ]]]*/
#include "linkedlist_sched.h"

typedef multi_acc_template_Np<NUM_AUS, linkedlist_sched<NUM_AUS>, linkedlist_acc, Config, RD_CHANNELS, WR_CHANNELS> dut_t;
//[[[end]]] (checksum: a18a194fca913b068202ef522b19b733)
#endif

#endif

