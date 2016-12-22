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
#include "linkedlist_hls.h"
//[[[end]]] (checksum: af1ec8d4f8f1eb38968b7addc793626b)
#include "kernel_mock_memory.h"

/*[[[cog
     cog.outl("class %s_hls_tb : public sc_module" % dut.nm)
  ]]]*/
class linkedlist_hls_tb : public sc_module
//[[[end]]] (checksum: 8e5aa5cbb53ff33740482c31d5dd880b)
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
  ga::tlm_fifo_in<MemSingleReadReqType<Node,State> > lstReqIn;
  ga::tlm_fifo_out<MemSingleReadRespType<Node,State> > lstRespOut;

  ga::tlm_fifo_in<MemTypedReadReqType<HeadPtr> > inpReqIn;
  ga::tlm_fifo_out<MemTypedReadRespType<HeadPtr> > inpRespOut;

  ga::tlm_fifo_in<MemTypedWriteReqType<HeadPtr> > outReqIn;
  ga::tlm_fifo_in<MemTypedWriteDataType<HeadPtr> > outDataIn;

  //[[[end]]] (checksum: 806e8682b8996fd9991fa75b6db6f797)
  sc_out<Config> config_out;

  Config config;

  /*[[[cog
       cog.outl("SC_HAS_PROCESS(%s_hls_tb);" % dut.nm)
    ]]]*/
  SC_HAS_PROCESS(linkedlist_hls_tb);
  //[[[end]]] (checksum: 0cdcda41f162ed4c246c1841eb6b2127)

  //scide_waive SCIDE.8.15
  /*[[[cog
       cog.outl("%s_hls_tb(sc_module_name modname) :" % dut.nm)
    ]]]*/
  linkedlist_hls_tb(sc_module_name modname) :
  //[[[end]]] (checksum: bac62eb3706be4baaa0503536454bac0)
    sc_module(modname)
  , clk("clk")
  , rst("rst"), done("done"), start("start")
  /*[[[cog
       for p in dut.inps:
         cog.outl(""", %s("%s"), %s("%s")""" % (p.reqNmT(),p.reqNmT(),p.respNmT(),p.respNmT()))
       for p in dut.outs:
         cog.outl(""", %s("%s"), %s("%s")""" % (p.reqNmT(),p.reqNmT(),p.dataNmT(),p.dataNmT()))
    ]]]*/
  , lstReqIn("lstReqIn"), lstRespOut("lstRespOut")
  , inpReqIn("inpReqIn"), inpRespOut("inpRespOut")
  , outReqIn("outReqIn"), outDataIn("outDataIn")
  //[[[end]]] (checksum: 516d32290416bdd057ee128e8947f45a)
  {
    SC_CTHREAD(init_proc, clk.pos());
    async_reset_signal_is(rst, false);

    /*[[[cog
         for p in dut.inps + dut.outs:
           cog.outl("SC_CTHREAD(%s_proc, clk.pos());" % (p.nm,))
           cog.outl("async_reset_signal_is(rst, false);")
           cog.outl("")
      ]]]*/
    SC_CTHREAD(lst_proc, clk.pos());
    async_reset_signal_is(rst, false);

    SC_CTHREAD(inp_proc, clk.pos());
    async_reset_signal_is(rst, false);

    SC_CTHREAD(out_proc, clk.pos());
    async_reset_signal_is(rst, false);

    //[[[end]]] (checksum: 9ed59e3f6e3b2f231ccb2fb70d34ce87)
    /*[[[cog
         for p in dut.inps:
           cog.outl("%s.clk_rst( clk, rst);" % (p.reqNmT(),))
           cog.outl("%s.clk_rst( clk, rst);" % (p.respNmT(),))
      ]]]*/
    lstReqIn.clk_rst( clk, rst);
    lstRespOut.clk_rst( clk, rst);
    inpReqIn.clk_rst( clk, rst);
    inpRespOut.clk_rst( clk, rst);
    //[[[end]]] (checksum: 91223548b5eae0fe01c6397710120695)

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
    MemTypedRead<HeadPtr> mem( inpReqIn, inpRespOut);
    wait();
    while (1) {
      mem();
      wait();
    }
  }

    //[[[end]]] (checksum: 73dcdeafa092053910ee59f633df7096)
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
  void lst_proc() {
    MemSingleRead<Node,State> mem( lstReqIn, lstRespOut);
    wait();
    while (1) {
      mem();
      wait();
    }
  }

  //[[[end]]] (checksum: 85685d2822199726c5dce0f5ee335d14)
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
    MemTypedWrite<HeadPtr> mem( outReqIn, outDataIn);
    wait();
    while (1) {
      mem();
      wait();
    }
  }

  //[[[end]]] (checksum: ae3d58d8ad92961e63efdffb336910a1)
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
  linkedlist_hls_tb tb;
  linkedlist_hls kernel;
  //[[[end]]] (checksum: 5e13bade61c38db37a3ad9c76a264fb3)
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
  ga::tlm_fifo<MemSingleReadReqType<Node,State>,2> lstReq;
  ga::tlm_fifo<MemSingleReadRespType<Node,State>,2> lstResp;

  ga::tlm_fifo<MemTypedReadReqType<HeadPtr>,2> inpReq;
  ga::tlm_fifo<MemTypedReadRespType<HeadPtr>,2> inpResp;

  //[[[end]]] (checksum: 8f0ce99ba70000a7266d2b23e5bf8327)
  // channel for memory write port
  /*[[[cog
       for p in dut.outs:
         cog.outl("ga::tlm_fifo<%s,2> %s;" % (p.reqTy(),p.reqNm()))
         cog.outl("ga::tlm_fifo<%s,2> %s;" % (p.dataTy(),p.dataNm()))
    ]]]*/
  ga::tlm_fifo<MemTypedWriteReqType<HeadPtr>,2> outReq;
  ga::tlm_fifo<MemTypedWriteDataType<HeadPtr>,2> outData;
  //[[[end]]] (checksum: 84073b3c23ebf5d4589b8379b9b17038)

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
    , lstReq("lstReq"), lstResp("lstResp")
    , inpReq("inpReq"), inpResp("inpResp")
    , outReq("outReq"), outData("outData")
    //[[[end]]] (checksum: b3764b7780985b1179c37399b647a92c)
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
    tb.lstReqIn(lstReq);
    kernel.lstReqOut(lstReq);
    tb.lstRespOut(lstResp);
    kernel.lstRespIn(lstResp);

    tb.inpReqIn(inpReq);
    kernel.inpReqOut(inpReq);
    tb.inpRespOut(inpResp);
    kernel.inpRespIn(inpResp);

    //[[[end]]] (checksum: 32df16aa197d18a316f65920ce890046)
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
