package imperative

import chisel3._
import chisel3.util._

import lexer._
import compiler._

// ¬  ∨ 

class LazyStackComb extends ImperativeModule( 
  Compiler.run(
    """
      |process LazyStackComb( In  : inp UInt(8),
      |                       Out : out UInt(8),
      |                       Put : out UInt(8),
      |                       Get : inp UInt(8))
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

// ¬  ∨ 

// Add wait statements to the original
// This gets translated to the second one
class LazyStackWait1 extends ImperativeModule( 
  Compiler.runHLS3(
    """
      |process LazyStackWait( In  : inp UInt(8),
      |                       Out : out UInt(8),
      |                       Put : out UInt(8),
      |                       Get : inp UInt(8))
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

class LazyStackWait2 extends ImperativeModule( 
  Compiler.runHLS3(
    """
      |process LazyStackWait( In  : inp UInt(8),
      |                       Out : out UInt(8),
      |                       Put : out UInt(8),
      |                       Get : inp UInt(8))
      |{
      |  var f : UInt(1)
      |  var x : UInt(8)
      |  f = 0
      |  while ( true) {
      |    if        ( f==1 ∧ In?) {
      |      Put!!x
      |      wait
      |      In?x
      |    } else if ( f==0 ∧ In?) {
      |      In?x
      |      f = 1
      |    } else if ( f==0 ∧ Out!) {
      |      Get??x
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

class LazyStackWait extends ImperativeModule( 
  Compiler.run(
    """
      |process LazyStackWait ( In  : inp UInt(8),
      |                        Out : out UInt(8),
      |                        Put : out UInt(8),
      |                        Get : inp UInt(8))
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

class LastStack extends ImperativeModule( 
  Compiler.run(
    """
      |process LastStack( In  : inp UInt(8),
      |                   Out : out UInt(8))
      |{
      |  while ( true) {
      |    wait
      |  }
      |}
    """.stripMargin.trim)
)

class StackIfc extends ImperativeIfc( 
  Compiler.run(
    """
      |process StackIfc( In  : inp UInt(8),
      |                  Out : out UInt(8))
      |{
      |}
    """.stripMargin.trim)
)

/*
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
 */

class LazyStackN[T <: ImperativeIfc]( n : Int, factory : () => T) extends StackIfc {
  val stacks = (0 until n).toList.map( _ => Module( factory()))
  val lastStack = Module( new LastStack)
  val ls = (io("In"),io("Out")) :: stacks.map( c => (c.io("Put"),c.io("Get")))
  val rs = stacks.map( c => (c.io("In"),c.io("Out"))) :+ (lastStack.io("In"),lastStack.io("Out"))
  (ls zip rs).foreach {
    case (l,r) => {
      l._1 <> r._1
      l._2 <> r._2
    }
  }
}

object LazyStackNWaitDriver extends App {
  Driver.execute( args, () => new LazyStackN( 16, () => new LazyStackWait))
}

object LazyStackNCombDriver extends App {
  Driver.execute( args, () => new LazyStackN( 16, () => new LazyStackComb))
}
