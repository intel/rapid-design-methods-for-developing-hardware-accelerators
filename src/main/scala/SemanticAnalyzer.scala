package imperative

import compiler._
import collection.immutable.ListMap

class NonConstantUnrollParametersException extends Exception

object SemanticAnalyzer {

  def apply( ast : Process) : Either[CompilationError, Process] = {
    for {
      ast1 <- pass1( ast).right
      ast2 <- pass2( ast1).right
    } yield ast2
  }

  type M = ListMap[Port,Int]

  def max( l : M, r : M) : M = {
    l.keys.foldLeft( r) { 
      case ( m, k) =>
        m.updated( k, scala.math.max( m.getOrElse( k, 0), l(k)))
    }
  }
  def plus( l : M, r : M) : M = {
    l.keys.foldLeft( r) { 
      case ( m, k) =>
        m.updated( k, m.getOrElse( k, 0) + l(k))
    }
  }
  def scalarMult( l : M, r : Int) : M = {
    ListMap( l.keys.map{ k => (k,l(k)*r)}.toSeq: _*)
  }

  def getCommunications( m : M, ast : Expression) : M = ListMap()

  def getCommunications( m : M, ast : BExpression) : M = ast match {
    case NBCanGet( Port( p)) => m.updated( Port( p), m.getOrElse( Port( p), 0) + 1)
    case NBCanPut( Port( p)) => m.updated( Port( p), m.getOrElse( Port( p), 0) + 1)
    case AndBExpression( l, r) => getCommunications( getCommunications( m, l), r)
    case NotBExpression( b) => getCommunications( m, b)
    case EqBExpression( l, r) => m
    case ConstantTrue => m
  }
  def getCommunications( m : M, ast : Command) : M = ast match {
    case Blk( _, seq) => (m /: seq){ getCommunications}
    case NBGet( Port( p), _) => m.updated( Port( p), m.getOrElse( Port( p), 0) + 1)
    case NBPut( Port( p), _) => m.updated( Port( p), m.getOrElse( Port( p), 0) + 1)
    case IfThenElse( b, t, e) => {
      val mb = getCommunications( m, b)
      val mt = getCommunications( m, t)
      val me = getCommunications( m, e)
      max( mb, max( mt, me))
    }
    case While( b, t) => {
      val mb = getCommunications( m, b)
      val mt = getCommunications( m, t)
      mt
    }
    case Assignment( _, _) => m
    case Wait => m
    case Unroll( Variable( v), ConstantInteger( lb), ConstantInteger( ub), cmd) => {
      val mcmd = getCommunications( ListMap[Port,Int](), cmd)
      plus( m, scalarMult( mcmd, ub-lb))
    }
    case Unroll( _, _, _, _) => throw new NonConstantUnrollParametersException
  }

  def pass1( ast : Process) : Either[CompilationError, Process] = {
    ast match {
      case Process( portDeclList, Blk( localVars, mainSeq)) => {
        val c = getCommunications( ListMap[Port,Int](), Blk( localVars, mainSeq))
        val badPorts = c.filter{ case (k,v) => v > 1}
        if ( badPorts.size > 0) {
          Left(SemanticAnalyzerError( s"Multiple communications (possibly) in the same cycle: ${badPorts.toList.mkString}"))
        } else {
          Right( ast)
        }
      }
      case _ => {
        Left(SemanticAnalyzerError("Can't have while at top level"))
      }
    }
  }

  def pass2( ast : Process) : Either[CompilationError, Process] = {
    ast match {
      case Process( _, Blk( _, _)) => {
//        println( s"Performing semantic analysis ${ast}")
        Right( ast)
      }
      case _ => {
        Left(SemanticAnalyzerError("Can't have while at top level"))
      }
    }
  }

}
