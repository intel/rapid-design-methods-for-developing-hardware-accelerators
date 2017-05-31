package imperative

import scala.annotation.tailrec

import compiler._
import collection.immutable.ListMap
import collection.immutable.Set

object HLS3 {

  case class State( m : Map[Int,Command], g : Int, c : List[(Int,Int)]) {
    def upM( k : Int, v : Command) : State = State( m.updated( k, v), g, c)
    def upG( g0 : Int) : State = State( m, g0, c)
    def upC( c0 : (Int,Int)) : State = State( m, g, c0 :: c)
  }

  def apply( ast : Process) : Either[CompilationError, Process] = {
    for {
      ast1 <- General( ast).right
      ast2 <- SemanticAnalyzer.pass1( ast1).right
      ast3 <- SemanticAnalyzer.loweredCheck( ast2).right
    } yield ast3
  }

  def waitFree( cmd : Command) : Boolean = cmd match {
    case IfThenElse( b, t, e) => waitFree( t) && waitFree( e)
    case While( _, _) => false
    case Wait => false
    case Blk( _, lst) => lst.foldLeft( true){ case (b,c) => b && waitFree( c)}
    case _ => true
  }

  @tailrec
  def split( lst : List[Command], accum : List[Command]): (List[Command],List[Command]) = {
    lst match {
      case Nil => (accum.reverse, List())
      case Wait::tl => ((Wait::accum).reverse, tl)
      case (hd@Blk( decls, lst0))::tl =>
        lst0 match {
          case Nil => split( tl, accum)
          case hd0::tl0 => split( hd0::Blk(decls,tl0)::tl, accum)
        }
      case hd::tl => 
        if ( waitFree( hd)) split( tl, hd::accum) else (accum.reverse, lst)
    }
  }


  def expand( st : State, lb : Int, ub : Int, cmd : Command) : State = cmd match {
    case IfThenElse( b, t, e) => {
      val (s1,s2,g0) = (st.g,st.g+1,st.g+2)
      val st0 = st.upM( lb, 
        IfThenElse( 
          AndBExpression( 
            EqBExpression( Variable( "s"), ConstantInteger( lb)),
            EqBExpression( Variable( "w"), ConstantInteger( 0))
          ),
          IfThenElse(
            b,
            Assignment( Variable( "s"), ConstantInteger( s1)),
            Assignment( Variable( "s"), ConstantInteger( s2))
          ),
          Blk( List(), List())
        )
      ).upC( (lb,s1)).upC( (lb,s2)).upG( g0)
      expand( expand( st0, s1, ub, t), s2, ub, e)
    }
    case While( b, e) => {
      val (s1,g0) = (st.g,st.g+1)
      val st0 = st.upM( lb, 
        IfThenElse( 
          AndBExpression( 
            EqBExpression( Variable( "s"), ConstantInteger( lb)),
            EqBExpression( Variable( "w"), ConstantInteger( 0))
          ),
          IfThenElse(
            b,
            Assignment( Variable( "s"), ConstantInteger( s1)),
            Assignment( Variable( "s"), ConstantInteger( ub))
          ),
          Blk( List(), List())
        )
      ).upC( (lb,s1)).upC( (lb,ub)).upG( g0)
      expand( st0, s1, lb, e)
    }
    case UntilFinallyBody( b, fin, body) => {
      def stripWait( cmd : Command) : (List[Command],Boolean) = cmd match {
        case Blk( _, l) if l.isEmpty => (l.toList, false)
        case Blk( _, l) if l.last == Wait => (l.toList.init, true)
        case Blk( _, l) => (l.toList, false)
        case Wait => (List(), true)
        case _ => (List(cmd), false)
      }

      def condWait( b : Boolean) : List[Command] =
        if ( b) List( Assignment( Variable( "w"), ConstantInteger( 1))) else List()

      val (lst0,w0) = stripWait( fin)
      val (lst1,w1) = stripWait( body)

      val st0 = st.upM( lb, 
        IfThenElse( 
          AndBExpression( 
            EqBExpression( Variable( "s"), ConstantInteger( lb)),
            EqBExpression( Variable( "w"), ConstantInteger( 0))
          ),
          IfThenElse(
            b,
            Blk(
              List(),
              lst0 ++ List( Assignment( Variable( "s"), ConstantInteger( ub))) ++ condWait( w0)

            ),
            Blk(
              List(),
              lst1 ++ condWait( w1)
            )
          ),
          Blk( List(), List())
        )
      ).upC( (lb,ub))
      if ( !w0)
        st0.upC( (lb,ub))
      else
        st0
    }
    case Wait => {
      st.upM( lb, 
        IfThenElse( 
          AndBExpression( 
            EqBExpression( Variable( "s"), ConstantInteger( lb)),
            EqBExpression( Variable( "w"), ConstantInteger( 0))
          ),
          Blk( 
            List(),
            List(
              Assignment( Variable( "s"), ConstantInteger( ub)),
              Assignment( Variable( "w"), ConstantInteger( 1))
            )
          ),
          Blk( List(), List())
        )
      )
    }
    case Blk( decls, Nil) => {
      st.upM( lb, 
        IfThenElse( 
          AndBExpression( 
            EqBExpression( Variable( "s"), ConstantInteger( lb)),
            EqBExpression( Variable( "w"), ConstantInteger( 0))
          ),
          Blk( 
            List(),
            List(
              Assignment( Variable( "s"), ConstantInteger( ub))
            )
          ),
          Blk( List(), List())
        )
      ).upC( lb, ub)
    }
    case Blk( decls, lst@(hd::tl)) => {
      val (seq0,seq1) = split( lst, List())
      if        (  seq0.isEmpty) {
        hd match {
          case While( NotBExpression( e), b) => {
            val (seq0,seq1) = split( tl, List())
            val (s1,g0) = if ( seq1.isEmpty)  (ub, st.g) else  (st.g,st.g+1)
            val st0 = expand( st, lb, s1, UntilFinallyBody( e, Blk( List(), seq0), b))
            if ( seq1.isEmpty)
              st0
            else
              expand( st0, s1, ub, Blk( List(), seq1))
          }
          case While( ConstantTrue, b) => {
            assert( tl.isEmpty)
            expand( st, lb, ub, hd)
          }
          case While( _, _) => throw new NotYetImplementedException( "Complex while expression")
          case _ => {
            val (s1,g0) = (st.g,st.g+1)
            expand( expand( st.upG(g0),lb,s1,hd), s1, ub, Blk( decls, tl))
          }
        }

      } else {
        val (s1,g0) = if ( seq1.isEmpty) (ub, st.g) else ( st.g, st.g+1)

        val (seq0prime, hasWait) =
          if ( !seq0.isEmpty && Wait == seq0.last) (seq0.init,true) else (seq0,false)

        val st0 = st.upG( g0).upM( lb,
          IfThenElse(
            AndBExpression(
              EqBExpression( Variable( "s"), ConstantInteger( lb)),
              EqBExpression( Variable( "w"), ConstantInteger( 0))
            ),
            Blk(
              List(),
              seq0prime ++ List(
                Assignment( Variable( "s"), ConstantInteger( s1)),
                Assignment( Variable( "w"), ConstantInteger( if ( hasWait) 1 else 0))
              )
            ),
            Blk( List(), List())
          )
        )
        val st1 = if ( hasWait) st0 else st0.upC( lb, s1)

        if ( seq1.isEmpty) 
          st1
        else
          expand( st1, s1, ub, Blk( List(), seq1))
      }
    }
    case cmd => {
      st.upM( lb, 
        IfThenElse( 
          AndBExpression( 
            EqBExpression( Variable( "s"), ConstantInteger( lb)),
            EqBExpression( Variable( "w"), ConstantInteger( 0))
          ),
          Blk( 
            List(),
            List(
              cmd,
              Assignment( Variable( "s"), ConstantInteger( ub))
            )
          ),
          Blk( List(), List())
        )
      ).upC( lb, ub)
    }
  }

  def log2( v : Int) = {
    @tailrec
    def log2( k : Int, n : Int, v : Int) : Int = if ( n>=v) k else log2( k+1, 2*n, v)
    log2( 0, 1, v)
  }

  def General( ast : Process) : Either[CompilationError, Process] = ast match {
    case Process( portDecls, cmd@Blk( decls, lst)) => {
      val finalCmd = Blk( List(), List())
      val st = expand( State( Map( 1 -> finalCmd), 2, List()), 0, 1, cmd)
      st.m.toSeq.foreach{ case (k,v) => PrintAST.p( 0, v)}
      val order = TopoSort( st.m.keys.toSeq, st.c)
      val lst = order.map{ idx => st.m(idx)}
      val sbits = log2( st.g)
      val cmd0 =
        Blk(
          decls ++ List(
            Decl( Variable("s"),UIntType(sbits))
          ),
          List(
            Assignment( Variable("s"),ConstantInteger(0)), 
            While(
              ConstantTrue,
              Blk(
                List(
                  Decl( Variable("w"),UIntType(1))
                ),
                List( Assignment( Variable("w"),ConstantInteger( 0))) ++
                lst ++ List(Wait)
              )
            )
          )
        )
              
      PrintAST.p( 0, cmd0)
      Right( Process( portDecls, cmd0))
    }
    case _ => throw new Exception("Needs to be Process")
  }

}
