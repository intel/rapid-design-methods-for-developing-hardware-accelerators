package imperative

import org.scalatest.{ Matchers, FlatSpec, GivenWhenThen}

import chisel3._
import chisel3.util._
import chisel3.iotesters._

import compiler._

class Add extends ImperativeModule( 
    Compiler.run(
    """
      |process Channel ( A : inp UInt(8),
      |                  B : inp UInt(8),
      |                  O : out UInt(8))
      |{
      |  while ( true) {
      |    if ( A? && B? && O!) {
      |      var a : UInt(8)
      |      var b : UInt(8)
      |      var o : UInt(8)
      |      A?a
      |      B?b
      |      o = a + b
      |      o = a + o
      |      o = b + o
      |      O!o
      |    }
      |    wait
      |  }
      |}
    """.stripMargin.trim)
)


class AddTester(c:Add) extends PeekPokeTester(c) {
  poke( c.io("O").ready, 1)

  poke( c.io("A").valid, 0)
  poke( c.io("B").valid, 0)

  expect( c.io("A").ready, 0) // Mealy
  expect( c.io("B").ready, 0) // Mealy

  step(1)

  expect( c.io("O").valid, 0) // Moore

  poke( c.io("A").valid, 1)
  poke( c.io("A").bits.asInstanceOf[UInt], 1)
  poke( c.io("B").valid, 1)
  poke( c.io("B").bits.asInstanceOf[UInt], 10)

  expect( c.io("O").ready, 1) // Mealy

  step(1)

  expect( c.io("O").valid, 1) // Moore
  expect( c.io("O").bits.asInstanceOf[UInt], 22)  // Moore

}

class AddTest extends FlatSpec with Matchers {
  behavior of "Add"
  it should "work" in {
    chisel3.iotesters.Driver( () => new Add, "firrtl") { c =>
      new AddTester( c)
    } should be ( true)
  }
}
