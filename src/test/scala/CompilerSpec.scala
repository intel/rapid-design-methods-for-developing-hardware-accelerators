package test

import compiler.{Location, Compiler, ParserError}

import imperative._

import parser._
import org.scalatest.{FlatSpec, Matchers}

class CompilerSpec extends FlatSpec with Matchers {

  val validCode =
    """
      |process ValidNoElse( P : inp UInt(8), Q : out UInt(8)) {
      |  while ( true) {
      |    if ( NBCanGet( P) && NBCanPut( Q)) {
      |      var v : UInt(8)
      |      NBGet( P, v)
      |      NBPut( Q, v)
      |    }
      |    wait
      |  }
      |}
    """.stripMargin.trim

  val validCodeWithElse =
    """
      |process ValidWithElse( P : inp UInt(8), Q : out UInt(8)) {
      |  while ( true) {
      |    if ( NBCanGet( P) && NBCanPut( Q)) {
      |      var v : UInt(8)
      |      NBGet( P, v)
      |      NBPut( Q, v)
      |    } else {
      |    }
      |    wait
      |  }
      |}
    """.stripMargin.trim

  val successfulAST = Program(PortDeclList(List(PortDecl(Port("P"),Inp,Type(8)), PortDecl(Port("Q"),Out,Type(8)))),Blk(List(),List(While(ConstantTrue,Blk(List(),List(IfThenElse(AndBExpression(NBCanGet(Port("P")),NBCanPut(Port("Q"))),Blk(List(Decl(Variable("v"),Type(8))),List(NBGet(Port("P"),Variable("v")), NBPut(Port("Q"),Variable("v")))),Blk(List(),List())), Wait))))))

  "Compiler" should "successfully parse a valid program" in {
    Compiler(validCode) shouldBe Right(successfulAST)
  }

  "Compiler" should "successfully parse a valid program (with else)" in {
    Compiler(validCodeWithElse) shouldBe Right(successfulAST)
  }

}

