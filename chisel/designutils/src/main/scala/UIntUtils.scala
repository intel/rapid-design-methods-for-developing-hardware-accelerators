// See LICENSE for license details.
package designutils
import chisel3._
import chisel3.util._


object UIntUtils {

 def replace(in:UInt, at:Int, data : UInt) : UInt = {
    val ary = Wire(Vec(in.getWidth, Bool()))
    for(w <- 0 until in.getWidth)   ary(w)    := in(w)
    for(w <- 0 until data.getWidth) ary(w+at) := data(w)
    Reverse(Cat(ary))
  }
}
 
