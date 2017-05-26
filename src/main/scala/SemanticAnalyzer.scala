package imperative

import scala.annotation.tailrec

import compiler._
import collection.immutable.ListMap
import collection.immutable.Set

class NonConstantUnrollParametersException extends Exception
class InnerResetWhileTrueWaitException extends Exception

// Return communication ports that are unguarded
class GuardStack( val portsWithErrors : Set[Port] = Set[Port](),
                  val activeGuards : List[Set[Port]] = List[Set[Port]]()) {
  def push = new GuardStack( portsWithErrors, Set[Port]() :: activeGuards)
  def pop = new GuardStack( portsWithErrors, activeGuards.tail)
  def addActiveGuard( p : Port) = new GuardStack( portsWithErrors, (activeGuards.head + p) :: activeGuards.tail)
  def addPortWithError( p : Port) = new GuardStack( portsWithErrors + p, activeGuards)
  def isActiveGuard( p : Port) : Boolean = activeGuards.foldLeft(false){ case (b,s) => b || s.contains( p)}
  def mergePortsWithErrors( that : GuardStack) = new GuardStack( portsWithErrors ++ that.portsWithErrors, activeGuards)

}

class PortHisto( private val q : ListMap[Port,Int] = ListMap[Port,Int]()) {

  def max( that : PortHisto) : PortHisto = {
    new PortHisto( q.toList.foldLeft( that.q) { 
      case ( m, (k,v)) =>
        m.updated( k, scala.math.max( m.getOrElse( k, 0), v))
    })
  }
  def min( that : PortHisto) : PortHisto = {
    new PortHisto( q.toList.foldLeft( that.q) { 
      case ( m, (k,v)) =>
        m.updated( k, scala.math.min( m.getOrElse( k, 0), v))
    })
  }
  def plus( that : PortHisto) : PortHisto = {
    new PortHisto( q.toList.foldLeft( that.q) { 
      case ( m, (k,v)) =>
        m.updated( k, m.getOrElse( k, 0) + v)
    })
  }
  def scalarMult( r : Int) : PortHisto = {
    new PortHisto( ListMap[Port,Int]( q.toList.map{ case (k,v) => (k,v*r)}: _*))
  }

  def updated( p : Port, v : Int) = new PortHisto( q.updated( p, v))
 
  def getOrElse( p : Port, v : Int) = q.getOrElse( p, v)

  def filter( f : ((Port,Int)) => Boolean ) = q.filter{ f}

}

object SemanticAnalyzer {

  def apply( ast : Process) : Either[CompilationError, Process] = {
    for {
      ast1 <- pass1( ast).right
      ast2 <- loweredCheck( ast1).right
    } yield ast2
  }

  def getGuards( m : PortHisto, ast : BExpression) : PortHisto = ast match {
    case NBCanGet( p) => m.updated( p, m.getOrElse( p, 0) + 1)
    case NBCanPut( p) => m.updated( p, m.getOrElse( p, 0) + 1)
    case AndBExpression( l, r) => getGuards( getGuards( m, l), r)
    case NotBExpression( b) => getGuards( m, b)
    case EqBExpression( l, r) => m
    case ConstantTrue => m
  }

  def getGuards( m : PortHisto, ast : Command) : PortHisto = ast match {
    case Blk( _, seq) => (m /: seq){ getGuards}
    case NBGet( p, _) => m
    case NBPut( p, _) => m
    case IfThenElse( b, t, e) => {
      val mb = getGuards( m, b)
      val mt = getGuards( m, t)
      val me = getGuards( m, e)
      mb.max( mt).max( me)
    }
    case While( b, t) => getGuards( getGuards( m, b), t)
    case Assignment( _, _) => m
    case Wait => m
    case Unroll( Variable( v), ConstantInteger( lb), ConstantInteger( ub), cmd) => {
      val mcmd = getGuards( new PortHisto, cmd)
      m.plus( mcmd.scalarMult( ub-lb))
    }
    case Unroll( _, _, _, _) => throw new NonConstantUnrollParametersException
    case ResetWhileTrueWait( _, _, _) => throw new InnerResetWhileTrueWaitException
  }

  def getCommunications( m : PortHisto, ast : Command) : PortHisto = ast match {
    case Blk( _, seq) => (m /: seq){ getCommunications}
    case NBGet( p, _) => m.updated( p, m.getOrElse( p, 0) + 1)
    case NBPut( p, _) => m.updated( p, m.getOrElse( p, 0) + 1)
    case IfThenElse( b, t, e) => {
      val mt = getCommunications( m, t)
      val me = getCommunications( m, e)
      mt.max( me)
    }
    case While( b, t) => getCommunications( m, t)
    case Assignment( _, _) => m
    case Wait => m
    case Unroll( Variable( v), ConstantInteger( lb), ConstantInteger( ub), cmd) => {
      val mcmd = getCommunications( new PortHisto, cmd)
      m.plus( mcmd.scalarMult( ub-lb))
    }
    case Unroll( _, _, _, _) => throw new NonConstantUnrollParametersException
    case ResetWhileTrueWait( _, _, _) => throw new InnerResetWhileTrueWaitException
  }

  def unguardedComms( m : GuardStack, ast : BExpression) : GuardStack = ast match {
    case NBCanGet( p) => m.addActiveGuard( p)
    case NBCanPut( p) => m.addActiveGuard( p)
    case AndBExpression( l, r) => unguardedComms( unguardedComms( m, l), r)
    case NotBExpression( b) => m // Ignore sub expression (for now)
    case EqBExpression( l, r) => m
    case ConstantTrue => m
  }

  def unguardedComms( m : GuardStack, ast : Command) : GuardStack = ast match {
    case Blk( _, seq) => (m /: seq){ unguardedComms}
    case NBGet( p, _) => if ( m.isActiveGuard( p)) m else m.addPortWithError( p)
    case NBPut( p, _) => if ( m.isActiveGuard( p)) m else m.addPortWithError( p)
    case IfThenElse( b, t, e) => {
      val mb = unguardedComms( m.push, b)
      val mt = unguardedComms( mb, t).pop
      val me = unguardedComms( m, e)
      mt.mergePortsWithErrors( me)
    }
    case While( b, t) => unguardedComms( m, t)
    case Assignment( _, _) => m
    case Wait => m
    case Unroll( Variable( v), ConstantInteger( lb), ConstantInteger( ub), cmd) => unguardedComms( m, cmd)
    case Unroll( _, _, _, _) => throw new NonConstantUnrollParametersException
    case ResetWhileTrueWait( _, _, _) => throw new InnerResetWhileTrueWaitException
  }

  def findUnguardedComms( mainSeg : Command) : Option[String] = {
    val p = unguardedComms( new GuardStack(), mainSeg).portsWithErrors
    if ( p.isEmpty) None else Some( s"Unguarded communications on: ${p}")
  }

  def findWhile( ast : Command, tag : String) : Option[String] = {
    def findWhile( m : Boolean, ast : Command) : Boolean = ast match {
      case Blk( _, seq) => (m /: seq){ findWhile}
      case IfThenElse( b, t, e) => findWhile( findWhile( m, t), e)
      case While( b, t) => true
      case Unroll( _, _, _, cmd) => findWhile( m, cmd)
      case _ => m
    }
    if ( findWhile( false, ast)) Some( s"While loop in ${tag}") else None
  }

  def initSegComms( ast : Command) : Option[String] = {
    val badPorts = getCommunications( new PortHisto, ast).filter{ case (k,v) => v > 0}
    if ( badPorts.isEmpty) None else Some( s"Communications in initial segment ${badPorts.toList.mkString}")
  }

  def initSegGuards( ast : Command) : Option[String] = {
    val badPorts = getGuards( new PortHisto, ast).filter{ case (k,v) => v > 0}
    if ( badPorts.isEmpty) None else Some( s"Guards in initial segment ${badPorts.toList.mkString}")
  }

  def mainSegMultiComms( ast : Command) : Option[String] = {
    val badPorts = getCommunications( new PortHisto, ast).filter{ case (k,v) => v > 1}
    if ( badPorts.isEmpty) None else Some( s"Multiple communications (possibly) in the same cycle: ${badPorts.toList.mkString}")
  }

  def loweredCheck( ast : Process) : Either[CompilationError, Process] = {
    ast match {
      case Process( portDeclList, ResetWhileTrueWait( localVars, initSeg, mainSeg)) => {
// Warning
        mainSegMultiComms( mainSeg)
// Errors
        val e = findWhile( Blk( localVars, initSeg), "initial segment") ::
                findWhile( mainSeg, "main segment") ::
                initSegComms( Blk( localVars, initSeg)) :: 
                initSegGuards( Blk( localVars, initSeg)) ::
                findUnguardedComms( mainSeg) ::
                List[Option[String]]()
        val errors = e.flatMap{ identity[Option[String]]}
        if ( errors.isEmpty) {
          Right( ast)
        } else {
          Left(SemanticAnalyzerError( errors.mkString("; ")))
        }
      }
      case _ => {
        Left(SemanticAnalyzerError("Can't have while at top level"))
      }
    }
  }

  def pass1( ast : Process) : Either[CompilationError, Process] = {
    ast match {
      case Process( portDecls, Blk( decls, seq)) if !seq.isEmpty =>
        seq.last match {
          case While( ConstantTrue, Blk( innerDecls, innerSeq)) =>
            if ( Wait != innerSeq.last) {
              Left(SemanticAnalyzerError("Don't have wait at end of segment"))
            } else {
              Right(Process( portDecls, ResetWhileTrueWait( decls, seq.init, Blk( innerDecls, innerSeq.init))))
            }
          case _ => Left(SemanticAnalyzerError("Don't have While( ConstantTrue, Blk(...)) combination"))
        }
      case Process( portDecls, Blk( decls, Nil)) => Left(SemanticAnalyzerError("Empty Sequence"))
      case _ => Left(SemanticAnalyzerError("Don't have top level process"))
    }
  }

}
