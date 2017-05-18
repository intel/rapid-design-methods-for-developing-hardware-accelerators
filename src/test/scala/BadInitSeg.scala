package imperative

import org.scalatest.{ Matchers, FlatSpec, FreeSpec}

import chisel3._
import chisel3.iotesters._

import compiler.{ Compiler, CompilationErrorException}

object BadInitSeg {
  val txt =
"""
process BadInitSeg ( O: out UInt(80))
{
  var x : UInt(80)
  x = 0
  if ( O!) {
  }
  while ( true) {
    if ( O!) {
      x = 3*x + 1       
      O!x
    }
    wait
  }
}
"""

  val txt2 =
"""
process BadInitSeg ( O: out UInt(80))
{
  var x : UInt(80)
  x = 0
  while ( true) {
  }
  while ( true) {
    if ( O!) {
      x = 3*x + 1       
      O!x
    }
    wait
  }
}
"""

  val txt3 =
"""
process BadInitSeg ( O: out UInt(80))
{
  var x : UInt(80)
  x = 0
  O!x
  while ( true) {
    if ( O!) {
      x = 3*x + 1       
      O!x
    }
    wait
  }
}
"""

  val good =
"""
process GoodInitSeg ( O: out UInt(80))
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
"""
}

class InitSegTest extends FreeSpec with Matchers {
  "Init Segments" - {
    "nbcanput in init segment should throw an exception" in {
      an [CompilationErrorException] should be thrownBy {
        Compiler.run(BadInitSeg.txt)
      }
    }
    "while in init segment should throw an exception" in {
      an [CompilationErrorException] should be thrownBy {
        Compiler.run(BadInitSeg.txt2)
      }
    }
    "nbput in init segment should throw an exception" in {
      an [CompilationErrorException] should be thrownBy {
        Compiler.run(BadInitSeg.txt3)
      }
    }
    "good init segment should compile" in {
      Compiler(BadInitSeg.good).isRight should be (true)
    }
  }
}

