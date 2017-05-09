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


  def apply(tokens: Seq[Token]): Either[ParserError, Process] = {
    val reader = new TokenReader(tokens)
    process(reader) match {
      case NoSuccess(msg, next) => Left(ParserError(Location(next.pos.line, next.pos.column), msg))
      case Success(result, next) => Right(result)
    }
  }


  def process: Parser[Process] = positioned {
    val p = PROCESS() ~ identifier ~ LPAREN() ~ portDeclList ~ RPAREN() ~ cmd ^^ {
      case _ ~ IDENTIFIER(progName) ~ _ ~ lst ~ _ ~ cmd =>
        Process( lst, cmd)
    }
    phrase(p)
  }

  def portDeclList: Parser[PortDeclList] = positioned {
    val pdl = portDecl ~ rep(COMMA() ~ portDecl) ^^ {
      case hd ~ tl => 
        PortDeclList( (tl.foldLeft(List(hd)) { case (acc, COMMA() ~ pd) => pd :: acc}).reverse)
    }
    pdl
  }

  def portDecl: Parser[PortDecl] = positioned {
    val pd = identifier ~ COLON() ~ dir ~ UINT() ~ LPAREN() ~ integer ~ RPAREN() ^^ {
      case IDENTIFIER(p) ~ _ ~ dir ~ _ ~ _ ~ INTEGER(w) ~ _ =>
        PortDecl( Port( p), dir, Type( w.toInt))
    }
    pd
  }

  def dir: Parser[Dir] = positioned { 
    val inp = INP() ^^ { _ => Inp}
    val out = OUT() ^^ { _ => Out}
    inp | out
  }

  def cmd: Parser[Command] = positioned {
    val whl = WHILE() ~ LPAREN() ~ bexpr ~ RPAREN() ~ cmd ^^ {
      case _ ~ _ ~ cond ~ _ ~ cmd => While(cond,cmd)
    }
    val ite = IF() ~ LPAREN() ~ bexpr ~ RPAREN() ~ cmd ~ ELSE() ~ cmd ^^ {
      case _ ~ _ ~ i ~ _ ~ t ~ _ ~ e => IfThenElse(i,t,e)
    }
    val it = IF() ~ LPAREN() ~ bexpr ~ RPAREN() ~ cmd ^^ {
      case _ ~ _ ~ i ~ _ ~ t => IfThenElse(i,t,Blk( List(), List()))
    }
    val sc = LBRACE() ~ rep(decl) ~ rep(cmd) ~ RBRACE() ^^ { 
      case _ ~ d ~ s ~ _ => Blk( d.toList, s.toList)
    }
    val w = WAIT() ^^ { _ => Wait }
    val g = identifier ~ QUERY() ~ identifier ^^ { 
      case IDENTIFIER(p) ~ _ ~ IDENTIFIER(v) => NBGet( Port(p), Variable(v))
    }
    val p = identifier ~ BANG() ~ expr ^^ { 
      case IDENTIFIER(s) ~ _ ~ e => NBPut( Port(s), e)
    }
    val a = identifier ~ ASSIGN() ~ expr ^^ {
      case IDENTIFIER( v) ~ _ ~ e => Assignment( Variable( v), e)
    }

    whl | ite | it | sc | w | g | p | a
  }

  def bexpr: Parser[BExpression] = positioned {
    val a = bterm ~ AND() ~ bexpr ^^ { 
      case l ~ _ ~ r => AndBExpression( l, r)
    }
    a | bterm
  }

  def bterm: Parser[BExpression] = positioned {
    val t = TRUE() ^^ { _ => ConstantTrue }
    val cg = identifier ~ QUERY() ^^ {
      case IDENTIFIER(s) ~ _ => NBCanGet( Port( s))
    }
    val cp = identifier ~ BANG() ^^ {
      case IDENTIFIER(s) ~ _ => NBCanPut( Port( s))
    }
    val n = BANG() ~ bexpr ^^ { 
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
    val m = term ~ ADD() ~ expr ^^ { 
      case t ~ _ ~ e => AddExpression( t, e)
    }
    val t = term ^^ { 
      case t => t
    }
    m | t
  }

  def term: Parser[Expression] = positioned {
    val a = prim ~ MUL() ~ term ^^ { 
      case p ~ _ ~ t => MulExpression( p, t)
    }
    val p = prim ^^ { 
      case p => p
    }
    a | p
  }

  def prim: Parser[Expression] = positioned {
    val v = identifier ^^ { 
      case IDENTIFIER(v) => Variable(v)
    }
    val i = integer ^^ { 
      case INTEGER(v) => ConstantInteger( v.toInt)
    }
    val e = LPAREN() ~ expr ~ RPAREN() ^^ { 
      case _ ~ e ~ _ => e
    }
    v | i | e
  }

  def decl: Parser[Decl] = positioned {
    val d = VAR() ~ identifier ~ COLON() ~ UINT() ~ LPAREN() ~ integer ~ RPAREN() ^^ { 
      case _ ~ IDENTIFIER(v) ~ _ ~ _ ~ _ ~ INTEGER(w) ~ _ => Decl( Variable(v), Type(w.toInt))
      case q => {
        println( s"Bad match: ${q}")
        Decl( Variable("<xxx>"), Type(1))
      }
    }
    d
  }

  private def identifier: Parser[IDENTIFIER] = positioned {
    accept("identifier", { case id @ IDENTIFIER(name) => id })
  }

  private def integer: Parser[INTEGER] = positioned {
    accept("integer", { case id @ INTEGER( bi) => id })
  }

}
