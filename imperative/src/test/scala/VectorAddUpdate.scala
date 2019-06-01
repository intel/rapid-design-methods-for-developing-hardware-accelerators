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
      |      o(1) = a(1) + o(0)
      |      o(2) = a(2) + o(1)
      |      o(3) = a(3) + o(2)
      |      O!o
      |    }
      |    wait
      |  }
      |}
    """.stripMargin.trim)
)

class VectorAddUpdateThen extends ImperativeModule( 
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
      |      if ( true) {
      |        o(0) = a(0)
      |        o(1) = a(1) + o(0)
      |        o(2) = a(2) + o(1)
      |        o(3) = a(3) + o(2)
      |      } else {
      |        o(0) = a(0)
      |        o(1) = a(1)
      |        o(2) = a(2)
      |        o(3) = a(3)
      |      }
      |      O!o
      |    }
      |    wait
      |  }
      |}
    """.stripMargin.trim)
)

class VectorAddUpdateElse extends ImperativeModule( 
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
      |      if ( !true) {
      |        o(0) = a(0)
      |        o(1) = a(1) + o(0)
      |        o(2) = a(2) + o(1)
      |        o(3) = a(3) + o(2)
      |      } else {
      |        o(0) = a(0)
      |        o(1) = a(1)
      |        o(2) = a(2)
      |        o(3) = a(3)
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

 // Mealy
  expect( c.io("O").valid, 0)
//  expect( c.io("A").ready, 1) /* Don't care */

  step(1)

  poke( c.io("O").ready, 1)
  poke( c.io("A").valid, 1)
  val a = for( i<-0 until 4) yield BigInt(i)
  val o = for( i<-0 until 4) yield BigInt(i*(i+1)/2)
  poke( c.io("A").bits.asInstanceOf[Vec[UInt]], a.reverse)

// Mealy
  expect( c.io("O").valid, 1)
  expect( c.io("O").bits.asInstanceOf[Vec[UInt]], o.reverse)
  expect( c.io("A").ready, 1)

  step(1)
}

class VectorAddUpdateThenTester(c:VectorAddUpdateThen) extends PeekPokeTester(c) {
  poke( c.io("O").ready, 1)
  poke( c.io("A").valid, 0)

// Mealy
  expect( c.io("O").valid, 0)
//  expect( c.io("A").ready, 0) /* Don't care */

  step(1)

  poke( c.io("O").ready, 1)
  poke( c.io("A").valid, 1)
  val a = for( i<-0 until 4) yield BigInt(i)
  val o = for( i<-0 until 4) yield BigInt(i*(i+1)/2)
  poke( c.io("A").bits.asInstanceOf[Vec[UInt]], a.reverse)

  expect( c.io("O").valid, 1) // Moore
  expect( c.io("O").bits.asInstanceOf[Vec[UInt]], o.reverse)
  expect( c.io("A").ready, 1) // Mealy

  step(1)
}

class VectorAddUpdateElseTester(c:VectorAddUpdateElse) extends PeekPokeTester(c) {
  poke( c.io("O").ready, 1)
  poke( c.io("A").valid, 0)

// Mealy
  expect( c.io("O").valid, 0)
//  expect( c.io("A").ready, 0) /* Don't care */

  step(1)

  poke( c.io("O").ready, 1)
  poke( c.io("A").valid, 1)
  val a = for( i<-0 until 4) yield BigInt(i)
  val o = for( i<-0 until 4) yield BigInt(i)
  poke( c.io("A").bits.asInstanceOf[Vec[UInt]], a.reverse)

// Mealy
  expect( c.io("O").valid, 1)
  expect( c.io("O").bits.asInstanceOf[Vec[UInt]], o.reverse)
  expect( c.io("A").ready, 1)

  step(1)
}

class VectorAddUpdateTest extends FlatSpec with Matchers {
  behavior of "VectorAddUpdate"
  it should "work" in {
    chisel3.iotesters.Driver( () => new VectorAddUpdate, "firrtl") { c =>
      new VectorAddUpdateTester( c)
    } should be ( true)
  }
}

class VectorAddUpdateThenTest extends FlatSpec with Matchers {
  behavior of "VectorAddUpdateThen"
  it should "work" in {
    chisel3.iotesters.Driver( () => new VectorAddUpdateThen, "firrtl") { c =>
      new VectorAddUpdateThenTester( c)
    } should be ( true)
  }
}

class VectorAddUpdateElseTest extends FlatSpec with Matchers {
  behavior of "VectorAddUpdateElse"
  it should "work" in {
    chisel3.iotesters.Driver( () => new VectorAddUpdateElse, "firrtl") { c =>
      new VectorAddUpdateElseTester( c)
    } should be ( true)
  }
}
