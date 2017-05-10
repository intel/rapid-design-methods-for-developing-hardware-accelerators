package imperative

import org.scalatest.{ Matchers, FlatSpec, GivenWhenThen}

import chisel3._
import chisel3.util._
import chisel3.iotesters._

import lexer._
import compiler._

class Vector2 extends ImperativeModule( 
  Compiler.run(
    """
      |process Vector2 ( A : inp Vec(2,UInt(8)),
      |                  O : out Vec(2,UInt(8)))
      |{
      |  while ( true) {
      |    if ( A? && O!) {
      |      var a : Vec(2,UInt(8))
      |      A?a
      |      O!a
      |    }
      |    wait
      |  }
      |}
    """.stripMargin.trim)
)


class Vector2Tester(c:Vector2) extends PeekPokeTester(c) {
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
  expect( c.io("O").bits.asInstanceOf[Vec[UInt]], IndexedSeq[BigInt](1,10))  // Moore

}

class Vector2Test extends FlatSpec with Matchers {
  behavior of "Vector2"
  it should "work" in {
    chisel3.iotesters.Driver( () => new Vector2, "firrtl") { c =>
      new Vector2Tester( c)
    } should be ( true)
  }
}
