package test

import lexer._

import org.scalatest.{FlatSpec, Matchers}

class LexerSpec extends FlatSpec with Matchers {

  val validCode =
    """
      |while ( true) {
      |  if ( NBCanGet( P) && NBCanPut( Q)) {
      |     NBGet( P)
      |     NBPut( Q, NBGetData( P))
      |  }
      |  wait
      |}
    """.stripMargin.trim

  val validTokens = List(WHILE(), LPAREN(), TRUE(), RPAREN(), LBRACE(), IF(), LPAREN(), NBCANGET(), LPAREN(), IDENTIFIER("P"), RPAREN(), AND(), NBCANPUT(), LPAREN(), IDENTIFIER("Q"), RPAREN(), RPAREN(), LBRACE(), NBGET(), LPAREN(), IDENTIFIER("P"), RPAREN(), NBPUT(), LPAREN(), IDENTIFIER("Q"), COMMA(), NBGETDATA(), LPAREN(), IDENTIFIER("P"), RPAREN(), RPAREN(), RBRACE(), WAIT(), RBRACE())

  "Lexer" should "successfully lex a valid code" in {
    Lexer(validCode) shouldBe Right(validTokens)
  }

}
