package imperative.LazyStackComb

import imperative.{ ImperativeModule}
import org.scalatest.{ Matchers, FlatSpec, GivenWhenThen}

import scala.util.control.Breaks

import chisel3._
import chisel3.util._

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

object LazyStackNDriver extends App {
  Driver.execute( args, () => new LazyStackN( 16))
}
