package imperative

import org.scalatest.{ Matchers, FlatSpec, GivenWhenThen}
import org.scalacheck.{ Properties, Gen, Arbitrary}
import org.scalacheck.Prop.{ forAll, AnyOperators, collect}

import chisel3._
import firrtl_interpreter.InterpretiveTester

object ThreeXPlus1 {
  val width = 80
}

class ThreeXPlus1 extends ImperativeModule( 
  List( ("o", Output(UInt(ThreeXPlus1.width.W))))
, While(
    ConstantTrue,
    SequentialComposition(
      List( Assignment( Variable( "o"), MulExpression( Variable( "o"), ConstantInteger( 3))),
            Assignment( Variable( "o"), AddExpression( Variable( "o"), ConstantInteger( 1))),
            Wait))))

class ThreeXPlus1Tester(c:TreeXPlus1) extends PeekPokeTester(c) {
  var x : BigInt = 0
  tester.expect( s"io_o", 0)

  for( i <- 0 until 10) {
    tester.step()
    x = 3*x+1
    tester.expect( s"io_o", x)
  }
}

object ThreeXPlus1Test extends FlatSpec with Matchers {
  behavior of "ThreeXPlus1"
  it should "work" in {
    chisel3.iotesters.Driver( () = new ThreeXPlus1, "firrtl") { c =>
      new ThreeXPlus1Tester( c)
    } should be ( true)
  }
}
