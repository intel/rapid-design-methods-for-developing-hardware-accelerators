package imperative

import org.scalatest.{ Matchers, FlatSpec, GivenWhenThen}

import chisel3._
import chisel3.util._
import chisel3.iotesters._

import lexer._
import compiler._

class Vector extends ImperativeModule( 
  Compiler.run(
    """
      |process Vector ( A : inp Vec(2,UInt(8)),
      |                 O : out UInt(8))
      |{
      |  while ( true) {
      |    if ( A? && O!) {
      |      var a : Vec(2,UInt(8))
      |      var o : UInt(8)
      |      A?a
      |      o = a(0) + a(1)
      |      o = a(0) + o
      |      o = a(1) + o
      |      O!o
      |    }
      |    wait
      |  }
      |}
    """.stripMargin.trim)
)


class VectorTester(c:Vector) extends PeekPokeTester(c) {
  poke( c.io("O").ready, 1)

  poke( c.io("A").valid, 0)

  expect( c.io("A").ready, 0) // Mealy

  step(1)

  expect( c.io("O").valid, 0) // Moore

  poke( c.io("A").valid, 1)
  poke( c.io("A").bits.asInstanceOf[Vec[UInt]], IndexedSeq[BigInt](1,10))

  expect( c.io("O").ready, 1) // Mealy

  step(1)

  expect( c.io("O").valid, 1) // Moore
  expect( c.io("O").bits.asInstanceOf[UInt], 22)  // Moore

}

class VectorTest extends FlatSpec with Matchers {
  behavior of "Vector"
  it should "work" in {
    chisel3.iotesters.Driver( () => new Vector, "firrtl") { c =>
      new VectorTester( c)
    } should be ( true)
  }
}
