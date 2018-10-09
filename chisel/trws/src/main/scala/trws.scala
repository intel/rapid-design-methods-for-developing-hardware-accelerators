package trws

import chisel3._
import chisel3.util._

import designutils.{ DecoupledStage, MooreStage}

class trws extends Module {

  val io = IO(new Bundle {
    val start = Input( Bool())
    val done = Output( Bool())

    val modeLoad = Input( Bool())
    val modeCompute = Input( Bool())

    val loadIdx = Input( UInt(log2_ncontexts.W))
    val computeIdx = Input( UInt(log2_ncontexts.W))

    val gi = Flipped( DecoupledIO( USIMD()))
    val wi = Flipped( DecoupledIO( SSIMD()))
    val mi = Flipped( DecoupledIO( USIMD()))

    val slc = Flipped( DecoupledIO( new Pair))
    val lof = Flipped( DecoupledIO( new Pair))

    val mo = DecoupledIO( new USIMDWithFlush)
  })

  val splitter = Module( new Splitter)
  val frontend = Module( new Frontend)
  val backend = Module( new Backend)
  val accumulator = Module( new Accumulator)

  val loaf = Module( new Loaf)
  val clamper = Module( new Clamper)

  splitter.io.mi <> io.mi
  splitter.io.start := io.start

  accumulator.io.mi0 <> DecoupledStage( splitter.io.mi0)
  accumulator.io.start := io.start

  frontend.io.a0 <> DecoupledStage( accumulator.io.a0)
  frontend.io.mi1 <> Queue( splitter.io.mi1, max_cl_per_row*max_cl_per_row*elements_per_cl, pipe=true)
  frontend.io.gi <> io.gi
  frontend.io.wi <> io.wi
  frontend.io.adj <> Queue( backend.io.adj, max_cl_per_row)
  frontend.io.start := io.start

  loaf.io.off <> DecoupledStage( frontend.io.off)
  loaf.io.slc <> io.slc
  loaf.io.lof <> io.lof

  loaf.io.start := io.start
  loaf.io.modeLoad := io.modeLoad
  loaf.io.modeCompute := io.modeCompute
  loaf.io.loadIdx := io.loadIdx
  loaf.io.computeIdx := io.computeIdx

  clamper.io.out <> DecoupledStage( loaf.io.out)
  clamper.io.start := io.start

  backend.io.res <> DecoupledStage( clamper.io.res)
  backend.io.start := io.start

  io.mo <> backend.io.mo
  io.done := ( !io.modeCompute || backend.io.done) && ( !io.modeLoad || loaf.io.doneLoading)

}

object trwsDriver extends App {
  Driver.execute( args, () => new trws)
}
