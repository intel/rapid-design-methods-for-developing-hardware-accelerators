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

class ImperativeModule( ast : Program) extends Module {

  val decl_lst = ast match { case Program( PortDeclList( decl_lst), _) => decl_lst}

  val iod_tuples = decl_lst.map{ 
    case PortDecl( Port(p), Inp, Type(w)) => (p,Flipped(Decoupled(UInt(w.W))))
    case PortDecl( Port(p), Out, Type(w)) => (p,Decoupled(UInt(w.W)))
  }

  val io = IO(new CustomDecoupledUIntBundle( iod_tuples: _*))

  def eval( sT : SymTbl, ast : Expression) : UInt = ast match {
    case Variable( s) => sT( s)
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

  def eval( sT : SymTbl, ast : Program) : SymTbl = ast match {
    case Program( lst : PortDeclList, cmd : Command) => {
      eval( sT, cmd)
    }
  }

  def eval( sT : SymTbl, ast : Command) : SymTbl = ast match {
    case Blk( seqDeclLst, List( While( ConstantTrue, Blk( declLst, lst)))) => {
      assert( lst.last == Wait) // Only accepting one format: { var ... while (true) { ...; wait }

      val sT0 = seqDeclLst.foldLeft(sT.push){ case (st, Decl( Variable(v), Type(i))) => {
        st.insert( v, RegInit( 0.U(i.W)))
      }}

      val sT1 = eval( sT0, Blk( declLst, lst.init))
      val changedKeys = sT0.keys.filter{ k => sT0(k) != sT1(k)}
      (changedKeys.foldLeft(sT1) { case (s,k) =>
        sT0(k) := sT1(k) // Next state update
        s.updated( k, sT0(k)) // Use the register output (not input)
      }).pop
    }
    case Blk( decl_lst, seq) => {
      val sT0 = decl_lst.foldLeft(sT.push){ case (st, Decl( Variable(v), Type(i))) => {
        st.insert( v, Wire( UInt(i.W)))
      }}
      seq.foldLeft(sT0){ eval}.pop
    }
    case Assignment( Variable( s), r) => sT.updated( s, eval( sT, r))
    case NBGet( Port( p), Variable( s)) => {
      val (r,v,d) = sT.pget( p)
      sT.pupdated( p, true.B, v, d).updated( s, d)
    }
    case NBPut( Port( p), e) => {
      val (r,v,d) = sT.pget( p)
      sT.pupdated( p, r, true.B, eval( sT, e))
    }
    case IfThenElse( b, t, e) => {
      val (bb, tST, eST) = ( eval( sT, b), eval( sT, t), eval( sT, e))
// using "!=" because I'm comparing whether the Chisel objects (not their values) are different
      val changedKeys = sT.keys.filter{ k => tST(k) != eST(k)}
// (sT /: changedKeys) is the same as changedKeys.foldLeft(sT) 
      val new_sT = (sT /: changedKeys) { case (s,k) =>
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
/*
        printf( s"${p} tr,tv,td: %d %d %d\n", tr, tv, td)
        printf( s"${p} er,ev,ed: %d %d %d\n", er, ev, ed)
        printf( s"${p} bb; r,v,d: %d %d %d %d\n", bb, r, v, d)
 */
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

  val sT = decl_lst.foldLeft(new SymTbl) {
    (s,pd) => pd match {
      case PortDecl( Port(p), Inp, Type(w)) => {
        val pp = io(p)
        val (r,v,d) = (pp.ready,pp.valid,pp.bits)
        s.pupdated( p, false.B, v, d)
      }
      case PortDecl( Port(p), Out, Type(w)) => {
        val pp = io(p)
        val (r,v,d) = (pp.ready,pp.valid,pp.bits)
        s.pupdated( p, r, false.B, Wire(d.cloneType))
      }
    }
  }

  val sTLast = eval( sT, ast)

  decl_lst.foreach {
    case PortDecl( Port(p), Inp, Type(w)) => {
      val (r,v,d) = sTLast.pget( p)
      println( s"${p} r: ${r}")
      io(p).ready := r
    }
    case PortDecl( Port(p), Out, Type(w)) => {
      val (r,v,d) = sTLast.pget( p)
      println( s"${p} v: ${v} d: ${d}")
      io( p).valid := RegNext( next=v, init=false.B)
      io( p).bits := RegNext( next=d)
    }
  }

}
