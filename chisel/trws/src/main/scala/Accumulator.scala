package trws

import chisel3._
import chisel3.util._

class Accumulator extends Module {
  val io = IO(new Bundle {
    val start = Input( Bool())

    val mi0 = Flipped( DecoupledIO( USIMD()))
    val a0 = DecoupledIO( USIMD())
  })

  val sc0 = new SliceCounter

  val localWeights = Reg( Vec( max_cl_per_row, USIMD()))
  val nsWeights = WireInit( USIMD(), init=DontCare)

  io.mi0.nodeq
  io.a0.noenq

  when ( io.mi0.valid && (io.a0.ready || sc0.s =/= (sc0.n-1).U)) {
    io.mi0.ready := true.B
    for ( j <- 0 until elements_per_cl) {
      when ( sc0.first_s) {
        nsWeights(j) := io.mi0.bits(j)
      } .otherwise {
        nsWeights(j) := localWeights(sc0.jj)(j) + io.mi0.bits(j)
      }
    }

    when ( sc0.s === (sc0.n-1).U) {
      io.a0.valid := true.B
      io.a0.bits := nsWeights
    } .otherwise {
      localWeights(sc0.jj) := nsWeights
    }

    sc0.incr
  }

}

object AccumulatorDriver extends App {
  Driver.execute( args, () => new Accumulator)
}

