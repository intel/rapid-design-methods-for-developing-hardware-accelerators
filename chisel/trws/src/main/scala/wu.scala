package trws

import chisel3._
import chisel3.util._

import designutils.{ DecoupledStage, MooreStage}

class wu extends Module {

  val io = IO(new Bundle {
    val start = Input( Bool())
    val done = Output( Bool())

    val gi = Flipped( DecoupledIO( USIMD()))
    val wi = Flipped( DecoupledIO( SSIMD()))
    val mi = Flipped( DecoupledIO( USIMD()))

    val mo = DecoupledIO( new USIMDWithFlush())
  })

  val splitter = Module( new Splitter)
  val frontend = Module( new Frontend)
  val backend = Module( new Backend)
  val accumulator = Module( new Accumulator)
  val passthrough = Module( new Passthrough)
  val clamper = Module( new Clamper)

  splitter.io.mi <> io.mi
  splitter.io.start := io.start

  accumulator.io.mi0 <> DecoupledStage( splitter.io.mi0)
  accumulator.io.start := io.start

  frontend.io.a0 <> DecoupledStage( accumulator.io.a0)
  frontend.io.mi1 <> Queue( splitter.io.mi1, 512, pipe=true)
  frontend.io.gi <> io.gi
  frontend.io.wi <> io.wi
  frontend.io.adj <> Queue( backend.io.adj, 2*max_cl_per_row)
  frontend.io.start := io.start

  passthrough.io.off <> DecoupledStage( frontend.io.off)
  passthrough.io.start := io.start

  clamper.io.out <> DecoupledStage( passthrough.io.res)
  clamper.io.start := io.start

  backend.io.res <> DecoupledStage( clamper.io.res)
  backend.io.start := io.start

  io.mo <> backend.io.mo
  io.done := backend.io.done

}

object wuDriver extends App {
  Driver.execute( args, () => new wu)
}
