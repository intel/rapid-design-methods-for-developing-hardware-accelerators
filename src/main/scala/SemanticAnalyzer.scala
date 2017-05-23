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

class LabeledGraph
case class LGWait() extends LabeledGraph
case class LGWhileTrue( lg : LabeledGraph) extends LabeledGraph
case class LGWhileNotProbeWait( g : BExpression, do_after : LabeledGraph, lbl : (Int,Int)) extends LabeledGraph
case class LGSeq( lgs : Seq[LabeledGraph])  extends LabeledGraph
case class LGPrim( cmd : Command)  extends LabeledGraph

object SemanticAnalyzer {

  def apply( ast : Process) : Either[CompilationError, Process] = {
    for {
      ast1 <- pass1( ast).right
//      ast1 <- TransformWaits( ast).right
      ast2 <- loweredCheck( ast1).right
    } yield ast2
  }

  def hls( ast : Process) : Either[CompilationError, Process] = {
    for {
      ast1 <- TransformWaits( ast).right
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

// C0; C1; wait; C2; C3; wait => { C0; C1}; wait; { C2; C3}; wait
// C0; C1; wait; while( B) { C2; C3; wait; C4; C5; wait} => { C0; C1}; wait; while( B) { { C2; C3}; wait; { C4; C5}; wait}

  def isPrimOrWait( lg : LabeledGraph) : Boolean = lg match {
    case LGPrim( _) => true
    case LGWait() => true
    case _ => false
  }

  def genLG( cmd : Command) : LabeledGraph = cmd match {
    case While( NotBExpression( g), Wait) => LGWhileNotProbeWait( g, LGSeq( List()), (0,0))
    case While( ConstantTrue, e) => LGWhileTrue( genLG( e))
    case Blk( decls, seq) => seq.foldLeft( LGSeq(List())) { case (x,y) =>
      val lgY = List( genLG( y))
      x match {
        case LGSeq( lst0@(hd::tl)) => {
          lst0.last match {
            case LGWhileNotProbeWait( g, LGSeq( lst), _) if isPrimOrWait( lgY.head) => {
//              println( s"Matching and extending ProbeWait: ${lgY.head}")
              LGSeq( lst0.init ++ List( LGWhileNotProbeWait( g, LGSeq( lst ++ lgY), (0,0))))
            }
            case _ => {
//              println( s"Not Matching and extending ProbeWait: ${hd::tl}")
              LGSeq( lst0 ++ lgY)
            }
          }
        }
        case LGSeq( lst) => {
//          println( s"Not Matching and extending ProbeWait: ${lst}")
          LGSeq( lst ++ lgY)
        }
      }
    }
    case NBGet( p, v) => LGPrim( cmd)
    case NBPut( p, v) => LGPrim( cmd)
    case Wait => LGWait()
    case _ => { println( s"\tgenLG: Unimplemented command ${cmd}"); LGPrim( cmd)}
  }

  def assignLabels( tup : (Int,LabeledGraph)) : (Int,LabeledGraph) = {
    val ( count, lg) = tup
    lg match {
      case LGSeq( lst) =>
        lst.foldLeft( ( count, LGSeq(List()))){ case ( ( count, LGSeq( lst0)), x) =>
          x match {
            case LGWhileTrue( lg0@LGSeq( seq)) => {
              val (c, LGSeq( lst)) = assignLabels( (count, lg0))
              val ( count0, x0) = lst.last match {
                case LGWhileNotProbeWait( g, seq, lbl) => {
                  val newl = LGSeq( lst.init ++ List( LGWhileNotProbeWait( g, seq, (lbl._1,count))))
                  ( c, newl)
                }
              }
              ( count0, LGSeq( lst0 ++ List(x0)))
            }
            case _ => {
              val ( count0, x0) = assignLabels( (count, x))
              ( count0, LGSeq( lst0 ++ List(x0)))
            }
          }
        }
      case LGWhileNotProbeWait( g, seq@LGSeq( lst), _) =>
        (count+1, LGWhileNotProbeWait( g, seq, (count,count+1)))
      case _ => { println( s"assignLabels: Not expecting ${count} ${lg}"); (count,lg)}
    }
  }

  def transLG( s : Command, lg : LabeledGraph) : Command = lg match {
    case LGSeq( Nil) => s
    case LGSeq( hd :: tl) => s match {
      case Blk( decls, seq) => 
        transLG( Blk( decls, seq ++ List(transLG( s, hd))), LGSeq( tl))
      case _ => throw new Exception("Wrong LGSeq form")
    }
    case LGWhileNotProbeWait( g, LGSeq( lst), lbl) => {
      val wAssignElse = List(LGPrim(Assignment(Variable("w"),ConstantInteger(1))))
      val sAssignThen = List(LGPrim(Assignment(Variable("s"),ConstantInteger(lbl._2))))
//      val sAssignElse = List(LGPrim(Assignment(Variable("s"),ConstantInteger(lbl._1))))
      val sAssignElse = List()
      val t = if ( !lst.isEmpty && lst.last == LGWait()) {
        val wAssignThen = List(LGPrim(Assignment(Variable("w"),ConstantInteger(1))))
        transLG( Blk( List(), List()), LGSeq( lst.init ++ sAssignThen ++ wAssignThen))
      } else {
        transLG( Blk( List(), List()), LGSeq( lst ++ sAssignThen))
      }
      IfThenElse( 
        AndBExpression( 
          EqBExpression( Variable("s"), ConstantInteger(lbl._1)),
          EqBExpression( Variable("w"), ConstantInteger(0))
        ),
        IfThenElse( g, t, transLG( Blk( List(), List()), LGSeq( sAssignElse ++ wAssignElse))),
        Blk( List(), List())
      )
    }
    case LGPrim( cmd) => cmd
    case LGWhileTrue( lg0) => throw new Exception("Wrong LG form")
    case LGWait() => throw new Exception("Wrong LG form")
  }

  def allLGWhileNotProbeWait( b : Boolean, lg : LabeledGraph) : Boolean = lg match {
    case LGWhileNotProbeWait( g, LGSeq( lst), _) => lst.foldLeft(b){ noLGWhileNotProbeWait}
    case _ => false
  }

  def noLGWhileNotProbeWait( b : Boolean, lg : LabeledGraph) : Boolean = lg match {
    case LGWhileNotProbeWait( g, LGSeq( lst), _) => false
    case _ => b
  }

  def testTL( lg : LabeledGraph) : Boolean = lg match {
    case LGSeq( Nil) => true
    case LGSeq( lst) => lst.last match {
      case LGSeq( lst2) =>
        lst2.foldLeft( lst.init.foldLeft( true){ allLGWhileNotProbeWait}){ allLGWhileNotProbeWait}
      case _ => false
    }
    case _ => false
  }

  def TransformWaits( ast : Process) : Either[CompilationError, Process] = {
    println(ast)

    def log2( v : Int) = {
      @tailrec
      def log2( k : Int, n : Int, v : Int) : Int = if ( n>=v) k else log2( k+1, 2*n, v)
      log2( 0, 1, v)
    }

    val ast0 = ast match {
      case Process( portDecls, Blk( decls, seq)) => {
        val (count0, lg0) = assignLabels( (0, genLG( Blk( decls, seq))))
        val sbits = log2( count0)
        println( s"Labeled graph: ${count0} ${sbits} ${lg0} testTL: ${testTL( lg0)}")

// Don't know why I need this, but I do.
        val lg1 = {
          val LGSeq( lst0) = lg0
          val LGSeq( y) = lst0.last
          LGSeq( lst0.init ++ y.toList)
        }

        val Blk( newDecls, newSeq) = transLG( Blk( List(), List()), lg1)
// Need to declare "s" (induction) and initialize to zero
        val decls0 = decls ++ List( Decl(Variable("s"),UIntType(sbits)))
// Need to declare "w" (combinational) and initialize to zero
        val newDecls0 = newDecls ++ List( Decl(Variable("w"),UIntType(1)))
        val seq0 = List( Assignment(Variable("w"),ConstantInteger(0))) ++ newSeq ++ List(Wait)
        val p =
          Process( portDecls,
            Blk( decls0,
              List(
                Assignment(Variable("s"),ConstantInteger(0)),
                While( ConstantTrue, Blk( newDecls0, seq0)))))
                    
        PrintAST.p( 0, p)
        p
      }
      case _ => throw new Exception("Wrong AST form")
    }
    pass1( ast0)
  }

}
