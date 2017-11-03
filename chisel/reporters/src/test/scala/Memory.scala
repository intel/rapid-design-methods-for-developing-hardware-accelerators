// See LICENSE for license details.
package memory

import chisel3._
import chisel3.util._
import chisel3.iotesters._
import org.scalatest.{Matchers, FlatSpec}

class Comb extends Module {
  val io = IO(new Bundle {
    val raddr = Input(UInt(4.W))
    val rdata = Output(UInt(32.W))
    val we = Input(Bool())
    val waddr = Input(UInt(4.W))
    val wdata = Input(UInt(32.W))
  })

  val m = Mem( 16, UInt(32.W))

  when( io.we) {
    m.write(io.waddr, io.wdata)
  }

  io.rdata := m.read( io.raddr)

  annotate(core.ChiselAnnotation(this, classOf[reporters.InlineAndReportArea], ""))
  annotate(core.ChiselAnnotation(this, classOf[reporters.InlineAndReportTimingFull], ""))


}

class Sync extends Module {
  val io = IO(new Bundle {
    val raddr = Input(UInt(4.W))
    val rdata = Output(UInt(32.W))
    val we = Input(Bool())
    val waddr = Input(UInt(4.W))
    val wdata = Input(UInt(32.W))
  })

  val m = SyncReadMem( 16, UInt(32.W))

  when( io.we) {
    m.write(io.waddr, io.wdata)
  }

  io.rdata := m.read( io.raddr)

  annotate(core.ChiselAnnotation(this, classOf[reporters.InlineAndReportArea], ""))
  annotate(core.ChiselAnnotation(this, classOf[reporters.InlineAndReportTimingFull], ""))


}

class CombTest extends FlatSpec with Matchers {
  it should "work" in {
    chisel3.iotesters.Driver( () => new Comb, "firrtl") { c =>
      new PeekPokeTester(c) {
        poke( c.io.we, 1)
        poke( c.io.waddr, 7)
        poke( c.io.wdata, 47)
        step( 1)

        poke( c.io.raddr, 7)
        expect( c.io.rdata, 47)

        poke( c.io.we, 1)
        poke( c.io.waddr, 8)
        poke( c.io.wdata, 48)
        step( 1)

        poke( c.io.raddr, 8)
        expect( c.io.rdata, 48)

        poke( c.io.we, 0)
        step( 1)

      }
    } should be (true)
  }
}

class SyncTest extends FlatSpec with Matchers {
  it should "work" in {
    chisel3.iotesters.Driver( () => new Sync, "firrtl") { c =>
      new PeekPokeTester(c) {
        poke( c.io.we, 1)
        poke( c.io.waddr, 7)
        poke( c.io.wdata, 47)
        step( 1)

        poke( c.io.raddr, 7)

        poke( c.io.we, 1)
        poke( c.io.waddr, 8)
        poke( c.io.wdata, 48)
        step( 1)

        expect( c.io.rdata, 47)
        poke( c.io.raddr, 8)
        expect( c.io.rdata, 47)

        poke( c.io.we, 0)

        step( 1)
        expect( c.io.rdata, 48)
      }
    } should be (true)
  }
}

