package test

import lexer._

import org.scalatest.{FlatSpec, Matchers}

class LexerSpec extends FlatSpec with Matchers {

  val validCode =
    """
      |while ( true) {
      |  if ( NBCanGet( P) && NBCanPut( Q)) {
      |     NBGet( P, v)
      |     NBPut( Q, v)
      |  }
      |  wait
      |}
    """.stripMargin.trim

  val validTokens = List(WHILE(), LPAREN(), TRUE(), RPAREN(), LBRACE(), IF(), LPAREN(), NBCANGET(), LPAREN(), IDENTIFIER("P"), RPAREN(), AND(), NBCANPUT(), LPAREN(), IDENTIFIER("Q"), RPAREN(), RPAREN(), LBRACE(), NBGET(), LPAREN(), IDENTIFIER("P"), COMMA(), IDENTIFIER("v"), RPAREN(), NBPUT(), LPAREN(), IDENTIFIER("Q"), COMMA(), IDENTIFIER("v"), RPAREN(), RBRACE(), WAIT(), RBRACE())

  "Lexer" should "successfully lex a valid code" in {
    Lexer(validCode) shouldBe Right(validTokens)
  }

}
