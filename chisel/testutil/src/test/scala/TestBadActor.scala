// See LICENSE for license details.
package testutil

import chisel3._
import chisel3.util._
import chisel3.iotesters._
import org.scalatest.{Matchers, FlatSpec}

class BadActor extends Module {
  val io = IO( new Bundle {
    val inp = Flipped(DecoupledIO(UInt(16.W)))
    val out = DecoupledIO(UInt(16.W))
  })

  private val (toggleVal, toggleWrap) = Counter(true.B, 2)

  def doNothing() = {
    io.inp.nodeq()
    io.out.noenq() 
  }

  when (reset.toBool || ShiftRegister(reset.toBool, 2, true.B)) {
    doNothing()
  } .otherwise {
    when (io.inp.valid & io.out.ready && toggleWrap) {  
      io.out.enq(io.inp.deq())
   } .otherwise {
      doNothing()   
   }
  }
}

class BadActorTester( dut: BadActor) extends DecoupledStreamingTestsUsingAdvTester(dut) {

  val stream = List( BigInt(0), BigInt(1), BigInt(2))

  val inpStreams = List( (dut.io.inp, stream))
  val outStreams = List( (dut.io.out, stream))

  testStreams( dut, inpStreams, outStreams, 0, 100)
}

class BadActorTestVerilator extends ChiselFlatSpec {
  "BadActorTest" should "compile and run without incident" in {
    chisel3.iotesters.Driver( () => new BadActor, "verilator") { c =>
      new BadActorTester(c)
    } should be(true)
  }
}

/*
class BadActorTestVCS extends ChiselFlatSpec {
  "BadActorTest" should "compile and run without incident" in {
    chisel3.iotesters.Driver( () => new BadActor, "vcs") { c =>
      new BadActorTester(c)
    } should be(true)
  }
}
 */

class BadActorTestFirrtl extends ChiselFlatSpec {
  "BadActorTest" should "compile and run without incident" in {
    chisel3.iotesters.Driver( () => new BadActor, "firrtl") { c =>
      new BadActorTester(c)
    } should be(true)
  }
}
