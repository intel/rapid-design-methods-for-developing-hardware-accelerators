package compiler

import lexer.Lexer
import parser.Parser
import imperative.Command

object Compiler {
  def apply(code: String): Either[CompilationError, Command] = {
    for {
      tokens <- Lexer(code).right
      ast <- Parser(tokens).right
    } yield ast
  }
}
