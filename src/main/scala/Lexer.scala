package lexer

import compiler.{Location, LexerError}

import scala.util.parsing.combinator.RegexParsers

object Lexer extends RegexParsers {
  override def skipWhitespace = true
  override val whiteSpace = "[ \t\r\f\n]+".r

  def apply(code: String): Either[LexerError, List[Token]] = {
    parse(tokens, code) match {
      case NoSuccess(msg, next) => Left(LexerError(Location(next.pos.line, next.pos.column), msg))
      case Success(result, next) => Right(result)
    }
  }

  def tokens: Parser[List[Token]] = {
    phrase(rep1(true_t | while_t | wait_t | var_t | uint_t | vec_t
      | if_t | else_t | process_t | inp_t | out_t
      | and_t | lbrace | rbrace | lparen | rparen | comma
      | eq_t | assign | colon_t | add_t | mul_t
      | query_t | bang_t
      | identifier | integer)) ^^ { rawTokens =>
      rawTokens
    }
  }

  def identifier: Parser[IDENTIFIER] = positioned {
    "[a-zA-Z_][a-zA-Z0-9_]*".r ^^ { str => IDENTIFIER(str) }
  }

  def integer: Parser[INTEGER] = positioned {
    "0|[1-9][0-9]*".r ^^ { str => INTEGER(BigInt(str)) }
  }

  def var_t         = positioned { "var"           ^^ (_ => VAR()) }
  def process_t     = positioned { "process"       ^^ (_ => PROCESS()) }
  def inp_t         = positioned { "inp"           ^^ (_ => INP()) }
  def out_t         = positioned { "out"           ^^ (_ => OUT()) }
  def uint_t        = positioned { "UInt"          ^^ (_ => UINT()) }
  def vec_t         = positioned { "Vec"           ^^ (_ => VEC()) }
  def true_t        = positioned { "true"          ^^ (_ => TRUE()) }
  def while_t       = positioned { "while"         ^^ (_ => WHILE()) }
  def wait_t        = positioned { "wait"          ^^ (_ => WAIT()) }
  def if_t          = positioned { "if"            ^^ (_ => IF()) }
  def else_t        = positioned { "else"          ^^ (_ => ELSE()) }
  def and_t         = positioned { "&&"            ^^ (_ => AND()) }
  def lbrace        = positioned { "{"             ^^ (_ => LBRACE()) }
  def rbrace        = positioned { "}"             ^^ (_ => RBRACE()) }
  def lparen        = positioned { "("             ^^ (_ => LPAREN()) }
  def rparen        = positioned { ")"             ^^ (_ => RPAREN()) }
  def comma         = positioned { ","             ^^ (_ => COMMA()) }
  def eq_t          = positioned { "=="            ^^ (_ => EQ()) }
  def assign        = positioned { "="             ^^ (_ => ASSIGN()) }
  def colon_t       = positioned { ":"             ^^ (_ => COLON()) }
  def add_t         = positioned { "+"             ^^ (_ => ADD()) }
  def mul_t         = positioned { "*"             ^^ (_ => MUL()) }
  def query_t       = positioned { "?"             ^^ (_ => QUERY()) }
  def bang_t        = positioned { "!"             ^^ (_ => BANG()) }

}
