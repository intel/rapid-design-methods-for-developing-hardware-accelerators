// See LICENSE for license details.
package testutil

import chisel3._
import chisel3.util._
import chisel3.iotesters._
import org.scalatest.{Matchers, FlatSpec}

class ZeroSlack extends Module {

  val io = IO( new Bundle {
    val inp = Flipped(DecoupledIO(UInt(16.W)))
    val out = DecoupledIO(UInt(16.W))
  })

  io.inp <> io.out

}

class ZeroSlackTester( dut: ZeroSlack) extends DecoupledStreamingTestsUsingAdvTester(dut) {

  val stream = List( BigInt(0), BigInt(1), BigInt(2))

  val inpStreams = List( (dut.io.inp, stream))
  val outStreams = List( (dut.io.out, stream))

  testStreams( dut, inpStreams, outStreams, 0, 100)
}

class ZeroSlackTestVerilator extends ChiselFlatSpec {
  "ZeroSlackTest" should "compile and run without incident" in {
    chisel3.iotesters.Driver( () => new ZeroSlack, "verilator") { c =>
      new ZeroSlackTester(c)
    } should be(true)
  }
}

/*
class ZeroSlackTestVCS extends ChiselFlatSpec {
  "ZeroSlackTest" should "compile and run without incident" in {
    chisel3.iotesters.Driver( () => new ZeroSlack, "vcs") { c =>
      new ZeroSlackTester(c)
    } should be(true)
  }
}
 */

class ZeroSlackTestFirrtl extends ChiselFlatSpec {
  "ZeroSlackTest" should "compile and run without incident" in {
    chisel3.iotesters.Driver( () => new ZeroSlack, "firrtl") { c =>
      new ZeroSlackTester(c)
    } should be(true)
  }
}
