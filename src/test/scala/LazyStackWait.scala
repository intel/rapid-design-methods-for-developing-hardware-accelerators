package imperative.LazyStackWait

import imperative.{ ImperativeModule}
import org.scalatest.{ Matchers, FlatSpec, GivenWhenThen}

import scala.util.control.Breaks

import chisel3._
import chisel3.util._
import chisel3.iotesters._

import lexer._
import compiler._

// ¬  ∨ 

// Add wait statements to the original
// This gets translated to the second one
/*
class LazyStack extends ImperativeModule( 
  Compiler.run(
    """
      |process LazyStack ( In   : inp UInt(8),
      |                    Out  : out UInt(8),
      |                    Put  : out UInt(8),
      |                    Get  : inp UInt(8))
      |{
      |  var f : UInt(1)
      |  var x : UInt(8)
      |  f = 0
      |  while ( true) {
      |    if        ( f==1 ∧ In? ∧ Put!) {
      |      Put!x
      |      wait
      |      In?x
      |    } else if ( f==0 ∧ In?) {
      |      In?x
      |      f = 1
      |    } else if ( f==0 ∧ Out! ∧ Get?) {
      |      Get?x
      |      wait
      |      Out!x
      |    } else if ( f==1 ∧ Out!) {  
      |      Out!x
      |      f = 0
      |    }
      |    wait
      |  }
      |}
    """.stripMargin.trim)
)
 */
/* or */
/*
class LazyStack extends ImperativeModule( 
  Compiler.run(
    """
      |process LazyStack ( In   : inp UInt(8),
      |                    Out  : out UInt(8),
      |                    Put  : out UInt(8),
      |                    Get  : inp UInt(8))
      |{
      |  var f : UInt(1)
      |  var x : UInt(8)
      |  f = 0
      |  while ( true) {
      |    if        ( f==1 ∧ In?) {
      |      while ( ¬ Put!) wait
      |      Put!x
      |      wait
      |      In?x
      |    } else if ( f==0 ∧ In?) {
      |      In?x
      |      f = 1
      |    } else if ( f==0 ∧ Out!) {
      |      while ( ¬ Get?) wait
      |      Get?x
      |      wait
      |      Out!x
      |    } else if ( f==1 ∧ Out!) {  
      |      Out!x
      |      f = 0
      |    }
      |    wait
      |  }
      |}
    """.stripMargin.trim)
)
 */

class LazyStack extends ImperativeModule( 
  Compiler.run(
    """
      |process LazyStack ( In   : inp UInt(8),
      |                    Out  : out UInt(8),
      |                    Put  : out UInt(8),
      |                    Get  : inp UInt(8))
      |{
      |  var f : UInt(1)
      |  var x : UInt(8)
      |  f = 0
      |  while ( true) {
      |    if        ( f==1 ∧ In? ∧ Put!) {
      |      Put!x
      |      f = 0
      |    } else if ( f==0 ∧ In?) {
      |      In?x
      |      f = 1
      |    } else if ( f==0 ∧ Out! ∧ Get?) {
      |      Get?x
      |      f = 1
      |    } else if ( f==1 ∧ Out!) {  
      |      Out!x
      |      f = 0
      |    }
      |    wait
      |  }
      |}
    """.stripMargin.trim)
)

class LastStack extends Module
{
  val io = IO(new Bundle {
    val In = Flipped(Decoupled(UInt(8.W)))
    val Out = Decoupled(UInt(8.W))
  })

  io.In.ready := true.B
  io.Out.valid := true.B

  io.Out.bits := 0.U
}

class LazyStackN( n : Int) extends Module {
  val io = IO(new Bundle {
    val In = Flipped(Decoupled(UInt(8.W)))
    val Out = Decoupled(UInt(8.W))
  })

  val stacks = (0 until n).toList.map( _ => Module( new LazyStack))
  val lastStack = Module( new LastStack)
  val ls = (io.In,io.Out) :: stacks.map( c => (c.io("Put"),c.io("Get")))
  val rs = stacks.map( c => (c.io("In"),c.io("Out"))) :+ (lastStack.io.In,lastStack.io.Out)
  (ls zip rs).foreach {
    case (l,r) => {
      l._1 <> r._1
      l._2 <> r._2
    }
  }
}

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
    chisel3.iotesters.Driver( () => new LazyStackN(10), "vcs") { c =>
      new LazyStackNTester( c)
    } should be ( true)
  }
}
