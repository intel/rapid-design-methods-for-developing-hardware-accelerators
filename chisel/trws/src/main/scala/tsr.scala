package trws

import chisel3._
import chisel3.util._

object TappedShiftRegister {
  def apply[T <: Bool]( n : Int, inp : T, initialValue : T, en : Bool) : Vec[T] = {
     val outs = IndexedSeq.fill( n){ RegInit( inp.cloneType, init=initialValue)}
     val inps = List( inp) ++ (outs.init.toList)
     when (en) {
       (inps,outs).zipped.foreach{ case (i,o) => o := i}
     }
     VecInit( outs)
  }
}
