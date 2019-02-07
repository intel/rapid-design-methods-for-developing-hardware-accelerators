package trws

import chisel3._
import chisel3.util._
import chisel3.iotesters._
import org.scalatest.{Matchers, FlatSpec}

class TSRTest extends FlatSpec with Matchers {
   "TSR" should "not have expect violations" in {
      val n = 2
      chisel3.iotesters.Driver( () => new Module {
         val io = IO(new Bundle {
	   val inp = Input( Bool())
	   val out = Output( Vec( n, Bool()))
	 })
         io.out := TappedShiftRegister( n, io.inp, false.B, true.B)
      }) ( (c) => new PeekPokeTester(c) {
       println( s"${peek( c.io.out(0))} ${peek( c.io.out(1))}")
       poke( c.io.inp, 0)
       step(1)
       println( s"${peek( c.io.out(0))} ${peek( c.io.out(1))}")
       poke( c.io.inp, 1)
       step(1)
       println( s"${peek( c.io.out(0))} ${peek( c.io.out(1))}")
       step(1)
       println( s"${peek( c.io.out(0))} ${peek( c.io.out(1))}")
       step(1)
       println( s"${peek( c.io.out(0))} ${peek( c.io.out(1))}")
       step(1)
       println( s"${peek( c.io.out(0))} ${peek( c.io.out(1))}")
       step(1)
      })
   }
}
