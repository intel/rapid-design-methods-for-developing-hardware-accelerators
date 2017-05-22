package imperative

import org.scalatest.{ Matchers, FlatSpec}

import chisel3._
import chisel3.util._
import chisel3.iotesters._

import compiler._

/*
    """
      |process Split( P : inp UInt(64), Q0 : out UInt(64), Q1 : out UInt(64)) {
      |  var x : UInt(64)
      |  while ( true) {
      |    while ( !P?) wait
      |    P?x
      |    while ( !Q0!) wait
      |    Q0!x
      |    wait //0 (No buffering, but wait for sequencing multiple communications
      |    while ( !P?) wait
      |    P?x
      |    while ( !Q1!) wait
      |    Q1!x
      |    wait //0 (No buffering, but wait for sequencing multiple communications
      |  }
      |}
    """.stripMargin.trim
 */
class Split extends ImperativeModule( 
  Compiler.run(
    """
      |process Split( P : inp UInt(64), Q0 : out UInt(64), Q1 : out UInt(64)) {
      |  var x : UInt(64)
      |  while ( !P?) wait
      |  P?x
      |  wait
      |  while ( true) {
      |    while ( !Q0!) wait
      |    Q0!x
      |    while ( !P?) wait
      |    P?x
      |    wait
      |    while ( !Q1!) wait
      |    Q1!x
      |    while ( !P?) wait
      |    P?x
      |    wait
      |  }
      |}
    """.stripMargin.trim))

/* // would like to write it like this
    """
      |process Split( P : inp UInt(64), Q0 : out UInt(64), Q1 : out UInt(64)) {
      |  var x : UInt(64)
      |  P?x //blocking
      |  wait //0 (The wait here make it buffered)
      |  while ( true) {
      |    Q0!x //blocking
      |    P?x //blocking
      |    wait //1 (The wait here make it buffered)
      |    Q1!x //blocking
      |    P?x //blocking
      |    wait //0 (The wait here make it buffered)
      |  }
      |}
    """.stripMargin.trim
 */
/*
class Split extends ImperativeModule( 
  Compiler.run(
    """
      |process Split( P : inp UInt(64), Q0 : out UInt(64), Q1 : out UInt(64)) {
      |  var s : UInt(4)
      |  var x : UInt(64)
      |  while ( true) {
      |    var w : UInt(1)
      |    w = 0
      |    if ( s == 0 && w == 0) {
      |      if ( P?) {
      |        P?x
      |        s = 1
      |      } 
      |      w = 1
      |    }
      |    if ( s == 1 && w == 0) {
      |      if ( Q0!) {
      |        Q0!x
      |        s = 2
      |      } else w = 1
      |    }
      |    if ( s == 2 && w == 0) {
      |      if ( P?) {
      |        P?x
      |        s = 3
      |      }
      |      w = 1
      |    }
      |    if ( s == 3 && w == 0) {
      |      if ( Q1!) {
      |        Q1!x
      |        s = 4
      |      } else w = 1
      |    }
      |    if ( s == 4 && w == 0) {
      |      if ( P?) {
      |        P?x
      |        s = 1
      |      }
      |      w = 1
      |    }
      |    wait
      |  }
      |}
    """.stripMargin.trim))
 */

/*
class Split extends ImperativeModule( 
  Compiler.run(
    """
      |process Split( P : inp UInt(64), Q0 : out UInt(64), Q1 : out UInt(64)) {
      |  var x : UInt(64)
      |  var s : UInt(1)
      |  var f : UInt(1)
      |  s = 1
      |  f = 0
      |  while ( true) {
      |    if ( f == 1) {
      |      if ( s == 0) {
      |        if ( Q0!) {
      |          Q0!x
      |          f = 0
      |        }
      |      } else {
      |        if ( Q1!) {
      |          Q1!x
      |          f = 0
      |        }
      |      }
      |    }
      |    if ( f == 0 && P?) {
      |      P?x
      |      f = 1
      |      if ( s == 1) {
      |        s = 0
      |      } else {
      |        s = 1
      |      }
      |    }
      |    wait
      |  }
      |}
    """.stripMargin.trim))
 */

class Merge extends ImperativeModule( 
  Compiler.run(
    """
      |process Merge( P0 : inp UInt(64), P1 : inp UInt(64), Q : out UInt(64)) {
      |  var x : UInt(64)
      |  var s : UInt(1)
      |  var f : UInt(1)
      |  s = 0
      |  f = 0
      |  while ( true) {
      |    if ( f == 1) {
      |      if ( Q!) {
      |        Q!x
      |        f = 0
      |      }
      |    }
      |    if ( f == 0) {
      |      if ( s == 0) {
      |        if ( P0?) {
      |          P0?x
      |          f = 1
      |          s = 1
      |        }
      |      } else {
      |        if ( P1?) {
      |          P1?x
      |          f = 1
      |          s = 0
      |        }
      |      }
      |    }
      |    wait
      |  }
      |}
    """.stripMargin.trim))

/*
class Merge extends ImperativeModule( 
  Compiler.run(
    """
      |process Merge( P0 : inp UInt(64), P1 : inp UInt(64), Q : out UInt(64)) {
      |  var x : UInt(64)
      |  var f : UInt(1)
      |  f = 0
      |  while ( true) {
      |    if ( f == 0 && P0? && Q!) {
      |      P0?x
      |      Q!x
      |      f = 1
      |    } else if ( f == 1 && P1? && Q!) {
      |      P1?x
      |      Q!x
      |      f = 0
      |    }
      |    wait
      |  }
      |}
    """.stripMargin.trim))
 */


/*
class TreeFifoTester(c:TreeFifo) extends PeekPokeTester(c) {
  poke( c.io.out.ready, 1)
  poke( c.io.inp.valid, 0)

//Mealy
  expect( c.io.out.valid, 0)
//  expect( c.io.inp.ready, 1) /* Don't care */

  step(1)

  poke( c.io.out.ready, 1)
  poke( c.io.inp.valid, 1)
  poke( c.io.inp.bits.asInstanceOf[UInt], 4747)

//Mealy
  expect( c.io.out.valid, 0)
  expect( c.io.inp.ready, 1)

  step(1)

  poke( c.io.out.ready, 0)
  poke( c.io.inp.valid, 0)

//Mealy
//  expect( c.io.out.valid, 1) /* Don't care */
//  expect( c.io.inp.ready, 1) /* Don't care */

  step(1)

  poke( c.io.out.ready, 1)
  poke( c.io.inp.valid, 1)
  poke( c.io.inp.bits.asInstanceOf[UInt], 5454)

//Mealy
  expect( c.io.inp.ready, 1)
  expect( c.io.out.valid, 1)
  expect( c.io.out.bits.asInstanceOf[UInt], 4747)

  step(1)

  poke( c.io.out.ready, 1)
  poke( c.io.inp.valid, 0)

//Mealy
  expect( c.io.out.valid, 1)
  expect( c.io.out.bits.asInstanceOf[UInt], 5454)
//  expect( c.io.inp.ready, 1) /* Don't care */

}

class TreeFifoTest extends FlatSpec with Matchers {
  behavior of "TreeFifo"
  it should "work" in {
    chisel3.iotesters.Driver( () => new TreeFifo, "firrtl") { c =>
      new TreeFifoTester( c)
    } should be ( true)
  }
}
 */
