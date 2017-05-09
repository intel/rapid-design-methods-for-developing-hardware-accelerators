package test

import lexer._

import org.scalatest.{FlatSpec, Matchers}

class LexerSpec extends FlatSpec with Matchers {

  val validCode =
    """
      |while ( true) {
      |  if ( P? && Q!) {
      |     P?v
      |     Q!v
      |  }
      |  wait
      |}
    """.stripMargin.trim

  val validTokens = List(WHILE(), LPAREN(), TRUE(), RPAREN(), LBRACE(), IF(), LPAREN(), IDENTIFIER("P"), QUERY(), AND(), IDENTIFIER("Q"), BANG(), RPAREN(), LBRACE(), IDENTIFIER("P"), QUERY(), IDENTIFIER("v"), IDENTIFIER("Q"), BANG(), IDENTIFIER("v"), RBRACE(), WAIT(), RBRACE())

  "Lexer" should "successfully lex a valid code" in {
    Lexer(validCode) shouldBe Right(validTokens)
  }

}
