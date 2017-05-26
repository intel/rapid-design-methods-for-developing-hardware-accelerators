package imperative

import org.scalatest.{ Matchers, FlatSpec, FreeSpec}

import org.scalameter.api._

class HLSTest extends FreeSpec with Matchers {
  "genLG" - {
    "empty" in {
      HLS.genLG0( Blk( List(), List())) should be ( LGSeq( List()))
    }
    "while true" in {
      HLS.genLG0( Blk( List(), List( While( ConstantTrue, Blk( List(), List()))))) should be ( LGSeq( List(LGWhileTrue(LGSeq(List())))))
    }
    "while not comm" in {
      HLS.genLG0( Blk( List(), List( While( NotBExpression( NBCanGet( Port( "P"))), Wait)))) should be ( LGSeq( List(LGWhileNotProbeWait(NBCanGet(Port("P")),LGSeq(List()),(0,0)))))
    }
    "while not comm, then comm" in {
      HLS.genLG0( Blk( List(), List( While( NotBExpression( NBCanGet( Port( "P"))), Wait), NBGet( Port( "P"), Variable( "x")), Wait))) should be ( LGSeq( List(LGWhileNotProbeWait(NBCanGet(Port("P")),LGSeq(List(LGPrim(NBGet(Port("P"),Variable("x"))),LGWait())),(0,0)))))
    }
    "assign and comm" in {
      val blob = Blk(List(Decl(Variable("x"),UIntType(80))),List(Assignment(Variable("x"),ConstantInteger(0)), While(ConstantTrue,Blk(List(),List(Blk(List(),List(While(NotBExpression(NBCanPut(Port("Q"))),Wait), NBPut(Port("Q"),Variable("x")))), Assignment(Variable("x"),AddExpression(Variable("x"),ConstantInteger(1))), Wait)))))
      val i = LGPrim(Assignment(Variable("x"),ConstantInteger(0)))
      val g = NBCanPut(Port("Q"))
      val h1 = LGPrim(NBPut(Port("Q"),Variable("x")))
      val h2 = LGPrim(Assignment(Variable("x"),AddExpression(Variable("x"),ConstantInteger(1))))
      val h = LGSeq(List(h1, h2, LGWait()))
      val j = LGSeq(List(LGWhileNotProbeWait(g,h,(0,0))))
      val blob2 = LGSeq(List(i, LGWhileTrue(j)))
      HLS.genLG0( blob) should be ( blob2)
    }
  }
  "assignLabels" - {
    "empty" in {
      HLS.assignLabels( (0, LGSeq( List()))) should be ( 0, LGSeq( List()))
    }
    "while not comm, then comm" in {
      val g = NBCanGet(Port("P"))
      val f = LGSeq(List(LGPrim(NBGet(Port("P"),Variable("x"))),LGWait()))
      val blob = LGWhileNotProbeWait(g,f,(0,0))
      val blob2 = LGWhileNotProbeWait(g,f,(0,1))
      HLS.assignLabels( (0, LGSeq( List(blob)))) should be (1, LGSeq( List(blob2)))
    }
    "double" in {
      val g = NBCanGet(Port("P"))
      val f = LGSeq(List(LGPrim(NBGet(Port("P"),Variable("x"))),LGWait()))
      val blob = LGWhileNotProbeWait(g,f,(0,0))
      val blob2 = LGWhileNotProbeWait(g,f,(0,1))
      val blob3 = LGWhileNotProbeWait(g,f,(1,2))
      HLS.assignLabels( (0, LGSeq( List(blob,blob)))) should be (2, LGSeq( List(blob2,blob3)))
    }
    "assign and comm" in {
      val i = LGPrim(Assignment(Variable("x"),ConstantInteger(0)))
      val g = NBCanPut(Port("Q"))
      val h = LGSeq(List( LGWait()))
      val j = LGSeq(List(LGWhileNotProbeWait(g,h,(0,0))))
      val blob2 = LGSeq(List(i, LGWhileTrue(j)))
      val blob3 = LGSeq(List(LGWhileNotProbeWait(ConstantTrue,LGSeq(List(i)),(0,1)), LGWhileNotProbeWait(g,h,(1,1))))
      HLS.assignLabels( ( 0, blob2)) should be ( ( 2, blob3))
    }
  }
  "testTL" - {
    "empty" in {
      val g = LGWhileNotProbeWait(ConstantTrue,LGSeq(List(LGPrim(Assignment(Variable("x"),ConstantInteger(0))))),(0,1))
      val h = LGWhileNotProbeWait(NBCanPut(Port("Q")),LGSeq(List(LGPrim(NBPut(Port("Q"),Variable("x"))), LGPrim(Assignment(Variable("x"),AddExpression(Variable("x"),ConstantInteger(1)))), LGWait())),(1,1))
      HLS.testTL( LGSeq(List( g, h))) should be ( true)
    }
  }
}
