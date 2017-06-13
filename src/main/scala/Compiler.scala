package compiler

import lexer.Lexer
import parser.Parser
import imperative.{Process, SemanticAnalyzer, HLS, HLS2, HLS3}

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
  def hls(code: String): Either[CompilationError, Process] = {
    for {
      tokens <- Lexer(code).right
      ast <- Parser(tokens).right
      ast2 <- HLS( ast).right
    } yield ast2
  }
  def hls2(code: String): Either[CompilationError, Process] = {
    for {
      tokens <- Lexer(code).right
      ast <- Parser(tokens).right
      ast2 <- HLS2( ast).right
    } yield ast2
  }
  def hls3(code: String): Either[CompilationError, Process] = {
    for {
      tokens <- Lexer(code).right
      ast <- Parser(tokens).right
      ast2 <- HLS3( ast).right
    } yield ast2
  }
  def runHLS(code: String): Process = {
    hls(code) match {
      case Right(ast) => ast
      case Left(ex) => {
        println( s"${ex}")
        throw new CompilationErrorException( ex)
      }
    }
  }
  def runHLS2(code: String): Process = {
    hls2(code) match {
      case Right(ast) => ast
      case Left(ex) => {
        println( s"${ex}")
        throw new CompilationErrorException( ex)
      }
    }
  }
  def runHLS3(code: String): Process = {
    hls3(code) match {
      case Right(ast) => ast
      case Left(ex) => {
        println( s"${ex}")
        throw new CompilationErrorException( ex)
      }
    }
  }
}
