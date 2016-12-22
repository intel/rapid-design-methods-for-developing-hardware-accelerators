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
#include "bwmatch_hls.h"
//[[[end]]] (checksum: b996b0b7fea12f0b1ad2e293d2df7677)
#include "kernel_mock_memory.h"

/*[[[cog
     cog.outl("class %s_hls_tb : public sc_module" % dut.nm)
  ]]]*/
class bwmatch_hls_tb : public sc_module
//[[[end]]] (checksum: 16c7e2e1eb10b9b264c839365b4c7738)
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
  ga::tlm_fifo_in<MemTypedReadReqType<BWPattern> > patReqIn;
  ga::tlm_fifo_out<MemTypedReadRespType<BWPattern> > patRespOut;

  ga::tlm_fifo_in<MemSingleReadReqType<BWCacheLine,BWState> > clReqIn;
  ga::tlm_fifo_out<MemSingleReadRespType<BWCacheLine,BWState> > clRespOut;

  ga::tlm_fifo_in<MemTypedReadReqType<BWResult> > preReqIn;
  ga::tlm_fifo_out<MemTypedReadRespType<BWResult> > preRespOut;

  ga::tlm_fifo_in<MemTypedWriteReqType<BWResultLine> > resReqIn;
  ga::tlm_fifo_in<MemTypedWriteDataType<BWResultLine> > resDataIn;

  //[[[end]]] (checksum: bb773ae15f7587c2b16b4a05fc7ee827)
  sc_out<Config> config_out;

  Config config;

  /*[[[cog
       cog.outl("SC_HAS_PROCESS(%s_hls_tb);" % dut.nm)
    ]]]*/
  SC_HAS_PROCESS(bwmatch_hls_tb);
  //[[[end]]] (checksum: 1747481b9326f848ed0b33b072a985c4)

  //scide_waive SCIDE.8.15
  /*[[[cog
       cog.outl("%s_hls_tb(sc_module_name modname) :" % dut.nm)
    ]]]*/
  bwmatch_hls_tb(sc_module_name modname) :
  //[[[end]]] (checksum: 8dcab191d2ff825adabc84d991f4a4e1)
    sc_module(modname)
  , clk("clk")
  , rst("rst"), done("done"), start("start")
  /*[[[cog
       for p in dut.inps:
         cog.outl(""", %s("%s"), %s("%s")""" % (p.reqNmT(),p.reqNmT(),p.respNmT(),p.respNmT()))
       for p in dut.outs:
         cog.outl(""", %s("%s"), %s("%s")""" % (p.reqNmT(),p.reqNmT(),p.dataNmT(),p.dataNmT()))
    ]]]*/
  , patReqIn("patReqIn"), patRespOut("patRespOut")
  , clReqIn("clReqIn"), clRespOut("clRespOut")
  , preReqIn("preReqIn"), preRespOut("preRespOut")
  , resReqIn("resReqIn"), resDataIn("resDataIn")
  //[[[end]]] (checksum: 49d5eba2c30713d23d6a946b50875d5a)
  {
    SC_CTHREAD(init_proc, clk.pos());
    async_reset_signal_is(rst, false);

    /*[[[cog
         for p in dut.inps + dut.outs:
           cog.outl("SC_CTHREAD(%s_proc, clk.pos());" % (p.nm,))
           cog.outl("async_reset_signal_is(rst, false);")
           cog.outl("")
      ]]]*/
    SC_CTHREAD(pat_proc, clk.pos());
    async_reset_signal_is(rst, false);

    SC_CTHREAD(cl_proc, clk.pos());
    async_reset_signal_is(rst, false);

    SC_CTHREAD(pre_proc, clk.pos());
    async_reset_signal_is(rst, false);

    SC_CTHREAD(res_proc, clk.pos());
    async_reset_signal_is(rst, false);

    //[[[end]]] (checksum: e3ebe464dc52ea47869f87610f5806c6)
    /*[[[cog
         for p in dut.inps:
           cog.outl("%s.clk_rst( clk, rst);" % (p.reqNmT(),))
           cog.outl("%s.clk_rst( clk, rst);" % (p.respNmT(),))
      ]]]*/
    patReqIn.clk_rst( clk, rst);
    patRespOut.clk_rst( clk, rst);
    clReqIn.clk_rst( clk, rst);
    clRespOut.clk_rst( clk, rst);
    preReqIn.clk_rst( clk, rst);
    preRespOut.clk_rst( clk, rst);
    //[[[end]]] (checksum: 94a05ff029c75464eb53820c2afb6bde)

    /*[[[cog
         for p in dut.outs:
           cog.outl("%s.clk_rst( clk, rst);" % (p.reqNmT(),))
           cog.outl("%s.clk_rst( clk, rst);" % (p.dataNmT(),))
      ]]]*/
    resReqIn.clk_rst( clk, rst);
    resDataIn.clk_rst( clk, rst);
    //[[[end]]] (checksum: a8d93f1e7db6319b49df38d60bcb9c2d)

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
  void pat_proc() {
    MemTypedRead<BWPattern> mem( patReqIn, patRespOut);
    wait();
    while (1) {
      mem();
      wait();
    }
  }

  void pre_proc() {
    MemTypedRead<BWResult> mem( preReqIn, preRespOut);
    wait();
    while (1) {
      mem();
      wait();
    }
  }

    //[[[end]]] (checksum: f20f4c39d4d96bb192f183f0c6ba662c)
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
  void cl_proc() {
    MemSingleRead<BWCacheLine,BWState> mem( clReqIn, clRespOut);
    wait();
    while (1) {
      mem();
      wait();
    }
  }

  //[[[end]]] (checksum: 8aee166bb625b67fc8497d0df1d5cabf)
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
  void res_proc() {
    MemTypedWrite<BWResultLine> mem( resReqIn, resDataIn);
    wait();
    while (1) {
      mem();
      wait();
    }
  }

  //[[[end]]] (checksum: 626bac2667183367001f72a16a4be051)
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
  bwmatch_hls_tb tb;
  bwmatch_hls kernel;
  //[[[end]]] (checksum: ced30fe3bdcc541a1a73339cac45728a)
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
  ga::tlm_fifo<MemTypedReadReqType<BWPattern>,2> patReq;
  ga::tlm_fifo<MemTypedReadRespType<BWPattern>,2> patResp;

  ga::tlm_fifo<MemSingleReadReqType<BWCacheLine,BWState>,2> clReq;
  ga::tlm_fifo<MemSingleReadRespType<BWCacheLine,BWState>,2> clResp;

  ga::tlm_fifo<MemTypedReadReqType<BWResult>,2> preReq;
  ga::tlm_fifo<MemTypedReadRespType<BWResult>,2> preResp;

  //[[[end]]] (checksum: 075701cef57506fe40fa89c4537392b2)
  // channel for memory write port
  /*[[[cog
       for p in dut.outs:
         cog.outl("ga::tlm_fifo<%s,2> %s;" % (p.reqTy(),p.reqNm()))
         cog.outl("ga::tlm_fifo<%s,2> %s;" % (p.dataTy(),p.dataNm()))
    ]]]*/
  ga::tlm_fifo<MemTypedWriteReqType<BWResultLine>,2> resReq;
  ga::tlm_fifo<MemTypedWriteDataType<BWResultLine>,2> resData;
  //[[[end]]] (checksum: 87d8a772a99c85df06ca2e8aa8a9bc44)

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
    , patReq("patReq"), patResp("patResp")
    , clReq("clReq"), clResp("clResp")
    , preReq("preReq"), preResp("preResp")
    , resReq("resReq"), resData("resData")
    //[[[end]]] (checksum: 5f3ea05c329070335190349e7daf1ff2)
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
    tb.patReqIn(patReq);
    kernel.patReqOut(patReq);
    tb.patRespOut(patResp);
    kernel.patRespIn(patResp);

    tb.clReqIn(clReq);
    kernel.clReqOut(clReq);
    tb.clRespOut(clResp);
    kernel.clRespIn(clResp);

    tb.preReqIn(preReq);
    kernel.preReqOut(preReq);
    tb.preRespOut(preResp);
    kernel.preRespIn(preResp);

    //[[[end]]] (checksum: 0f0d14db2697e32bce6309f2b2217195)
    /*[[[cog
         for p in dut.outs:
           cog.outl("tb.%s(%s);" % (p.reqNmT(),p.reqNm()))
           cog.outl("kernel.%s(%s);" % (p.reqNmK(),p.reqNm()))
           cog.outl("kernel.%s(%s);" % (p.dataNmK(),p.dataNm()))
           cog.outl("tb.%s(%s);" % (p.dataNmT(),p.dataNm()))
           cog.outl("")
      ]]]*/
    tb.resReqIn(resReq);
    kernel.resReqOut(resReq);
    kernel.resDataOut(resData);
    tb.resDataIn(resData);

    //[[[end]]] (checksum: 8c571dd7ab60e36da51ebb8d1fc59e94)
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
