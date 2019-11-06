// See LICENSE for license details.
package testutil

import chisel3._
import chisel3.util._
import chisel3.iotesters._
import org.scalatest.{Matchers, FlatSpec}

class SlowDecoupledAdderIn extends Bundle {
  val a = UInt(16.W)
  val b = UInt(16.W)
}

class SlowDecoupledAdderOut extends Bundle {
  val c = Output(UInt(16.W))
}

class SlowDecoupledAdder extends Module {
  val delay_value = 10
  val io = IO(new Bundle {
    val in  = Flipped(Decoupled(new SlowDecoupledAdderIn))
    val out = Decoupled(new SlowDecoupledAdderOut)
  })
  val busy    = RegInit(false.B)
  val a_reg   = RegInit(0.U(16.W))
  val b_reg   = RegInit(0.U(16.W))
  val wait_counter = RegInit(0.U(16.W))

  io.in.ready := !busy

  /*printf("in: ready %d   valid %d   a %d b %d   -- out:  ready %d  valid %d  c %d",
         io.in.ready, io.in.valid, io.in.bits.a, io.in.bits.b,
         io.out.ready, io.out.valid, io.out.bits.c)*/

  when(io.in.valid && !busy) {
    a_reg        := io.in.bits.a
    b_reg        := io.in.bits.b
    busy         := true.B
    wait_counter := 0.U
  }
  io.out.bits.c := DontCare
  when(busy) {
    when(wait_counter > delay_value.asUInt) {
      io.out.bits.c := a_reg + b_reg
    }.otherwise {
      wait_counter := wait_counter + 1.U
    }
  }

  io.out.valid := (io.out.bits.c === a_reg + b_reg ) && busy

  when(io.out.valid) {
    busy          := false.B
  }
}


class SlowDecoupledAdderTester( dut: SlowDecoupledAdder) extends DecoupledStreamingTestsUsingAdvTester(dut) {


  val operands0 = Map( "a" -> BigInt(1), "b" -> BigInt(2))
  val operands1 = Map( "a" -> BigInt(3), "b" -> BigInt(4))
  val operands2 = Map( "a" -> BigInt(5), "b" -> BigInt(6))
  val operandsStream = List(operands0,operands1,operands2)
  val resultsStream = List( Map("c" -> BigInt(3)), Map("c" -> BigInt(7)), Map("c" -> BigInt(11)))

  val inpStreams = List( (dut.io.in, operandsStream))
  val outStreams = List( (dut.io.out, resultsStream))

  testStreams( dut, inpStreams, outStreams, 0, 100)
}

class SlowDecoupledAdderTestVerilator extends ChiselFlatSpec {
  "SlowDecoupledAdderTest" should "compile and run without incident" in {
    chisel3.iotesters.Driver( () => new SlowDecoupledAdder, "verilator") { c =>
      new SlowDecoupledAdderTester(c)
    } should be(true)
  }
}

/*
class SlowDecoupledAdderTestVCS extends ChiselFlatSpec {
  "SlowDecoupledAdderTest" should "compile and run without incident" in {
    chisel3.iotesters.Driver( () => new SlowDecoupledAdder, "vcs") { c =>
      new SlowDecoupledAdderTester(c)
    } should be(true)
  }
}
 */

class SlowDecoupledAdderTestFirrtl extends ChiselFlatSpec {
  "SlowDecoupledAdderTest" should "compile and run without incident" in {
    chisel3.iotesters.Driver( () => new SlowDecoupledAdder, "firrtl") { c =>
      new SlowDecoupledAdderTester(c)
    } should be(true)
  }
}
