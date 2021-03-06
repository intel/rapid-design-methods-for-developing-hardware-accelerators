package imperative

import transform.{ShannonFactor}

import chisel3._
import chisel3.util._
import collection.immutable.ListMap

import chisel3.experimental.ChiselAnnotation

import firrtl.annotations.{Annotation}

trait ShannonFactorAnnotator {
  self: Module =>
//     annotate(ChiselAnnotation( self, classOf[ShannonFactor], "RunShannonFactor"))
}

class CustomDecoupledBundle(elts: (String, DecoupledIO[Data], Type)*) extends Record {
  val elements = ListMap(elts map { case (field, elt, ty) => field -> elt.chiselCloneType }: _*)
  def apply(elt: String): DecoupledIO[Data] = elements(elt)

  override def cloneType = (new CustomDecoupledBundle(elts.toList: _*)).asInstanceOf[this.type]
}

class LoweredFormException extends Exception
class ImproperLeftHandSideException extends Exception
class NonConstantUnrollBoundsException extends Exception

class ImperativeIfc( ast : Process) extends Module {
  val decl_lst = ast match { case Process( PortDeclList( decl_lst), _) => decl_lst}

  val iod_tuples = decl_lst.map{ 
    case PortDecl( Port(p), Inp, t @ UIntType(w)) => (p,Flipped(Decoupled(UInt(w.W))),t)
    case PortDecl( Port(p), Out, t @ UIntType(w)) => (p,Decoupled(UInt(w.W)),t)
    case PortDecl( Port(p), Inp, t @ VecType(n,UIntType(w))) => (p,Flipped(Decoupled(Vec(n,UInt(w.W)))),t)
    case PortDecl( Port(p), Out, t @ VecType(n,UIntType(w))) => (p,Decoupled(Vec(n,UInt(w.W))),t)
  }

  val io = IO(new CustomDecoupledBundle( iod_tuples: _*))

}


class ImperativeModule( ast : Process) extends ImperativeIfc( ast) with ShannonFactorAnnotator {

  def eval( sT : SymTbl, ast : BExpression) : Bool = ast match {
    case ConstantTrue => true.B
    case EqBExpression( l, r) => eval( sT, l).asInstanceOf[UInt] === eval( sT, r).asInstanceOf[UInt]
    case LtBExpression( l, r) => eval( sT, l).asInstanceOf[UInt] < eval( sT, r).asInstanceOf[UInt]
    case AndBExpression( l, r) => eval( sT, l) && eval( sT, r)
    case NotBExpression( e) => !eval( sT, e)
    case NBCanGet( Port( p)) => sT.pget( p)._2
    case NBCanPut( Port( p)) => sT.pget( p)._1
  }

  def evalWithoutPort( p : Port)( sT : SymTbl, ast : BExpression) : Bool = ast match {
    case ConstantTrue => true.B
    case EqBExpression( l, r) => eval( sT, l).asInstanceOf[UInt] === eval( sT, r).asInstanceOf[UInt]
    case LtBExpression( l, r) => eval( sT, l).asInstanceOf[UInt] < eval( sT, r).asInstanceOf[UInt]
    case AndBExpression( l, r) => evalWithoutPort( p)( sT, l) && evalWithoutPort( p)( sT, r)
    case NotBExpression( e) => !evalWithoutPort( p)( sT, e)
    case NBCanGet( Port( pp)) if Port( pp) == p => true.B
    case NBCanGet( Port( pp)) if Port( pp) != p => sT.pget( pp)._2
    case NBCanPut( Port( pp)) if Port( pp) == p => true.B
    case NBCanPut( Port( pp)) if Port( pp) != p => sT.pget( pp)._1
  }

  def eval( sT : SymTbl, ast : Expression) : Data = ast match {
    case VectorIndex( s, ConstantInteger( i)) => {
      val whole = sT( s).asInstanceOf[Vec[UInt]]
      val part = whole(i)
//      printf( s"vector eval: ${s}(${i}) %d %d %d %d %d\n", whole(0), whole(1), whole(2), whole(3), part)
      part
    }
    case VectorIndex( s, e : Expression) => {
      val index = eval( sT, e).asInstanceOf[UInt]
      if ( index.isLit()) {
        println( s"Converting to constant index: ${s}(${index.litValue()})")
        eval( sT, VectorIndex( s, ConstantInteger( index.litValue().toInt)))
      } else {
        val whole = sT( s).asInstanceOf[Vec[UInt]]
        val part = whole(index)
//        printf( s"vector eval: ${s}(${e}) %d %d %d %d %d %d\n", whole(0), whole(1), whole(2), whole(3), index, part)
        part
      }
    }
    case Variable( s) => sT( s)
    case AddExpression( l, r) => {
      val eL = eval( sT, l).asInstanceOf[UInt]
      val eR = eval( sT, r).asInstanceOf[UInt]
      if ( eL.isLit() && eR.isLit()) {
        (eL.litValue() + eR.litValue()).U
      } else {
        eL + eR
      }
    }
    case SubExpression( l, r) => {
      val eL = eval( sT, l).asInstanceOf[UInt]
      val eR = eval( sT, r).asInstanceOf[UInt]
      if ( eL.isLit() && eR.isLit()) {
        (eL.litValue() - eR.litValue()).U
      } else {
        eL - eR
      }
    }
    case MulExpression( l, r) => {
      val eL = eval( sT, l).asInstanceOf[UInt]
      val eR = eval( sT, r).asInstanceOf[UInt]
      if ( eL.isLit() && eR.isLit()) {
        (eL.litValue() * eR.litValue()).U
      } else {
        eL * eR
      }
    }
    case ConstantInteger( i) => i.U
  }


  def eval( sT : SymTbl, ast : Process) : SymTbl = ast match {
    case Process( _, ResetWhileTrueWait( seqDeclLst, initSeq, mainBlk)) => {

        val sTinit = seqDeclLst.foldLeft(sT.push){
          case (st, Decl( Variable(v), UIntType(w))) => st.insert( v, WireInit( UInt(w.W), init=DontCare))
          case (st, Decl( Variable(v), VecType(n,UIntType(w)))) => st.insert( v, WireInit( Vec(n,UInt(w.W)), init=DontCare))
        }

        val sTinit0 = eval( sTinit, Blk( List(), initSeq))

        val sT0 = seqDeclLst.foldLeft(sT.push){
          case (st, Decl( Variable(v), UIntType(w))) => st.insert( v, WireInit( UInt(w.W), init=DontCare))
          case (st, Decl( Variable(v), VecType(n,UIntType(w)))) => st.insert( v, WireInit( Vec(n,UInt(w.W)), init=DontCare))
        }

        val sT1 = eval( sT0, mainBlk)

        sT0.keys.foreach{
          k => {
            sT0(k) := sTinit0(k)
            if ( sT0(k) != sT1(k)) {
              if ( sTinit(k) == sTinit0(k)) {
//                println( s"-I- Induction variable ${k} is not initialized but is updated")
                sT0(k) := RegNext( next=sT1(k))
              } else {
//                println( s"-I- Induction variable ${k} is both initialized and updated")
                sT0(k) := RegNext( next=sT1(k), init=sTinit0(k))
              }
            } else {
              if ( sTinit(k) != sTinit0(k)) {
//                println( s"-I- Induction variable ${k} is initialized but not updated")
                sT0(k) := sTinit0(k)
              } else {
//                println( s"-I- Induction variable ${k} is not initialized and not updated")
              }
            }
          }
        }
        sT1.pop
      }
    case _ => throw new LoweredFormException
  }

  def eval( sT : SymTbl, ast : Command) : SymTbl = ast match {
    case While( _, _) => throw new LoweredFormException
    case Wait => throw new LoweredFormException
    case ResetWhileTrueWait( _, _, _) => throw new LoweredFormException
    case Unroll( Variable( v), ConstantInteger( lb), ConstantInteger( ub), cmd) => 
      (lb until ub).foldLeft( sT){ case( st, i) => eval( st.push.insert( v, i.U), cmd).pop}
    case Unroll( Variable( v), _, _, cmd) => throw new NonConstantUnrollBoundsException
    case Blk( decl_lst, seq) => {
      val sT0 = decl_lst.foldLeft(sT.push){
        case (st, Decl( Variable(v), UIntType(w))) => {
          st.insert( v, WireInit( UInt(w.W), init=DontCare))
        }
        case (st, Decl( Variable(v), VecType( n, UIntType(w)))) => {
          st.insert( v, WireInit( Vec(n,UInt(w.W)), init=DontCare))
        }
      }
      seq.foldLeft(sT0){ eval}.pop
    }
    case Assignment( Variable( s), r) => sT.updated( s, eval( sT, r))

    case Assignment( VectorIndex( s, ConstantInteger( i)), r) => {
      sT( s) match {
        case v : Vec[UInt] => {
          val whole : Vec[UInt] = WireInit(init=v)
          val part : UInt = eval( sT, r).asInstanceOf[UInt]
          whole(i) := part
          sT.updated( s, whole)
        }
      }
    }

/*
    case Assignment( VectorIndex( s, ConstantInteger( i)), r) => {
      sT( s) match {
        case v : Vec[UInt] => {
          val whole : Vec[UInt] = v
          val part : UInt = Wire( init=eval( sT, r).asInstanceOf[UInt])
          val w : IndexedSeq[UInt] = whole.updated( i, part)
// Vec still makes copies of everything
          val vv = Vec(w)
          println( s"YYYvector assignment: ${r} ${v} ${vv} ${part}")
          sT.updated( s, vv)
        }
      }
    }
 */

    case Assignment( VectorIndex( s, i), r) => {
      val index = eval( sT, i).asInstanceOf[UInt]
      if ( index.isLit()) {
        println( s"Vector assignment: convert to constant index ${s}(${index.litValue()})")
        eval( sT, Assignment( VectorIndex( s, ConstantInteger( index.litValue().toInt)), r))
      } else {
        sT( s) match {
          case v : Vec[UInt] => {
            val whole : Vec[UInt] = WireInit(init=v)
            val part : UInt = eval( sT, r).asInstanceOf[UInt]
            whole(index) := part
            sT.updated( s, whole)
          }
        }
      }
    }
    case Assignment( _, _) => throw new ImproperLeftHandSideException
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

      def mx[T <: Data]( bb : Bool, p : T, t : T, e : T) : T = {
// using "!=" because I'm comparing whether the Chisel objects (not their values) are different
        if ( p != t || p != e) {
          val w = WireInit( init=e)
          when( bb) { w := t}
          w
        } else p
      }

      val new_sT = (sT /: sT.keys) { case (s,k) =>
        s.updated( k, mx( bb, sT(k), tST(k), eST(k)))
      }

      sT.pkeys.foldLeft(new_sT){ (s,p) => {
        val bbb = evalWithoutPort( Port( p))( sT, b)
// Performing the fix in a FIRRTL transform
//        val bbb = bb

        val (pr,pv,pd) = sT.pget( p) // previous
        val (tr,tv,td) = tST.pget( p)
        val (er,ev,ed) = eST.pget( p)
        s.pupdated( p, mx( bbb, pr, tr, er), mx( bbb, pv, tv, ev), mx( bbb, pd, td, ed))
      }}
    }
    case _ => throw new LoweredFormException
  }

  val sT = decl_lst.foldLeft(new SymTbl) {
    (s,pd) => pd match {
      case PortDecl( Port(p), Inp, UIntType(w)) => {
        val pp = io(p)
        val (r,v,d) = (pp.ready,pp.valid,pp.bits)
        s.pupdated( p, false.B, v, d.asInstanceOf[UInt])
      }
      case PortDecl( Port(p), Out, UIntType(w)) => {
        val pp = io(p)
        val (r,v,d) = (pp.ready,pp.valid,pp.bits)
        s.pupdated( p, r, false.B, WireInit(d.asInstanceOf[UInt].cloneType,init=DontCare))
      }
      case PortDecl( Port(p), Inp, VecType(n,UIntType(w))) => {
        val pp = io(p)
        val (r,v,d) = (pp.ready,pp.valid,pp.bits)
        s.pupdated( p, false.B, v, d.asInstanceOf[Vec[UInt]])
      }
      case PortDecl( Port(p), Out, VecType(n,UIntType(w))) => {
        val pp = io(p)
        val (r,v,d) = (pp.ready,pp.valid,pp.bits)
        s.pupdated( p, r, false.B, WireInit(d.asInstanceOf[Vec[UInt]].cloneType,init=DontCare))
      }
    }
  }

  val sTLast = eval( sT, ast)

  decl_lst.foreach {
    case PortDecl( Port(p), Inp, _) => {
      val (r,v,d) = sTLast.pget( p)
      io(p).ready := r
    }
    case PortDecl( Port(p), Out, _) => {
      val (r,v,d) = sTLast.pget( p)
      io( p).valid := v
      io( p).bits := d
    }
  }

}
