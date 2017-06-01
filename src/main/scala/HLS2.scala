package imperative

import scala.annotation.tailrec

import compiler._
import collection.immutable.ListMap
import collection.immutable.Set

object HLS2 {

  case class State( m : Map[Int,Command], g : Int, c : List[(Int,Int)]) {
    def upM( k : Int, v : Command) : State = State( m.updated( k, v), g, c)
    def upG( g0 : Int) : State = State( m, g0, c)
    def upC( c0 : (Int,Int)) : State = State( m, g, c0 :: c)
  }

  def apply( ast : Process) : Either[CompilationError, Process] = {
    for {
      ast1 <- General( ast).right
      ast2 <- SemanticAnalyzer.pass1( ast1).right
      ast3 <- SemanticAnalyzer.loweredCheck( ast2, false).right
    } yield ast3
  }

  def condWait( b : Boolean) : List[Command] =
    if ( b) List( Assignment( Variable( "w"), ConstantInteger( 1))) else List()

  def assignS( s : Int) : Command =
    Assignment( Variable( "s"), ConstantInteger( s))

  def guardS( s : Int, cmd : Command) : Command = {
    println( s"guardS: ${s}")
    PrintAST.p( 0, cmd)
    IfThenElse( 
      AndBExpression(
        EqBExpression( Variable( "s"), ConstantInteger( s)),
        EqBExpression( Variable( "w"), ConstantInteger( 0))
      ),
      cmd,
      Blk( List(), List())
    )
  }

  def wrapBlk( lst : List[Command]) = Blk( List(), lst)

  def expand( st : State, lb : Int, ub : Int, cmd : Command) : State = cmd match {
    case IfThenElse( b, t, e) => {
      val (s1,s2,g0) = (st.g,st.g+1,st.g+2)
      val st0 = st.upM( lb,
        guardS( lb, IfThenElse( b, assignS( s1), assignS( s2)))
      ).upC( (lb,s1)).upC( (lb,s2)).upG( g0)
      expand( expand( st0, s1, ub, t), s2, ub, e)
    }
    case While( b, e) => {
      val (s1,g0) = (st.g,st.g+1)
      val st0 = st.upM( lb, 
        guardS( lb, IfThenElse( b, assignS( s1), assignS( ub)))
      ).upC( (lb,s1)).upC( (lb,ub)).upG( g0)
      expand( st0, s1, lb, e)
    }
    case Wait => st.upM( lb, guardS( lb, wrapBlk( List( assignS( ub)) ++ condWait( true))))
    case Blk( decls, Nil) => st.upM( lb, guardS( lb, wrapBlk( List( assignS( ub))))).upC( lb, ub)
    case Blk( decls, hd :: Nil) => expand( st, lb, ub, hd)
    case Blk( decls, hd :: tl) => {
      val (s1,g0) = (st.g,st.g+1)
      expand( expand( st.upG(g0),lb,s1,hd), s1, ub, Blk( decls, tl))
    }
    case cmd => st.upM( lb, guardS( lb, wrapBlk( List( cmd, assignS( ub))))).upC( lb, ub)
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
      st.m.toSeq.foreach{ case (k,v) => { print( s"$k: "); PrintAST.p( 0, v)}}
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
