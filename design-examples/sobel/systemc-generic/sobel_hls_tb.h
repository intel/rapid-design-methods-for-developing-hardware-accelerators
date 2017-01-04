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
#include "sobel_hls.h"
//[[[end]]] (checksum: 62f77302f6b837ce4a974bb3e2f0115d)
#include "kernel_mock_memory.h"

/*[[[cog
     cog.outl("class %s_hls_tb : public sc_module" % dut.nm)
  ]]]*/
class sobel_hls_tb : public sc_module
//[[[end]]] (checksum: 9e6cb95f49d13e89aa2607786b521158)
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
  ga::tlm_fifo_in<MemTypedReadReqType<BlkInp> > inpReqIn;
  ga::tlm_fifo_out<MemTypedReadRespType<BlkInp> > inpRespOut;

  ga::tlm_fifo_in<MemTypedWriteReqType<BlkOut> > outReqIn;
  ga::tlm_fifo_in<MemTypedWriteDataType<BlkOut> > outDataIn;

  //[[[end]]] (checksum: 8bba9ba45e79ca9cf96e0195ba4e01b2)
  sc_out<Config> config_out;

  Config config;

  /*[[[cog
       cog.outl("SC_HAS_PROCESS(%s_hls_tb);" % dut.nm)
    ]]]*/
  SC_HAS_PROCESS(sobel_hls_tb);
  //[[[end]]] (checksum: c437f8cf4ffdf8ac5bf0e66f5411efbc)

  //scide_waive SCIDE.8.15
  /*[[[cog
       cog.outl("%s_hls_tb(sc_module_name modname) :" % dut.nm)
    ]]]*/
  sobel_hls_tb(sc_module_name modname) :
  //[[[end]]] (checksum: f3e42998be0120185c727038bb803a3e)
    sc_module(modname)
  , clk("clk")
  , rst("rst"), done("done"), start("start")
  /*[[[cog
       for p in dut.inps:
         cog.outl(""", %s("%s"), %s("%s")""" % (p.reqNmT(),p.reqNmT(),p.respNmT(),p.respNmT()))
       for p in dut.outs:
         cog.outl(""", %s("%s"), %s("%s")""" % (p.reqNmT(),p.reqNmT(),p.dataNmT(),p.dataNmT()))
    ]]]*/
  , inpReqIn("inpReqIn"), inpRespOut("inpRespOut")
  , outReqIn("outReqIn"), outDataIn("outDataIn")
  //[[[end]]] (checksum: 598b4e54c1f71c81bdf6f0d5b7488839)
  {
    SC_CTHREAD(init_proc, clk.pos());
    async_reset_signal_is(rst, false);

    /*[[[cog
         for p in dut.inps + dut.outs:
           cog.outl("SC_CTHREAD(%s_proc, clk.pos());" % (p.nm,))
           cog.outl("async_reset_signal_is(rst, false);")
           cog.outl("")
      ]]]*/
    SC_CTHREAD(inp_proc, clk.pos());
    async_reset_signal_is(rst, false);

    SC_CTHREAD(out_proc, clk.pos());
    async_reset_signal_is(rst, false);

    //[[[end]]] (checksum: 0e9d3b848f8605301c36b77e4884ecee)
    /*[[[cog
         for p in dut.inps:
           cog.outl("%s.clk_rst( clk, rst);" % (p.reqNmT(),))
           cog.outl("%s.clk_rst( clk, rst);" % (p.respNmT(),))
      ]]]*/
    inpReqIn.clk_rst( clk, rst);
    inpRespOut.clk_rst( clk, rst);
    //[[[end]]] (checksum: d4c6a6cb223dc1bfcb4410ea18f2ae5f)

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
  void inp_proc() {
    MemTypedRead<BlkInp> mem( inpReqIn, inpRespOut);
    wait();
    while (1) {
      mem();
      wait();
    }
  }

    //[[[end]]] (checksum: f7dfd1b4d6d64c78b210961d95472294)
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
    MemTypedWrite<BlkOut> mem( outReqIn, outDataIn);
    wait();
    while (1) {
      mem();
      wait();
    }
  }

  //[[[end]]] (checksum: 575c78824646a46f73b7c3b1bb8b7527)
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
  sobel_hls_tb tb;
  sobel_hls kernel;
  //[[[end]]] (checksum: ae33c2cfbd622e71c248a7cc379946d6)
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
  ga::tlm_fifo<MemTypedReadReqType<BlkInp>,2> inpReq;
  ga::tlm_fifo<MemTypedReadRespType<BlkInp>,2> inpResp;

  //[[[end]]] (checksum: 00044d9733c1eb030ee74dda7a783609)
  // channel for memory write port
  /*[[[cog
       for p in dut.outs:
         cog.outl("ga::tlm_fifo<%s,2> %s;" % (p.reqTy(),p.reqNm()))
         cog.outl("ga::tlm_fifo<%s,2> %s;" % (p.dataTy(),p.dataNm()))
    ]]]*/
  ga::tlm_fifo<MemTypedWriteReqType<BlkOut>,2> outReq;
  ga::tlm_fifo<MemTypedWriteDataType<BlkOut>,2> outData;
  //[[[end]]] (checksum: 6814e7292d26c247b67a917cd0c3cbfc)

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
    , inpReq("inpReq"), inpResp("inpResp")
    , outReq("outReq"), outData("outData")
    //[[[end]]] (checksum: 19771bc7903f775beae3d68c8d6f6c64)
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
    tb.inpReqIn(inpReq);
    kernel.inpReqOut(inpReq);
    tb.inpRespOut(inpResp);
    kernel.inpRespIn(inpResp);

    //[[[end]]] (checksum: d18f7b0ceb59c203a6aeca3befa80c6b)
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
