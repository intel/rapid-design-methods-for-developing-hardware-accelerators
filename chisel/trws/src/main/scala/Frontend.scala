package trws

import chisel3._
import chisel3.util._

class Frontend extends Module {
  val io = IO(new Bundle {
    val start = Input( Bool())

    val gi = Flipped( DecoupledIO( USIMD()))
    val wi = Flipped( DecoupledIO( SSIMD()))
    val a0 = Flipped( DecoupledIO( USIMD()))
    val mi1 = Flipped( DecoupledIO( USIMD()))
    val adj = Flipped( DecoupledIO( USIMD()))

    val off = DecoupledIO( SSIMD())
  })


  val phase = RegInit( 0.U(1.W))
  val jj = RegInit( 0.U(log2_max_cl_per_row.W))

  val sc1 = new SliceCounter

  val gamma = Reg( Vec( max_cl_per_row, USIMD()))

  val localWeights = Reg( Vec( max_cl_per_row, SSIMD()))

  io.gi.nodeq
  io.wi.nodeq
  io.a0.nodeq
  io.mi1.nodeq
  io.adj.nodeq

  io.off.noenq

  when ( io.start) {
    when( phase === 0.U) {
      when ( io.gi.valid) {
        io.gi.ready := true.B
        gamma(jj) := io.gi.bits
        when ( jj =/= (cl_per_row-1).U) {
          jj := jj + 1.U
        } .otherwise {
          jj := 0.U
          phase := 1.U
        }
      }
    } .elsewhen ( phase === 1.U) {
//      printf( "io.off.ready,io.mi1.valid,io.a0.valid,sc1.first_s,io.wi.valid,io.adj.valid,sc1.s,sc1.t: %d,%d,%d,%d,%d,%d,%d,%d\n", io.off.ready, io.mi1.valid, io.a0.valid, sc1.first_s, io.wi.valid,io.adj.valid,sc1.s,sc1.t)

      when ( io.off.ready &&
             io.mi1.valid &&
             ( io.a0.valid || !sc1.first_s) &&
             ( io.wi.valid || !sc1.first_s) &&
             ( io.adj.valid || !sc1.first_s || sc1.t === 0.U)) {
        val localNS1 = Wire( SSIMD())
        when( sc1.first_s) {
          val localNS0 = Wire( SSIMD())
          io.a0.ready := true.B
          io.wi.ready := true.B
          for ( j<- 0 until elements_per_cl) {
            localNS0(j) := io.a0.bits(j).asSInt + io.wi.bits(j)
          }
          when( sc1.t =/= 0.U) {
            io.adj.ready := true.B
            for ( j<- 0 until elements_per_cl) {
              localNS1(j) := localNS0(j) + io.adj.bits(j).asSInt
            }
          } .otherwise {
            localNS1 := localNS0
          }
          localWeights(sc1.jj) := localNS1
        } .otherwise {
          localNS1 := localWeights(sc1.jj)
        }

        io.off.valid := true.B
        io.mi1.ready := true.B
        val hi = sc1.s>>log2_elements_per_cl
        val lo = sc1.s&(elements_per_cl-1).U
        val g = gamma(hi)(lo).asSInt
        for ( j<- 0 until elements_per_cl) {
          io.off.bits(j) := ((g * localNS1(j))>>radixPoint) - io.mi1.bits(j).asSInt
        }
        sc1.incr
      }
    }
  }


}

object FrontendDriver extends App {
  Driver.execute( args, () => new Frontend)
}
