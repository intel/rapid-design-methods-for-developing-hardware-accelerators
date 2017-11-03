// See LICENSE for license details.
package arithmetic

import chisel3._
import chisel3.util._
import chisel3.iotesters._
import org.scalatest.{Matchers, FlatSpec}

class Mult( w : Int) extends Module {
  val io = IO(new Bundle {
    val x = Input(UInt(w.W))
    val y = Input(UInt(w.W))
    val z = Output(UInt(w.W))
    val zTimes8 = Output(UInt(w.W))
    val zTimes7 = Output(UInt(w.W))
    val zTimes5 = Output(UInt(w.W))
    val zConst = Output(UInt(w.W))
  })

  io.z := io.x * io.y
  io.zTimes8 := io.x * 8.U // Should be a wire
  io.zTimes7 := io.x * 7.U // Should be 8x - x so the same as an adder
  io.zTimes5 := io.x * 5.U // Should be 4x + x so the same as an adder
  io.zConst := 8.U * 7.U

  annotate(core.ChiselAnnotation(this, classOf[reporters.InlineAndReportArea], ""))
  annotate(core.ChiselAnnotation(this, classOf[reporters.InlineAndReportTimingInterfacePaths], ""))
}


class MultTest extends FlatSpec with Matchers {
  it should "work" in {
    chisel3.iotesters.Driver( () => new Mult(64), "firrtl") { c =>
      new PeekPokeTester(c) {
        poke( c.io.x, 47)
        poke( c.io.y, 48)
        expect( c.io.z, 47*48)
        expect( c.io.zTimes5, 47*5)
        expect( c.io.zTimes7, 47*7)
        expect( c.io.zTimes8, 47*8)
        expect( c.io.zConst, 56)
        step( 1)
      }
    } should be (true)
  }
}

