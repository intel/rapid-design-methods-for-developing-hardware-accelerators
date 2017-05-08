package imperative

import org.scalatest.{ Matchers, FlatSpec}

import chisel3._
import chisel3.util._
import chisel3.iotesters._

import compiler._

object Squash {
  val width = 64
}

class Squash extends ImperativeModule( 
  List(),
  List( ("P", Flipped(DecoupledIO(UInt(Squash.width.W)))),
        ("Q",         DecoupledIO(UInt(Squash.width.W)))),
  {
    val code = 
    """
      |while ( true) {
      |  if ( f == 1 && NBCanPut( Q)) {
      |     NBPut( Q, v)
      |     f = 0
      |  }
      |  if ( f == 0 && NBCanGet( P)) {
      |     NBGet( P)
      |     v = NBGetData( P)
      |     f = 1
      |  }
      |  wait
      |}
    """.stripMargin.trim
    Compiler(code) match {
      case Right(ast) => ast
      case Left(ex) => {
        println( ex)
        SequentialComposition( List())
      }
    }
  })

class SquashTester(c:Squash) extends PeekPokeTester(c) {
  poke( c.io("Q").ready, 1)
  poke( c.io("P").valid, 0)

  expect( c.io("P").ready, 0) // Mealy

  step(1)

  expect( c.io("Q").valid, 0) // Moore

  poke( c.io("P").valid, 1)
  poke( c.io("P").bits, 4747)

  expect( c.io("P").ready, 1) // Mealy

  step(1)

  expect( c.io("Q").valid, 0) // Moore

  poke( c.io("Q").ready, 0)
  poke( c.io("P").valid, 0)

  expect( c.io("P").ready, 0) // Mealy

  step(1)

  expect( c.io("Q").valid, 0) // Moore

  poke( c.io("Q").ready, 1)
  poke( c.io("P").valid, 1)
  poke( c.io("P").bits, 5454)

  expect( c.io("P").ready, 1) // Mealy

  step(1)

  expect( c.io("Q").valid, 1) // Moore
  expect( c.io("Q").bits, 4747)  // Moore

}

class SquashTest extends FlatSpec with Matchers {
  behavior of "Squash"
  it should "work" in {
    chisel3.iotesters.Driver( () => new Squash, "firrtl") { c =>
      new SquashTester( c)
    } should be ( true)
  }
}
