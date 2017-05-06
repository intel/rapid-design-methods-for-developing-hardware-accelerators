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

class ImperativeModule( io_tuples : List[(String,UInt)],
                        iod_tuples : List[(String,DecoupledIO[UInt])],
                        ast : Command) extends Module {

//  val io = IO(new CustomUIntBundle( io_tuples: _*))
  val io = IO(new CustomDecoupledUIntBundle( iod_tuples: _*))

  def eval( sT : SymTbl, ast : Expression) : UInt = ast match {
    case Variable( s) => sT( s)
    case NBGetData( Port( p)) => sT.pget( p)._3
    case AddExpression( l, r) => eval( sT, l) + eval( sT, r)
    case MulExpression( l, r) => eval( sT, l) * eval( sT, r)
    case ConstantInteger( i) => i.U
  }

  def eval( sT : SymTbl, ast : BExpression) : Bool = ast match {
    case ConstantTrue => true.B
    case EqBExpression( l, r) => eval( sT, l) === eval( sT, r)
    case AndBExpression( l, r) => eval( sT, l) && eval( sT, r)
    case NotBExpression( e) => !eval( sT, e)
    case NBCanGet( Port( p)) => sT.pget( p)._2
    case NBCanPut( Port( p)) => sT.pget( p)._1
  }

  def eval( sT : SymTbl, ast : Command) : SymTbl = ast match {
    case While( ConstantTrue, SequentialComposition( lst)) => {
      // set up
      assert( lst.last == Wait) // Only accepting one format: while (true) { ...; wait }
      val new_sT = eval( sT, SequentialComposition( lst.init))
      val changedKeys = sT.keys.filter{ k => sT(k) != new_sT(k)}
      println( s"Registers: ${changedKeys}")
      changedKeys.foldLeft(new_sT) { case (s,k) =>
        sT(k) := new_sT(k) // Next state update
        s.updated( k, sT(k)) // Use the register output (not input)
      }
    }
    case SequentialComposition( seq) => seq.foldLeft(sT){ eval}
    case Assignment( Variable( s), r) => sT.updated( s, eval( sT, r))
    case NBGet( Port( p)) => {
      val (r,v,d) = sT.pget( p)
      sT.pupdated( p, true.B, v, d)
    }
    case NBPut( Port( p), e) => {
      val (r,v,d) = sT.pget( p)
      sT.pupdated( p, r, true.B, eval( sT, e))
    }
    case IfThenElse( b, t, e) => {
      val (bb, tST, eST) = ( eval( sT, b), eval( sT, t), eval( sT, e))
// using "!=" because I'm comparing whether the Chisel objects (not their values) are different
      val changedKeys = sT.keys.filter{ k => tST(k) != eST(k)}
      val new_sT = changedKeys.foldLeft(sT){ case (s,k) =>
        val w = Wire( init=eST(k))
        when( bb) { w := tST(k) }
        s.updated( k, w)
      }
      sT.pkeys.foldLeft(new_sT){ (s,p) => {
        val (r,v,d) = (Wire(Bool()),Wire(Bool()),Wire(UInt()))
        val (er,ev,ed) = eST.pget( p)
        val (tr,tv,td) = tST.pget( p)
        println( s"${p} tr,tv,td: ${tr} ${tv} ${td}")
        println( s"${p} er,ev,ed: ${er} ${ev} ${ed}")
        println( s"${p} r ,v ,d : ${r} ${v} ${d}")
        printf( s"${p} tr,tv,td: %d %d %d\n", tr, tv, td)
        printf( s"${p} er,ev,ed: %d %d %d\n", er, ev, ed)
        printf( s"${p} bb; r,v,d: %d %d %d %d\n", bb, r, v, d)
        when( bb) {
          r := tr
          v := tv
          d := td
        } .otherwise {
          r := er
          v := ev
          d := ed
        }
        s.pupdated( p, r, v, d)
      }}
    }
  }

  val pp = io("P")
  val (pr,pv,pd) = (pp.ready,pp.valid,pp.bits)
  val qq = io("Q")
  val (qr,qv,qd) = (qq.ready,qq.valid,qq.bits)

  val sT = (new SymTbl).pupdated( "P", false.B, pv, pd).pupdated( "Q", qr, false.B, Wire(qd.cloneType))
/*
  val inps = io.elements.filter{ case (k,v) => v.dir == core.Direction.Input}
  val outs = io.elements.filter{ case (k,v) => v.dir == core.Direction.Output}

  val sTInps = inps.foldLeft( sT    ){ case (s,(k,v)) => s.updated( k, io(k))}
  val sTOuts = outs.foldLeft( sTInps){
    case (s,(k,v)) => s.updated( k, RegInit( io(k).cloneType, init=0.U))
  }
 */
// Only for Channel
//  val sTOuts = sT
// Only for Squash
  val sTOuts = sT.updated( "f", RegInit( UInt(1.W), init=0.U)).updated( "v", Reg( qd.cloneType))

  val sTLast = eval( sTOuts, ast)

  {
    val (r,v,d) = sTLast.pget( "P")
    println( s"P r: ${r}")
    io("P").ready := r
  }
  {
    val (r,v,d) = sTLast.pget( "Q")
    println( s"Q v: ${v} d: ${d}")
    io("Q").valid := RegNext( next=v, init=false.B)
    io("Q").bits := RegNext( next=d)
  }

//  outs.foreach{ case (k,v) => io(k) := sTLast(k) }

}
