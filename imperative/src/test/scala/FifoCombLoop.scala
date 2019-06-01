package imperative

import org.scalatest.{ Matchers, FlatSpec, FreeSpec}

import chisel3._
import chisel3.util._
import chisel3.iotesters._

import compiler._

class StandaloneSquashP extends Module {
  val io = IO( new Bundle{})
  val m = Module( new Squash)
  m.io("P").valid := m.io("P").ready
}

class StandaloneSquashQ extends Module {
  val io = IO( new Bundle{})
  val m = Module( new Squash)
  m.io("Q").ready := m.io("Q").valid
}

class StandaloneSquashPTester(c:StandaloneSquashP) extends PeekPokeTester( c)
class StandaloneSquashQTester(c:StandaloneSquashQ) extends PeekPokeTester( c)

class StandaloneSquashPTest extends FreeSpec with Matchers {
  "Squash doesn't have a combination path through interface" - {
    "StandaloneSquashP (wire connecting P.ready to P.valid) should not throw" in {
//      a [firrtl_interpreter.InterpreterException] should be thrownBy {
        chisel3.iotesters.Driver( () => new StandaloneSquashP, "firrtl") { c =>
          new StandaloneSquashPTester( c)
        }
//    }
    }
  }
}
class StandaloneSquashQTest extends FreeSpec with Matchers {
  "Squash doesn't have a combinational path through interface" - {
    "StandaloneSquashQ (wire connecting Q.valid to Q.ready) should not throw" in {
//      a [firrtl_interpreter.InterpreterException] should be thrownBy {
        chisel3.iotesters.Driver( () => new StandaloneSquashQ, "firrtl") { c =>
          new StandaloneSquashQTester( c)
        }
//    }
    }
  }
}

