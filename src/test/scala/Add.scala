package imperative

import org.scalatest.{ Matchers, FlatSpec, GivenWhenThen}
import org.scalacheck.{ Properties, Gen, Arbitrary}
import org.scalacheck.Prop.{ forAll, AnyOperators, collect}

import chisel3._
import firrtl_interpreter.InterpretiveTester

object G {
  val width = 8
}

class Add extends ImperativeModule( 
  List( ("a", Input(UInt(G.width.W))),
        ("b", Input(UInt(G.width.W))),
        ("o", Output(UInt(G.width.W))))
, While(
    ConstantTrue,
    SequentialComposition(
      List( Assignment( Variable( "o"), AddExpression( Variable( "a"), Variable( "b"))),
            Assignment( Variable( "o"), AddExpression( Variable( "a"), Variable( "o"))),
            Assignment( Variable( "o"), AddExpression( Variable( "b"), Variable( "o"))),
            Wait)))) 


class AddTester {
  val s = chisel3.Driver.emit( () => new Add)
  val tester = new InterpretiveTester( s)

  tester.poke( s"reset", 1)
  tester.step()
  tester.poke( s"reset", 0)
  tester.step()

  def run( a : Int, b : Int) = {
    val result = (a + b + a + b) & ((1 << G.width)-1)
    tester.poke( s"io_a", a)
    tester.poke( s"io_b", b)
    tester.step()
    tester.peek( s"io_o") ?= result
  }
}

object AddTest extends Properties("Add") {
  val t = new AddTester
  val gen = Gen.choose(0,(1 << G.width)-1)

  property("Add") = forAll( gen, gen) {
    case (a:Int,b:Int) => t.run( a, b)
  }
}
