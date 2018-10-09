// See LICENSE for license details.
package type_repacker

import chisel3._

import chisel3.util._
import accio._
import designutils._
import scala.language.implicitConversions
import designutils.DebugMacros._

/**
 * This module takes a vector of objects of type T1 and convert it to a vector of objects of type T2. 
 * Vector sizes are parameters and can be arbitrary. It is possible for one input produce multiple outputs
 * One example this module can be used is to split a cacheline into user objects
 * Another example is to use this module to convert production rate to consumption rate in a SDF 
 */

class TypeRepacker[T1 <: Data, T2<: Data] (gen1 : T1, gen2 : T2) extends Module with AccioDebug {
  val t1width = gen1.getWidth
  val t2width = gen2.getWidth

  val in1width = gen1.getWidth
  val in2width = gen2.getWidth

  val io = IO(new Bundle {
    val in  = Flipped(Decoupled(gen1.cloneType))
    val out = Decoupled(gen2.cloneType)
    val flush = Input(Bool())
    val cnt = Output(UInt((in2width / in1width).W))
  })
  io.cnt := DontCare

  //println("TypeRepacker: in1, in2 widths: ", in1width, in2width) 
  if (in1width == in2width) {
    io.out.valid := io.in.valid
    io.in.ready := io.out.ready
    io.out.bits:= io.in.bits.asTypeOf(io.out.bits)
  } else if (in1width > in2width) {
    assert((in1width % in2width) == 0, s"TypeRepacker for in1, in2 widths: $in1width, $in2width is not implemented")
    val ratio : Int = (in1width/in2width) 
    
    val inAsT2 = Reg(Vec(ratio,UInt(in2width.W)))
    
    val remainNum = RegInit(ratio.U.cloneType, init = 0.U)     
    
    io.in.nodeq()
    io.out.noenq()
    
    val w_finish = Wire(init = (remainNum === 0.U)) 
    when (io.out.ready && remainNum > 0.U) {
      val outData = inAsT2(ratio.U - remainNum)
      remainNum := remainNum - 1.U 
      
      io.out.enq(io.out.bits.fromBits(outData.asUInt))
      // last output, can read a new input
      when (remainNum === 1.U) {
        w_finish := true.B
      }
    }
    
    when (io.in.valid && w_finish) {
      inAsT2 := inAsT2.fromBits(io.in.deq.asUInt)
      remainNum := ratio.U
    }
    //printf("io.in.valid = %d io.in.bits.asUInt()=%d, remainNum = %d inAsT2(0) = %d io.out(0) = %d\n", io.in.valid, io.in.bits.asUInt(), remainNum, inAsT2(0), io.out.bits(0).asUInt())
    
  } else {
    //in1width < in2width
    assert((in2width % in1width) == 0, s"Not implemented $in2width $in1width ")
    val ratio : Int = (in2width/in1width) 
    //println("in1, in2 widths, ratio: ", in1width, in2width, ratio) 
    
    val outAsT1 = Reg(Vec(ratio,UInt(in1width.W)))
    val remainNum = RegInit(ratio.U.cloneType, init = 0.U)     
    val w_remainNum = Wire(ratio.U.cloneType, init = remainNum) 

    io.in.nodeq()
    io.out.noenq()

    when (io.out.ready && (remainNum === ratio.U || (remainNum != 0.U && io.flush))) {
      io.out.enq(io.out.bits.fromBits(outAsT1.asUInt))
      io.cnt := remainNum
      // last input, can push output and get a new input
      w_remainNum := 0.U
    }

    when (io.in.valid && w_remainNum != ratio.U) {
      val inData = io.in.deq
      outAsT1(w_remainNum) := outAsT1(w_remainNum).fromBits(inData.asUInt())
      remainNum := w_remainNum + 1.U
    }.otherwise {
      remainNum := w_remainNum
    }
    pf("io.in.valid = %d io.in.bits.asUInt()=%d, remainNum = %d outAsT1(0) = %d io.out.valid = %d, io.out(0) = %d\n", io.in.valid, io.in.bits.asUInt(), remainNum, outAsT1(0), io.out.valid, io.out.bits.asUInt())
    
    //assert(false, "Not implemented")
  }
}

object TypeRepacker {
  def apply[T1 <: Data, T2 <: Data] (in: DecoupledIO[T1], to : T2, moore: Boolean = false): DecoupledIO[T2]  = {
    val tr = Module(new TypeRepacker(in.bits, to))
    tr.io.in.valid := in.valid 
    tr.io.in.bits := in.bits
    in.ready := tr.io.in.ready
    tr.io.flush := Wire(init=false.B)
    if (moore) 
      MooreStage(tr.io.out)
    else 
      DecoupledStage(tr.io.out)
    
  }

  def apply[T1 <: Data, T2 <: Data with PackedWithCount] (in: DecoupledIO[T1], to : T2, flush: Bool): DecoupledIO[T2]  = {
    val tr = Module(new TypeRepacker(in.bits, to.packed))
    tr.io.in.valid := in.valid 
    tr.io.in.bits := in.bits
    in.ready := tr.io.in.ready
    tr.io.flush := Wire(init=flush)
    val out = Wire(Decoupled(to))
    out.valid := tr.io.out.valid
    tr.io.out.ready := out.ready
    out.bits.packed := tr.io.out.bits
    out.bits.count := tr.io.cnt
    DecoupledStage(out)
  }

}   

object TypeRepackerComb {
  def apply[T1 <: Data, T2 <: Data] (in: DecoupledIO[T1], to: T2): DecoupledIO[T2]  = {
    val tr = Module(new TypeRepacker(in.bits, to))
    tr.io.in.valid := in.valid // not using <> so that override is allowed
    tr.io.in.bits := in.bits
    in.ready := tr.io.in.ready
    tr.io.flush := Wire(init=false.B)
    tr.io.out
  }
  def apply[T1 <: Data, T2 <: Data with PackedWithCount] (in: DecoupledIO[T1], to: T2, flush: Bool): DecoupledIO[T2]  = {
    val tr = Module(new TypeRepacker(in.bits, to.packed))
    tr.io.in.valid := in.valid // not using <> so that override is allowed
    tr.io.in.bits := in.bits
    in.ready := tr.io.in.ready
    tr.io.flush := Wire(init=flush)
    val out = Wire(Decoupled(to))
    out.valid := tr.io.out.valid
    tr.io.out.ready := out.ready
    out.bits.packed := tr.io.out.bits
    out.bits.count := tr.io.cnt
    out
  }

}   


/**
 * This module takes a vector of objects of type T1 and convert it to a vector of objects of type T2. 
 * Vector sizes are parameters and can be arbitrary. It is possible for one input produce multiple outputs
 * One example this module can be used is to split a cacheline into user objects
 * Another example is to use this module to convert production rate to consumption rate in a SDF 
 */
class TypeRepackerVec[T1 <: Data, T2<: Data] (gen1 : T1, rate1 : Int, gen2 : T2, rate2 : Int) extends Module {
  val io = IO(new Bundle {
    val in  = Flipped(Decoupled(Vec(rate1, gen1.cloneType)))
    val out = Decoupled(Vec(rate2, gen2.cloneType))
  })
  
  val tr = Module(new TypeRepacker(Vec(rate1,gen1), Vec(rate2, gen2)))
  
  tr.io.flush := DontCare

  tr.io.in.valid <> io.in.valid
  tr.io.in.ready <> io.in.ready
  tr.io.in.bits := tr.io.in.bits.fromBits(io.in.bits.asUInt())
  
  io.out.valid <> tr.io.out.valid
  io.out.ready <> tr.io.out.ready
  io.out.bits := io.out.bits.fromBits(tr.io.out.bits.asUInt())
}

class TypeRepackerVecReg[T1 <: Data, T2<: Data] (gen1 : T1, rate1 : Int, gen2 : T2, rate2 : Int) extends Module {
  val io = IO(new Bundle {
    val in  = Flipped(Decoupled(Vec(rate1, gen1.cloneType)))
    val out = Decoupled(Vec(rate2, gen2.cloneType))
  })
  val tr = Module (new TypeRepackerVec(gen1, rate1, gen2, rate2))  
  
  tr.io.in <> io.in 
  
  io.out <> DecoupledStage(tr.io.out)
  
}
