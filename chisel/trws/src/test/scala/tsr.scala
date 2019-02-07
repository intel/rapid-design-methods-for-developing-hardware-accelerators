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
	   val out = Output( Vec( n+1, Bool()))
	 })
         io.out := TappedShiftRegister( n, io.inp, false.B, true.B)
      }) ( (c) => new PeekPokeTester(c) {
       poke( c.io.inp, 0)
       step(1)
       poke( c.io.inp, 1)
       expect( c.io.out(0), 1)
       expect( c.io.out(1), 0)
       expect( c.io.out(1), 0)
       step(1)
       poke( c.io.inp, 0)
       expect( c.io.out(0), 0)
       expect( c.io.out(1), 1)
       expect( c.io.out(2), 0)
       step(1)
       poke( c.io.inp, 1)
       expect( c.io.out(0), 1)
       expect( c.io.out(1), 0)
       expect( c.io.out(2), 1)
      })
   }
}
