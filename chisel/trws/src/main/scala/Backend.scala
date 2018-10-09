package trws

import chisel3._
import chisel3.util._

import accio._

class USIMDWithFlush extends Bundle {
  val bits = USIMD()
  val flush = Bool()
  override def cloneType = (new USIMDWithFlush).asInstanceOf[this.type]
}

class Backend extends Module {
  val io = IO(new Bundle {
    val start = Input( Bool())
    val done = Output( Bool())

    val res = Flipped( DecoupledIO( USIMD()))

    val mo = DecoupledIO( new USIMDWithFlush)
//    val mo = DecoupledIO( USIMD())
    val adj = DecoupledIO( USIMD())
  })

  val sc2 = new SliceCounter

  val sz = 1<<(log2_max_cl_per_row+log2_elements_per_cl+log2_max_cl_per_row)

  assert( sz == max_n*max_n/elements_per_cl)

  val weights = Mem( sz, UInt( bits_per_cl.W))

  io.done := sc2.done

  io.res.nodeq

  io.mo.noenq
  io.adj.noenq

  io.mo.bits.flush := false.B

  when ( io.start && !io.done) {

//    printf("io.res.valid,io.mo.ready,sc2.s,sc2.t,io.adj.ready: %d,%d,%d,%d,%d\n", io.res.valid,io.mo.ready,sc2.s,sc2.t,io.adj.ready)

    when( io.res.valid &&
          io.mo.ready &&
          ( sc2.s-1.U =/= sc2.t || io.adj.ready)) {
      io.res.ready := true.B

      val ns = Wire(USIMD())

      val idx = (sc2.s<<log2_max_cl_per_row)+sc2.jj
      when ( sc2.t === 0.U) {
        ns := io.res.bits
      } .otherwise {
        val tmp = weights.read(idx).asTypeOf(USIMD())
        for ( j<-0 until elements_per_cl) {
          ns(j) := tmp(j) + io.res.bits(j)
        }
      }

      when ( sc2.s-1.U === sc2.t) {
        io.adj.valid := true.B
        io.adj.bits := ns
      }

      weights.write(idx,ns.asUInt)

      io.mo.valid := true.B
//      io.mo.bits := io.res.bits
      io.mo.bits.bits := io.res.bits
      io.mo.bits.flush := sc2.doneWire

      assert( sc2.doneWire === sc2.lastMessage)

      sc2.incr

//      printf( p"sc2.jj: ${sc2.jj} sc2.s: ${sc2.s} sc2.t: ${sc2.t} sc2.doneWire: ${sc2.doneWire}\n")


    }
  }

}

object BackendDriver extends App {
  Driver.execute( args, () => new Backend)
}
