// See LICENSE for license details.
package pipe_it

import chisel3._
import chisel3.util._
import pipe_insert_transform._
import chisel3.core.ChiselAnnotation
import firrtl.transforms.Flatten

class PipeIf[INT <: Data, OUTT <: Data, CONFT <: Data] (inGen : INT, outGen : OUTT, confGen : CONFT) extends Module {
  val io = IO(new Bundle {
    val config = Input(confGen.cloneType)
    val in = Input(inGen.cloneType)
    val out = Output(outGen.cloneType)
  })
}

class StallIt[INT <: Data, OUTT <: Data, CONFT <: Data] (latency : Int, clockPeriod : Int, modFac: () => PipeIf[INT, OUTT, CONFT]) extends Module with HldRetimer {
  val mod = Module(modFac())
  val io = IO(new Bundle {
		val enable = Input(Bool())
		val config = Input(mod.io.config.cloneType)
		val in = Input(mod.io.in.cloneType)
	  val out = Output(mod.io.out.cloneType)
	})
	io.in <> mod.io.in
	io.config <> mod.io.config
	
	if (latency == 0) {
    io.out <> mod.io.out
	} else {
    io.out <> ShiftRegister(mod.io.out, latency, io.enable)
    annotate(ChiselAnnotation(this, classOf[Flatten], ""))
    //annotate(ChiselAnnotation(this, classOf[ReportTiming], ""))
    hld_retime(this, clockPeriod, false)
	}
}
class PipeIt[INT <: Data, OUTT <: Data, CONFT <: Data] (latency : Int, clockPeriod : Int, modFac: () => PipeIf[INT, OUTT, CONFT], confGen: CONFT) extends Module {
  val stallableMod = Module(new StallIt(latency, clockPeriod, modFac))
  val io = IO(new Bundle {
    val config = Input(confGen.cloneType)
		val in = Flipped(Decoupled(stallableMod.io.in.cloneType))
	  val out = Decoupled(stallableMod.io.out.cloneType)
	})
	
	stallableMod.io.enable := io.out.ready
	stallableMod.io.config := io.config
	io.in.bits <> stallableMod.io.in
  io.out.bits <> stallableMod.io.out

  io.out.valid <> ShiftRegister(io.in.valid, latency, false.B, io.out.ready)
  io.in.ready <> io.out.ready
}

