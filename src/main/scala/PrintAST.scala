package imperative

class WrongASTFormException( tag : String = "") extends Exception( tag)

object PrintAST {
  def i( n : Int) : String = s"""${(0 until n).map{ x => " "}.mkString("")}"""

  def p( indent : Int, ast : Process) : Unit = ast match {
    case Process( _, cmd) => {
      println( s"${i(indent)}Process")
      p( indent + 2, cmd)
    }
    case _ => throw new WrongASTFormException( s"${ast}")
  }
  def p( indent : Int, ast : Command) : Unit = ast match {
    case While( b, e) => {
      print( s"${i(indent)}while (")
      p( 0, b)
      println( ")")
      p( indent + 2, e)
    }
    case Blk( decls, seq) => {
      println( s"${i(indent)}{")
      decls.foreach{ x => println( s"${i(indent+2)}${x}")}
      seq.foreach{ x => p( indent + 2, x)}
      println( s"${i(indent)}}")
    }
    case NBGet( Port( p), Variable( v)) => println( s"${i(indent)}${p}?${v}")
    case NBPut( Port( p), Variable( v)) => println( s"${i(indent)}${p}!${v}")
    case Assignment( Variable( v), e) => {
      print( s"${i(indent)}${v} = ")
      p( 0, e)
      println( "")
    }
    case Wait => println( s"${i(indent)}wait")
    case IfThenElse( b, t, e) => {
      print( s"${i(indent)}if (")
      p( indent + 2 , b)
      println( s")")
      p( indent + 2, t)
      e match {
        case Blk( _, Nil) => ()
        case _ => {
          println( s"${i(indent)}else")
          p( indent + 2, e)
        }
      }
    }
    case _ => throw new WrongASTFormException( s"${ast}")
  }
  def p( indent : Int, ast : BExpression) : Unit = ast match {
    case ConstantTrue => print( s"true")
    case AndBExpression( l, r) => {
      p( 0, l)
      print( s" && ")
      p( 0, r)
    }
    case EqBExpression( l, r) => {
      p( 0, l)
      print( s"==")
      p( 0, r)
    }
    case NBCanGet( Port( p)) => print( s"${p}?")
    case NBCanPut( Port( p)) => print( s"${p}!")
    case _ => throw new WrongASTFormException( s"${ast}")
  }
  def p( indent : Int, ast : Expression) : Unit = ast match {
    case ConstantInteger( i) => print( s"${i}")
    case Variable( v) => print( s"${v}")
    case AddExpression( l, r) => {
      p( 0, l)
      print( s"+")
      p( 0, r)
    }
    case _ => throw new WrongASTFormException( s"${ast}")
  }

}
