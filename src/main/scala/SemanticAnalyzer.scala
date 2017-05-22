package imperative

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

abstract trait LabeledGraph
case class LGWait() extends LabeledGraph
case class LGWhileTrue( lg : LabeledGraph) extends LabeledGraph
case class LGWhileNotProbeWait( g : BExpression, do_after : LabeledGraph) extends LabeledGraph
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
// Warnings
        mainSegMultiComms( mainSeg)
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
    case While( NotBExpression( g), Wait) => LGWhileNotProbeWait( g, LGSeq( List()))
    case While( ConstantTrue, e) => LGWhileTrue( genLG( e))
    case Blk( decls, seq) => seq.foldLeft( LGSeq(List())) { case (x,y) =>
      val lgY = List( genLG( y))
      x match {
        case LGSeq( hd :: tl) => {
          (hd::tl).last match {
            case LGWhileNotProbeWait( g, LGSeq( lst)) if isPrimOrWait( lgY.head) => {
//              println( s"Matching and extending ProbeWait: ${lgY.head}")
              LGSeq( (hd::tl).init ++ List( LGWhileNotProbeWait( g, LGSeq( lst ++ lgY))))
            }
            case _ => {
//              println( s"Not Matching and extending ProbeWait: ${hd::tl}")
              LGSeq( (hd::tl) ++ lgY)
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

  def allLGWhileNotProbeWait( b : Boolean, lg : LabeledGraph) : Boolean = lg match {
    case LGWhileNotProbeWait( g, LGSeq( lst)) => lst.foldLeft(b){ noLGWhileNotProbeWait}
    case _ => false
  }

  def noLGWhileNotProbeWait( b : Boolean, lg : LabeledGraph) : Boolean = lg match {
    case LGWhileNotProbeWait( g, LGSeq( lst)) => false
    case _ => b
  }

  def testTL( lg : LabeledGraph) : Boolean = lg match {
    case LGSeq( Nil) => true
    case LGSeq( lst) => lst.last match {
      case LGWhileTrue( LGSeq( lst2)) =>
        lst2.foldLeft( lst.init.foldLeft( true){ allLGWhileNotProbeWait}){ allLGWhileNotProbeWait}
      case _ => false
    }
    case _ => false
  }


  def transLG( s : Command, lg : LabeledGraph) : Command = lg match {
    case LGSeq( Nil) => s
    case LGSeq( hd :: tl) => s match {
      case Blk( decls, seq) => 
        transLG( Blk( decls, seq ++ List(transLG( s, hd))), LGSeq( tl))
      case _ => throw new Exception("Wrong LGSeq form")
    }
    case LGWhileNotProbeWait( g, LGSeq( lst)) => {
      val wAssign = List(LGPrim(Assignment(Variable("w"),ConstantInteger(1))))
      val t = if ( !lst.isEmpty && lst.last == LGWait()) {
        transLG( Blk( List(), List()), LGSeq( lst.init ++ wAssign))
      } else {
        transLG( Blk( List(), List()), LGSeq( lst))
      }
      IfThenElse( g, t, transLG( Blk( List(), List()), LGSeq( wAssign)))
    }
    case LGWhileTrue( LGSeq( lst)) => transLG( s, LGSeq( lst))
    case LGPrim( cmd) => cmd
    case LGWait() => Wait
  }

  def TransformWaits( ast : Process) : Either[CompilationError, Process] = {
    println(ast)

    val ast0 = ast match {
      case Process( portDecls, Blk( decls, seq)) => {
        val lg = genLG( Blk( decls, seq))
//        println( s"Labeled graph: ${lg} testTL: ${testTL( lg)}")
        val Blk( newDecls, newSeq) = transLG( Blk( List(), List()), lg)
        val decls0 = decls ++ List( Decl(Variable("s"),UIntType(4)), Decl(Variable("w"),UIntType(1)))
        val p = Process( portDecls, Blk( decls0, List(While( ConstantTrue, Blk( newDecls, newSeq ++ List(Wait))))))
        PrintAST.p( 0, p)
        p
      }
      case _ => throw new Exception("Wrong AST form")
    }

    pass1( ast0)
  }

}

object PrintAST {
  def i( n : Int) : String = s"""${(0 until n).map{ x => " "}.mkString("")}"""

  def p( indent : Int, ast : Process) : Unit = ast match {
    case Process( _, cmd) => {
      println( s"${i(indent)}Process")
      p( indent + 2, cmd)
    }
    case _ => throw new Exception("Wrong AST form")
  }
  def p( indent : Int, ast : Command) : Unit = ast match {
    case While( b, e) => {
      println( s"${i(indent)}While")
      p( indent + 2, b)
      p( indent + 2, e)
    }
    case Blk( decls, seq) => {
      println( s"${i(indent)}Blk")
      decls.foreach{ x => println( s"${i(indent+2)}${x}")}
      seq.foreach{ x => p( indent + 2, x)}
    }
    case NBGet( _, _) => println( s"${i(indent)}${ast}")
    case NBPut( _, _) => println( s"${i(indent)}${ast}")
    case Assignment( _, _) => println( s"${i(indent)}${ast}")
    case Wait => println( s"${i(indent)}${ast}")
    case IfThenElse( b, t, e) => {
      println( s"${i(indent)}If")
      p( indent + 2 , b)
      println( s"${i(indent)}Then")
      p( indent + 2, t)
      println( s"${i(indent)}Else")
      p( indent + 2, e)
    }
    case _ => throw new Exception(s"Wrong AST form: ${ast}")
  }
  def p( indent : Int, ast : BExpression) : Unit = ast match {
    case ConstantTrue => println( s"${i(indent)}${ast}")
    case AndBExpression( l, r) => {
      println( s"${i(indent)}And")
      p( indent+2, l)
      p( indent+2, r)
    }
    case EqBExpression( l, r) => {
      println( s"${i(indent)}Eq")
      p( indent+2, l)
      p( indent+2, r)
    }
    case NBCanGet( _) => println( s"${i(indent)}${ast}")
    case NBCanPut( _) => println( s"${i(indent)}${ast}")
    case _ => throw new Exception(s"Wrong AST form: ${ast}")
  }
  def p( indent : Int, ast : Expression) : Unit = ast match {
    case ConstantInteger( _) => println( s"${i(indent)}${ast}")
    case Variable( _) => println( s"${i(indent)}${ast}")
    case _ => throw new Exception(s"Wrong AST form: ${ast}")
  }

}
