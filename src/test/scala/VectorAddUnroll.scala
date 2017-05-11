package imperative

import org.scalatest.{ Matchers, FlatSpec, GivenWhenThen}

import chisel3._
import chisel3.util._
import chisel3.iotesters._

import lexer._
import compiler._

class VectorAddUnroll extends ImperativeModule( 
  Compiler.run(
    """
      |process VectorAddUnroll ( A : inp Vec(64,UInt(8)),
      |                          B : inp Vec(64,UInt(8)),
      |                          O : out Vec(64,UInt(8)))
      |{
      |  while ( true) {
      |    if ( A? && B? && O!) {
      |      var a : Vec(64,UInt(8))
      |      var b : Vec(64,UInt(8))
      |      var o : Vec(64,UInt(8))
      |      A?a
      |      B?b
      |      unroll( i, 0, 64) {
      |        o(i) = a(i) + b(i)
      |      }
      |      O!o
      |    }
      |    wait
      |  }
      |}
    """.stripMargin.trim)
)


class VectorAddUnrollTester(c:VectorAddUnroll) extends PeekPokeTester(c) {
  poke( c.io("O").ready, 1)

  poke( c.io("A").valid, 0)
  poke( c.io("B").valid, 0)

  expect( c.io("A").ready, 0) // Mealy
  expect( c.io("B").ready, 0) // Mealy

  step(1)

  expect( c.io("O").valid, 0) // Moore

  poke( c.io("A").valid, 1)
  poke( c.io("B").valid, 1)
  val a = for( i<-0 until 64) yield BigInt(i)
  val b = for( i<-0 until 64) yield BigInt(64-i)
  val o = for( i<-0 until 64) yield BigInt(64)
  poke( c.io("A").bits.asInstanceOf[Vec[UInt]], a)
  poke( c.io("B").bits.asInstanceOf[Vec[UInt]], b)

  expect( c.io("A").ready, 1) // Mealy
  expect( c.io("B").ready, 1) // Mealy

  step(1)

  expect( c.io("O").valid, 1) // Moore
  expect( c.io("O").bits.asInstanceOf[Vec[UInt]], o)

}

class VectorAddUnrollTest extends FlatSpec with Matchers {
  behavior of "VectorAddUnroll"
  it should "work" in {
    chisel3.iotesters.Driver( () => new VectorAddUnroll, "firrtl") { c =>
      new VectorAddUnrollTester( c)
    } should be ( true)
  }
}
