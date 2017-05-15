package imperative

import org.scalatest.{ Matchers, FlatSpec}

import compiler._

import chisel3._
import chisel3.util._
import chisel3.iotesters._

object Channel {
  val width = 64
}

class Channel extends ImperativeModule( 
  Compiler.run(
    """
      |process Channel ( P : inp UInt(64), Q : out UInt(64))
      |{
      |  while ( true) {
      |    if ( P? && Q!) {
      |      var v : UInt(64)
      |      P?v
      |      Q!v
      |    }
      |    wait
      |  }
      |}
    """.stripMargin.trim))

class ChannelTester(c:Channel) extends PeekPokeTester(c) {
  poke( c.io("Q").ready, 1)
  poke( c.io("P").valid, 0)

// Mealy
  expect( c.io("Q").valid, 0)
//  expect( c.io("P").ready, 1) /* Don't care */

  step(1)

  poke( c.io("Q").ready, 1)
  poke( c.io("P").valid, 1)
  poke( c.io("P").bits.asInstanceOf[UInt], 4747)

  expect( c.io("Q").valid, 1) // Moore
  expect( c.io("Q").bits.asInstanceOf[UInt], 4747)  // Moore
  expect( c.io("P").ready, 1) // Mealy

  step(1)

  poke( c.io("Q").ready, 0)
  poke( c.io("P").valid, 0)

//Mealy
//  expect( c.io("P").ready, 1) /* Don't care */
//  expect( c.io("Q").valid, 1) /* Don't care */

  step(1)

  poke( c.io("Q").ready, 1)
  poke( c.io("P").valid, 1)
  poke( c.io("P").bits.asInstanceOf[UInt], 5454)

 // Mealy
  expect( c.io("Q").valid, 1)
  expect( c.io("Q").bits.asInstanceOf[UInt], 5454)
  expect( c.io("P").ready, 1)

  step(1)
}

class ChannelTest extends FlatSpec with Matchers {
  behavior of "Channel"
  it should "work" in {
    chisel3.iotesters.Driver( () => new Channel, "firrtl") { c =>
      new ChannelTester( c)
    } should be ( true)
  }
}
