package imperative

import org.scalatest.{ Matchers, FlatSpec, FreeSpec}

import chisel3._
import chisel3.util._
import chisel3.iotesters._

import compiler._

/*
class Split extends ImperativeModule( 
  Compiler.runHLS3(
    """
      |process Split( P : inp UInt(64), Q0 : out UInt(64), Q1 : out UInt(64)) {
      |  var x : UInt(64)
      |  P??x
      |  wait
      |  while ( true) {
      |    Q0!!x
      |    P??x
      |    wait
      |    Q1!!x
      |    P??x
      |    wait
      |  }
      |}
    """.stripMargin.trim))
 */
// this currently adds an extra wait that the end
class Split extends ImperativeModule( 
  Compiler.runHLS3(
    """
      |process Split( P : inp UInt(64), Q0 : out UInt(64), Q1 : out UInt(64)) {
      |  var x : UInt(64)
      |  while ( true) {
      |    P??x
      |    wait
      |    Q0!!x
      |    P??x
      |    wait
      |    Q1!!x
      |  }
      |}
    """.stripMargin.trim))

class Split1 extends ImperativeModule( 
  Compiler.run(
    """
    process Split( P : inp UInt(64), Q0 : out UInt(64), Q1 : out UInt(64)) {
      var x : UInt(64)
      var s : UInt(3)
      s = 0
      while ( true) {
        if (s==3) {
          if (Q1!)
            {
              Q1!x
              if (P?)
                {
                  P?x
                  s = 1
                }
              else s = 0
            }
        } else if (s==0) {
          if (P?)
            {
              P?x
              s = 1
            }
        } else if (s==1) {
          if (Q0!)
            {
              Q0!x
              if (P?)
                {
                  P?x
                  s = 3
                }
              else s = 2
            }
        } else if (s==2) {
          if (P?)
            {
              P?x
              s = 3
            }
        }
        wait
      }
  }
"""))




/* // would like to write it like this (all comms blocking); waits between P and Q make it buffered
    """
      |process Split( P : inp UInt(64), Q0 : out UInt(64), Q1 : out UInt(64)) {
      |  var x : UInt(64)
      |  while ( true) {
      |    P?x
      |    wait
      |    Q0!x
      |    P?x
      |    wait
      |    Q1!x
      |  }
      |}
    """.stripMargin.trim
 */
/*
class Split extends ImperativeModule( 
  Compiler.run(
    """
      |process Split( P : inp UInt(64), Q0 : out UInt(64), Q1 : out UInt(64)) {
      |  var s : UInt(4)
      |  var x : UInt(64)
      |  while ( true) {
      |    var w : UInt(1)
      |    w = 0
      |    if ( s == 0 && w == 0) {
      |      if ( P?) {
      |        P?x
      |        s = 1
      |      } 
      |      w = 1
      |    }
      |    if ( s == 1 && w == 0) {
      |      if ( Q0!) {
      |        Q0!x
      |        s = 2
      |      } else w = 1
      |    }
      |    if ( s == 2 && w == 0) {
      |      if ( P?) {
      |        P?x
      |        s = 3
      |      }
      |      w = 1
      |    }
      |    if ( s == 3 && w == 0) {
      |      if ( Q1!) {
      |        Q1!x
      |        s = 4
      |      } else w = 1
      |    }
      |    if ( s == 4 && w == 0) {
      |      if ( P?) {
      |        P?x
      |        s = 1
      |      }
      |      w = 1
      |    }
      |    wait
      |  }
      |}
    """.stripMargin.trim))
 */

/*
class Split extends ImperativeModule( 
  Compiler.run(
    """
      |process Split( P : inp UInt(64), Q0 : out UInt(64), Q1 : out UInt(64)) {
      |  var x : UInt(64)
      |  var s : UInt(1)
      |  var f : UInt(1)
      |  s = 1
      |  f = 0
      |  while ( true) {
      |    if ( f == 1) {
      |      if ( s == 0) {
      |        if ( Q0!) {
      |          Q0!x
      |          f = 0
      |        }
      |      } else {
      |        if ( Q1!) {
      |          Q1!x
      |          f = 0
      |        }
      |      }
      |    }
      |    if ( f == 0 && P?) {
      |      P?x
      |      f = 1
      |      if ( s == 1) {
      |        s = 0
      |      } else {
      |        s = 1
      |      }
      |    }
      |    wait
      |  }
      |}
    """.stripMargin.trim))
 */


/*
class Merge extends ImperativeModule( 
  Compiler.run(
    """
      |process Merge( P0 : inp UInt(64), P1 : inp UInt(64), Q : out UInt(64)) {
      |  var x : UInt(64)
      |  var s : UInt(1)
      |  var f : UInt(1)
      |  s = 0
      |  f = 0
      |  while ( true) {
      |    if ( f == 1) {
      |      if ( Q!) {
      |        Q!x
      |        f = 0
      |      }
      |    }
      |    if ( f == 0) {
      |      if ( s == 0) {
      |        if ( P0?) {
      |          P0?x
      |          f = 1
      |          s = 1
      |        }
      |      } else {
      |        if ( P1?) {
      |          P1?x
      |          f = 1
      |          s = 0
      |        }
      |      }
      |    }
      |    wait
      |  }
      |}
    """.stripMargin.trim))
 */

class MergeFalseCombLoop extends ImperativeModule( 
  Compiler.runHLS3(
    """
      |process Merge( P0 : inp UInt(64), P1 : inp UInt(64), Q : out UInt(64)) {
      |  var x : UInt(64)
      |  while ( !P0?) wait
      |  P0?x
      |  wait
      |  while ( true) {
      |    while ( !Q!) wait
      |    Q!x
      |    while ( !P1?) wait
      |    P1?x
      |    wait
      |    while ( !Q!) wait
      |    Q!x
      |    while ( !P0?) wait
      |    P0?x
      |    wait
      |  }
      |}
    """.stripMargin.trim))

class Merge extends ImperativeModule( 
  Compiler.runHLS3(
    """
      |process Merge( P0 : inp UInt(64), P1 : inp UInt(64), Q : out UInt(64)) {
      |  var x : UInt(64)
      |  while ( true) {
      |    P0??x
      |    wait
      |    Q!!x
      |    P1??x
      |    wait
      |    Q!!x
      |  }
      |}
    """.stripMargin.trim))

/* Last block moved to beginning to avoid the stall */
class Merge1 extends ImperativeModule( 
  Compiler.run(
    """process Merge( P0 : inp UInt(64), P1 : inp UInt(64), Q : out UInt(64)) {
  var x : UInt(64)
  var s : UInt(3)
  s = 0
  while (true) {
    var w : UInt(1)
    w = 0
    if (s==3 && w==0) {
      if (Q!) {
        Q!x
        s = 0
      } else {
        w = 1
      }
    }
    if (s==0 && w==0) {
      if (P0?) {
        P0?x
        s = 1
      }
      w = 1
    }
    if (s==1 && w==0) {
       if (Q!) {
         Q!x
         s = 2
       } else {
         w = 1
       }
    }
    if (s==2 && w==0) {
      if (P1?) {
        P1?x
        s = 3
      }
      w = 1
    }
    wait
  }
}
"""))

/*
class Merge extends ImperativeModule( 
  Compiler.run(
    """
      |process Merge( P0 : inp UInt(64), P1 : inp UInt(64), Q : out UInt(64)) {
      |  var x : UInt(64)
      |  var f : UInt(1)
      |  f = 0
      |  while ( true) {
      |    if ( f == 0 && P0? && Q!) {
      |      P0?x
      |      Q!x
      |      f = 1
      |    } else if ( f == 1 && P1? && Q!) {
      |      P1?x
      |      Q!x
      |      f = 0
      |    }
      |    wait
      |  }
      |}
    """.stripMargin.trim))
 */


/*
class TreeFifoTester(c:TreeFifo) extends PeekPokeTester(c) {
  poke( c.io.out.ready, 1)
  poke( c.io.inp.valid, 0)

//Mealy
  expect( c.io.out.valid, 0)
//  expect( c.io.inp.ready, 1) /* Don't care */

  step(1)

  poke( c.io.out.ready, 1)
  poke( c.io.inp.valid, 1)
  poke( c.io.inp.bits.asInstanceOf[UInt], 4747)

//Mealy
  expect( c.io.out.valid, 0)
  expect( c.io.inp.ready, 1)

  step(1)

  poke( c.io.out.ready, 0)
  poke( c.io.inp.valid, 0)

//Mealy
//  expect( c.io.out.valid, 1) /* Don't care */
//  expect( c.io.inp.ready, 1) /* Don't care */

  step(1)

  poke( c.io.out.ready, 1)
  poke( c.io.inp.valid, 1)
  poke( c.io.inp.bits.asInstanceOf[UInt], 5454)

//Mealy
  expect( c.io.inp.ready, 1)
  expect( c.io.out.valid, 1)
  expect( c.io.out.bits.asInstanceOf[UInt], 4747)

  step(1)

  poke( c.io.out.ready, 1)
  poke( c.io.inp.valid, 0)

//Mealy
  expect( c.io.out.valid, 1)
  expect( c.io.out.bits.asInstanceOf[UInt], 5454)
//  expect( c.io.inp.ready, 1) /* Don't care */

}

class TreeFifoTest extends FlatSpec with Matchers {
  behavior of "TreeFifo"
  it should "work" in {
    chisel3.iotesters.Driver( () => new TreeFifo, "firrtl") { c =>
      new TreeFifoTester( c)
    } should be ( true)
  }
}
 */


class MergeTester[T <: ImperativeIfc](c: T) extends PeekPokeTester(c) {

  poke( c.io("Q").ready, 1)
  poke( c.io("P0").valid, 0)
  poke( c.io("P1").valid, 0)

//Mealy
  expect( c.io("Q").valid, 0)
//  expect( c.io("P0").ready, 1) // Don't care
//  expect( c.io("P1").ready, 0) // Don't care

  step(1)

  poke( c.io("P0").valid, 1)
  poke( c.io("P1").valid, 1)
  poke( c.io("P0").bits.asInstanceOf[UInt], 47)
  poke( c.io("P1").bits.asInstanceOf[UInt], 48)
  poke( c.io("Q").ready, 1)

//Mealy
  expect( c.io("P0").ready, 1)
  expect( c.io("P1").ready, 0)

  poke( c.io("P0").valid, 0)
//  expect( c.io("P0").ready, 1)
  expect( c.io("P1").ready, 0)

  poke( c.io("P0").valid, 1)
  expect( c.io("P0").ready, 1)
  expect( c.io("P1").ready, 0)

  poke( c.io("P1").valid, 0)
//  expect( c.io("P0").ready, 1)
  expect( c.io("P1").ready, 0)

  poke( c.io("P1").valid, 1)
  expect( c.io("P0").ready, 1)
  expect( c.io("P1").ready, 0)

  step(1)

  expect( c.io("Q").valid, 1)
  expect( c.io("Q").bits.asInstanceOf[UInt], 47)

  expect( c.io("P0").ready, 0)
  expect( c.io("P1").ready, 1)

  poke( c.io("P0").valid, 0)
//  expect( c.io("P0").ready, 0)
  expect( c.io("P1").ready, 1)

  poke( c.io("P0").valid, 1)
  expect( c.io("P0").ready, 0)
  expect( c.io("P1").ready, 1)

  poke( c.io("P1").valid, 0)
  expect( c.io("P0").ready, 0)
//  expect( c.io("P1").ready, 1)

  poke( c.io("P1").valid, 1)
  expect( c.io("P0").ready, 0)
  expect( c.io("P1").ready, 1)

  poke( c.io("P0").valid, 0)
//  expect( c.io("P0").ready, 0)
  expect( c.io("P1").ready, 1)

  step(1)

  expect( c.io("Q").valid, 1)
  expect( c.io("Q").bits.asInstanceOf[UInt], 48)

  step(1)

  expect( c.io("Q").valid, 0)


}

class StandaloneMergeP0 extends Module {
  val io = IO( new Bundle{})
  val m = Module( new MergeFalseCombLoop)
  m.io("P0").valid := m.io("P0").ready
}

class StandaloneMergeP1 extends Module {
  val io = IO( new Bundle{})
  val m = Module( new MergeFalseCombLoop)
  m.io("P1").valid := m.io("P1").ready
}

class StandaloneMergeQ extends Module {
  val io = IO( new Bundle{})
  val m = Module( new MergeFalseCombLoop)
  m.io("Q").ready := m.io("Q").valid
}

class StandaloneMergeP0Tester(c:StandaloneMergeP0) extends PeekPokeTester( c)
class StandaloneMergeP1Tester(c:StandaloneMergeP1) extends PeekPokeTester( c)
class StandaloneMergeQTester(c:StandaloneMergeQ) extends PeekPokeTester( c)

class MergeFalseCombLoopTest extends FlatSpec with Matchers {
  behavior of "TreeFifo"
  it should "work" in {
    chisel3.iotesters.Driver( () => new MergeFalseCombLoop, "firrtl") { c =>
      new MergeTester( c)
    } should be ( true)
  }
}

class MergeTest extends FlatSpec with Matchers {
  behavior of "TreeFifo"
  it should "work" in {
    chisel3.iotesters.Driver( () => new Merge, "firrtl") { c =>
      new MergeTester( c)
    } should be ( true)
  }
}

class StandaloneMergeP0Test extends FreeSpec with Matchers {
  "Merge with combinational path (false)" - {
    "StandaloneMergeP0 (wire connecting P0.ready to P0.valid) should fail" in {
      a [firrtl.graph.DiGraph$PathNotFoundException] should be thrownBy {
        chisel3.iotesters.Driver( () => new StandaloneMergeP0, "firrtl") { c =>
          new StandaloneMergeP0Tester( c)
        }
      }
    }
  }
}
class StandaloneMergeP1Test extends FreeSpec with Matchers {
  "Merge with combinational path (false)" - {
    "StandaloneMergeP1 (wire connecting P1.ready to P1.valid) should fail" in {
        chisel3.iotesters.Driver( () => new StandaloneMergeP1, "firrtl") { c =>
          new StandaloneMergeP1Tester( c)
        }
    }
  }
}
class StandaloneMergeQTest extends FreeSpec with Matchers {
  "Merge with combinational path (false)" - {
    "StandaloneMergeQ (wire connecting Q.valid to Q.ready) should fail" in {
      a [firrtl.graph.DiGraph$PathNotFoundException] should be thrownBy {
        chisel3.iotesters.Driver( () => new StandaloneMergeQ, "firrtl") { c =>
          new StandaloneMergeQTester( c)
        }
      }
    }
  }
}

class StandaloneSplitP extends Module {
  val io = IO( new Bundle { val dummy = Input(Bool())})
  val m = Module( new Split)
  m.io("P").valid := m.io("P").ready
}

class StandaloneSplitQ0 extends Module {
  val io = IO( new Bundle{})
  val m = Module( new Split)
  m.io("Q0").ready := m.io("Q0").valid
}

class StandaloneSplitQ1 extends Module {
  val io = IO( new Bundle{})
  val m = Module( new Split)
  m.io("Q1").ready := m.io("Q1").valid
}

class StandaloneSplitPTester(c:StandaloneSplitP) extends PeekPokeTester( c)
class StandaloneSplitQ0Tester(c:StandaloneSplitQ0) extends PeekPokeTester( c)
class StandaloneSplitQ1Tester(c:StandaloneSplitQ1) extends PeekPokeTester( c)

class StandaloneSplitPTest extends FreeSpec with Matchers {
  "Split input has a (false) combinational path" - {
    "StandaloneSplitP (wire connecting P.ready to P.valid) should fail" in {
      a [firrtl.graph.DiGraph$PathNotFoundException] should be thrownBy {
        chisel3.iotesters.Driver.execute( Array( "--backend-name", "verilator"), () => new StandaloneSplitP) { c =>
          new StandaloneSplitPTester( c)
        }
      }
    }
  }
}
class StandaloneSplitQ0Test extends FreeSpec with Matchers {
  "Split outputs should not have combinational paths" - {
    "StandaloneSplitQ0 (wire connecting Q0.valid to Q0.ready) should not fail" in {
//      a [firrtl_interpreter.InterpreterException] should be thrownBy {
        chisel3.iotesters.Driver.execute( Array( "--backend-name", "verilator"), () => new StandaloneSplitQ0) { c =>
          new StandaloneSplitQ0Tester( c)
        }
//      }
    }
  }
}
class StandaloneSplitQ1Test extends FreeSpec with Matchers {
  "Split outputs should not have combinational paths" - {
    "StandaloneSplitQ1 (wire connecting Q1.valid to Q1.ready) should not fail" in {
//      a [firrtl_interpreter.InterpreterException] should be thrownBy {
        chisel3.iotesters.Driver.execute( Array( "--backend-name", "verilator"), () => new StandaloneSplitQ1) { c =>
          new StandaloneSplitQ1Tester( c)
        }
//      }
    }
  }
}

