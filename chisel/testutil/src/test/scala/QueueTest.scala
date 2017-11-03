// See LICENSE for license details.
package testutil

import chisel3._
import chisel3.util._
import chisel3.iotesters._
import org.scalatest.{Matchers, FlatSpec}

class QueueSlack( sz : Int = 1, pipe : Boolean = false) extends Module {

  val io = IO( new Bundle {
    val inp = Flipped(DecoupledIO(UInt(16.W)))
    val out = DecoupledIO(UInt(16.W))
  })

  val q = Module( new Queue( UInt(16.W), sz, pipe=pipe))

  io.inp <> q.io.enq
  q.io.deq <> io.out

}

class QueueSlackTester( dut: QueueSlack) extends DecoupledStreamingTestsUsingAdvTester(dut) {

  val stream = List( BigInt(0), BigInt(1), BigInt(2))

  val inpStreams = List( (dut.io.inp, stream))
  val outStreams = List( (dut.io.out, stream))

  testStreams( dut, inpStreams, outStreams, 0, 100)
}

class QueueSlackTestVerilator extends ChiselFlatSpec {
  "QueueSlackTest" should "compile and run without incident" in {
    chisel3.iotesters.Driver( () => new QueueSlack, "verilator") { c =>
      new QueueSlackTester(c)
    } should be(true)
  }
}

class QueueSlackTestFirrtl extends ChiselFlatSpec {
  "QueueSlackTest" should "compile and run without incident" in {
    chisel3.iotesters.Driver( () => new QueueSlack, "firrtl") { c =>
      new QueueSlackTester(c)
    } should be(true)
  }
}

class QueueSlackPipeTestVerilator extends ChiselFlatSpec {
  "QueueSlackPipeTest" should "compile and run without incident" in {
    chisel3.iotesters.Driver( () => new QueueSlack( 1, true), "verilator") { c =>
      new QueueSlackTester(c)
    } should be(true)
  }
}

class QueueSlack2TestVerilator extends ChiselFlatSpec {
  "QueueSlack2Test" should "compile and run without incident" in {
    chisel3.iotesters.Driver( () => new QueueSlack( 2), "verilator") { c =>
      new QueueSlackTester(c)
    } should be(true)
  }
}

class QueueSlack2PipeTestVerilator extends ChiselFlatSpec {
  "QueueSlack2PipeTest" should "compile and run without incident" in {
    chisel3.iotesters.Driver( () => new QueueSlack( 2, true), "verilator") { c =>
      new QueueSlackTester(c)
    } should be(true)
  }
}
