package MemArbiter

import chisel3._
import chisel3.iotesters._
import chisel3.util._

class ArbiterRegedTest  extends OrderedDecoupledHWIOTester {
    val WIDTH = 32
    val device_under_test = Module(new ArbiterReged(UInt(width=WIDTH), 3))

    inputEvent(device_under_test.io.qin(0).bits -> 1)
    inputEvent(device_under_test.io.qin(2).bits -> 2) 
    inputEvent(device_under_test.io.qin(1).bits -> 3)
    inputEvent(device_under_test.io.qin(0).bits -> 4)
    inputEvent(device_under_test.io.qin(1).bits -> 5)
    inputEvent(device_under_test.io.qin(2).bits -> 6)
    inputEvent(device_under_test.io.qin(1).bits -> 7)

    outputEvent(device_under_test.io.qout.bits -> 1)
    outputEvent(device_under_test.io.qout.bits -> 2)
    outputEvent(device_under_test.io.qout.bits -> 3)
    outputEvent(device_under_test.io.qout.bits -> 4)
    outputEvent(device_under_test.io.qout.bits -> 5)
    outputEvent(device_under_test.io.qout.bits -> 6)
    outputEvent(device_under_test.io.qout.bits -> 7)
}

class ArbiterRegedTester extends ChiselFlatSpec {
  "ArbiterRegedTest" should "compile and run without incident" in {
    assertTesterPasses { new ArbiterRegedTest }
  }
}


class SteerRegedTest2AU  extends OrderedDecoupledHWIOTester {
    val WIDTH = 8
    val device_under_test = Module(new SteerReged(UInt(width=WIDTH), 2, WIDTH-1))
    
    inputEvent(device_under_test.io.qin.bits -> ((0<<(WIDTH-1)) + 1))
    inputEvent(device_under_test.io.qin.bits -> ((1<<(WIDTH-1)) + 2))
    inputEvent(device_under_test.io.qin.bits -> ((1<<(WIDTH-1)) + 3))
    outputEvent(device_under_test.io.qout(0).bits -> ((0<<(WIDTH-1)) + 1))
    outputEvent(device_under_test.io.qout(1).bits -> ((1<<(WIDTH-1)) + 2))
    outputEvent(device_under_test.io.qout(1).bits -> ((1<<(WIDTH-1)) + 3))
}

class SteerRegedTest4AU  extends OrderedDecoupledHWIOTester {
    val WIDTH = 32
    val AUS = 4
    val device_under_test = Module(new SteerReged(UInt(width=WIDTH), AUS, WIDTH-log2Up(AUS)))
    
    inputEvent(device_under_test.io.qin.bits -> ((BigInt(0)<<(WIDTH-log2Up(AUS))) + 1))
    inputEvent(device_under_test.io.qin.bits -> ((BigInt(1)<<(WIDTH-log2Up(AUS))) + 2))
    inputEvent(device_under_test.io.qin.bits -> ((BigInt(3)<<(WIDTH-log2Up(AUS))) + 3))
    outputEvent(device_under_test.io.qout(0).bits -> ((BigInt(0)<<(WIDTH-log2Up(AUS))) + 1))
    outputEvent(device_under_test.io.qout(1).bits -> ((BigInt(1)<<(WIDTH-log2Up(AUS))) + 2))
    outputEvent(device_under_test.io.qout(3).bits -> ((BigInt(3)<<(WIDTH-log2Up(AUS))) + 3))
    // make bit right next to AUID be 1 as corner case
    inputEvent(device_under_test.io.qin.bits -> ((BigInt(0)<<(WIDTH-log2Up(AUS))) + (BigInt(1)<<((WIDTH-log2Up(AUS))-1)) + 1))
    inputEvent(device_under_test.io.qin.bits -> ((BigInt(1)<<(WIDTH-log2Up(AUS))) + (BigInt(1)<<((WIDTH-log2Up(AUS))-1)) + 2))
    inputEvent(device_under_test.io.qin.bits -> ((BigInt(3)<<(WIDTH-log2Up(AUS))) + (BigInt(1)<<((WIDTH-log2Up(AUS))-1)) + 3))
    outputEvent(device_under_test.io.qout(0).bits -> ((BigInt(0)<<(WIDTH-log2Up(AUS))) + (BigInt(1)<<((WIDTH-log2Up(AUS))-1)) + 1))
    outputEvent(device_under_test.io.qout(1).bits -> ((BigInt(1)<<(WIDTH-log2Up(AUS))) + (BigInt(1)<<((WIDTH-log2Up(AUS))-1)) + 2))
    outputEvent(device_under_test.io.qout(3).bits -> ((BigInt(3)<<(WIDTH-log2Up(AUS))) + (BigInt(1)<<((WIDTH-log2Up(AUS))-1)) + 3))
}

class SteerRegedSplitTest2AU  extends OrderedDecoupledHWIOTester {
    val WIDTH = 63
    val device_under_test = Module(new SteerRegedSplit(UInt(width=WIDTH), 2, WIDTH-1, 4))
    
    inputEvent(device_under_test.io.qin.bits -> ((BigInt(0)<<(WIDTH-1)) + 1))
    inputEvent(device_under_test.io.qin.bits -> ((BigInt(1)<<(WIDTH-1)) + 2))
    inputEvent(device_under_test.io.qin.bits -> ((BigInt(1)<<(WIDTH-1)) + 3))
    outputEvent(device_under_test.io.qout(0).bits -> ((BigInt(0)<<(WIDTH-1)) + 1))
    outputEvent(device_under_test.io.qout(1).bits -> ((BigInt(1)<<(WIDTH-1)) + 2))
    outputEvent(device_under_test.io.qout(1).bits -> ((BigInt(1)<<(WIDTH-1)) + 3))
}

class SteerRegedSplitTest3AU  extends OrderedDecoupledHWIOTester {
    val WIDTH = 32
    val device_under_test = Module(new SteerRegedSplit(UInt(width=WIDTH), 3, WIDTH-2, 4))
    
    inputEvent(device_under_test.io.qin.bits -> ((BigInt(0)<<(WIDTH-2)) + 1))
    inputEvent(device_under_test.io.qin.bits -> ((BigInt(1)<<(WIDTH-2)) + 2))
    inputEvent(device_under_test.io.qin.bits -> ((BigInt(2)<<(WIDTH-2)) + 3))
    inputEvent(device_under_test.io.qin.bits -> ((BigInt(2)<<(WIDTH-2)) + 4))
    inputEvent(device_under_test.io.qin.bits -> ((BigInt(1)<<(WIDTH-2)) + 5))
    outputEvent(device_under_test.io.qout(0).bits -> ((BigInt(0)<<(WIDTH-2)) + 1))
    outputEvent(device_under_test.io.qout(1).bits -> ((BigInt(1)<<(WIDTH-2)) + 2))
    outputEvent(device_under_test.io.qout(2).bits -> ((BigInt(2)<<(WIDTH-2)) + 3))
    outputEvent(device_under_test.io.qout(2).bits -> ((BigInt(2)<<(WIDTH-2)) + 4))
    outputEvent(device_under_test.io.qout(1).bits -> ((BigInt(1)<<(WIDTH-2)) + 5))
}


class SteerRegedTester extends ChiselFlatSpec {
  "SteerRegedTest2AU" should "compile and run without incident" in {
    assertTesterPasses { new SteerRegedTest2AU }
  }
/*  "SteerRegedTest4AU" should "compile and run without incident" in {
    assertTesterPasses { new SteerRegedTest4AU }
  }
  "SteerRegedSplitTest2AU" should "compile and run without incident" in {
    assertTesterPasses { new SteerRegedSplitTest2AU }
  }
  "SteerRegedSplitTest3AU" should "compile and run without incident" in {
    assertTesterPasses { new SteerRegedSplitTest3AU }
  }
*/  
}
