package imperative

import org.scalatest.{ Matchers, FlatSpec, FreeSpec}

import chisel3._
import chisel3.iotesters._

import compiler.Compiler

class BadInitSeg extends ImperativeModule(
  Compiler.run("""
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
"""))

class BadInitSeg2 extends ImperativeModule(
  Compiler.run("""
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
"""))

class BadInitSeg3 extends ImperativeModule(
  Compiler.run("""
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
"""))

class GoodInitSeg extends ImperativeModule(
  Compiler.run("""
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
"""))


class InitSegTest extends FreeSpec with Matchers {
  "Init Segments" - {
    "nbcanput in init segment should throw an assertion" in {
      an [InitialSegmentContainsCommunicationException] should be thrownBy {
        chisel3.Driver.execute( Array[String](), () => new BadInitSeg)
      }
    }
    "while in init segment should throw an assertion" in {
      an [WhileUsedIncorrectlyException] should be thrownBy {
        chisel3.Driver.execute( Array[String](), () => new BadInitSeg2)
      }
    }
    "nbput in init segment should throw an assertion" in {
      an [InitialSegmentContainsCommunicationException] should be thrownBy {
        chisel3.Driver.execute( Array[String](), () => new BadInitSeg3)
      }
    }
    "good init segment should not throw an assertion" in {
      chisel3.Driver.execute( Array[String](), () => new GoodInitSeg)
    }
  }
}

