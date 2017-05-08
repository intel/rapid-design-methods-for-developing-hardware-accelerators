package parser

import imperative._

import compiler.{Location, ParserError}
import lexer._

import scala.util.parsing.combinator.Parsers
import scala.util.parsing.input.{NoPosition, Position, Reader}

object Parser extends Parsers {
  override type Elem = Token

  class TokenReader(tokens: Seq[Token]) extends Reader[Token] {
    override def first: Token = tokens.head
    override def atEnd: Boolean = tokens.isEmpty
    override def pos: Position = tokens.headOption.map(_.pos).getOrElse(NoPosition)
    override def rest: Reader[Token] = new TokenReader(tokens.tail)
  }


  def apply(tokens: Seq[Token]): Either[ParserError, Command] = {
    val reader = new TokenReader(tokens)
    program(reader) match {
      case NoSuccess(msg, next) => Left(ParserError(Location(next.pos.line, next.pos.column), msg))
      case Success(result, next) => Right(result)
    }
  }


  def program: Parser[Command] = positioned {
    phrase(cmd)
  }

  def cmd: Parser[Command] = positioned {
    val whl = WHILE() ~ LPAREN() ~ bexpr ~ RPAREN() ~ cmd ^^ {
      case _ ~ _ ~ cond ~ _ ~ cmd => While(cond,cmd)
    }
    val ite = IF() ~ LPAREN() ~ bexpr ~ RPAREN() ~ cmd ~ ELSE() ~ cmd ^^ {
      case _ ~ _ ~ i ~ _ ~ t ~ _ ~ e => IfThenElse(i,t,e)
    }
    val it = IF() ~ LPAREN() ~ bexpr ~ RPAREN() ~ cmd ^^ {
      case _ ~ _ ~ i ~ _ ~ t => IfThenElse(i,t,SequentialComposition( List()))
    }
    val sc = LBRACE() ~ rep(cmd) ~ RBRACE() ^^ { 
      case _ ~ s ~ _ => SequentialComposition( s.toList)
    }
    val g = NBGET() ~ LPAREN() ~ identifier ~ RPAREN() ^^ { 
      case _ ~ _ ~ IDENTIFIER(s) ~ _ => NBGet( Port(s))
    }
    val p = NBPUT() ~ LPAREN() ~ identifier ~ COMMA() ~ expr ~ RPAREN() ^^ { 
      case _ ~ _ ~ IDENTIFIER(s) ~ _ ~ e ~ _ => NBPut( Port(s), e)
    }
    val w = WAIT() ^^ { _ => Wait }
    val a = identifier ~ ASSIGN() ~ expr ^^ {
      case IDENTIFIER( v) ~ _ ~ e => Assignment( Variable( v), e)
    }

    whl | ite | it | sc | g | p | w | a
  }

  def bexpr: Parser[BExpression] = positioned {
    val a = bterm ~ AND() ~ bexpr ^^ { 
      case l ~ _ ~ r => AndBExpression( l, r)
    }
    a | bterm
  }

  def bterm: Parser[BExpression] = positioned {
    val t = TRUE() ^^ { _ => ConstantTrue }
    val cg = NBCANGET() ~ LPAREN() ~ identifier ~ RPAREN() ^^ {
      case _ ~ _ ~ IDENTIFIER(s) ~ _ => NBCanGet( Port( s))
    }
    val cp = NBCANPUT() ~ LPAREN() ~ identifier ~ RPAREN() ^^ {
      case _ ~ _ ~ IDENTIFIER(s) ~ _ => NBCanPut( Port( s))
    }
    val n = NOT() ~ bexpr ^^ { 
      case _ ~ n => NotBExpression( n)
    }
    val g = LPAREN() ~ bexpr ~ RPAREN() ^^ { 
      case _ ~ e ~ _ => e
    }
    val e = expr ~ EQ() ~ expr ^^ { 
      case l ~ _ ~ r => EqBExpression( l, r)
    }
    t | cg | cp | n | g | e
  }

  def expr: Parser[Expression] = positioned {
    val v = identifier ^^ { 
      case IDENTIFIER(v) => Variable(v)
    }
    val i = integer ^^ { 
      case INTEGER(v) => ConstantInteger( v.toInt)
    }
    val gd = NBGETDATA() ~ LPAREN() ~ identifier ~ RPAREN() ^^ { 
      case _ ~ _ ~ IDENTIFIER(s) ~ _ => NBGetData( Port( s))
    }
    v | i | gd
  }

  private def identifier: Parser[IDENTIFIER] = positioned {
    accept("identifier", { case id @ IDENTIFIER(name) => id })
  }

  private def integer: Parser[INTEGER] = positioned {
    accept("integer", { case id @ INTEGER( bi) => id })
  }

}
