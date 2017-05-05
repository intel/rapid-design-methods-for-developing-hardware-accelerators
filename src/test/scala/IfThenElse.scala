package imperative

import org.scalatest.{ Matchers, FlatSpec, GivenWhenThen}
import org.scalacheck.{ Properties, Gen, Arbitrary}
import org.scalacheck.Prop.{ forAll, AnyOperators, collect}

import chisel3._
import firrtl_interpreter.InterpretiveTester

object ITE {
  val width = 8
}

class ITE_then extends ImperativeModule( 
  List( ("a", Input(UInt(ITE.width.W))),
        ("b", Input(UInt(ITE.width.W))),
        ("o", Output(UInt(ITE.width.W))))
, While(
    ConstantTrue,
    SequentialComposition(
      List( Assignment( Variable( "o"), AddExpression( Variable( "a"), Variable( "b"))),
            IfThenElse( ConstantTrue, 
                        Assignment( Variable( "o"), AddExpression( Variable( "a"), Variable( "o"))),
                        Assignment( Variable( "o"), AddExpression( Variable( "b"), Variable( "o")))),
            Wait))))

class ITE_else extends ImperativeModule( 
  List( ("a", Input(UInt(ITE.width.W))),
        ("b", Input(UInt(ITE.width.W))),
        ("o", Output(UInt(ITE.width.W))))
, While(
    ConstantTrue,
    SequentialComposition(
      List( Assignment( Variable( "o"), AddExpression( Variable( "a"), Variable( "b"))),
            IfThenElse( NotBExpression( ConstantTrue), 
                        Assignment( Variable( "o"), AddExpression( Variable( "a"), Variable( "o"))),
                        Assignment( Variable( "o"), AddExpression( Variable( "b"), Variable( "o")))),
            Wait))))

class ITE_then_Tester {
  val s = chisel3.Driver.emit( () => new ITE_then)
  val tester = new InterpretiveTester( s)
  def run( a : Int, b : Int) = {
    val result = (a + b + a) & ((1 << ITE.width)-1)
    tester.poke( s"io_a", a)
    tester.poke( s"io_b", b)
    tester.step()
    tester.peek( s"io_o") ?= result
  }
}

class ITE_else_Tester {
  val s = chisel3.Driver.emit( () => new ITE_else)
  val tester = new InterpretiveTester( s)
  def run( a : Int, b : Int) = {
    val result = (a + b + b) & ((1 << ITE.width)-1)
    tester.poke( s"io_a", a)
    tester.poke( s"io_b", b)
    tester.step()
    tester.peek( s"io_o") ?= result
  }
}

object ITE_then_Test extends Properties("ITE_then") {
  val t = new ITE_then_Tester
  val gen = Gen.choose(0,(1 << ITE.width)-1)
  property("ITE_then") = forAll( gen, gen) {
    case (a:Int,b:Int) => t.run( a, b)
  }
}

object ITE_else_Test extends Properties("ITE_else") {
  val t = new ITE_else_Tester
  val gen = Gen.choose(0,(1 << ITE.width)-1)
  property("ITE_else") = forAll( gen, gen) {
    case (a:Int,b:Int) => t.run( a, b)
  }
}
