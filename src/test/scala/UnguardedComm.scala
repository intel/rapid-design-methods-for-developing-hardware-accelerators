package imperative

import org.scalatest.{ Matchers, FlatSpec, GivenWhenThen, FreeSpec}

import scala.util.control.Breaks

import chisel3._
import chisel3.util._
import chisel3.iotesters._

import compiler._

object UnguardedComm {
  val txt =
    """
      |process Div2 ( P : inp UInt(8),
      |               Q : out UInt(8))
      |{
      |  while ( true) {
      |    if ( P?) {
      |      var a : UInt(8)
      |      P?a
      |      Q!a
      |    }
      |    wait
      |  }
      |}
    """.stripMargin.trim
  val txt2 =
    """
      |process Div2 ( P : inp UInt(8),
      |               Q : out UInt(8))
      |{
      |  while ( true) {
      |    if ( P?) {
      |      var a : UInt(8)
      |      if ( Q!) {
      |        P?a
      |        Q!a
      |      }
      |    }
      |    wait
      |  }
      |}
    """.stripMargin.trim
  val txt3 =
    """
      |process Div2 ( P : inp UInt(8),
      |               Q : out UInt(8))
      |{
      |  while ( true) {
      |    if ( P?) {
      |      var a : UInt(8)
      |      if ( Q!) {
      |        P?a
      |      }
      |      Q!a
      |    }
      |    wait
      |  }
      |}
    """.stripMargin.trim
  val txt4 =
    """
      |process Div2 ( P : inp UInt(8),
      |               Q : out UInt(8))
      |{
      |  while ( true) {
      |    if ( !!P? && Q!) {
      |      var a : UInt(8)
      |      P?a
      |      Q!a
      |    }
      |    wait
      |  }
      |}
    """.stripMargin.trim
}

class UnguardedCommTest extends FreeSpec with Matchers {
  "Unguarded comm test" - {
    "Missing Q! guard should throw an exception" in {
      an [CompilationErrorException] should be thrownBy {
        Compiler.run(UnguardedComm.txt)
      }
    }
    "Second Guard should work as well" in {
      Compiler(UnguardedComm.txt2).isRight should be (true)
    }
    "Out of scope Q! should throw an exception" in {
      an [CompilationErrorException] should be thrownBy {
        Compiler.run(UnguardedComm.txt3)
      }
    }
    "Negation of guards is not processed and P? will be ignored causing an exception to be thrown" in {
      an [CompilationErrorException] should be thrownBy {
        Compiler.run(UnguardedComm.txt4)
      }
    }
  }
}
