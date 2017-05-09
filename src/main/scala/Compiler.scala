package compiler

import lexer.Lexer
import parser.Parser
import imperative.{Process, PortDeclList, Blk}

object Compiler {
  def apply(code: String): Either[CompilationError, Process] = {
    for {
      tokens <- Lexer(code).right
      ast <- Parser(tokens).right
    } yield ast
  }
  def run(code: String): Process = {
    apply(code) match {
      case Right(ast) => ast
      case Left(ex) => {
        println( ex)
        assert( false)
        Process( PortDeclList( List()), Blk( List(), List()))
      }
    }
  }
}
