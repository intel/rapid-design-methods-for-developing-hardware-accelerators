package imperative

import org.scalatest.{ Matchers, FlatSpec, GivenWhenThen, FreeSpec}

import scala.util.control.Breaks

import chisel3._
import chisel3.util._
import chisel3.iotesters._

import compiler._

object DoubleComm {
  val txt =
    """
      |process Div2 ( P : inp UInt(8),
      |               Q : out UInt(8))
      |{
      |  while ( true) {
      |    if ( P? && Q!) {
      |      var a : UInt(8)
      |      var b : UInt(8)
      |      P?a
      |      wait
      |      P?b
      |      Q!( a + 10*b)
      |    }
      |    wait
      |  }
      |}
    """.stripMargin.trim
}

class BadDoubleCommunicationTest extends FreeSpec with Matchers {
  "Bad Double Communication" - {
    "Two nbget commands in same cycle should return error" in {
      val e_ast = Compiler( DoubleComm.txt)
      if ( e_ast.isLeft ) {
        println( s"${e_ast.left}")
      }
      e_ast.isLeft should be (true)
    }
  }
}

class Div2 extends ImperativeModule( 
    Compiler.run(
    """
      |process Div2 ( P : inp UInt(8),
      |               Q : out UInt(8))
      |{
      |  var f : UInt(1)
      |  var a : UInt(8)
      |  f = 0
      |  while ( true) {
      |    if ( f == 0 && P?) {
      |      P?a
      |      f = 1
      |    } else if ( f == 1 && P? && Q!) {
      |      var b : UInt(8)
      |      P?b
      |      Q!( a + 10*b)
      |      f = 0
      |    }
      |    wait
      |  }
      |}
    """.stripMargin.trim)
)

class Div2Tester(c:Div2) extends PeekPokeTester(c) {
  poke( c.io("Q").ready, 1)
  poke( c.io("P").valid, 0)

// Mealy
  expect( c.io("Q").valid, 0)
//  expect( c.io("P").ready, 0)

  step(1)

  poke( c.io("Q").ready, 1)
  poke( c.io("P").valid, 1)
  poke( c.io("P").bits.asInstanceOf[UInt], 1)

// Mealy
  expect( c.io("Q").valid, 0)
  expect( c.io("P").ready, 1)

  step(1)

  poke( c.io("Q").ready, 1)
  poke( c.io("P").valid, 1)
  poke( c.io("P").bits.asInstanceOf[UInt], 2)

// Mealy
  expect( c.io("Q").valid, 1)
  expect( c.io("Q").bits.asInstanceOf[UInt], 21)
  expect( c.io("P").ready, 1)

  step(1)

}

class Div2Test extends FlatSpec with Matchers {
  behavior of "Div2"
  it should "work" in {
    chisel3.iotesters.Driver( () => new Div2, "firrtl") { c =>
      new Div2Tester( c)
    } should be ( true)
  }
}
