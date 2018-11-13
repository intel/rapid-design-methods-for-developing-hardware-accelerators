// See LICENSE for license details.
//
package designutils
import chisel3._
import chisel3.util._


class DecoupledStage[T <: Data](gen: T) extends Module {
  val io = IO(new Bundle { 
    val inp = Flipped(Decoupled( gen ))
    val out = Decoupled( gen )
  })

  val out_valid = RegInit( init=false.B )
  val out_bits = Reg( gen )

  io.out.valid := out_valid
  io.out.bits  := out_bits

  io.inp.ready := io.out.ready | ~io.out.valid

  out_valid := io.inp.valid | ~io.inp.ready
  when ( io.inp.ready) {
     out_bits := io.inp.bits
  }
}

object DecoupledStage {
  def apply[T <: Data](enq: DecoupledIO[T]): DecoupledIO[T]  = {
    val q = Module(new DecoupledStage(enq.bits.cloneType))
    q.io.inp.valid := enq.valid // not using <> so that override is allowed
    q.io.inp.bits := enq.bits
    enq.ready := q.io.inp.ready
    q.io.out
  }
}

object DecoupledStageInv {
  def apply[T <: Data](enq: DecoupledIO[T]): DecoupledIO[T]  = {
    val q = Module(new DecoupledStage(enq.bits.cloneType))
    q.io.inp.valid := enq.valid // not using <> so that override is allowed
    q.io.inp.bits := (~enq.bits.asUInt).asTypeOf(q.io.inp.bits)
    enq.ready := q.io.inp.ready
    q.io.out
  }
}

class MooreStage[T <: Data](gen: T) extends Module {
  val io = IO(new Bundle { 
    val inp = Flipped(Decoupled( gen ))
    val out = Decoupled( gen )
  })

  val data_aux = Reg( gen.cloneType)
  val out_bits = Reg( gen.cloneType)

  val out_valid = RegInit( false.B) 
  val inp_ready = RegInit( true.B) 

  io.inp.ready := inp_ready
  io.out.valid := out_valid
  io.out.bits  := out_bits

  when        (  inp_ready && !out_valid) {
    when ( io.inp.valid) {
      inp_ready := true.B; out_valid := true.B;
      out_bits := io.inp.bits;
    }
  } .elsewhen (  inp_ready &&  out_valid) {
    when        ( !io.inp.valid && io.out.ready) {
      inp_ready := true.B; out_valid := false.B;
    } .elsewhen (  io.inp.valid && io.out.ready) {
      inp_ready := true.B; out_valid := true.B;
	  out_bits := io.inp.bits;
    } .elsewhen (  io.inp.valid && !io.out.ready) {
      inp_ready := false.B; out_valid := true.B;
      data_aux := io.inp.bits;
    }
  } .elsewhen ( !inp_ready && out_valid) {
    when ( io.out.ready) {
      inp_ready := true.B; out_valid := true.B;
	  out_bits := data_aux;
    }
  }
}

object MooreStage {
  def apply[T <: Data](enq: DecoupledIO[T]): DecoupledIO[T]  = {
    val q = Module(new MooreStage(enq.bits.cloneType))
    q.io.inp.valid := enq.valid // not using <> so that override is allowed
    q.io.inp.bits := enq.bits
    enq.ready := q.io.inp.ready
    q.io.out
  }
}
