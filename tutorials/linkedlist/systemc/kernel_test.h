// See LICENSE for license details.
#ifndef KERNEL_TB_H_
#define KERNEL_TB_H_

/*[[[cog
     import cog
     from cog_acctempl import *
     from dut_params import *
  ]]]*/
//[[[end]]]

#include "systemc.h"
#include "clock_generator.h"
#include "ga_tlm_fifo.h"

class KernelTest : public sc_module
{
public:
  sc_in_clk clk;
  sc_in<bool> rst;
  sc_in<bool> done;
  sc_out<bool> start;

  // functional IOs

  /*[[[cog
       for p in inps:
         cog.outl("ga::tlm_fifo_in<%s > %s;" % (p.reqTy(), p.reqNmT()))
         cog.outl("ga::tlm_fifo_out<%s > %s;" % (p.respTy(), p.respNmT()))
         cog.outl("")
       for p in outs:
         cog.outl("ga::tlm_fifo_in<%s > %s;" % (p.reqTy(), p.reqNmT()))
         cog.outl("ga::tlm_fifo_in<%s > %s;" % (p.dataTy(), p.dataNmT()))
         cog.outl("")
    ]]]*/
  ga::tlm_fifo_in<MemTypedReadReqType<Node> > inpReqIn;
  ga::tlm_fifo_out<MemTypedReadRespType<Node> > inpRespOut;

  ga::tlm_fifo_in<MemTypedWriteReqType<CycleExistInfo> > outReqIn;
  ga::tlm_fifo_in<MemTypedWriteDataType<CycleExistInfo> > outDataIn;

  //[[[end]]]
  sc_out<Config> config_out;

  Config config;

  SC_HAS_PROCESS(KernelTest);

  //scide_waive SCIDE.8.15
  KernelTest(sc_module_name modname) :
    sc_module(modname)
  , clk("clk")
  , rst("rst"), done("done"), start("start")
  /*[[[cog
       for p in inps:
         cog.outl(""", %s("%s"), %s("%s")""" % (p.reqNmT(),p.reqNmT(),p.respNmT(),p.respNmT()))
       for p in outs:
         cog.outl(""", %s("%s"), %s("%s")""" % (p.reqNmT(),p.reqNmT(),p.dataNmT(),p.dataNmT()))
    ]]]*/
  , inpReqIn("inpReqIn"), inpRespOut("inpRespOut")
  , outReqIn("outReqIn"), outDataIn("outDataIn")
  //[[[end]]]
  {
    SC_CTHREAD(init_proc, clk.pos());
    async_reset_signal_is(rst, false);

    /*[[[cog
         for p in inps + outs:
           cog.outl("SC_CTHREAD(%s_proc, clk.pos());" % (p.nm,))
           cog.outl("async_reset_signal_is(rst, false);")
           cog.outl("")
      ]]]*/
    SC_CTHREAD(inp_proc, clk.pos());
    async_reset_signal_is(rst, false);

    SC_CTHREAD(out_proc, clk.pos());
    async_reset_signal_is(rst, false);

    //[[[end]]]
    /*[[[cog
         for p in inps:
           cog.outl("%s.clk_rst( clk, rst);" % (p.reqNmT(),))
           cog.outl("%s.clk_rst( clk, rst);" % (p.respNmT(),))
      ]]]*/
    inpReqIn.clk_rst( clk, rst);
    inpRespOut.clk_rst( clk, rst);
    //[[[end]]]

    /*[[[cog
         for p in outs:
           cog.outl("%s.clk_rst( clk, rst);" % (p.reqNmT(),))
           cog.outl("%s.clk_rst( clk, rst);" % (p.dataNmT(),))
      ]]]*/
    outReqIn.clk_rst( clk, rst);
    outDataIn.clk_rst( clk, rst);
    //[[[end]]]

  }

  /*[[[cog
       for p in inps:
         if type(p) is not TypedRead: continue
         cog.outl("void %s_proc() {" % (p.nm,))
         cog.outl("  %s.reset_get();" % (p.reqNmT(),))
         cog.outl("  %s.reset_put();" % (p.respNmT(),))
         cog.outl("  wait();")
         cog.outl("  while (1) {")
         cog.outl("    %s read_req;" % (p.reqTy(),))
         cog.outl("    %s.get(read_req);" % (p.reqNmT(),))
         cog.outl("")
         cog.outl("    const %s *ptr = reinterpret_cast<const %s*>( read_req.addr);" % (p.ty,p.ty))
         cog.outl("")
         cog.outl("    for (unsigned i=0; i<read_req.size; ++i) {")
         cog.outl("      %s read_resp;" % (p.respTy(),))
         cog.outl("")
         cog.outl("      read_resp.data = ptr[i];")
         cog.outl("")
         cog.outl("      %s.put(read_resp);" % (p.respNmT(),))
         cog.outl("      wait();")
         cog.outl("    }")
         cog.outl("  }")
         cog.outl("}")
         cog.outl("")
    ]]]*/
  void inp_proc() {
    inpReqIn.reset_get();
    inpRespOut.reset_put();
    wait();
    while (1) {
      MemTypedReadReqType<Node> read_req;
      inpReqIn.get(read_req);

      const Node *ptr = reinterpret_cast<const Node*>( read_req.addr);

      for (unsigned i=0; i<read_req.size; ++i) {
        MemTypedReadRespType<Node> read_resp;

        read_resp.data = ptr[i];

        inpRespOut.put(read_resp);
        wait();
      }
    }
  }

    //[[[end]]]
  /*[[[cog
       for p in inps:
         if type(p) is not SingleRead: continue
         cog.outl("void %s_proc() {" % (p.nm,))
         cog.outl("  %s.reset_get();" % (p.reqNmT(),))
         cog.outl("  %s.reset_put();" % (p.respNmT(),))
         cog.outl("  wait();")
         cog.outl("  while (1) {")
         cog.outl("    %s read_req;" % (p.reqTy(),))
         cog.outl("    %s.get(read_req);" % (p.reqNmT(),))
         cog.outl("")
         cog.outl("    const %s *ptr = reinterpret_cast<const %s*>( read_req.addr);" % (p.ty,p.ty))
         cog.outl("")
         cog.outl("    %s read_resp;" % (p.respTy(),))
         cog.outl("    read_resp.data = *ptr;")
         cog.outl("    read_resp.utag = read_req.utag;")
         cog.outl("")
         cog.outl("    %s.put(read_resp);" % (p.respNmT(),))
         cog.outl("    wait();")
         cog.outl("  }")
         cog.outl("}")
         cog.outl("")
    ]]]*/
  //[[[end]]]
  /*[[[cog
       for p in outs:
         cog.outl("void %s_proc() {" % (p.nm,))
         cog.outl("  %s.reset_get();" % (p.reqNmT(),))
         cog.outl("  %s.reset_get();" % (p.dataNmT(),))
         cog.outl("  wait();")
         cog.outl("  while (1) {")
         cog.outl("    %s write_req;" % (p.reqTy(),))
         cog.outl("    %s.get(write_req);" % (p.reqNmT(),))
         cog.outl("")
         cog.outl("    %s *ptr = reinterpret_cast<%s*>( write_req.addr);" % (p.ty, p.ty))
         cog.outl("")
         cog.outl("    assert(write_req.size);")
         cog.outl("    for (unsigned i=0; i<write_req.size; ++i) {")
         cog.outl("      %s write_data;" % (p.dataTy(),))
         cog.outl("      %s.get(write_data);" % (p.dataNmT(),))
         cog.outl("      ptr[i] = write_data.data;")
         cog.outl("      wait();")
         cog.outl("    }")
         cog.outl("  }")
         cog.outl("}")
         cog.outl("")
    ]]]*/
  void out_proc() {
    outReqIn.reset_get();
    outDataIn.reset_get();
    wait();
    while (1) {
      MemTypedWriteReqType<CycleExistInfo> write_req;
      outReqIn.get(write_req);

      CycleExistInfo *ptr = reinterpret_cast<CycleExistInfo*>( write_req.addr);

      assert(write_req.size);
      for (unsigned i=0; i<write_req.size; ++i) {
        MemTypedWriteDataType<CycleExistInfo> write_data;
        outDataIn.get(write_data);
        ptr[i] = write_data.data;
        wait();
      }
    }
  }

  //[[[end]]]
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
  KernelTest tb;
  /*[[[cog
       cog.outl("%s kernel;" % module_name)
    ]]]*/
  cycle_detection_hls kernel;
  //[[[end]]]
  sc_signal<bool> clk;
  sc_signal<bool> rst;
  sc_signal<Config> config_ch;
  sc_signal<bool> start_ch;
  sc_signal<bool> done_ch;

  // channel for memory read port
  /*[[[cog
       for p in inps:
         cog.outl("ga::tlm_fifo<%s,2> %s;" % (p.reqTy(),p.reqNm()))
         cog.outl("ga::tlm_fifo<%s,2> %s;" % (p.respTy(),p.respNm()))
         cog.outl("")
    ]]]*/
  ga::tlm_fifo<MemTypedReadReqType<Node>,2> inpReq;
  ga::tlm_fifo<MemTypedReadRespType<Node>,2> inpResp;

  //[[[end]]]
  // channel for memory write port
  /*[[[cog
       for p in outs:
         cog.outl("ga::tlm_fifo<%s,2> %s;" % (p.reqTy(),p.reqNm()))
         cog.outl("ga::tlm_fifo<%s,2> %s;" % (p.dataTy(),p.dataNm()))
    ]]]*/
  ga::tlm_fifo<MemTypedWriteReqType<CycleExistInfo>,2> outReq;
  ga::tlm_fifo<MemTypedWriteDataType<CycleExistInfo>,2> outData;
  //[[[end]]]

  SC_HAS_PROCESS(AcclApp);

  AcclApp(sc_module_name name=sc_core::sc_gen_unique_name("AcclApp")) :
    clkgen("clkgen"), tb("tb"),
    kernel("kernel"),
    clk("clk"), rst("rst"),
    start_ch("start_ch"), done_ch("done_ch")
    /*[[[cog
         for p in inps:
           cog.outl(""", %s("%s"), %s("%s")""" % (p.reqNm(),p.reqNm(),p.respNm(),p.respNm()))
         for p in outs:
           cog.outl(""", %s("%s"), %s("%s")""" % (p.reqNm(),p.reqNm(),p.dataNm(),p.dataNm()))
      ]]]*/
    , inpReq("inpReq"), inpResp("inpResp")
    , outReq("outReq"), outData("outData")
    //[[[end]]]
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
    /*[[[cog
         cog.outl("kernel.%s(config_ch);" % kernel_config_name)
      ]]]*/
    kernel.config(config_ch);
    //[[[end]]]

    /*[[[cog
         for p in inps:
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

    //[[[end]]]
    /*[[[cog
         for p in outs:
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

    //[[[end]]]
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
