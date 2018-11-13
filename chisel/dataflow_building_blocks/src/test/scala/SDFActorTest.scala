// See LICENSE for license details.
package dataflow

import chisel3._
import chisel3.util._
import chisel3.iotesters.{SteppedHWIOTester, AdvTester, ChiselFlatSpec}
import testutil._
import designutils._
import dataflow._
import type_repacker.TypeRepacker

object BigIntGen {
  def apply(widths : List[Int], values : List[Int]) : BigInt = {
    assert(values.length > 0)
    assert(values.length == widths.length)
    var result : BigInt = values(0);
    var bundle = widths zip values
    
    for ((width, value) <- bundle.tail) {
      result = (result <<width) + value
    }
    result
  }
}

class TopSdf() extends Module {
  val vecType = Vec(1, UInt(3.W))
  val io = IO(new Bundle {
    val in  = Flipped(Decoupled( UInt (vecType.getWidth.W)) )
  })
  val n1 = Module(new SDFActor(1, 1) {
    override lazy val io = IO(new ActorIO {
     val in0 = In(vecType, 1) 
     val out0 = Out(vecType, 1) 
    })
    override def func = {
      io.out0.bits := io.in0.bits
    }
  })
  
  val q1 = Module(new FillableQueue(vecType, 16, 4))
  val lat1 = Module (new LatencyPipe(vecType, 8))
  
  n1.io.in0 <> q1.io.deq
  
  n1.io.out0 <> lat1.io.in
  
  lat1.io.out <> q1.io.enq

  q1.io.fenq.valid <> io.in.valid
  io.in.ready <> q1.io.fenq.ready
  
  q1.io.fenq.bits := io.in.bits.asTypeOf(q1.io.fenq.bits)
}


class SDFActorFirstTest (dut : TopSdf) extends DecoupledStreamingTestsUsingAdvTester(dut) {
  val inType = dut.vecType
  val in_stream = List(
                       BigIntGen(List(inType.getWidth), List(2)), 
                       BigIntGen(List(inType.getWidth), List(4)), 
                       BigIntGen(List(inType.getWidth), List(6)), 
                       BigIntGen(List(inType.getWidth), List(7)) 
                      )
                      
  val instreams = List((dut.io.in, in_stream))
  //val outstreams = List((dut.io.acc_out,out_stream))
  testStreams (dut, instreams, List.empty, 0, 100 )
  step(20)
}

// *****************************************************************************
class SDFActorTester extends ChiselFlatSpec{
  "sdf circuit" should "simulate" in {
    chisel3.iotesters.Driver(() => new TopSdf,"verilator"){ c =>
      new SDFActorFirstTest(c)
    }should be(true)
  }
}
