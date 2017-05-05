package imperative

import chisel3._
import collection.immutable.ListMap

class CustomUIntBundle(elts: (String, UInt)*) extends Record {
  val elements = ListMap(elts map { case (field, elt) => field -> elt.chiselCloneType }: _*)
  def apply(elt: String): UInt = elements(elt)
  override def cloneType = (new CustomUIntBundle(elements.toList: _*)).asInstanceOf[this.type]
}

class ImperativeModule( io_tuples : List[(String,UInt)], ast : AST) extends Module {

  val io = IO(new CustomUIntBundle( io_tuples: _*))
  val inps = io.elements.filter{ case (k,v) => v.dir == core.Direction.Input}
  val outs = io.elements.filter{ case (k,v) => v.dir == core.Direction.Output}

  type SymTbl = ListMap[String,UInt]

  def wireCopy( st : SymTbl) = st

  def evalExpression( sT : SymTbl, ast : Expression) : UInt = ast match {
    case Variable( s) => sT( s)
    case AddExpression( l, r) => evalExpression( sT, l) + evalExpression( sT, r)
  }

  def evalBExpression( sT : SymTbl, ast : BExpression) : Bool = ast match {
    case ConstantTrue => true.B
    case AndBExpression( l, r) => evalBExpression( sT, l) && evalBExpression( sT, r)
    case NotBExpression( e) => !evalBExpression( sT, e)
  }

  def evalCommand( sT : SymTbl, ast : AST) : SymTbl = ast match {
    case While( ConstantTrue, b) => evalCommand( sT, b)
    case SequentialComposition( seq) => seq.foldLeft(sT){ evalCommand}
    case Assignment( Variable( s), r) => sT.updated( s, evalExpression( sT, r))
    case IfThenElse( b, t, e) => {
      val bb = evalBExpression( sT, b)
      val tST = evalCommand( sT, t)
      val eST = evalCommand( sT, e)
      tST
    }
  }

  val sT : SymTbl = ListMap()
  val sTInps = inps.foldLeft( sT    ){ case (s,(k,v)) => s.updated( k, io(k))}
  val sTOuts = outs.foldLeft( sTInps){ case (s,(k,v)) => s.updated( k, io(k).cloneType)}
  val sTLast = evalCommand( sTOuts, ast)
  outs.foreach{ case (k,v) => io(k) := sTLast(k) }

}
