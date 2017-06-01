package lexer

import compiler.{Location, LexerError}

import scala.util.parsing.combinator.RegexParsers

object Lexer extends RegexParsers {
//  override def skipWhitespace = true
//  override val whiteSpace = "[ \t\r\f\n]+".r

  def apply(code: String): Either[LexerError, List[Token]] = {
    parse(tokens, code) match {
      case NoSuccess(msg, next) => Left(LexerError(Location(next.pos.line, next.pos.column), msg))
      case Success(result, next) => Right(result)
    }
  }

  def identifier: Parser[IDENTIFIER] = positioned {
    "[a-zA-Z_][a-zA-Z0-9_]*".r ^^ { str => IDENTIFIER(str) }
  }

  def integer: Parser[INTEGER] = positioned {
    "0|[1-9][0-9]*".r ^^ { str => INTEGER(BigInt(str)) }
  }

  def tokens: Parser[List[Token]] = {
    phrase(rep1(

// Word boundary to keep prefixes from matching
        positioned { "var\\b".r      ^^ (_ => VAR()) }
      | positioned { "process\\b".r  ^^ (_ => PROCESS()) }
      | positioned { "inp\\b".r      ^^ (_ => INP()) }
      | positioned { "out\\b".r      ^^ (_ => OUT()) }
      | positioned { "UInt\\b".r     ^^ (_ => UINT()) }
      | positioned { "Vec\\b".r      ^^ (_ => VEC()) }
      | positioned { "true\\b".r     ^^ (_ => TRUE()) }
      | positioned { "while\\b".r    ^^ (_ => WHILE()) }
      | positioned { "wait\\b".r     ^^ (_ => WAIT()) }
      | positioned { "if\\b".r       ^^ (_ => IF()) }
      | positioned { "else\\b".r     ^^ (_ => ELSE()) }
      | positioned { "unroll\\b".r   ^^ (_ => UNROLL()) }
      | positioned { "&&"            ^^ (_ => AND()) }
      | positioned { "??"            ^^ (_ => QUERYQUERY()) }
      | positioned { "!!"            ^^ (_ => BANGBANG()) }
      | positioned { "{"             ^^ (_ => LBRACE()) }
      | positioned { "}"             ^^ (_ => RBRACE()) }
      | positioned { "("             ^^ (_ => LPAREN()) }
      | positioned { ")"             ^^ (_ => RPAREN()) }
      | positioned { ","             ^^ (_ => COMMA()) }
      | positioned { "=="            ^^ (_ => EQ()) }
      | positioned { "<"             ^^ (_ => LT()) }
      | positioned { "="             ^^ (_ => ASSIGN()) }
      | positioned { ":"             ^^ (_ => COLON()) }
      | positioned { "+"             ^^ (_ => ADD()) }
      | positioned { "-"             ^^ (_ => SUB()) }
      | positioned { "*"             ^^ (_ => MUL()) }
      | positioned { "?"             ^^ (_ => QUERY()) }
      | positioned { "!"             ^^ (_ => BANG()) }
      | positioned { "¬"             ^^ (_ => LNOT()) }
      | positioned { "∧"            ^^ (_ => LAND()) }
      | positioned { "∨"            ^^ (_ => LOR()) }

      | identifier | integer)) ^^ { rawTokens =>
      rawTokens
    }
  }



}
