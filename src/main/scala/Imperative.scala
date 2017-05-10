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
    case Process( _ : PortDeclList, cmd : Command) => cmd match {
      case Blk( seqDeclLst, lst2) => {
        val (declLst, lst) = lst2.last match {
          case While( ConstantTrue, Blk( declLst, lst)) => (declLst, lst)
          case _ => { assert(false); (List(), List()) }
        }
        assert( lst.last == Wait) // Only accepting one format: { var ... while (true) { ...; wait }
        assert( lst2.init == Nil) // Initial segments empty

// might want to be cleverer about which registers are initialized
        val sTinit = seqDeclLst.foldLeft(sT.push){
          case (st, Decl( Variable(v), Type(i))) => st.insert( v, 0.U(i.W))
        }
        val sTinit0 = eval( sTinit, Blk( List(), lst2.init))

        val sT0 = seqDeclLst.foldLeft(sT.push){
          case (st, Decl( Variable(v), Type(i))) => st.insert( v, Wire( UInt(i.W)))
        }

        val sT1 = eval( sT0, Blk( declLst, lst.init))

        sT0.keys.foreach{
          k => {
            sT0(k) := sTinit0(k)
            if ( sT0(k) != sT1(k)) {
              sT0(k) := RegNext( next=sT1(k), init=sTinit0(k))
            }
          }
        }
        sT1.pop
      }
      case _ => { assert(false); sT}
    }
    case _ => { assert(false); sT}
  }

  def eval( sT : SymTbl, ast : Command) : SymTbl = ast match {
    case While( _, _) => { assert( false); sT} // Currently excluded except at top-level
    case Wait => { assert( false); sT} // Currently excluded except at top-level
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

      def mx[T <: Data]( p : T, t : T, e : T) : T = {
// using "!=" because I'm comparing whether the Chisel objects (not their values) are different
        if ( p != t || p != e) {
          val w = Wire( init=e)
          when( bb) { w := t}
          w
        } else p
      }

      val new_sT = (sT /: sT.keys) { case (s,k) =>
        s.updated( k, mx( sT(k), tST(k), eST(k)))
      }

      sT.pkeys.foldLeft(new_sT){ (s,p) => {
        val (pr,pv,pd) = sT.pget( p) // previous
        val (tr,tv,td) = tST.pget( p)
        val (er,ev,ed) = eST.pget( p)
        s.pupdated( p, mx( pr, tr, er), mx( pv, tv, ev), mx( pd, td, ed))
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
