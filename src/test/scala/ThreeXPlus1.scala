package imperative

import org.scalatest.{ Matchers, FlatSpec}

import chisel3._
import chisel3.iotesters._

import compiler.Compiler

object ThreeXPlus1 {
  val width = 80
}

class ThreeXPlus1 extends ImperativeModule(
  Compiler.run("""
process ThreeXPlus1 ( O: out UInt(80))
{
  var x : UInt(80)
  x = 0
  while ( true) {
    if ( O!) {
      x = 3*x + 1       
      O!x
    }
    wait
  }
}
"""))

class ThreeXPlus1Tester(c:ThreeXPlus1) extends PeekPokeTester(c) {
  var x : BigInt = 0

  for( i <- 0 until 10) {
    poke( c.io("O").ready, 1)

    step(1)

    if ( peek( c.io("O").valid) == 1) {
      x = 3*x+1
      expect( c.io("O").bits.asInstanceOf[UInt], x)
    }

  }
}

class ThreeXPlus1Test extends FlatSpec with Matchers {
  behavior of "ThreeXPlus1"
  it should "work" in {
    chisel3.iotesters.Driver( () => new ThreeXPlus1, "firrtl") { c =>
      new ThreeXPlus1Tester( c)
    } should be ( true)
  }
}
