package imperative

import org.scalatest.{ Matchers, FlatSpec, FreeSpec}

class HLS3Test extends FreeSpec with Matchers {
  "split" - {
    "empty" in {
      HLS3.split( List()) should be ( List(), List())
    }
    "complex" in {
      val lst = List(While(NotBExpression(NBCanPut(Port("Q"))),Wait), NBPut(Port("Q"),Variable("x")), Wait)
      HLS3.split( lst) should be ( List(), lst)
    }
  }
  "expand" - {
    "complex" in {
      val lst = List(While(NotBExpression(NBCanPut(Port("Q"))),Wait), NBPut(Port("Q"),Variable("x")), Wait)
      val res = HLS3.State(Map(0 -> IfThenElse(AndBExpression(EqBExpression(Variable("s"),ConstantInteger(0)),EqBExpression(Variable("w"),ConstantInteger(0))),IfThenElse(NBCanPut(Port("Q")),Blk(List(),List(NBPut(Port("Q"),Variable("x")), Assignment(Variable("s"),ConstantInteger(1)), Assignment(Variable("w"),ConstantInteger(1)))),Blk(List(),List(Assignment(Variable("w"),ConstantInteger(1))))),Blk(List(),List()))),2,List())

      val st = HLS3.expand( HLS3.State( Map(), 2, List()), 0, 1, Blk( List(), lst))
      st.m.keys.foreach{ i => PrintAST.p( 0, st.m(i))}
      st should be ( res)
    }
    "complex2" in {
      val lst = List(Blk( List(), List( While(NotBExpression(NBCanPut(Port("Q"))),Wait), NBPut(Port("Q"),Variable("x")))), Wait)
      val res = HLS3.State(Map(0 -> IfThenElse(AndBExpression(EqBExpression(Variable("s"),ConstantInteger(0)),EqBExpression(Variable("w"),ConstantInteger(0))),IfThenElse(NBCanPut(Port("Q")),Blk(List(),List(NBPut(Port("Q"),Variable("x")), Assignment(Variable("s"),ConstantInteger(1)), Assignment(Variable("w"),ConstantInteger(1)))),Blk(List(),List(Assignment(Variable("w"),ConstantInteger(1))))),Blk(List(),List()))),2,List())

      val st = HLS3.expand( HLS3.State( Map(), 2, List()), 0, 1, Blk( List(), lst))
      st.m.keys.foreach{ i => PrintAST.p( 0, st.m(i))}
      st should be ( res)
    }
  }
}


