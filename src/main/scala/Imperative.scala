package imperative

import chisel3._
import chisel3.util._
import collection.immutable.ListMap

class CustomDecoupledUIntBundle(elts: (String, DecoupledIO[UInt])*) extends Record {
  val elements = ListMap(elts map { case (field, elt) => field -> elt.chiselCloneType }: _*)
  def apply(elt: String): DecoupledIO[UInt] = elements(elt)
  override def cloneType = (new CustomDecoupledUIntBundle(elements.toList: _*)).asInstanceOf[this.type]
}

class ImperativeModule( ast : Process) extends Module {

  val decl_lst = ast match { case Process( PortDeclList( decl_lst), _) => decl_lst}

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

  def eval( sT : SymTbl, ast : Process) : SymTbl = ast match {
    case Process( lst : PortDeclList, cmd : Command) => cmd match {
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
    }
  }

  def eval( sT : SymTbl, ast : Command) : SymTbl = ast match {
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

// can't make || work. Chisel.Bool found instead of Boolean
//      val changedKeys = sT.keys.filter{ k => sT(k) != tST(k) || sT(k) != eST(k)}
      def or( l : Boolean, r : Boolean) : Boolean = l || r
// using "!=" because I'm comparing whether the Chisel objects (not their values) are different
      val changedKeys = sT.keys.filter{ k => or( sT(k) != tST(k), sT(k) != eST(k))}
// (sT /: changedKeys) is the same as changedKeys.foldLeft(sT) 
      val new_sT = (sT /: changedKeys) { case (s,k) =>
        val w = Wire( init=eST(k))
        when( bb) { w := tST(k) }
        s.updated( k, w)
      }

      sT.pkeys.foldLeft(new_sT){ (s,p) => {
        val (pr,pv,pd) = sT.pget( p) // previous
        val (tr,tv,td) = tST.pget( p)
        val (er,ev,ed) = eST.pget( p)

        val r = if ( or(pr != tr, pr != er)) {
//          println( s"${p} pr,tr,er: ${pr} ${tr} ${er}")
          val r = Wire(Bool())
          r := er
          when( bb) { r := tr}
          r
        } else pr

        val v = if ( or(pv != tv, pv != ev)) {
//          println( s"${p} pv,tv,ev: ${pv} ${tv} ${ev}")
          val v = Wire(Bool())
          v := ev
          when( bb) { v := tv}
          v
        } else pv

        val d = if ( or(pd != td, pd != ed)) {
//          println( s"${p} pd,td,ed: ${pd} ${td} ${ed}")
          val d = Wire(UInt())
          d := ed
          when( bb) { d := td}
          d
        } else pd

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
//      println( s"${p} r: ${r}")
      io(p).ready := r
    }
    case PortDecl( Port(p), Out, Type(w)) => {
      val (r,v,d) = sTLast.pget( p)
//      println( s"${p} v: ${v} d: ${d}")
      io( p).valid := RegNext( next=v, init=false.B)
      io( p).bits := RegNext( next=d)
    }
  }

}
