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
      |    var a : UInt(8)
      |    while ( !P?) wait
      |    P?a
      |    wait
      |    while ( !(P? && Q!)) wait
      |    {
      |      var b : UInt(8)
      |      P?b
      |      Q!( a + 10*b)
      |    }
      |    wait
      |  }
      |}
    """.stripMargin.trim

  val txt2 =
    """
      |process Div2 ( P : inp UInt(8),
      |               Q : out UInt(8))
      |{
      |  // state=0
      |  while ( true) {
      |    var a : UInt(8)
      |    while ( !P?) wait
      |    // state=0 && P?
      |    P?a
      |    wait
      |    // state=1
      |    while ( !(P? && Q!)) wait
      |    // state=1 && P? && Q!
      |    {
      |      var b : UInt(8)
      |      P?b
      |      Q!( a + 10*b)
      |    }
      |    wait
      |    // state=0
      |  }
      |}
    """.stripMargin.trim

  val txt3 =
    """
      |process Div2 ( P : inp UInt(8),
      |               Q : out UInt(8))
      |{
      |  var a : UInt(8) // now an induction variable
      |  var state : UInt(1)
      |  state=0
      |  while ( true) {
      |    if ( state == 0 && P?) {
      |      P?a
      |      state=1
      |    } else if ( state == 1 && P? && Q!) {
      |      var b : UInt(8)
      |      P?b
      |      Q!( a + 10*b)
      |      state=0
      |    }
      |    wait
      |  }
      |}
    """.stripMargin.trim
}

class BadDoubleCommunicationTest extends FreeSpec with Matchers {
  "Bad Double Communication" - {
    "Two nbget commands in same cycle should return error" in {
      a [CompilationErrorException] should be thrownBy {
        Compiler.run( DoubleComm.txt)
      }
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
