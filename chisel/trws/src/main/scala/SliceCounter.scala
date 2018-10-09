package trws

import chisel3._
import chisel3.util._

class SliceCounter {
  val jj = RegInit( 0.U(log2_max_cl_per_row.W))
  val s  = RegInit( 1.U((log2_max_cl_per_row+log2_elements_per_cl).W))
  val t  = RegInit( 0.U((log2_max_cl_per_row+log2_elements_per_cl).W))
  val doneWire = WireInit( false.B)
  val done = RegInit( false.B)
  val first_s = RegInit( true.B)

  def lastMessage =
    (jj === (cl_per_row-1).U) &&
    ( s === (n-1).U) &&
    ( t === (n-2).U)

  val n = cl_per_row<<log2_elements_per_cl

  def incr = {
    when ( jj =/= (cl_per_row-1).U) {
      jj := jj + 1.U
    } .otherwise {
      jj := 0.U
      when ( s =/= (n-1).U) {
        s := s + 1.U
        first_s := false.B
      } .otherwise {
        first_s := true.B
        when ( t =/= (n-2).U) {
          t := t + 1.U
          s := t + 2.U
        } .otherwise {
          t := 0.U
          s := 1.U
          done := true.B
          doneWire := true.B
        }
      }
    }
  }

}

