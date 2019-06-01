package imperative

import scala.annotation.tailrec

import compiler._
import collection.immutable.ListMap
import collection.immutable.Set

class NotYetImplementedException( tag : String) extends Exception(tag)

class LabeledGraph
case class LGWait() extends LabeledGraph
case class LGWhileTrue( lg : LabeledGraph) extends LabeledGraph
case class LGWhileNotProbeWait( g : BExpression, do_after : LabeledGraph, lbl : (Int,Int)) extends LabeledGraph
case class LGSeq( lgs : Seq[LabeledGraph])  extends LabeledGraph
case class LGPrim( cmd : Command)  extends LabeledGraph

object HLS {

  def apply( ast : Process) : Either[CompilationError, Process] = {
    for {
      ast1 <- TransformWaits( ast).right
      ast2 <- SemanticAnalyzer.pass1( ast1).right
      ast3 <- SemanticAnalyzer.loweredCheck( ast2).right
    } yield ast3
  }

  def isPrimOrWait( lg : LabeledGraph) : Boolean = lg match {
    case LGPrim( _) => true
    case LGWait() => true
    case _ => false
  }

  def processBlk( decls : Seq[Decl], seq : Seq[Command]) : LabeledGraph = {
    if ( !decls.isEmpty) throw new NotYetImplementedException( s"genLG: inner block declarations currently dropped ${decls}")
    processBlk0( decls, seq)
  }

  def processBlk0( decls : Seq[Decl], seq : Seq[Command]) : LabeledGraph = {
      seq.foldLeft( LGSeq(List())) { case (x,y) =>
// removing the hierarchical blocks
        val lgY = genLG( y) match {
          case LGSeq( seq) => seq
          case yy => List(yy)
        }
        x match {
          case LGSeq( lst0@(hd::tl)) => {
            lst0.last match {
              case LGWhileNotProbeWait( g, LGSeq( lst), _) if isPrimOrWait( lgY.head) =>
                LGSeq( lst0.init ++ List( LGWhileNotProbeWait( g, LGSeq( lst ++ lgY), (0,0))))
              case _ => LGSeq( lst0 ++ lgY)
            }
          }
          case LGSeq( Nil) => LGSeq( lgY)
        }
      }
  }

  def genLG( cmd : Command) : LabeledGraph = cmd match {
    case While( NotBExpression( g), Wait) => LGWhileNotProbeWait( g, LGSeq( List()), (0,0))
    case While( ConstantTrue, e) => LGWhileTrue( genLG( e))
    case Blk( decls, seq) => processBlk( decls, seq)
    case NBGet( p, v) => LGPrim( cmd)
    case NBPut( p, e) => LGPrim( cmd)
    case Assignment( v, e) => LGPrim( cmd)
    case Wait => LGWait()
    case _ => { println( s"genLG: Unimplemented command ${cmd}"); LGPrim( cmd)}
  }

  def genLG0( cmd : Command) : LabeledGraph = cmd match {
    case Blk( decls, seq) => {
      val lg = processBlk0( decls, seq)
//      println( s"genLG0: ${lg}")
      lg
    }
    case _ => throw new Exception("Wrong toplevel form")
  }

  def assignLabels( tup : (Int,LabeledGraph)) : (Int,LabeledGraph) = {
    val ( count, lg) = tup
    lg match {
      case LGSeq( lst) =>
        lst.foldLeft( ( count, LGSeq(List()))){ case ( ( count, LGSeq( lst0)), x) =>
          val ( count0, x0) = x match {
            case LGWhileTrue( lg0) => {
              val (c, LGSeq( lst)) = assignLabels( (count, lg0))
              assert( !lst.isEmpty)
              lst.last match {
                case LGWhileNotProbeWait( g, seq, lbl) => {
                  val newl = LGSeq( lst.init ++ List( LGWhileNotProbeWait( g, seq, (lbl._1,count))))
                  ( c, newl)
                }
              }
            }
            case _ => assignLabels( (count, x))
          }
          x0 match {
            case LGSeq( l0) => ( count0, LGSeq( lst0 ++ l0))
            case _ => ( count0, LGSeq( lst0 ++ List( x0)))
          }
        }
      case LGWhileNotProbeWait( g, seq@LGSeq( lst), _) =>
        (count+1, LGWhileNotProbeWait( g, seq, (count,count+1)))
      case LGPrim( prim) => 
        assignLabels( (count, LGSeq( List( LGWhileNotProbeWait( ConstantTrue, LGSeq( List( lg)), (0,0))))))
      case _ => throw new NotYetImplementedException( s"assignLabels: Not expecting ${count} ${lg}")
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


  //
  // Looks like I can make this a lot simplier and faster by first building a
  //    Map[Int,(Boolean,LGWhileProbeWait)]
  // This will replace, lbls, histo, and find
  //
  def transLG1( lg : LabeledGraph) : Command = {
    lg match {
      case LGSeq( lst) => {
        val lbls = lst.map{ case LGWhileNotProbeWait( g, LGSeq( lst0), lbl) => 
          ( lst0.isEmpty || lst0.last != LGWait(), lbl)
        }

        val histo = lbls.foldLeft( Map[Int,Int]()){ case (m,(_,(x,_))) =>
          m.updated( x, m.getOrElse( x, 0)+1)
        }

        if ( !histo.filter{ case (k,v) => v > 1}.isEmpty) {
          throw new Exception( s"transLG1: multi objects with same lower lbl; lbls: ${lbls} histo: ${histo}")
        }

        def find( i : Int) = {
          val matches = lst.filter{ case LGWhileNotProbeWait( g, LGSeq( lst0), lbl) => i == lbl._1}
          if ( matches.isEmpty) throw new Exception( s"transLG1: no match for ${i} in ${lst}")
          if ( matches.size > 1) throw new Exception( s"transLG1: multiple matches ${matches} for ${i} in ${lst}")
          matches.head
        }

        val nodes = for { lbl <- lbls} yield lbl._2._1
        val arcs = for { lbl <- lbls if lbl._1} yield lbl._2

        val t = try {
          TopoSort( nodes, arcs)
        } catch {
          case e : Exception => {
            println( s"Cycle in this graph: lg: ${lg} lbls: ${lbls} nodes: ${nodes.toList} ${arcs.toList}")

/*
          LGSeq(
            List(
              LGWhileNotProbeWait(
                NBCanGet(Port(A)),
                LGSeq(
                  List(
                    LGPrim(NBGet(Port(A),Variable(a)))
                  )
                ),
                (0,1)
              ),
              LGWhileNotProbeWait(
                NBCanGet(Port(B)),
                LGSeq(
                  List(
                    LGPrim(NBGet(Port(B),Variable(b))),
                    LGWait(),
                    LGPrim(Assignment(VectorIndex(o,ConstantInteger(0)),AddExpression(VectorIndex(a,ConstantInteger(0)),VectorIndex(b,ConstantInteger(0))))),
                    LGPrim(Assignment(VectorIndex(o,ConstantInteger(1)),AddExpression(VectorIndex(a,ConstantInteger(1)),VectorIndex(b,ConstantInteger(1)))))
                  )
                ),
                (1,2)
              ),
              LGWhileNotProbeWait(
                NBCanPut(Port(O)),
                LGSeq(
                  List(
                    LGPrim(NBPut(Port(O),Variable(o)))
                  )
                ),
                (2,0)
              )
            )
          )
          lbls: List((true,(0,1)), (true,(1,2)), (true,(2,0))) nodes: List(0, 1, 2) List((0,1), (1,2), (2,0))
 */

            throw e
          }
        }

        transLG( Blk( List(), List()), LGSeq( t.map{ find}))
      }
      case _ => throw new Exception("Wrong LG form")
    }
  }

  def transLG0( lg : LabeledGraph) : (Command,Command) = {

//    println( s"transLG0: ${lg}")


// Handle the special case of combinational initialization
    lg match {
      case LGSeq( LGWhileNotProbeWait( ConstantTrue, LGSeq( lst), lbl) :: tl) if lst.isEmpty || lst.last != LGWait => {
        val initSeg = transLG( Blk( List(), List()), LGSeq( lst))
        val body = transLG1( LGSeq( tl))
        ( initSeg, body)
      }
      case _ => (Blk( List(), List()), transLG1( lg))
    }
  }

  def allLGWhileNotProbeWait( b : Boolean, lg : LabeledGraph) : Boolean = lg match {
    case LGWhileNotProbeWait( g, LGSeq( lst), _) => lst.foldLeft(b){ noLGWhileNotProbeWait}
    case _ => false
  }

  def noLGWhileNotProbeWait( b : Boolean, lg : LabeledGraph) : Boolean = lg match {
    case LGWhileNotProbeWait( g, LGSeq( lst), _) => false
    case _ => b
  }

  def testTL( lg : LabeledGraph) : Boolean = {
    val result = lg match {
      case LGSeq( lst) => lst.foldLeft( true){ allLGWhileNotProbeWait}
      case _ => false
    }
    if ( !result) throw new Exception(s"testTL not satisfied: ${lg}")
    result
  }

  def TransformWaits( ast : Process) : Either[CompilationError, Process] = {
//    println(ast)

    def log2( v : Int) = {
      @tailrec
      def log2( k : Int, n : Int, v : Int) : Int = if ( n>=v) k else log2( k+1, 2*n, v)
      log2( 0, 1, v)
    }

    ast match {
      case Process( portDecls, Blk( decls, seq)) => {
        val (count0, lg0) = assignLabels( (0, genLG0( Blk( decls, seq))))
        val sbits = log2( count0)

        testTL( lg0)

        val (Blk( _, initSeg), Blk( newDecls, newSeq)) = transLG0( lg0)
// Need to declare "s" (induction) and initialize to zero
        val decls0 = decls ++ List( Decl(Variable("s"),UIntType(sbits)))
// Need to declare "w" (combinational) and initialize to zero
        val newDecls0 = newDecls ++ List( Decl(Variable("w"),UIntType(1)))
        val seq0 = List( Assignment(Variable("w"),ConstantInteger(0))) ++ newSeq ++ List(Wait)
        val p =
          Process( portDecls,
            Blk( decls0,
              List( Assignment(Variable("s"),ConstantInteger(if (initSeg.isEmpty) 0 else 1))) ++
                initSeg ++ List( While( ConstantTrue, Blk( newDecls0, seq0)))))
                    
        PrintAST.p( 0, p)
        Right(p)
      }
      case _ => throw new Exception("Wrong AST form")
    }
  }

}
