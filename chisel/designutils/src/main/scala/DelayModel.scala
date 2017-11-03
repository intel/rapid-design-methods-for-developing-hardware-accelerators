// See LICENSE for license details.
package designutils
import chisel3._
import chisel3.util._


class DelayModel[T <: Data] (gen : T, lat: Int, ii : Int) extends Module {
  val io = IO(new Bundle {
    val in = Flipped(Decoupled(gen))
    val out = Decoupled(gen)
  })
  val iiCnt = RegInit((ii-1).U.cloneType, init=0.U)
  val latPipe = Module(new LatencyPipe(gen, lat))
  
  
  io.in.nodeq
  latPipe.io.in.noenq
  
  when(io.in.fire) {
    iiCnt := (ii - 1).U
  }.otherwise {
    when(iiCnt != 0.U) {
      iiCnt := iiCnt - 1.U
    }
  }
  
  when(iiCnt === 0.U && latPipe.io.in.ready) {
    val d = io.in.deq
    when (io.in.valid) {
      latPipe.io.in.enq(d)
    }
  }
  
  io.out <> latPipe.io.out
}

object DelayModel {
  def apply[T <: Data](in: DecoupledIO[T], lat: Int, ii : Int): DecoupledIO[T] = {
    val dm = Module(new DelayModel(in.bits, lat, ii))
    dm.io.in <> in
    dm.io.out
  }
}

