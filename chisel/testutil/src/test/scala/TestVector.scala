// See LICENSE for license details.
package testutil

import chisel3._
import chisel3.util._
import chisel3.iotesters._
import org.scalatest.{Matchers, FlatSpec}

class ZeroSlackVector extends Module {

  val vector = Vec(4,UInt(16.W))

  val io = IO( new Bundle {
    val inp = Flipped(DecoupledIO(vector))
    val out = DecoupledIO(vector)
  })

  io.inp <> io.out

}

class ZeroSlackVectorTester( dut: ZeroSlackVector) extends DecoupledStreamingTestsUsingAdvTester(dut) {

  val m0 = IndexedSeq( BigInt(0),  BigInt(1),  BigInt(2),  BigInt(3))
  val m1 = IndexedSeq( BigInt(10), BigInt(11), BigInt(12), BigInt(13))
  val m2 = IndexedSeq( BigInt(20), BigInt(21), BigInt(22), BigInt(23))

  val stream = List( m0, m1, m2)

  val inpStreams = List( (dut.io.inp, stream))
  val outStreams = List( (dut.io.out, stream))

  testStreams( dut, inpStreams, outStreams, 0, 100)
}

class ZeroSlackVectorTestVerilator extends ChiselFlatSpec {
  "ZeroSlackVectorTest" should "compile and run without incident" in {
    chisel3.iotesters.Driver( () => new ZeroSlackVector, "verilator") { c =>
      new ZeroSlackVectorTester(c)
    } should be(true)
  }
}

class ZeroSlackVectorTestFirrtl extends ChiselFlatSpec {
  "ZeroSlackVectorTest" should "compile and run without incident" in {
    chisel3.iotesters.Driver( () => new ZeroSlackVector, "firrtl") { c =>
      new ZeroSlackVectorTester(c)
    } should be(true)
  }
}
