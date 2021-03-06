package imperative

import org.scalatest.{ Matchers, FlatSpec, GivenWhenThen}

import chisel3._
import chisel3.util._
import chisel3.iotesters._

import lexer._
import compiler._

class VectorAdd2 extends ImperativeModule( 
  Compiler.run(
    """
      |process VectorAdd2 ( A : inp Vec(2,UInt(8)),
      |                     B : inp Vec(2,UInt(8)),
      |                     O : out Vec(2,UInt(8)))
      |{
      |  while ( true) {
      |    if ( A? && B? && O!) {
      |      var a : Vec(2,UInt(8))
      |      var b : Vec(2,UInt(8))
      |      var o : Vec(2,UInt(8))
      |      A?a
      |      B?b
      |      o(0) = a(0) + b(0)
      |      o(1) = a(1) + b(1)
      |      O!o
      |    }
      |    wait
      |  }
      |}
    """.stripMargin.trim)
)


class VectorAdd2Tester(c:VectorAdd2) extends PeekPokeTester(c) {
  poke( c.io("O").ready, 1)

  poke( c.io("A").valid, 0)
  poke( c.io("B").valid, 0)

  expect( c.io("A").ready, 0) // Mealy
  expect( c.io("B").ready, 0) // Mealy

  expect( c.io("O").valid, 0) // Mealy

  step(1)

  poke( c.io("A").valid, 1)
  poke( c.io("B").valid, 1)
  poke( c.io("A").bits.asInstanceOf[Vec[UInt]], IndexedSeq[BigInt](1,10))
  poke( c.io("B").bits.asInstanceOf[Vec[UInt]], IndexedSeq[BigInt](7,2))

  expect( c.io("A").ready, 1) // Mealy
  expect( c.io("B").ready, 1) // Mealy

  expect( c.io("O").valid, 1) // Mealy
  expect( c.io("O").bits.asInstanceOf[Vec[UInt]], IndexedSeq[BigInt](8,12)) // Mealy

  step(1)
}

class VectorAdd2Test extends FlatSpec with Matchers {
  behavior of "VectorAdd2"
  it should "work" in {
    chisel3.iotesters.Driver( () => new VectorAdd2, "firrtl") { c =>
      new VectorAdd2Tester( c)
    } should be ( true)
  }
}
