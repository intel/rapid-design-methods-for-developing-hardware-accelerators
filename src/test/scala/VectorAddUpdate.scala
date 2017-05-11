package imperative

import org.scalatest.{ Matchers, FlatSpec, GivenWhenThen}

import chisel3._
import chisel3.util._
import chisel3.iotesters._

import lexer._
import compiler._

class VectorAddUpdate extends ImperativeModule( 
  Compiler.run(
    """
      |process VectorAddUpdate ( A : inp Vec(4,UInt(6)),
      |                          O : out Vec(4,UInt(6)))
      |{
      |  while ( true) {
      |    if ( A? && O!) {
      |      var a : Vec(4,UInt(6))
      |      var o : Vec(4,UInt(6))
      |      A?a
      |      o(0) = a(0)
      |      unroll( i, 0, 3) {
      |        o(i+1) = a(i+1) + o(i)
      |      }
      |      O!o
      |    }
      |    wait
      |  }
      |}
    """.stripMargin.trim)
)


class VectorAddUpdateTester(c:VectorAddUpdate) extends PeekPokeTester(c) {
  poke( c.io("O").ready, 1)

  poke( c.io("A").valid, 0)

  expect( c.io("A").ready, 0) // Mealy

  step(1)

  expect( c.io("O").valid, 0) // Moore

  poke( c.io("A").valid, 1)
  val a = for( i<-0 until 4) yield BigInt(i)
  val o = for( i<-0 until 4) yield BigInt(i*(i+1)/2)
  poke( c.io("A").bits.asInstanceOf[Vec[UInt]], a)

  expect( c.io("A").ready, 1) // Mealy

  step(1)

  expect( c.io("O").valid, 1) // Moore
//  expect( c.io("O").bits.asInstanceOf[Vec[UInt]], o)
  val v = peek( c.io("O").bits.asInstanceOf[Vec[UInt]])
  println( s"$a")
  println( s"${v.reverse}")
  println( s"$o")

}

class VectorAddUpdateTest extends FlatSpec with Matchers {
  behavior of "VectorAddUpdate"
  it should "work" in {
    chisel3.iotesters.Driver( () => new VectorAddUpdate, "firrtl") { c =>
      new VectorAddUpdateTester( c)
    } should be ( true)
  }
}
