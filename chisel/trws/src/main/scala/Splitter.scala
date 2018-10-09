package trws

import chisel3._
import chisel3.util._

class Splitter extends Module {
  val io = IO(new Bundle {
    val start = Input( Bool())

    val mi = Flipped( DecoupledIO( USIMD()))
    val mi0 = DecoupledIO( USIMD())
    val mi1 = DecoupledIO( USIMD())
  })

  io.mi.nodeq
  io.mi0.noenq
  io.mi1.noenq

  when ( io.mi.valid && io.mi0.ready && io.mi1.ready) {
    io.mi.ready := true.B
    io.mi0.valid := true.B
    io.mi1.valid := true.B
    io.mi0.bits := io.mi.bits
    io.mi1.bits := io.mi.bits
  }

}

object SplitterDriver extends App {
  Driver.execute( args, () => new Splitter)
}
