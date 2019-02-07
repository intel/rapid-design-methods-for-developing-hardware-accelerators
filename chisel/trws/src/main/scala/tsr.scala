package trws

import chisel3._
import chisel3.util._

class TappedShiftRegister2[T <: Data]( n : Int, proto : T, initialValue : T) {
  val inp = WireInit( proto.cloneType, init=DontCare)
  val regs = IndexedSeq.fill( n){ RegInit( inp.cloneType, init=initialValue)}

  val inps = List( inp) ++ (regs.init.toList)
  val outvec = VecInit( List(inp) ++ regs)

  def shift( other : T) {
    inp := other
    (inps,regs).zipped.foreach{ case (i,o) => o := i}
  }
//  def :=( other : T) { inp := other}
  def apply() : Vec[T] = outvec
  def apply( idx : Int) : T = outvec(idx)
}


object TappedShiftRegister {
  def apply[T <: Data]( n : Int, inp : T, initialValue : T, en : Bool) : Vec[T] = {
     val outs = IndexedSeq.fill( n){ RegInit( inp.cloneType, init=initialValue)}
     val inps = List( inp) ++ (outs.init.toList)
     when (en) {
       (inps,outs).zipped.foreach{ case (i,o) => o := i}
     }
     VecInit( List( inp) ++ outs)
  }
}
