// See LICENSE for license details.
package dataflow

import chisel3._
import chisel3.util._
import chisel3.iotesters.{PeekPokeTester, AdvTester, ChiselFlatSpec}
import testutil._
import dataflow._
import type_repacker.TypeRepacker
import accio._
import designutils._
import accio.MemRdReqRespTuple._  
import accio.MemWrReqRespTuple._  
//import accio.DefaultAccParams._

object MemInit {
  def init_func (in : UInt): UInt = {
    in + 1.U
  }
}

class VecAddTb[T <: UInt](gen : T, vecSize : Int)(implicit params : AccParams) extends Module {
  val memSize = 128 //cachelines
  val io = IO(new Bundle {
    val start = Input(Bool())
    val config = Input(new Config)
  	val mem_probe = Input((memSize-1).U.cloneType)
  	val mem_monitor = Output(UInt(params.CacheLineWidth.W))
    val done = Output(Bool())
  })
  val vecType = Vec(vecSize, gen)
  val va = Module(new VectorAddTop(gen, vecSize))
  val mem = Module(new MockMemory(memSize, Some(MemInit.init_func), 20))

  
  va.io.start := io.start
  io.done := va.io.done
  io.mem_monitor := mem.io.mem_monitor
  mem.io.mem_probe := io.mem_probe
  mem.io.mem_rd_in <> va.io.mem_rd_req 
  mem.io.mem_rd_out <> va.io.mem_rd_resp 

  mem.io.mem_wr_in <> va.io.mem_wr_req 
  mem.io.mem_wr_out <> va.io.mem_wr_resp 
  
  io.config <> va.io.config    
}

class VectorAddFirstTest (tb : VecAddTb[UInt])(implicit params : AccParams) extends PeekPokeTester(tb) {
  
  val confType = new Config
  val inWidthList = List(confType.in0addr.getWidth,confType.out0addr.getWidth, confType.vecCount.getWidth)

  poke(tb.io.config, Map("in0addr"->BigInt(0),"out0addr"->BigInt(16*(params.CacheLineWidth>>3)),"vecCount"->BigInt(4)))
  poke(tb.io.start, true)
  
  while(peek(tb.io.done) != 1) step(1)
  
  step(1)
  poke(dut.io.mem_probe, 0)
  expect(dut.io.mem_monitor, 1)
  poke(dut.io.mem_probe, 15)
  expect(dut.io.mem_monitor, 16)
  poke(dut.io.mem_probe, 18)
  expect(dut.io.mem_monitor, 19)
  
  // we produce two cachelines (4 vectors each 2x16 bit with Cacheline being 64 bits for this test. see the tester below)
  (0 until 2).foreach {i =>
    step(1)
    poke(dut.io.mem_probe, 16+i)
    expect(dut.io.mem_monitor, (i+1)*2)
  }

}

class VecAddNoConfigTb[T <: UInt](val dut_factory : ()=>VecAddNoConfigIO[T])(implicit params : AccParams) extends Module {
  val memSize = 1024
  val io = IO(new Bundle {
    val acc_rd_req1 = Flipped(Decoupled(new AccMemBurstRdReq))
    val acc_rd_req2 = Flipped(Decoupled(new AccMemBurstRdReq))
    val acc_wr_req = Flipped(Decoupled(new AccMemBurstWrReq))
    val mem_probe = Input((memSize-1).U.cloneType)
    val mem_monitor = Output(UInt(params.CacheLineWidth.W))
    val mem_perf_monitor = Output(new MockMemoryPerfCounters)
    val cnt_computed = Output(UInt(Int.MaxValue).cloneType)
  })

  val mem = Module(new MockMemory(memSize,Some(MemInit.init_func), 100))
  val vecadd = Module(dut_factory())
  
  io.mem_monitor := mem.io.mem_monitor
  mem.io.mem_probe := io.mem_probe
  io.mem_perf_monitor := mem.io.perf_monitor
  
  (mem.io.mem_rd_in, mem.io.mem_rd_out) <> (vecadd.io.mem_rd_req, vecadd.io.mem_rd_resp)
  (mem.io.mem_wr_in, mem.io.mem_wr_out) <> (vecadd.io.mem_wr_req, vecadd.io.mem_wr_resp) 

   vecadd.io.acc_rd_req1 <> io.acc_rd_req1
   vecadd.io.acc_rd_req2 <> io.acc_rd_req2
   vecadd.io.acc_wr_req  <> io.acc_wr_req
   io.cnt_computed := vecadd.io.cnt_computed
}

class VectorAddTestNoConfig (tb : VecAddNoConfigTb[UInt])(implicit params : AccParams) extends DecoupledStreamingTestsUsingAdvTester(tb) {
  val burst = 1024
  val clburst = burst*tb.vecadd.gen.getWidth*tb.vecadd.vecLen/params.CacheLineWidth
  println ("written " + clburst.toString() + " cachelines")
  val instreams = List((tb.io.acc_rd_req1, List(Map("addr"->BigInt(0),"burstSize"->BigInt(clburst)))),
                       (tb.io.acc_rd_req2, List(Map("addr"->BigInt(0),"burstSize"->BigInt(clburst)))),
                       (tb.io.acc_wr_req,  List(Map("addr"->BigInt(clburst),"burstSize"->BigInt(clburst))))
                      )

  testStreams (tb, instreams, List.empty, 0, 100 )
  while (peek(tb.io.cnt_computed) < burst) step(1)
  step(100)
  println ("written " + clburst.toString() + " cachelines")
  for (i <- 0 until clburst) {
    poke(tb.io.mem_probe,clburst+i)
    expect(tb.io.mem_monitor, 2*(i+1))
  }
  
  val rd_idle = peek(tb.io.mem_perf_monitor.read_idle_cycles)
  val wr_idle = peek(tb.io.mem_perf_monitor.write_idle_cycles)
  val tot_cycles = peek(tb.io.mem_perf_monitor.total_cycles)
  
  val rd_ratio = 1 - rd_idle.toDouble/tot_cycles.toDouble
  val wr_ratio = 1 - wr_idle.toDouble/tot_cycles.toDouble
  
  println (s"Total          cycle count $tot_cycles")
  println (f"Mem read  idle cycle count $rd_idle (util $rd_ratio%1.2f)")
  println (f"Mem write idle cycle count $wr_idle (util $wr_ratio%1.2f)")
      //+ " (" + 100.*rd_idle/tot_cycles + "%)" )
  //peek(tb 
	
}


// *****************************************************************************

class VectorAddTester extends ChiselFlatSpec{
  implicit val params = new AccParams {
    val AddrWidth = 64
    val CacheLineWidth = 64
    
    val DefMemTagWidth = 12
    val DefBurstSizeWidth = 32
  }
  
  "sdf circuit" should "simulate" in {
    chisel3.iotesters.Driver(() => new VecAddTb(UInt(16.W), 2),"verilator"){ c =>
      new VectorAddFirstTest(c)
    }should be(true)
  }
}

class VectorAddNoConfigTester extends ChiselFlatSpec{
  implicit val params = new AccParams {
    val AddrWidth = 64
    val CacheLineWidth = 512
     
    val DefMemTagWidth = 12
    val DefBurstSizeWidth = 32
  }
  def createDutFactory[T <: UInt] (gen : T, size : Int) : VecAddNoConfigIO[T] = new VecAddNoConfig(gen, size)
  "sdf circuit" should "simulate" in {
    chisel3.iotesters.Driver(() => new VecAddNoConfigTb(()=>new VecAddNoConfig(UInt(16.W), 16)),"verilator"){ c =>
      new VectorAddTestNoConfig(c)
    }should be(true)
  }
}
