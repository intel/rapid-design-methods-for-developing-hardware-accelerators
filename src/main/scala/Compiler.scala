package compiler

import lexer.Lexer
import parser.Parser
import imperative.{Program, PortDeclList, Blk}

object Compiler {
  def apply(code: String): Either[CompilationError, Program] = {
    for {
      tokens <- Lexer(code).right
      ast <- Parser(tokens).right
    } yield ast
  }
  def run(code: String): Program = {
    apply(code) match {
      case Right(ast) => ast
      case Left(ex) => {
        println( ex)
        assert( false)
        Program( PortDeclList( List()), Blk( List(), List()))
      }
    }
  }
}
