package imperative

import org.scalatest.{ Matchers, FlatSpec, GivenWhenThen}

import chisel3._
import chisel3.util._
import chisel3.iotesters._

class VecCopy extends Module {

  val io = IO(new Bundle {
    val outv = Output(Vec(2,UInt(8.W)))
    val outc = Output(Vec(2,UInt(8.W)))
  })

  val v = Wire(Vec(2,UInt(8.W)))
  v(0) := 0.U
  v(1) := 1.U

  val u = 47.U + 0.U
//  val c = v.updated[UInt,Vec[UInt]]( 0, u)
  val c : IndexedSeq[UInt] = v.updated( 0, u)

  println( s"v: ${v} c: ${c} u: ${u}")


  val vv = Wire(Vec(2,UInt(8.W)), init=Vec(c))

  io.outv := v
  io.outc := vv

}


class VecCopyTester(c:VecCopy) extends PeekPokeTester(c) {
  expect( c.io.outv, IndexedSeq( BigInt(1), BigInt(0)))
  expect( c.io.outc, IndexedSeq( BigInt(1), BigInt(47)))
}

class VecCopyTest extends FlatSpec with Matchers {
  behavior of "VecCopy"
  it should "work" in {
    chisel3.iotesters.Driver( () => new VecCopy, "firrtl") { c =>
      new VecCopyTester( c)
    } should be ( true)
  }
}
