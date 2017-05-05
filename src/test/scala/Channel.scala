package imperative

import org.scalatest.{ Matchers, FlatSpec}

import chisel3._
import chisel3.util._
import chisel3.iotesters._

object Channel {
  val width = 64
}

class Channel extends ImperativeModule( 
  List(),
  List( ("P", Flipped(DecoupledIO(UInt(Channel.width.W)))),
        ("Q",         DecoupledIO(UInt(Channel.width.W)))),
  While(
    ConstantTrue,
    SequentialComposition(
      List( IfThenElse( AndBExpression( NBCanGet( Port( "P")), NBCanPut( Port( "Q"))),
                        SequentialComposition( List(
                          NBGet( Port( "P")),
                          NBPut( Port( "Q"), NBGetData( Port( "P"))))),
                        SequentialComposition( List())),
            Wait))))

class ChannelTester(c:Channel) extends PeekPokeTester(c) {
  poke( c.io("Q").ready, 1)
  poke( c.io("P").valid, 0)

  expect( c.io("P").ready, 0)

  step(1)

  expect( c.io("Q").valid, 0)

  poke( c.io("P").valid, 1)
  poke( c.io("P").bits, 4747)

  expect( c.io("P").ready, 1)

  step(1)

  expect( c.io("Q").valid, 1)

}

class ChannelTest extends FlatSpec with Matchers {
  behavior of "Channel"
  it should "work" in {
    chisel3.iotesters.Driver( () => new Channel, "firrtl") { c =>
      new ChannelTester( c)
    } should be ( true)
  }
}
