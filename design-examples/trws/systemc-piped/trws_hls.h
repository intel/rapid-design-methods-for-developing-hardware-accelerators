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
#ifndef __TRWS_HLS_H__
#define __TRWS_HLS_H__
//[[[end]]] (checksum: cd7588b84268da2ba399d688f5c45ebe)

#ifndef __SYNTHESIS__
#include <type_traits>
#endif

#include "systemc.h"

#include "types.h"

#ifndef __gi_Slots__
#define __gi_Slots__ 4
#endif

#ifndef __wi_Slots__
#define __wi_Slots__ 32
#endif

#ifndef __mi_Slots__
#define __mi_Slots__ 128
#endif

#ifndef __inp_Slots__
#define __inp_Slots__ 64
#endif

#ifndef __inp_Slots2__
#define __inp_Slots2__ 64
#endif

#include "Config.h"

#include "ga_tlm_fifo.h"

#include "hls_utils.h"

class SliceCounter {
public:

  unsigned int jj;
  unsigned int s;
  unsigned int t;
  unsigned int k;
  bool done; 
  bool first_t;

 SliceCounter() : jj(0), s(1), t(0), k(0), done(false), first_t(true) {}
    

  unsigned int computeK( unsigned int n) {
    return t*n+s-(t+1)*(t+2)/2;
  }

  friend std::ostream& operator<<( std::ostream& os, const SliceCounter& sc) {
      os << "(" << sc.s << "," << sc.t << "," << sc.k << "," << sc.done << "," << sc.jj << ")";
      return os;
  }

  void incr( const unsigned int nCLperRow, const unsigned int n) {
    assert( k == computeK( n));
    if ( jj != nCLperRow-1) {
      ++jj;
    } else {
      jj = 0;
      if ( s != n-1) {
        ++s; ++k;
        first_t = false;
      } else {
        s = t+1;
        if ( t != n-2) { // The last pair is s,t = n-1,n-2
          ++t; ++k;
          s = t+1;
        } else {
          k = 0;
          t = 0; s = 1; // first pair is s,t = 1,0
          done = true;

        }
        first_t = true;
      }
    }
    assert( k == computeK( n));
  }
};

/*[[[cog
     for m in dut.modules.values():
       cog.outl("#include \"%s.h\"" % m.nm)
  ]]]*/
#include "trws_wu.h"
#include "trws_loaf.h"
#include "trws_clamper.h"
//[[[end]]] (checksum: 972ddf7e7db7aef92bf0ed854822e545)

/*[[[cog
     cog.outl("class %s_hls : public sc_module" % dut.nm)
  ]]]*/
class trws_hls : public sc_module
//[[[end]]] (checksum: ef34da973ef4b9e223aa53213fed552b)
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
  ga::tlm_fifo_hier_out<MemTypedReadReqType<UCacheLine> > giReqOut;
  ga::tlm_fifo_hier_in<MemTypedReadRespType<UCacheLine> > giRespIn;

  ga::tlm_fifo_hier_out<MemTypedReadReqType<CacheLine> > wiReqOut;
  ga::tlm_fifo_hier_in<MemTypedReadRespType<CacheLine> > wiRespIn;

  ga::tlm_fifo_hier_out<MemTypedReadReqType<UCacheLine> > miReqOut;
  ga::tlm_fifo_hier_in<MemTypedReadRespType<UCacheLine> > miRespIn;

  ga::tlm_fifo_hier_out<MemTypedReadReqType<Pair> > inpReqOut;
  ga::tlm_fifo_hier_in<MemTypedReadRespType<Pair> > inpRespIn;

  ga::tlm_fifo_hier_out<MemTypedWriteReqType<UCacheLine> > moReqOut;
  ga::tlm_fifo_hier_out<MemTypedWriteDataType<UCacheLine> > moDataOut;

  //[[[end]]] (checksum: 3ac35e39ea83e9c4b6a5a0c2d5277fb1)
  // Instantiate modules
  /*[[[cog
       for m in dut.modules.values():
         cog.outl("%s %s_inst;" % (m.nm, m.nm))
    ]]]*/
  trws_wu trws_wu_inst;
  trws_loaf trws_loaf_inst;
  trws_clamper trws_clamper_inst;
  //[[[end]]] (checksum: c9a405fd1625c559408216678d548c89)

  // TLM fifos (between modules)
  /*[[[cog
       for f in dut.tlm_fifos:
         cog.outl("ga::tlm_fifo<%s, %d> %s;" % (f.ty, f.capacity, f.nm))
    ]]]*/
  ga::tlm_fifo<CacheLine, 2> off;
  ga::tlm_fifo<CacheLine, 2> out;
  ga::tlm_fifo<UCacheLine, 2> res;
  //[[[end]]] (checksum: b134e8735d5d8a6266cd20e2d07fcea5)

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
  SC_HAS_PROCESS(trws_hls);
  //[[[end]]] (checksum: 6a5a270fd4e203a78cc10b4dce16399b)

  /*[[[cog
       cog.outl("%s_hls(sc_module_name modname) :" % (dut.nm,))
    ]]]*/
  trws_hls(sc_module_name modname) :
  //[[[end]]] (checksum: 18c3d250b1374c8fc01b2fa5d07f7daa)
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
       , giReqOut("giReqOut")
       , giRespIn("giRespIn")
       , wiReqOut("wiReqOut")
       , wiRespIn("wiRespIn")
       , miReqOut("miReqOut")
       , miRespIn("miRespIn")
       , inpReqOut("inpReqOut")
       , inpRespIn("inpRespIn")
       , moReqOut("moReqOut")
       , moDataOut("moDataOut")
       , trws_wu_inst("trws_wu_inst")
       , trws_loaf_inst("trws_loaf_inst")
       , trws_clamper_inst("trws_clamper_inst")
       //[[[end]]] (checksum: 9ceb598aed4a32f07d08dec8808981a9)
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
               if m.portOf( EnqueuePort(f.nm)) or m.portOf( DequeuePort(f.nm)):
                 cog.outl("{0}_inst.{1}({1});".format( m.nm, f.nm))
      ]]]*/
    trws_wu_inst.clk(clk);
    trws_wu_inst.rst(rst);
    trws_wu_inst.start(start);
    trws_wu_inst.config(config);
    trws_wu_inst.done(done);
    trws_wu_inst.giReqOut(giReqOut);
    trws_wu_inst.giRespIn(giRespIn);
    trws_wu_inst.wiReqOut(wiReqOut);
    trws_wu_inst.wiRespIn(wiRespIn);
    trws_wu_inst.miReqOut(miReqOut);
    trws_wu_inst.miRespIn(miRespIn);
    trws_wu_inst.moReqOut(moReqOut);
    trws_wu_inst.moDataOut(moDataOut);
    trws_wu_inst.off(off);
    trws_wu_inst.res(res);
    trws_loaf_inst.clk(clk);
    trws_loaf_inst.rst(rst);
    trws_loaf_inst.start(start);
    trws_loaf_inst.config(config);
    trws_loaf_inst.inpReqOut(inpReqOut);
    trws_loaf_inst.inpRespIn(inpRespIn);
    trws_loaf_inst.off(off);
    trws_loaf_inst.out(out);
    trws_clamper_inst.clk(clk);
    trws_clamper_inst.rst(rst);
    trws_clamper_inst.start(start);
    trws_clamper_inst.config(config);
    trws_clamper_inst.out(out);
    trws_clamper_inst.res(res);
    //[[[end]]] (checksum: 754e2484ab90d9b85879ba5f8fad3269)

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
#include "trws_acc.h"
//[[[end]]] (checksum: 16015279a30ec1dad1fd4cf535814787)

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
typedef trws_acc dut_t;
//[[[end]]] (checksum: 46352af897df010cf16e2f1c82f3eebb)
#else
/*[[[cog
     cog.outl("#include \"%s_sched.h\"" % dut.nm)
     cog.outl("")
     cog.outl("typedef multi_acc_template_Np<NUM_AUS, %s_sched<NUM_AUS>, %s_acc, Config, RD_CHANNELS, WR_CHANNELS> dut_t;" % (dut.nm,dut.nm))
  ]]]*/
#include "trws_sched.h"

typedef multi_acc_template_Np<NUM_AUS, trws_sched<NUM_AUS>, trws_acc, Config, RD_CHANNELS, WR_CHANNELS> dut_t;
//[[[end]]] (checksum: caca68b701ab7b84e4444cfc43c91367)
#endif

#endif

