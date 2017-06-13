package bug

import org.scalatest.{ Matchers, FlatSpec}

import chisel3._
import chisel3.iotesters._

class VCSBug extends Module {
  val io = IO(new Bundle)
}

class Tester(c:VCSBug) extends PeekPokeTester(c) {
  step(1)
}

class VCSTest extends FlatSpec with Matchers {
  behavior of "VCSBug"
  it should "work" in {
    chisel3.iotesters.Driver( () => new VCSBug, "vcs") { c =>
      new Tester( c)
    } should be ( true)
  }
}

class VerilatorTest extends FlatSpec with Matchers {
  behavior of "VCSBug"
  it should "work" in {
    chisel3.iotesters.Driver( () => new VCSBug, "verilator") { c =>
      new Tester( c)
    } should be ( true)
  }
}

class FirrtlTest extends FlatSpec with Matchers {
  behavior of "VCSBug"
  it should "work" in {
    chisel3.iotesters.Driver( () => new VCSBug, "firrtl") { c =>
      new Tester( c)
    } should be ( true)
  }
}
