// See LICENSE for license details.
package pipe_insert_transform


import org.scalatest.{ Matchers, FlatSpec, GivenWhenThen}

// import scala.util.control.Breaks

import scala.annotation.tailrec


import chisel3._
import chisel3.experimental.ChiselAnnotation
import chisel3.internal.InstanceId
import chisel3.testers.BasicTester
import firrtl.{CircuitForm, CircuitState, LowForm, Transform}
import firrtl.annotations.{Annotation, ModuleName, Named}
import org.scalatest._
import chisel3.iotesters.PeekPokeTester
import chisel3.iotesters.ChiselFlatSpec
import chisel3.util.Decoupled
import chisel3.util.ShiftRegister


class PipeModDecoupledGold(widthC: Int)  extends Module  {
  val io = IO(new Bundle {
    val in = Flipped(Decoupled(UInt(widthC.W)))
    val out = Decoupled(UInt((widthC+1).W))
  })
  
  io.in.nodeq()
  io.out.noenq()
  
  val next_data1 = Reg(io.in.bits.cloneType) 
  val next_valid1 = RegInit(init = false.B) 
  val next_data2 = Reg(io.in.bits.cloneType) 
  val next_valid2 = RegInit(init = false.B) 
  //val next_data = io.in.bits + 1.U 
  //val next_valid = io.in.valid 
  
  when(io.out.ready) {
    //val tmp = Wire(init = io.in.bits + 1.U)
    next_data1 := io.in.bits + 2.U
    next_valid1 := io.in.valid
    
    next_data2 := next_data1
    next_valid2 := next_valid1
  }
  
  io.out.ready <> io.in.ready 

  io.out.bits := next_data2
  io.out.valid := next_valid2
}
class PipeModDecoupledToRetime(widthC: Int) extends Module with HldRetimer {
  val io = IO(new Bundle {
    val in = Flipped(Decoupled(UInt(widthC.W)))
    val out = Decoupled(UInt((widthC+1).W))
  })
  
  io.in.nodeq()
  io.out.noenq()
  
  val next_data = Reg(io.in.bits.cloneType) 
  val next_valid = RegInit(init = false.B) 
  val next_data2 = Reg(io.in.bits.cloneType) 
  val next_valid2 = RegInit(init = false.B) 
  //val next_data = io.in.bits + 1.U 
  //val next_valid = io.in.valid 
  
  when(io.out.ready) {
    //val tmp = Wire(init = io.in.bits + 1.U)
    next_data := io.in.bits + 2.U
    next_valid := io.in.valid
    
    next_data2 := next_data
    next_valid2 := next_valid
  }
  
  io.out.ready <> io.in.ready 

  io.out.bits := next_data2
  io.out.valid := next_valid2  
  
  hld_retime(this, 20, false)
  //insertBefore(io.out.valid, s"io_out_valid", s"io_out_ready")
}

class PipeModSRToRetime(widthC: Int) extends Module with HldRetimer {
  val io = IO(new Bundle {
    val in = Flipped(Decoupled(UInt(widthC.W)))
    val out = Decoupled(UInt((widthC+1).W))
  })
  
  io.in.nodeq()
  io.out.noenq()
  
  
  val w1 = Wire(init = io.in.bits + 2.U) 
  val fifo_out = ShiftRegister(w1, 4, io.out.ready)
  io.out.valid := ShiftRegister(io.in.valid, 4, false.B, io.out.ready)
  
  io.out.bits := fifo_out
  
  io.out.ready <> io.in.ready 

  hld_retime(this, 3, false)
  //insertBefore(io.out.valid, s"io_out_valid", s"io_out_ready")
}

class TestWithGolden(widthC: Int) extends Module {
  val io = IO (new Bundle {
    val in = Flipped(Decoupled(UInt(widthC.W)))
    val out_dut = Decoupled(UInt((widthC+1).W))
    val out_gold = Decoupled(UInt((widthC+1).W))
    val out_comp = Output(Bool())
  })
  
  //val dut = Module (new PipeModDecoupledToRetime(widthC))
  val dut = Module (new PipeModSRToRetime(widthC))
  val golden = Module (new PipeModDecoupledGold(widthC))
  
  dut.io.in <> io.in
  golden.io.in <> io.in
  
  dut.io.out <> io.out_dut
  golden.io.out <> io.out_gold
  
  io.out_comp := true.B
  when(dut.io.out.valid === golden.io.out.valid && dut.io.out.bits != golden.io.out.bits) {
    io.out_comp := false.B
  }
}

class PipeInsertTester(mod : TestWithGolden) extends PeekPokeTester(mod) {
  
  //val dut = Module(new PipeMod(10))
  val bits = scala.util.Random
  for (i <- 0 until 1000) {
    poke(mod.io.in.bits, bits.nextInt(100))
    poke(mod.io.in.valid, bits.nextInt(2))
    val backpressure = bits.nextInt(2)
    poke(mod.io.out_dut.ready, backpressure)
    poke(mod.io.out_gold.ready, backpressure)
    step(1)
  }
  
}
class PipeInsertTesterDutOnly(mod : PipeModDecoupledToRetime) extends PeekPokeTester(mod) {
  
  //val dut = Module(new PipeMod(10))
  poke(mod.io.in.valid, true.B)
  poke(mod.io.out.ready, true.B)
  poke(mod.io.in.bits, 1.U)
  step(1)
  step(1)
  expect(mod.io.out.valid,true.B)
  expect(mod.io.out.bits,3.U)

  poke(mod.io.in.valid, true.B)
  poke(mod.io.out.ready, false.B)
  poke(mod.io.in.bits, 4.U)
  step(1)
  step(1)
  expect(mod.io.out.valid,true.B)
  expect(mod.io.out.bits,3.U)
  poke(mod.io.in.bits, 4.U)
  poke(mod.io.out.ready, true.B)
  step(1)
  step(1)
  expect(mod.io.out.valid,true.B)
  expect(mod.io.out.bits,6.U)


}


class PipeInsertTestSpec extends ChiselFlatSpec with Matchers {
//	"fdsafasd" should "run" in {
//
//		Driver.execute(Array("--target-dir", "test_run_dir"), () => new PipeModDecoupled(10)) match {
//		case ChiselExecutionSuccess(Some(circuit), emitted, _) =>
//  		val annos = circuit.annotations
//  		//annos.length should be (1)
//		case _ =>
//  		assert(false)
//		}
//	}
	"A simple test" should "run" in {
		chisel3.iotesters.Driver.execute(Array("--target-dir", "test_run_dir", "--backend-name", "firrtl"), () => new TestWithGolden(10)) {
		  c => new PipeInsertTester(c)
	  } should be(true)
	}
}
