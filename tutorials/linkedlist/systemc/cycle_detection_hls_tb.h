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
     cog.outl("#include \"%s_hls.h\"" % dut_name)
  ]]]*/
#include "cycle_detection_hls.h"
//[[[end]]] (checksum: 4490a8f47b99043136f554cba88c4a45)

/*[[[cog
     cog.outl("class %s_hls_tb : public sc_module" % dut_name)
  ]]]*/
class cycle_detection_hls_tb : public sc_module
//[[[end]]] (checksum: 750c1c616c992b27bd22768b9af4890d)
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

  //[[[end]]] (checksum: 7c43c1635ca97528460831395513be2d)
  sc_out<Config> config_out;

  Config config;

  /*[[[cog
       cog.outl("SC_HAS_PROCESS(%s_hls_tb);" % dut_name)
    ]]]*/
  SC_HAS_PROCESS(cycle_detection_hls_tb);
  //[[[end]]] (checksum: 346a08304bff007d47f5bcbf68ad306e)

  //scide_waive SCIDE.8.15
  /*[[[cog
       cog.outl("%s_hls_tb(sc_module_name modname) :" % dut_name)
    ]]]*/
  cycle_detection_hls_tb(sc_module_name modname) :
  //[[[end]]] (checksum: 2362265884797b598f12e44923accc47)
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
  //[[[end]]] (checksum: 598b4e54c1f71c81bdf6f0d5b7488839)
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

    //[[[end]]] (checksum: 0e9d3b848f8605301c36b77e4884ecee)
    /*[[[cog
         for p in inps:
           cog.outl("%s.clk_rst( clk, rst);" % (p.reqNmT(),))
           cog.outl("%s.clk_rst( clk, rst);" % (p.respNmT(),))
      ]]]*/
    inpReqIn.clk_rst( clk, rst);
    inpRespOut.clk_rst( clk, rst);
    //[[[end]]] (checksum: d4c6a6cb223dc1bfcb4410ea18f2ae5f)

    /*[[[cog
         for p in outs:
           cog.outl("%s.clk_rst( clk, rst);" % (p.reqNmT(),))
           cog.outl("%s.clk_rst( clk, rst);" % (p.dataNmT(),))
      ]]]*/
    outReqIn.clk_rst( clk, rst);
    outDataIn.clk_rst( clk, rst);
    //[[[end]]] (checksum: 4310c5c17ae912afba1b245676ce7dee)

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
         cog.outl("    const %s *ptr = reinterpret_cast<const %s*>( (size_t) read_req.addr);" % (p.ty,p.ty))
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

      const Node *ptr = reinterpret_cast<const Node*>( (size_t) read_req.addr);

      for (unsigned i=0; i<read_req.size; ++i) {
        MemTypedReadRespType<Node> read_resp;

        read_resp.data = ptr[i];

        inpRespOut.put(read_resp);
        wait();
      }
    }
  }

    //[[[end]]] (checksum: 57416cf372caa3afd4980ec4e4e7c4e8)
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
         cog.outl("    const %s *ptr = reinterpret_cast<const %s*>( (size_t) read_req.addr);" % (p.ty,p.ty))
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
  //[[[end]]] (checksum: d41d8cd98f00b204e9800998ecf8427e)
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
         cog.outl("    %s *ptr = reinterpret_cast<%s*>( (size_t) write_req.addr);" % (p.ty, p.ty))
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

      CycleExistInfo *ptr = reinterpret_cast<CycleExistInfo*>( (size_t) write_req.addr);

      assert(write_req.size);
      for (unsigned i=0; i<write_req.size; ++i) {
        MemTypedWriteDataType<CycleExistInfo> write_data;
        outDataIn.get(write_data);
        ptr[i] = write_data.data;
        wait();
      }
    }
  }

  //[[[end]]] (checksum: 06586adc13e963dfa29f713c8611cc86)
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
       cog.outl("%s_hls_tb tb;" % dut_name)
       cog.outl("%s_hls kernel;" % dut_name)
    ]]]*/
  cycle_detection_hls_tb tb;
  cycle_detection_hls kernel;
  //[[[end]]] (checksum: 383680a8768c0943eb87bfcedafeec99)
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

  //[[[end]]] (checksum: 0b5afd56d24c459852e8244efcecf1a5)
  // channel for memory write port
  /*[[[cog
       for p in outs:
         cog.outl("ga::tlm_fifo<%s,2> %s;" % (p.reqTy(),p.reqNm()))
         cog.outl("ga::tlm_fifo<%s,2> %s;" % (p.dataTy(),p.dataNm()))
    ]]]*/
  ga::tlm_fifo<MemTypedWriteReqType<CycleExistInfo>,2> outReq;
  ga::tlm_fifo<MemTypedWriteDataType<CycleExistInfo>,2> outData;
  //[[[end]]] (checksum: a805391e604b59794c838c9cb5de796c)

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

    //[[[end]]] (checksum: d18f7b0ceb59c203a6aeca3befa80c6b)
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
