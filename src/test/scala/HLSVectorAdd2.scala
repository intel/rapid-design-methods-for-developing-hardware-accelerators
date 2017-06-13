package imperative

import org.scalatest.{ Matchers, FlatSpec, GivenWhenThen}

import chisel3._
import chisel3.util._
import chisel3.iotesters._

import lexer._
import compiler._

class HLSVectorAdd2 extends ImperativeModule( 
  Compiler.runHLS(
    """
      |process HLSVectorAdd2 ( A : inp Vec(2,UInt(8)),
      |                        B : inp Vec(2,UInt(8)),
      |                        O : out Vec(2,UInt(8)))
      |{
      |  var a : Vec(2,UInt(8))
      |  var b : Vec(2,UInt(8))
      |  var o : Vec(2,UInt(8))
      |  while ( true) {
      |    A??a
      |    B??b
      |    o(0) = a(0) + b(0)
      |    o(1) = a(1) + b(1)
      |    wait
      |    O!!o
      |  }
      |}
    """.stripMargin.trim)
)

class HLSVectorAdd2Tester[T <: ImperativeModule]( c : T) extends PeekPokeTester(c) {
  poke( c.io("O").ready, 1)

  poke( c.io("A").valid, 0)
  poke( c.io("B").valid, 0)

// Mealy
  expect( c.io("O").valid, 0)
//  expect( c.io("A").ready, 0) // don't care
//  expect( c.io("B").ready, 0) // don't care

  step(1)

  poke( c.io("A").valid, 1)
  poke( c.io("B").valid, 1)
  poke( c.io("A").bits.asInstanceOf[Vec[UInt]], IndexedSeq[BigInt](1,10))
  poke( c.io("B").bits.asInstanceOf[Vec[UInt]], IndexedSeq[BigInt](7,2))

// Mealy
  expect( c.io("A").ready, 1)
  expect( c.io("B").ready, 1)

  step(1)

// Mealy
  expect( c.io("O").valid, 1)
  expect( c.io("O").bits.asInstanceOf[Vec[UInt]], IndexedSeq[BigInt](8,12))


}

class HLSVectorAdd2Test extends FlatSpec with Matchers {
  behavior of "HLSVectorAdd2"
  it should "work" in {
    chisel3.iotesters.Driver( () => new HLSVectorAdd2, "firrtl") { c =>
      new HLSVectorAdd2Tester( c)
    } should be ( true)
  }
}
