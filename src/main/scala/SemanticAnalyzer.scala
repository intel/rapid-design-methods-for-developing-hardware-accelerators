package imperative

import compiler._
import collection.immutable.ListMap
import collection.immutable.Set

class NonConstantUnrollParametersException extends Exception

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

class BadPortMap( val q : ListMap[Port,Int] = ListMap[Port,Int]()) {

  def max( that : BadPortMap) : BadPortMap = {
    new BadPortMap( q.toList.foldLeft( that.q) { 
      case ( m, (k,v)) =>
        m.updated( k, scala.math.max( m.getOrElse( k, 0), v))
    })
  }
  def plus( that : BadPortMap) : BadPortMap = {
    new BadPortMap( q.toList.foldLeft( that.q) { 
      case ( m, (k,v)) =>
        m.updated( k, m.getOrElse( k, 0) + v)
    })
  }
  def scalarMult( r : Int) : BadPortMap = {
    new BadPortMap( ListMap[Port,Int]( q.toList.map{ case (k,v) => (k,v*r)}: _*))
  }

  def updated( p : Port, v : Int) = new BadPortMap( q.updated( p, v))
 
  def getOrElse( p : Port, v : Int) = q.getOrElse( p, v)


}

object SemanticAnalyzer {

  def apply( ast : Process) : Either[CompilationError, Process] = {
    for {
      ast1 <- pass1( ast).right
      ast2 <- pass2( ast1).right
    } yield ast2
  }

  def getCommunications( m : BadPortMap, ast : Command) : BadPortMap = ast match {
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
      val mcmd = getCommunications( new BadPortMap, cmd)
      m.plus( mcmd.scalarMult( ub-lb))
    }
    case Unroll( _, _, _, _) => throw new NonConstantUnrollParametersException
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
  }

  def findWhile( m : Boolean, ast : Command) : Boolean = ast match {
    case Blk( _, seq) => (m /: seq){ findWhile}
    case IfThenElse( b, t, e) => findWhile( findWhile( m, t), e)
    case While( b, t) => true
    case Unroll( _, _, _, cmd) => findWhile( m, cmd)
    case _ => m
  }

  def lastWhileWithFinalWait( ast : Command) : Boolean = ast match {
    case While( ConstantTrue, Blk( declLst, lst)) => lst.last == Wait
    case _ => false
  }

  def initSegComms( ast : Command) : Boolean = {
    val badPorts = getCommunications( new BadPortMap, ast).q.filter{ case (k,v) => v > 0}
    if ( badPorts.size > 0) {
      println( s"Communications in initial segment ${badPorts.toList.mkString}")
    }
    return badPorts.size > 0
  }

  def mainSegMultiComms( ast : Command) : Boolean = {
    val badPorts = getCommunications( new BadPortMap, ast).q.filter{ case (k,v) => v > 1}
    if ( badPorts.size > 0) {
      println( s"Multiple communications (possibly) in the same cycle: ${badPorts.toList.mkString}")
    }
    return badPorts.size > 0
  }

  def pass1( ast : Process) : Either[CompilationError, Process] = {
    ast match {
      case Process( portDeclList, Blk( localVars, seq)) => {
        val initSeg = seq.init
        val mainSeg = seq.last
        if ( findWhile( false, Blk( localVars, initSeg))) {
          Left(SemanticAnalyzerError( s"While in initial segment"))
        } else if( !lastWhileWithFinalWait( mainSeg)) {
          Left(SemanticAnalyzerError( s"Last segment not a while true with final wait"))
        } else if ( initSegComms( Blk( localVars, initSeg))) {
          Left(SemanticAnalyzerError( s"Communications in initial segment"))
        } else if ( mainSegMultiComms( mainSeg)) {
          Left(SemanticAnalyzerError( s"Multiple communications (possibly) in the same cycle"))
        } else if ( unguardedComms( new GuardStack(), mainSeg).portsWithErrors.size > 0) {
          Left(SemanticAnalyzerError( s"Unguarded communications"))
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
      case Process( _, Blk( _, seq)) => {
//        println( s"Performing semantic analysis ${ast}")
        Right( ast)
      }
      case _ => {
        Left(SemanticAnalyzerError("Can't have while at top level"))
      }
    }
  }

}
