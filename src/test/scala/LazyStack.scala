package imperative

import org.scalatest.{ Matchers, FlatSpec, GivenWhenThen}

import scala.util.control.Breaks

import chisel3._
import chisel3.util._
import chisel3.iotesters._

import lexer._
import compiler._

// ¬  ∨ 

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
      |      In?x
      |    } else if ( f==0 ∧ In?) {
      |      In?x
      |      f = 1
      |    } else if ( f==0 ∧ Out! ∧ Get?) {
      |      Get?x
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

  poke( c.io.Out.ready, 0)
  poke( c.io.In.valid, 0)

  expect( c.io.In.ready, 0) // Mealy

  step(1)

  expect( c.io.Out.valid, 0) // Moore

//

  val mybreaks = new Breaks
  import mybreaks.{break, breakable}

  val timeOut = 10

  def doIn( v : BigInt) {
    poke( c.io.Out.ready, 0)
    poke( c.io.In.valid, 1)
    poke( c.io.In.bits, v)

    var steps = 0

    println( s"doIn init: ${peek( c.io.In.ready)}")

    breakable {
      while ( peek( c.io.In.ready) == BigInt(0)) {
        step(1)
        expect( c.io.Out.valid, 0) // Moore
        if ( steps >= timeOut) {
          println( s"In loop timeOut")
          break()
        }
        steps += 1
      }
    }
    step( 1)
    expect( c.io.Out.valid, 0) // Moore
  }


  def doOut( v : BigInt) {
    poke( c.io.Out.ready, 1)
    poke( c.io.In.valid, 0)

    expect( c.io.In.ready, 0) // Mealy

    var steps = 0

    step(1)
    println( s"doOut init: ${peek( c.io.Out.valid)}")

    breakable {
    while( peek( c.io.Out.valid) == BigInt(0)) {
      step(1)
      if ( steps >= timeOut) {
        println( s"Out loop timeOut")
        break()
      }
      steps += 1
    }
    }
    expect( c.io.Out.bits, v)
  }

  doIn( 47)
  doOut( 47)

  doIn( 100)
// This doesn't work yet
//  doIn( 101)

//  doOut( 101)
  doOut( 100)
}

class LazyStackNTest extends FlatSpec with Matchers {
  behavior of "LazyStackN"
  it should "work" in {
    chisel3.iotesters.Driver( () => new LazyStackN(2), "vcs") { c =>
      new LazyStackNTester( c)
    } should be ( true)
  }
}
