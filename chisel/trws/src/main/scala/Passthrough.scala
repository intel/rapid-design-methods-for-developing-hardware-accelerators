package trws

import chisel3._
import chisel3.util._
import designutils.{ LatencyPipe}

class Passthrough extends Module {
  val io = IO(new Bundle {
    val start = Input( Bool())

    val off = Flipped( DecoupledIO( SSIMD()))
    val res = DecoupledIO( SSIMD())
  })

/*
  val r = RegInit( 0.U(log2_max_cl_per_row.W))
  val c = RegInit( 0.U(log2_max_cl_per_row.W))

  io.res.nodeq
  io.off.noenq

  when ( io.start) {

    when ( io.res.valid && io.off.ready) {

      when ( c === 0.U) {
        io.off.ready
      }



      when ( r =/= (cl_per_row-1).U) {
        r := r + 1.U
      } .otherwise {
        r := 0.U
        when ( c =/= (cl_per_row-1).U) {
          c := c + 1.U
        } .otherwise {
          c := 0.U
        }
      }
    }


  }
 */

  io.res <> LatencyPipe( io.off, 4*4 + 14 + 100)
}

object PassthroughDriver extends App {
  Driver.execute( args, () => new Passthrough)
}
