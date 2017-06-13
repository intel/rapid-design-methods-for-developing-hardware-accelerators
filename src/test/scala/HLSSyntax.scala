package imperative

import org.scalatest.{ Matchers, FlatSpec, FreeSpec}

import chisel3._
import chisel3.iotesters._

import compiler.{ Compiler, CompilationErrorException}

object HLSSyntax {
  val txt =
"""
process Bad ( P: inp UInt(80), Q: out UInt(80))
{
  var x : UInt(80)
  while ( true) {
    var y : UInt(80)
    P??x
    Q!!x
    wait
  }
}
"""

  val txt2 =
"""
process Bad ( Q: out UInt(80))
{
  var x : UInt(80)
  x = 0
  while ( true) {
    Q!!x
    x = x + 1
    wait
  }
}
"""
}

class HLSSyntaxTest extends FreeSpec with Matchers {
  "HLSSyntax" - {
    "an inner variable declaration should throw an exception" in {
      a [NotYetImplementedException] should be thrownBy {
        Compiler.runHLS(HLSSyntax.txt)
      }
    }
    "assignment should be legal" in {
//      a [NotYetImplementedException] should be thrownBy {
        Compiler.runHLS(HLSSyntax.txt2)
//      }
    }
  }
}

