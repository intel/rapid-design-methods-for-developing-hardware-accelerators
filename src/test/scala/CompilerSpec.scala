package test

import compiler.{Location, Compiler, ParserError}

import imperative._

import parser._
import org.scalatest.{FlatSpec, Matchers}

class CompilerSpec extends FlatSpec with Matchers {

  val validCode =
    """
      |while ( true) {
      |  if ( NBCanGet( P) && NBCanPut( Q)) {
      |     NBGet( P)
      |     NBPut( Q, NBGetData( P))
      |  }
      |  wait
      |}
    """.stripMargin.trim

  val validCodeWithElse =
    """
      |while ( true) {
      |  if ( NBCanGet( P) && NBCanPut( Q)) {
      |     NBGet( P)
      |     NBPut( Q, NBGetData( P))
      |  } else {
      |  }
      |  wait
      |}
    """.stripMargin.trim

  val successfulAST = While(ConstantTrue,SequentialComposition(List(IfThenElse(AndBExpression(NBCanGet(Port("P")),NBCanPut(Port("Q"))),SequentialComposition(List(NBGet(Port("P")), NBPut(Port("Q"),NBGetData(Port("P"))))),SequentialComposition(List())), Wait)))

  "Compiler" should "successfully parse a valid program" in {
    Compiler(validCode) shouldBe Right(successfulAST)
  }

  "Compiler" should "successfully parse a valid program (with else)" in {
    Compiler(validCodeWithElse) shouldBe Right(successfulAST)
  }

}

