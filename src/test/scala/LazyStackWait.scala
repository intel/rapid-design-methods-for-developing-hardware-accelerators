package imperative.LazyStackWait

import org.scalatest.{ Matchers, FlatSpec, GivenWhenThen}

import scala.util.control.Breaks

import chisel3._
import chisel3.util._
import chisel3.iotesters._

class LazyStackNTester(c:LazyStackN) extends PeekPokeTester(c) {

//

  val mybreaks = new Breaks
  import mybreaks.{break, breakable}

  val timeOut = 10

  def doIn( v : BigInt) {
    poke( c.io.Out.ready, 0)
    poke( c.io.In.valid, 1)
    poke( c.io.In.bits, v)

    var steps = 0

    breakable {
      while ( peek( c.io.In.ready) == BigInt(0)) {
        step(1)
        if ( steps >= timeOut) {
          println( s"In loop timeOut")
          break()
        }
        steps += 1
      }
      println( s"doIn transfer: ${v}")
    }
  }

  def doOut( v : BigInt) {
    poke( c.io.Out.ready, 1)
    poke( c.io.In.valid, 0)

    var steps = 0

    breakable {
      while( peek( c.io.Out.valid) == BigInt(0)) {
        step(1)
        if ( steps >= timeOut) {
          println( s"Out loop timeOut")
          break()
        }
        steps += 1
      }
      println( s"doOut transfer: ${peek( c.io.Out.bits)} ${v}")
      expect( c.io.Out.bits, v)
    }
  }

  poke( c.io.Out.ready, 0)
  poke( c.io.In.valid, 0)

//Mealy
//  expect( c.io.In.ready, 1) /* Don't care */
//  expect( c.io.Out.valid, 1) /* Don't care */

  step(1)

  doIn(  47); step(1)
  doOut( 47); step(1)

  doIn( 100); step(1)
  doIn( 101); step(1)

  doOut( 101); step(1)
  doIn( 102); step(1)
  doIn( 103); step(1)
  doIn( 104); step(1)

  doOut( 104); step(1)
  doOut( 103); step(1)
  doOut( 102); step(1)
  doOut( 100); step(1)
}

class LazyStackNTest extends FlatSpec with Matchers {
  behavior of "LazyStackN"
  it should "work" in {
    chisel3.iotesters.Driver( () => new LazyStackN(10), "firrtl") { c =>
      new LazyStackNTester( c)
    } should be ( true)
  }
}
