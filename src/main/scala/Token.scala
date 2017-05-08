package lexer

import scala.util.parsing.input.Positional

sealed trait Token extends Positional

case class IDENTIFIER(str: String) extends Token
case class INTEGER(i: BigInt) extends Token
case class WHILE() extends Token
case class TRUE() extends Token
case class LBRACE() extends Token
case class RBRACE() extends Token
case class IF() extends Token
case class ELSE() extends Token
case class AND() extends Token
case class NOT() extends Token
case class LPAREN() extends Token
case class RPAREN() extends Token
case class NBCANGET() extends Token
case class NBCANPUT() extends Token
case class NBGET() extends Token
case class NBPUT() extends Token
case class NBGETDATA() extends Token
case class COMMA() extends Token
case class VAR() extends Token
case class UINT() extends Token
case class WAIT() extends Token
case class ASSIGN() extends Token
case class EQ() extends Token
case class COLON() extends Token
