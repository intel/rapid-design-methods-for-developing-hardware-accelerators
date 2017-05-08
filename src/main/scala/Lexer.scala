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
    phrase(rep1(true_t | while_t | wait_t
      | nbgetdata | nbcanget | nbcanput | nbget | nbput
      | if_t | else_t
      | and_t | not_t | lbrace | rbrace | lparen | rparen | comma
      | identifier)) ^^ { rawTokens =>
      rawTokens
    }
  }

  def identifier: Parser[IDENTIFIER] = positioned {
    "[a-zA-Z_][a-zA-Z0-9_]*".r ^^ { str => IDENTIFIER(str) }
  }

  def true_t        = positioned { "true"          ^^ (_ => TRUE()) }
  def while_t       = positioned { "while"         ^^ (_ => WHILE()) }
  def wait_t        = positioned { "wait"          ^^ (_ => WAIT()) }
  def nbcanget      = positioned { "NBCanGet"      ^^ (_ => NBCANGET()) }
  def nbcanput      = positioned { "NBCanPut"      ^^ (_ => NBCANPUT()) }
  def nbgetdata     = positioned { "NBGetData"     ^^ (_ => NBGETDATA()) }
  def nbget         = positioned { "NBGet"         ^^ (_ => NBGET()) }
  def nbput         = positioned { "NBPut"         ^^ (_ => NBPUT()) }
  def if_t          = positioned { "if"            ^^ (_ => IF()) }
  def else_t        = positioned { "else"          ^^ (_ => ELSE()) }
  def and_t         = positioned { "&&"            ^^ (_ => AND()) }
  def not_t         = positioned { "!"             ^^ (_ => NOT()) }
  def lbrace        = positioned { "{"             ^^ (_ => LBRACE()) }
  def rbrace        = positioned { "}"             ^^ (_ => RBRACE()) }
  def lparen        = positioned { "("             ^^ (_ => LPAREN()) }
  def rparen        = positioned { ")"             ^^ (_ => RPAREN()) }
  def comma         = positioned { ","             ^^ (_ => COMMA()) }

}
