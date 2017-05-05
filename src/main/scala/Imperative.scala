package imperative

import chisel3._
import chisel3.util._
import collection.immutable.ListMap

class CustomUIntBundle(elts: (String, UInt)*) extends Record {
  val elements = ListMap(elts map { case (field, elt) => field -> elt.chiselCloneType }: _*)
  def apply(elt: String): UInt = elements(elt)
  override def cloneType = (new CustomUIntBundle(elements.toList: _*)).asInstanceOf[this.type]
}

class CustomDecoupledUIntBundle(elts: (String, DecoupledIO[UInt])*) extends Record {
  val elements = ListMap(elts map { case (field, elt) => field -> elt.chiselCloneType }: _*)
  def apply(elt: String): DecoupledIO[UInt] = elements(elt)
  override def cloneType = (new CustomDecoupledUIntBundle(elements.toList: _*)).asInstanceOf[this.type]
}

class ImperativeModule( io_tuples : List[(String,UInt)], ast : Command) extends Module {

  val io = IO(new CustomUIntBundle( io_tuples: _*))
  val inps = io.elements.filter{ case (k,v) => v.dir == core.Direction.Input}
  val outs = io.elements.filter{ case (k,v) => v.dir == core.Direction.Output}

  type SymTbl = ListMap[String,UInt]

  def eval( sT : SymTbl, ast : Expression) : UInt = ast match {
    case Variable( s) => sT( s)
    case NBGet( Port( p)) => 0.U // mocked
    case AddExpression( l, r) => eval( sT, l) + eval( sT, r)
    case MulExpression( l, r) => eval( sT, l) * eval( sT, r)
    case ConstantInteger( i) => i.U
  }

  def eval( sT : SymTbl, ast : BExpression) : Bool = ast match {
    case ConstantTrue => true.B
    case AndBExpression( l, r) => eval( sT, l) && eval( sT, r)
    case NotBExpression( e) => !eval( sT, e)
    case NBCanGet( Port( p)) => true.B
    case NBCanPut( Port( p)) => true.B
  }

  def eval( sT : SymTbl, ast : Command) : SymTbl = ast match {
    case While( ConstantTrue, SequentialComposition( lst)) => {
      assert( lst.last == Wait) // Only accepting one format: while (true) { ...; wait }
      val new_sT = eval( sT, SequentialComposition( lst.init))
      val changedKeys = sT.keys.filter{ k => sT(k) != new_sT(k)}
      println( changedKeys)
      changedKeys.foldLeft(new_sT) { case (s,k) =>
        sT(k) := new_sT(k) // Next state update
        s.updated( k, sT(k)) // Use the register output (not input)
      }
    }
    case SequentialComposition( seq) => seq.foldLeft(sT){ eval}
    case Assignment( Variable( s), r) => sT.updated( s, eval( sT, r))
    case NBPut( Port( p), e) => sT
    case IfThenElse( b, t, e) => {
      val (bb, tST, eST) = ( eval( sT, b), eval( sT, t), eval( sT, e))
// using "!=" because I'm comparing whether the Chisel objects (not their values) are different
      val changedKeys = sT.keys.filter{ k => tST(k) != eST(k)}
      changedKeys.foldLeft(sT){ case (s,k) =>
        val w = Wire( init=eST(k))
        when( bb) { w := tST(k) }
        s.updated( k, w)
      }
    }
  }

  val sT : SymTbl = ListMap()
  val sTInps = inps.foldLeft( sT    ){ case (s,(k,v)) => s.updated( k, io(k))}
  val sTOuts = outs.foldLeft( sTInps){
    case (s,(k,v)) => s.updated( k, RegInit( io(k).cloneType, init=0.U))
  }
  val sTLast = eval( sTOuts, ast)
  outs.foreach{ case (k,v) => io(k) := sTLast(k) }

}
