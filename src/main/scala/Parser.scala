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
    val pd = identifier ~ COLON() ~ dir ~ type_p ^^ {
      case IDENTIFIER(p) ~ _ ~ dir ~ t =>
        PortDecl( Port( p), dir, t)
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
    val unrl = UNROLL() ~ LPAREN() ~ identifier ~ COMMA() ~ expr ~ COMMA() ~ expr ~ RPAREN() ~ cmd ^^ {
      case _ ~ _ ~ IDENTIFIER(v) ~ _ ~ lb ~ _ ~ ub ~ _ ~ cmd => Unroll(Variable(v),lb,ub,cmd)
    }
    val ite = IF() ~ LPAREN() ~ bexpr ~ RPAREN() ~ cmd ~ ELSE() ~ cmd ^^ {
      case _ ~ _ ~ i ~ _ ~ t ~ _ ~ e => IfThenElse(i,t,e)
    }
    val it = IF() ~ LPAREN() ~ bexpr ~ RPAREN() ~ cmd ^^ {
      case _ ~ _ ~ i ~ _ ~ t => IfThenElse(i,t,Blk( List(), List()))
    }
    val sc = LBRACE() ~! rep(decl) ~ rep(cmd) ~ RBRACE() ^^ { 
      case _ ~ d ~ s ~ _ => Blk( d.toList, s.toList)
    }
    val w = WAIT() ^^ { _ => Wait }
    val g = identifier ~ QUERY() ~ identifier ^^ { 
      case IDENTIFIER(p) ~ _ ~ IDENTIFIER(v) => NBGet( Port(p), Variable(v))
    }
    val p = identifier ~ BANG() ~ expr ^^ { 
      case IDENTIFIER(s) ~ _ ~ e => NBPut( Port(s), e)
    }
    val va = identifier ~ LPAREN() ~ expr ~ RPAREN() ~ ASSIGN() ~ expr ^^ { 
      case IDENTIFIER(v) ~ _ ~ i ~ _ ~ _ ~ e => Assignment( VectorIndex(v,i), e)
    }
    val a = identifier ~ ASSIGN() ~ expr ^^ {
      case IDENTIFIER( v) ~ _ ~ e => Assignment( Variable( v), e)
    }

    whl | unrl | ite | it | sc | w | g | p | va | a
  }

  def bexpr: Parser[BExpression] = positioned {
    val a = bterm ~ ( AND() | LAND()) ~ bexpr ^^ { 
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
    val n = ( BANG() | LNOT()) ~ bexpr ^^ { 
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
    val s = term ~ SUB() ~ expr ^^ { 
      case t ~ _ ~ e => SubExpression( t, e)
    }
    val t = term ^^ { 
      case t => t
    }
    m | s | t
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
    val vi = identifier ~ LPAREN() ~ expr ~ RPAREN() ^^ { 
      case IDENTIFIER(v) ~ _ ~ e ~ _ => VectorIndex(v,e)
    }
    val v = identifier ^^ { 
      case IDENTIFIER(v) => Variable(v)
    }
    val i = integer ^^ { 
      case INTEGER(v) => ConstantInteger( v.toInt)
    }
    val e = LPAREN() ~ expr ~ RPAREN() ^^ { 
      case _ ~ e ~ _ => e
    }
    vi | v | i | e
  }

  def decl: Parser[Decl] = positioned {
    val d = VAR() ~ identifier ~ COLON() ~ type_p ^^ { 
      case _ ~ IDENTIFIER(v) ~ _ ~ t => Decl( Variable(v), t)
    }
    d
  }

  def type_p: Parser[Type] = positioned {
    val v = VEC() ~ LPAREN() ~ integer ~ COMMA() ~ type_p ~ RPAREN() ^^ { 
      case _ ~ _ ~ INTEGER(n) ~ _ ~ t ~ _ => VecType( n.toInt, t)
    }
    val t = UINT() ~ LPAREN() ~ integer ~ RPAREN() ^^ { 
      case _ ~ _ ~ INTEGER(w) ~ _ => UIntType(w.toInt)
    }
    v | t
  }

  private def identifier: Parser[IDENTIFIER] = positioned {
    accept("identifier", { case id @ IDENTIFIER(name) => id })
  }

  private def integer: Parser[INTEGER] = positioned {
    accept("integer", { case id @ INTEGER( bi) => id })
  }

}
