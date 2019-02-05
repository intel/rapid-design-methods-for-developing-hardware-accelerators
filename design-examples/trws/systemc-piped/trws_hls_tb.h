// See LICENSE for license details.
#ifndef KERNEL_TB_H_
#define KERNEL_TB_H_

/*[[[cog
     import cog
     from cog_acctempl import *
     from dut_params import *
  ]]]*/
//[[[end]]] (checksum: d41d8cd98f00b204e9800998ecf8427e)

#include "systemc.h"
#include "clock_generator.h"
#include "ga_tlm_fifo.h"
/*[[[cog
     cog.outl("#include \"%s_hls.h\"" % dut.nm)
  ]]]*/
#include "trws_hls.h"
//[[[end]]] (checksum: 0a18d1a559f143ae813be1e4b9dae3be)
#include "kernel_mock_memory.h"

/*[[[cog
     cog.outl("class %s_hls_tb : public sc_module" % dut.nm)
  ]]]*/
class trws_hls_tb : public sc_module
//[[[end]]] (checksum: 06da07e79bc9fad2bfbacf15935bcda1)
{
public:
  sc_in_clk clk;
  sc_in<bool> rst;
  sc_in<bool> done;
  sc_out<bool> start;

  // functional IOs

  /*[[[cog
       for p in dut.inps:
         cog.outl("ga::tlm_fifo_in<%s > %s;" % (p.reqTy(), p.reqNmT()))
         cog.outl("ga::tlm_fifo_out<%s > %s;" % (p.respTy(), p.respNmT()))
         cog.outl("")
       for p in dut.outs:
         cog.outl("ga::tlm_fifo_in<%s > %s;" % (p.reqTy(), p.reqNmT()))
         cog.outl("ga::tlm_fifo_in<%s > %s;" % (p.dataTy(), p.dataNmT()))
         cog.outl("")
    ]]]*/
  ga::tlm_fifo_in<MemTypedReadReqType<UCacheLine> > giReqIn;
  ga::tlm_fifo_out<MemTypedReadRespType<UCacheLine> > giRespOut;

  ga::tlm_fifo_in<MemTypedReadReqType<CacheLine> > wiReqIn;
  ga::tlm_fifo_out<MemTypedReadRespType<CacheLine> > wiRespOut;

  ga::tlm_fifo_in<MemTypedReadReqType<UCacheLine> > miReqIn;
  ga::tlm_fifo_out<MemTypedReadRespType<UCacheLine> > miRespOut;

  ga::tlm_fifo_in<MemTypedReadReqType<Pair> > inpReqIn;
  ga::tlm_fifo_out<MemTypedReadRespType<Pair> > inpRespOut;

  ga::tlm_fifo_in<MemTypedWriteReqType<UCacheLine> > moReqIn;
  ga::tlm_fifo_in<MemTypedWriteDataType<UCacheLine> > moDataIn;

  //[[[end]]] (checksum: 3ae5abdfab321006b9a525f805e3e092)
  sc_out<Config> config_out;

  Config config;

  /*[[[cog
       cog.outl("SC_HAS_PROCESS(%s_hls_tb);" % dut.nm)
    ]]]*/
  SC_HAS_PROCESS(trws_hls_tb);
  //[[[end]]] (checksum: 2880ceff530f9462666b0cc79808b6fb)

  //scide_waive SCIDE.8.15
  /*[[[cog
       cog.outl("%s_hls_tb(sc_module_name modname) :" % dut.nm)
    ]]]*/
  trws_hls_tb(sc_module_name modname) :
  //[[[end]]] (checksum: 3c54f94eb23870d1844f38bb5fb92f64)
    sc_module(modname)
  , clk("clk")
  , rst("rst"), done("done"), start("start")
  /*[[[cog
       for p in dut.inps:
         cog.outl(""", %s("%s"), %s("%s")""" % (p.reqNmT(),p.reqNmT(),p.respNmT(),p.respNmT()))
       for p in dut.outs:
         cog.outl(""", %s("%s"), %s("%s")""" % (p.reqNmT(),p.reqNmT(),p.dataNmT(),p.dataNmT()))
    ]]]*/
  , giReqIn("giReqIn"), giRespOut("giRespOut")
  , wiReqIn("wiReqIn"), wiRespOut("wiRespOut")
  , miReqIn("miReqIn"), miRespOut("miRespOut")
  , inpReqIn("inpReqIn"), inpRespOut("inpRespOut")
  , moReqIn("moReqIn"), moDataIn("moDataIn")
  //[[[end]]] (checksum: ae97a7192d01b4722b7d52c1b8c9b7f3)
  {
    SC_CTHREAD(init_proc, clk.pos());
    async_reset_signal_is(rst, false);

    /*[[[cog
         for p in dut.inps + dut.outs:
           cog.outl("SC_CTHREAD(%s_proc, clk.pos());" % (p.nm,))
           cog.outl("async_reset_signal_is(rst, false);")
           cog.outl("")
      ]]]*/
    SC_CTHREAD(gi_proc, clk.pos());
    async_reset_signal_is(rst, false);

    SC_CTHREAD(wi_proc, clk.pos());
    async_reset_signal_is(rst, false);

    SC_CTHREAD(mi_proc, clk.pos());
    async_reset_signal_is(rst, false);

    SC_CTHREAD(inp_proc, clk.pos());
    async_reset_signal_is(rst, false);

    SC_CTHREAD(mo_proc, clk.pos());
    async_reset_signal_is(rst, false);

    //[[[end]]] (checksum: 0469f9bb5f0be05fdf3b1fa5f4444851)
    /*[[[cog
         for p in dut.inps:
           cog.outl("%s.clk_rst( clk, rst);" % (p.reqNmT(),))
           cog.outl("%s.clk_rst( clk, rst);" % (p.respNmT(),))
      ]]]*/
    giReqIn.clk_rst( clk, rst);
    giRespOut.clk_rst( clk, rst);
    wiReqIn.clk_rst( clk, rst);
    wiRespOut.clk_rst( clk, rst);
    miReqIn.clk_rst( clk, rst);
    miRespOut.clk_rst( clk, rst);
    inpReqIn.clk_rst( clk, rst);
    inpRespOut.clk_rst( clk, rst);
    //[[[end]]] (checksum: fc6b034be03b45c4ed2eb37983deee42)

    /*[[[cog
         for p in dut.outs:
           cog.outl("%s.clk_rst( clk, rst);" % (p.reqNmT(),))
           cog.outl("%s.clk_rst( clk, rst);" % (p.dataNmT(),))
      ]]]*/
    moReqIn.clk_rst( clk, rst);
    moDataIn.clk_rst( clk, rst);
    //[[[end]]] (checksum: 3ab12568baeba0b4fcc78b9ba211d5f7)

  }

  /*[[[cog
       for p in dut.inps:
         if type(p) is not TypedRead: continue
         cog.outl("void %s_proc() {" % (p.nm,))
         cog.outl("  MemTypedRead<%s> mem( %s, %s);" % (p.ty, p.reqNmT(), p.respNmT()))
         cog.outl("  wait();")
         cog.outl("  while (1) {")
         cog.outl("    mem();")
         cog.outl("    wait();")
         cog.outl("  }")
         cog.outl("}")
         cog.outl("")
    ]]]*/
  void gi_proc() {
    MemTypedRead<UCacheLine> mem( giReqIn, giRespOut);
    wait();
    while (1) {
      mem();
      wait();
    }
  }

  void wi_proc() {
    MemTypedRead<CacheLine> mem( wiReqIn, wiRespOut);
    wait();
    while (1) {
      mem();
      wait();
    }
  }

  void mi_proc() {
    MemTypedRead<UCacheLine> mem( miReqIn, miRespOut);
    wait();
    while (1) {
      mem();
      wait();
    }
  }

  void inp_proc() {
    MemTypedRead<Pair> mem( inpReqIn, inpRespOut);
    wait();
    while (1) {
      mem();
      wait();
    }
  }

    //[[[end]]] (checksum: efe43e77d7c1edad14f2e743c90f350e)
  /*[[[cog
       for p in dut.inps:
         if type(p) is not SingleRead: continue
         cog.outl("void %s_proc() {" % (p.nm,))
         cog.outl("  MemSingleRead<%s,%s> mem( %s, %s);" % (p.ty, p.tag_ty, p.reqNmT(), p.respNmT()))
         cog.outl("  wait();")
         cog.outl("  while (1) {")
         cog.outl("    mem();")
         cog.outl("    wait();")
         cog.outl("  }")
         cog.outl("}")
         cog.outl("")
    ]]]*/
  //[[[end]]] (checksum: d41d8cd98f00b204e9800998ecf8427e)
  /*[[[cog
       for p in dut.outs:
         cog.outl("void %s_proc() {" % (p.nm,))
         cog.outl("  MemTypedWrite<%s> mem( %s, %s);" % (p.ty, p.reqNmT(), p.dataNmT()))
         cog.outl("  wait();")
         cog.outl("  while (1) {")
         cog.outl("    mem();")
         cog.outl("    wait();")
         cog.outl("  }")
         cog.outl("}")
         cog.outl("")
    ]]]*/
  void mo_proc() {
    MemTypedWrite<UCacheLine> mem( moReqIn, moDataIn);
    wait();
    while (1) {
      mem();
      wait();
    }
  }

  //[[[end]]] (checksum: 30ba402985f801480dc958124d0a95c1)
  void init_proc() {
    start = false;
    size_t wait_cycles_after_last_input = 0;
    config_out.write(config);
    wait();
    while (1) {
        start = true;
        if (done) {
          wait_cycles_after_last_input++;
          if (wait_cycles_after_last_input > 1000) {
            sc_pause();
          }
        }
        wait();
      }
    }
};


#include "fpga_app_sw.h"
class AcclApp: public sc_module, public FpgaAppSwAlloc {
public:

  ClockGenerator clkgen;
  /*[[[cog
       cog.outl("%s_hls_tb tb;" % dut.nm)
       cog.outl("%s_hls kernel;" % dut.nm)
    ]]]*/
  trws_hls_tb tb;
  trws_hls kernel;
  //[[[end]]] (checksum: 80f7d796f8e85ca12393d1e9124165cb)
  sc_signal<bool> clk;
  sc_signal<bool> rst;
  sc_signal<Config> config_ch;
  sc_signal<bool> start_ch;
  sc_signal<bool> done_ch;

  // channel for memory read port
  /*[[[cog
       for p in dut.inps:
         cog.outl("ga::tlm_fifo<%s,2> %s;" % (p.reqTy(),p.reqNm()))
         cog.outl("ga::tlm_fifo<%s,2> %s;" % (p.respTy(),p.respNm()))
         cog.outl("")
    ]]]*/
  ga::tlm_fifo<MemTypedReadReqType<UCacheLine>,2> giReq;
  ga::tlm_fifo<MemTypedReadRespType<UCacheLine>,2> giResp;

  ga::tlm_fifo<MemTypedReadReqType<CacheLine>,2> wiReq;
  ga::tlm_fifo<MemTypedReadRespType<CacheLine>,2> wiResp;

  ga::tlm_fifo<MemTypedReadReqType<UCacheLine>,2> miReq;
  ga::tlm_fifo<MemTypedReadRespType<UCacheLine>,2> miResp;

  ga::tlm_fifo<MemTypedReadReqType<Pair>,2> inpReq;
  ga::tlm_fifo<MemTypedReadRespType<Pair>,2> inpResp;

  //[[[end]]] (checksum: 6ce6226a8c9d9a19db370f115b49f25f)
  // channel for memory write port
  /*[[[cog
       for p in dut.outs:
         cog.outl("ga::tlm_fifo<%s,2> %s;" % (p.reqTy(),p.reqNm()))
         cog.outl("ga::tlm_fifo<%s,2> %s;" % (p.dataTy(),p.dataNm()))
    ]]]*/
  ga::tlm_fifo<MemTypedWriteReqType<UCacheLine>,2> moReq;
  ga::tlm_fifo<MemTypedWriteDataType<UCacheLine>,2> moData;
  //[[[end]]] (checksum: 0bc8d2ea339a5a72f17d90339b17d5e4)

  SC_HAS_PROCESS(AcclApp);

  AcclApp(sc_module_name name=sc_core::sc_gen_unique_name("AcclApp")) :
    clkgen("clkgen"), tb("tb"),
    kernel("kernel"),
    clk("clk"), rst("rst"),
    start_ch("start_ch"), done_ch("done_ch")
    /*[[[cog
         for p in dut.inps:
           cog.outl(""", %s("%s"), %s("%s")""" % (p.reqNm(),p.reqNm(),p.respNm(),p.respNm()))
         for p in dut.outs:
           cog.outl(""", %s("%s"), %s("%s")""" % (p.reqNm(),p.reqNm(),p.dataNm(),p.dataNm()))
      ]]]*/
    , giReq("giReq"), giResp("giResp")
    , wiReq("wiReq"), wiResp("wiResp")
    , miReq("miReq"), miResp("miResp")
    , inpReq("inpReq"), inpResp("inpResp")
    , moReq("moReq"), moData("moData")
    //[[[end]]] (checksum: 955f0ca4ebe3deea17fca9bfacdc5316)
  {
    tb.clk(clk);
    clkgen.clk(clk);
    tb.rst(rst);
    clkgen.rst(rst);

    kernel.clk(clk);
    kernel.rst(rst);

    tb.start(start_ch);
    kernel.start(start_ch);

    tb.done(done_ch);
    kernel.done(done_ch);

    tb.config_out(config_ch);
    kernel.config(config_ch);

    /*[[[cog
         for p in dut.inps:
           cog.outl("tb.%s(%s);" % (p.reqNmT(),p.reqNm()))
           cog.outl("kernel.%s(%s);" % (p.reqNmK(),p.reqNm()))
           cog.outl("tb.%s(%s);" % (p.respNmT(),p.respNm()))
           cog.outl("kernel.%s(%s);" % (p.respNmK(),p.respNm()))
           cog.outl("")
      ]]]*/
    tb.giReqIn(giReq);
    kernel.giReqOut(giReq);
    tb.giRespOut(giResp);
    kernel.giRespIn(giResp);

    tb.wiReqIn(wiReq);
    kernel.wiReqOut(wiReq);
    tb.wiRespOut(wiResp);
    kernel.wiRespIn(wiResp);

    tb.miReqIn(miReq);
    kernel.miReqOut(miReq);
    tb.miRespOut(miResp);
    kernel.miRespIn(miResp);

    tb.inpReqIn(inpReq);
    kernel.inpReqOut(inpReq);
    tb.inpRespOut(inpResp);
    kernel.inpRespIn(inpResp);

    //[[[end]]] (checksum: 60d2fed409f8df5b541dd835fccd16e3)
    /*[[[cog
         for p in dut.outs:
           cog.outl("tb.%s(%s);" % (p.reqNmT(),p.reqNm()))
           cog.outl("kernel.%s(%s);" % (p.reqNmK(),p.reqNm()))
           cog.outl("kernel.%s(%s);" % (p.dataNmK(),p.dataNm()))
           cog.outl("tb.%s(%s);" % (p.dataNmT(),p.dataNm()))
           cog.outl("")
      ]]]*/
    tb.moReqIn(moReq);
    kernel.moReqOut(moReq);
    kernel.moDataOut(moData);
    tb.moDataIn(moData);

    //[[[end]]] (checksum: 1dd563d88570be973a110e7a7e6bd227)
  }

  virtual void  compute( const void * config_ptr,
                         const unsigned int config_size) {
    tb.config = *static_cast<const Config *>(config_ptr);
    clkgen.reset_state = true;
    sc_start();
    //reset done
    cout << "HW compute.." << endl;
    sc_start();
  }

  virtual void join() {}

};
#endif /* KERNEL_TB_H_ */
