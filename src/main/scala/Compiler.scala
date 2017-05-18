package compiler

import lexer.Lexer
import parser.Parser
import imperative.{Process, PortDeclList, Blk, SemanticAnalyzer}

object Compiler {
  def apply(code: String): Either[CompilationError, Process] = {
    for {
      tokens <- Lexer(code).right
      ast <- Parser(tokens).right
      ast2 <- SemanticAnalyzer( ast).right
    } yield ast2
  }
  def run(code: String): Process = {
    apply(code) match {
      case Right(ast) => ast
      case Left(ex) => {
        println( s"${ex}")
        throw new CompilationErrorException( ex)
      }
    }
  }
}
