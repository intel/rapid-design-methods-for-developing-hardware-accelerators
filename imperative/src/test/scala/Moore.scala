package imperative

import org.scalatest.{ Matchers, FlatSpec}

import chisel3._
import chisel3.util._
import chisel3.iotesters._

import compiler._

//
// Careful design means Q.valid and P.ready depend only on f
//
class Moore extends ImperativeModule( 
  Compiler.run(
    """
      |process Moore( P : inp UInt(64), Q : out UInt(64)) {
      |  var f : UInt(2)
      |  var v : UInt(64)
      |  var w : UInt(64)
      |  f = 0
      |  while ( true) {
      |    var pp : UInt(64)
      |    var p : UInt(1)
      |    var q : UInt(1)
      |    p = 0
      |    q = 0
      |    if ( f == 1) {
      |      if ( Q!) {
      |        Q!v
      |        q = 1
      |      }
      |    } else if ( f == 2) {
      |      if ( Q!) {
      |        Q!w
      |        q = 1
      |      }
      |    }
      |    if ( !(f == 2)) {
      |      if ( P?) {
      |        P?pp
      |        p = 1
      |      }
      |    }
      |    if ( f == 1 && p == 1 && q == 0) {
      |      w = v
      |    }
      |    f = (f + p) - q
      |    if ( p == 1) {
      |      v = pp
      |    }
      |    wait
      |  }
      |}
    """.stripMargin.trim))

class MooreTester(c:Moore) extends PeekPokeTester(c) {
  poke( c.io("Q").ready, 1)
  poke( c.io("P").valid, 0)

//Mealy
  expect( c.io("Q").valid, 0)
//  expect( c.io("P").ready, 1) /* Don't care */

  step(1)

  poke( c.io("Q").ready, 1)
  poke( c.io("P").valid, 1)
  poke( c.io("P").bits.asInstanceOf[UInt], 4747)

//Mealy
  expect( c.io("Q").valid, 0)
  expect( c.io("P").ready, 1)

  step(1)

  poke( c.io("Q").ready, 0)
  poke( c.io("P").valid, 0)

//Mealy
//  expect( c.io("Q").valid, 1) /* Don't care */
//  expect( c.io("P").ready, 1) /* Don't care */

  step(1)

  poke( c.io("Q").ready, 1)
  poke( c.io("P").valid, 1)
  poke( c.io("P").bits.asInstanceOf[UInt], 5454)

//Mealy
  expect( c.io("P").ready, 1)
  expect( c.io("Q").valid, 1)
  expect( c.io("Q").bits.asInstanceOf[UInt], 4747)

  step(1)

  poke( c.io("Q").ready, 1)
  poke( c.io("P").valid, 0)

//Mealy
  expect( c.io("Q").valid, 1)
  expect( c.io("Q").bits.asInstanceOf[UInt], 5454)
//  expect( c.io("P").ready, 1) /* Don't care */

}

class MooreTest extends FlatSpec with Matchers {
  behavior of "Moore"
  it should "work" in {
    chisel3.iotesters.Driver( () => new Moore, "firrtl") { c =>
      new MooreTester( c)
    } should be ( true)
  }
}
