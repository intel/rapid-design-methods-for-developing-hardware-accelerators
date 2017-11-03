// See LICENSE for license details.
package dataflow

import chisel3._
import chisel3.util._
import chisel3.iotesters.{SteppedHWIOTester, AdvTester, ChiselFlatSpec}
import testutil._
import dataflow._
import type_repacker.TypeRepacker
import accio._
import designutils._
import accio.MemRdReqRespTuple._  
import accio.MemWrReqRespTuple._  
//import accio.DefaultAccParams._

class PipeWithSquashIO(gen : UInt) extends Module{
  val io = IO(new Bundle {
    val in = Flipped(Decoupled(gen))
    val out = Decoupled(gen)
  })
  
}
class PipeWithSquash(gen : UInt) extends PipeWithSquashIO(gen) {

  val stage1 = RegInit(init = InvalidToken(gen)) 
  val stage2 = RegInit(init = InvalidToken(gen)) 
  
  io.out.noenq
  io.in.nodeq
  
  val w_stage2ready = Wire(init = !stage2.valid)
  val w_stage1ready = Wire(init = !stage1.valid)
  
  //we use anti-dependency coding style to model pipeline 
  
  // third stage 
  when (stage2.valid) {
    io.out.enq(stage2.token)
    w_stage2ready := io.out.ready
  }
  
  // second stage
  when (w_stage2ready) {
    when (stage1.valid) {
      stage2.set(stage1.token + 1.U)
      w_stage1ready := true.B
    }.otherwise {
      stage2.reset
    }
  }
  
  // first stage
  when (w_stage1ready) {
    val in = io.in.deq
    when (io.in.valid) {
      stage1.set(in + 1.U)
    }.otherwise {
      stage1.reset
    }
  }
}

class PipeWithSquashSimple(gen : UInt) extends PipeWithSquashIO(gen) {

  val stage1 = RegInit(init = InvalidToken(gen)) 
  val stage2 = RegInit(init = InvalidToken(gen)) 
  
  io.out.noenq
  io.in.nodeq
  
  val w_stage2ready = Wire(init = !stage2.valid)
  val w_stage1ready = Wire(init = !stage1.valid)
  
  //we use anti-dependency coding style to model pipeline 
  
  // third stage 
  when (stage2.valid && io.out.ready) {
    io.out.enq(stage2.token)
    w_stage2ready := true.B
  }
  
  // second stage
  when (w_stage2ready && stage1.valid) {
    stage2.set(stage1.token + 1.U)
    w_stage1ready := true.B
  }
  
  // first stage
  when (w_stage1ready && io.in.valid) {
    val in = io.in.deq
    stage1.set(in + 1.U)
  }
}

class TopPipeSquash(dut_factory : () => PipeWithSquashIO) extends Module {
  val gen = Module(dut_factory())
  val io = IO(new Bundle {
    val in  = Flipped(Decoupled( gen.io.in.bits.cloneType ) )
    val out  = Decoupled( gen.io.out.bits.cloneType ) 
  })

  val pipe1 = Module(dut_factory())
  val pipe2 = Module(dut_factory())
  
  pipe1.io.in <> io.in
  pipe2.io.in <> DecoupledStage(pipe1.io.out)
  io.out <> pipe2.io.out
}

class PipeWithSquashTest (tb : TopPipeSquash) extends DecoupledStreamingTestsUsingAdvTester(tb) {
  val instreams = List((tb.io.in, List(BigInt(1),BigInt(3), BigInt(5))))
  val outstreams = List((tb.io.out, List(BigInt(5), BigInt(7), BigInt(9))))
  testStreams (tb, instreams, outstreams, 0, 100 )
  step(20)


}


class PipeWithSquashTester extends ChiselFlatSpec{
  
  val factory_ideal = () => new PipeWithSquash(UInt(32.W)) 
  val factory_simple = () => new PipeWithSquashSimple(UInt(32.W)) 
  "ideal circuit" should "simulate" in {
    chisel3.iotesters.Driver(() => new TopPipeSquash(factory_ideal),"firrtl"){ c =>
      new PipeWithSquashTest(c)
    }should be(true)
  }

  "simple circuit" should "simulate" in {
    chisel3.iotesters.Driver(() => new TopPipeSquash(factory_simple),"firrtl"){ c =>
      new PipeWithSquashTest(c)
    }should be(true)
  }

}
