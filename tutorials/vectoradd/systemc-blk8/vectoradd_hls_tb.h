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
#include "vectoradd_hls.h"
//[[[end]]] (checksum: 734a28abf555b586bcf9359ace5dd4ae)
#include "kernel_mock_memory.h"

/*[[[cog
     cog.outl("class %s_hls_tb : public sc_module" % dut.nm)
  ]]]*/
class vectoradd_hls_tb : public sc_module
//[[[end]]] (checksum: 0670ea081df0319c4db54f2f8620eaea)
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
  ga::tlm_fifo_in<MemTypedReadReqType<Blk> > inaReqIn;
  ga::tlm_fifo_out<MemTypedReadRespType<Blk> > inaRespOut;

  ga::tlm_fifo_in<MemTypedReadReqType<Blk> > inbReqIn;
  ga::tlm_fifo_out<MemTypedReadRespType<Blk> > inbRespOut;

  ga::tlm_fifo_in<MemTypedWriteReqType<Blk> > outReqIn;
  ga::tlm_fifo_in<MemTypedWriteDataType<Blk> > outDataIn;

  //[[[end]]] (checksum: 3f7b732147b47ac6748dfea04b7fb570)
  sc_out<Config> config_out;

  Config config;

  /*[[[cog
       cog.outl("SC_HAS_PROCESS(%s_hls_tb);" % dut.nm)
    ]]]*/
  SC_HAS_PROCESS(vectoradd_hls_tb);
  //[[[end]]] (checksum: de58bc727025507f1d9a5d62f1cc10ff)

  //scide_waive SCIDE.8.15
  /*[[[cog
       cog.outl("%s_hls_tb(sc_module_name modname) :" % dut.nm)
    ]]]*/
  vectoradd_hls_tb(sc_module_name modname) :
  //[[[end]]] (checksum: 19841b5db980999931c3d98c704ebd1f)
    sc_module(modname)
  , clk("clk")
  , rst("rst"), done("done"), start("start")
  /*[[[cog
       for p in dut.inps:
         cog.outl(""", %s("%s"), %s("%s")""" % (p.reqNmT(),p.reqNmT(),p.respNmT(),p.respNmT()))
       for p in dut.outs:
         cog.outl(""", %s("%s"), %s("%s")""" % (p.reqNmT(),p.reqNmT(),p.dataNmT(),p.dataNmT()))
    ]]]*/
  , inaReqIn("inaReqIn"), inaRespOut("inaRespOut")
  , inbReqIn("inbReqIn"), inbRespOut("inbRespOut")
  , outReqIn("outReqIn"), outDataIn("outDataIn")
  //[[[end]]] (checksum: 6554fb0d25079ba7d3b72b1aca29442f)
  {
    SC_CTHREAD(init_proc, clk.pos());
    async_reset_signal_is(rst, false);

    /*[[[cog
         for p in dut.inps + dut.outs:
           cog.outl("SC_CTHREAD(%s_proc, clk.pos());" % (p.nm,))
           cog.outl("async_reset_signal_is(rst, false);")
           cog.outl("")
      ]]]*/
    SC_CTHREAD(ina_proc, clk.pos());
    async_reset_signal_is(rst, false);

    SC_CTHREAD(inb_proc, clk.pos());
    async_reset_signal_is(rst, false);

    SC_CTHREAD(out_proc, clk.pos());
    async_reset_signal_is(rst, false);

    //[[[end]]] (checksum: 63852458fbb8457718d6dd86e3ee7aa3)
    /*[[[cog
         for p in dut.inps:
           cog.outl("%s.clk_rst( clk, rst);" % (p.reqNmT(),))
           cog.outl("%s.clk_rst( clk, rst);" % (p.respNmT(),))
      ]]]*/
    inaReqIn.clk_rst( clk, rst);
    inaRespOut.clk_rst( clk, rst);
    inbReqIn.clk_rst( clk, rst);
    inbRespOut.clk_rst( clk, rst);
    //[[[end]]] (checksum: c4d53a2c3e58c360c1bee2fedb5e70cc)

    /*[[[cog
         for p in dut.outs:
           cog.outl("%s.clk_rst( clk, rst);" % (p.reqNmT(),))
           cog.outl("%s.clk_rst( clk, rst);" % (p.dataNmT(),))
      ]]]*/
    outReqIn.clk_rst( clk, rst);
    outDataIn.clk_rst( clk, rst);
    //[[[end]]] (checksum: 4310c5c17ae912afba1b245676ce7dee)

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
  void ina_proc() {
    MemTypedRead<Blk> mem( inaReqIn, inaRespOut);
    wait();
    while (1) {
      mem();
      wait();
    }
  }

  void inb_proc() {
    MemTypedRead<Blk> mem( inbReqIn, inbRespOut);
    wait();
    while (1) {
      mem();
      wait();
    }
  }

    //[[[end]]] (checksum: 00b75eb4d265a20795d979e0a2e9cbdd)
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
  void out_proc() {
    MemTypedWrite<Blk> mem( outReqIn, outDataIn);
    wait();
    while (1) {
      mem();
      wait();
    }
  }

  //[[[end]]] (checksum: 3a40b1a931669c5ee79c5ec54faa0c8f)
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
  vectoradd_hls_tb tb;
  vectoradd_hls kernel;
  //[[[end]]] (checksum: 088740b5f17d6af395f15285b014366c)
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
  ga::tlm_fifo<MemTypedReadReqType<Blk>,2> inaReq;
  ga::tlm_fifo<MemTypedReadRespType<Blk>,2> inaResp;

  ga::tlm_fifo<MemTypedReadReqType<Blk>,2> inbReq;
  ga::tlm_fifo<MemTypedReadRespType<Blk>,2> inbResp;

  //[[[end]]] (checksum: bcd2177d2e81cb6bfb103d46b2718890)
  // channel for memory write port
  /*[[[cog
       for p in dut.outs:
         cog.outl("ga::tlm_fifo<%s,2> %s;" % (p.reqTy(),p.reqNm()))
         cog.outl("ga::tlm_fifo<%s,2> %s;" % (p.dataTy(),p.dataNm()))
    ]]]*/
  ga::tlm_fifo<MemTypedWriteReqType<Blk>,2> outReq;
  ga::tlm_fifo<MemTypedWriteDataType<Blk>,2> outData;
  //[[[end]]] (checksum: 0eda87d91a1fc549f4a193ff7933772c)

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
    , inaReq("inaReq"), inaResp("inaResp")
    , inbReq("inbReq"), inbResp("inbResp")
    , outReq("outReq"), outData("outData")
    //[[[end]]] (checksum: 8852d584b082796b17cb4d147ff33bbc)
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
    tb.inaReqIn(inaReq);
    kernel.inaReqOut(inaReq);
    tb.inaRespOut(inaResp);
    kernel.inaRespIn(inaResp);

    tb.inbReqIn(inbReq);
    kernel.inbReqOut(inbReq);
    tb.inbRespOut(inbResp);
    kernel.inbRespIn(inbResp);

    //[[[end]]] (checksum: 61c8412cc4379b1078524191c4160718)
    /*[[[cog
         for p in dut.outs:
           cog.outl("tb.%s(%s);" % (p.reqNmT(),p.reqNm()))
           cog.outl("kernel.%s(%s);" % (p.reqNmK(),p.reqNm()))
           cog.outl("kernel.%s(%s);" % (p.dataNmK(),p.dataNm()))
           cog.outl("tb.%s(%s);" % (p.dataNmT(),p.dataNm()))
           cog.outl("")
      ]]]*/
    tb.outReqIn(outReq);
    kernel.outReqOut(outReq);
    kernel.outDataOut(outData);
    tb.outDataIn(outData);

    //[[[end]]] (checksum: 0947e1f83e5fe1da1c5264f69bf702b6)
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
