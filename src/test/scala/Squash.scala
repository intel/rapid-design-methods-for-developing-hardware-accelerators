package imperative

import org.scalatest.{ Matchers, FlatSpec}

import chisel3._
import chisel3.util._
import chisel3.iotesters._

import compiler._

class Squash extends ImperativeModule( 
  Compiler.run(
    """
      |process Squash( P : inp UInt(64), Q : out UInt(64)) {
      |  var f : UInt(1)
      |  var v : UInt(64)
      |  var unused : UInt(64)
      |  f = 0
      |  while ( true) {
      |    if ( f == 1 && Q!) {
      |      Q!v
      |      f = 0
      |    }
      |    if ( f == 0 && P?) {
      |      P?v
      |      f = 1
      |    }
      |    wait
      |  }
      |}
    """.stripMargin.trim))

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
