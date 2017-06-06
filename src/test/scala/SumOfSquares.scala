package imperative

import org.scalatest.{ Matchers, FlatSpec, GivenWhenThen, FreeSpec}

import chisel3._
import chisel3.util._
import chisel3.iotesters._

import compiler._

class SumOfSquares extends ImperativeModule(
  Compiler.runHLS3(
    """
      |process SumOfSquares ( Q : out UInt(32))
      |{
      |  var x : UInt(32)
      |  var y : UInt(32)
      |  var sum : UInt(32)
      |  x = 0
      |  while ( x < 10) {
      |    y = 0
      |    sum = 0
      |    while ( y < x) {
      |      sum = sum + y*y
      |      y = y + 1
      |      wait
      |    }
      |    Q!!sum
      |    x = x + 1
      |    wait
      |  }
      |}
    """.stripMargin.trim))

class SumOfSquaresTester[T <: ImperativeModule](c:T) extends PeekPokeTester(c) {
  var ts = 0
  poke( c.io("Q").ready, 1)
  for( i <- 0 until 10) {
    var steps = 0
    while ( peek( c.io("Q").valid) == BigInt(0) && steps < 100) {
      step(1)
      println( s"Waiting at ${ts}")
      ts += 1
      steps += 1
    }
    println( s"Sent ${peek( c.io("Q").bits.asInstanceOf[UInt])} at time ${ts}")
    expect( c.io("Q").valid, BigInt(1))
    expect( c.io("Q").bits.asInstanceOf[UInt], BigInt( (2*i-1)*i*(i-1)/6))
    step(1)
    ts += 1
  }
// No more comms
  for( i <- 0 until 20) {
    expect( c.io("Q").valid, BigInt(0))
    step(1)
    println( s"Checking for extra comms at ${ts}")
    ts += 1
  }
}

class SumOfSquaresTest extends FlatSpec with Matchers {
  behavior of "SumOfSquares"
  it should "work" in {
    chisel3.iotesters.Driver.execute( Array( /*"--fint-write-vcd",*/ "-fct", "imperative.transform.ShannonFactor", "--backend-name", "firrtl"), () => new SumOfSquares) { c =>
      new SumOfSquaresTester( c)
    } should be ( true)
  }
}
