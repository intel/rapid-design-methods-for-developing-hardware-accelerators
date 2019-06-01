package imperative

import chisel3._
import chisel3.util._

class EModule extends Module {
  val n = 2
  val w = 8

  val proto = Vec(n,UInt(w.W))

  val iod_tuples = List(
    ("P", Flipped(Decoupled(proto)),VecType(n,UIntType(w))),
    ("Q", Decoupled(UInt(8.W)),UIntType(w))
  )

  val io = IO( new CustomDecoupledBundle( iod_tuples: _*))

  io("P").ready := io("Q").ready

  io("Q").valid := io("P").valid
  io("Q").bits  := io("P").bits.asInstanceOf[Vec[UInt]](0.U) + io("P").bits.asInstanceOf[Vec[UInt]](1.U)
}

object EModuleDriver extends App {
  Driver.execute( Array[String](), () => new EModule)
}
