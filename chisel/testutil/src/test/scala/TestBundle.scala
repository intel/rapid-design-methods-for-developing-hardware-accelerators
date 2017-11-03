// See LICENSE for license details.
package testutil

import chisel3._
import chisel3.util._
import chisel3.iotesters._
import org.scalatest.{Matchers, FlatSpec}

class ZeroSlackBundle extends Module {

  val bundle = new Bundle {
    val a = UInt(16.W)
    val b = new Bundle {
      val a = UInt(16.W)
      val b = UInt(16.W)
    }
  }

  val io = IO( new Bundle {
    val inp = Flipped(DecoupledIO(bundle))
    val out = DecoupledIO(bundle)
  })

  io.inp <> io.out

}

class ZeroSlackBundleTester( dut: ZeroSlackBundle) extends DecoupledStreamingTestsUsingAdvTester(dut) {

  val m0 = Map( "a" -> BigInt(0), "b.a" -> BigInt(1), "b.b" -> BigInt(2))
  val m1 = Map( "a" -> BigInt(10), "b.a" -> BigInt(11), "b.b" -> BigInt(12))
  val m2 = Map( "a" -> BigInt(20), "b.a" -> BigInt(21), "b.b" -> BigInt(22))

  val stream = List( m0, m1, m2)

  val inpStreams = List( (dut.io.inp, stream))
  val outStreams = List( (dut.io.out, stream))

  testStreams( dut, inpStreams, outStreams, 0, 100)
}

class ZeroSlackBundleTestVerilator extends ChiselFlatSpec {
  "ZeroSlackBundleTest" should "compile and run without incident" in {
    chisel3.iotesters.Driver( () => new ZeroSlackBundle, "verilator") { c =>
      new ZeroSlackBundleTester(c)
    } should be(true)
  }
}

class ZeroSlackBundleTestFirrtl extends ChiselFlatSpec {
  "ZeroSlackBundleTest" should "compile and run without incident" in {
    chisel3.iotesters.Driver( () => new ZeroSlackBundle, "firrtl") { c =>
      new ZeroSlackBundleTester(c)
    } should be(true)
  }
}
