// See LICENSE for license details.
package accio

import chisel3._
import chisel3.iotesters._
import chisel3.util._
import accio.DefaultAccParams._

class ArbiterRegedTest  extends OrderedDecoupledHWIOTester {
    val WIDTH = 32
    val device_under_test = Module(new MemArbiterReged(new MemRdReq, 3, 0))

    inputEvent(device_under_test.io.qin(0).bits.addr -> 1, device_under_test.io.qin(0).bits.tag -> 0)
    inputEvent(device_under_test.io.qin(2).bits.addr -> 2, device_under_test.io.qin(2).bits.tag -> 0) 
    inputEvent(device_under_test.io.qin(1).bits.addr -> 3, device_under_test.io.qin(1).bits.tag -> 0)
    inputEvent(device_under_test.io.qin(0).bits.addr -> 4, device_under_test.io.qin(0).bits.tag -> 0)
    inputEvent(device_under_test.io.qin(1).bits.addr -> 5, device_under_test.io.qin(1).bits.tag -> 0)
    inputEvent(device_under_test.io.qin(2).bits.addr -> 6, device_under_test.io.qin(2).bits.tag -> 0)
    inputEvent(device_under_test.io.qin(1).bits.addr -> 7, device_under_test.io.qin(1).bits.tag -> 0)

    outputEvent(device_under_test.io.qout.bits.addr -> 1, device_under_test.io.qout.bits.tag -> 0)
    outputEvent(device_under_test.io.qout.bits.addr -> 2, device_under_test.io.qout.bits.tag -> 2)
    outputEvent(device_under_test.io.qout.bits.addr -> 3, device_under_test.io.qout.bits.tag -> 1)
    outputEvent(device_under_test.io.qout.bits.addr -> 4, device_under_test.io.qout.bits.tag -> 0)
    outputEvent(device_under_test.io.qout.bits.addr -> 5, device_under_test.io.qout.bits.tag -> 1)
    outputEvent(device_under_test.io.qout.bits.addr -> 6, device_under_test.io.qout.bits.tag -> 2)
    outputEvent(device_under_test.io.qout.bits.addr -> 7, device_under_test.io.qout.bits.tag -> 1)
}

class ArbiterRegedTester extends ChiselFlatSpec {
  "ArbiterRegedTest" should "compile and run without incident" in {
    assertTesterPasses { new ArbiterRegedTest }
  }
}


class SteerRegedTest2AU  extends OrderedDecoupledHWIOTester {
    val WIDTH = 8
    val device_under_test = Module(new MemSteerReged(new MemRdResp, 2, 0))
    
    inputEvent(device_under_test.io.qin.bits.data -> 64, device_under_test.io.qin.bits.tag -> 2)
    inputEvent(device_under_test.io.qin.bits.data -> 32, device_under_test.io.qin.bits.tag -> 3)
    
    inputEvent(device_under_test.io.qin.bits.data -> 15, device_under_test.io.qin.bits.tag -> 5)
    inputEvent(device_under_test.io.qin.bits.data -> 18, device_under_test.io.qin.bits.tag -> 6)

    
    outputEvent(device_under_test.io.qout(0).bits.data -> 64, device_under_test.io.qout(0).bits.tag -> 2)
    outputEvent(device_under_test.io.qout(1).bits.data -> 32, device_under_test.io.qout(1).bits.tag -> 2)

    outputEvent(device_under_test.io.qout(1).bits.data -> 15, device_under_test.io.qout(1).bits.tag -> 4)
    outputEvent(device_under_test.io.qout(0).bits.data -> 18, device_under_test.io.qout(0).bits.tag -> 6)    
}

class SteerRegedTest4AU  extends OrderedDecoupledHWIOTester {
    val WIDTH = 32
    val AUS = 4
    val device_under_test = Module(new MemSteerReged(new MemRdResp, AUS, WIDTH-log2Ceil(AUS)))
    val indata = MemRdResp(123.U, 1.U).asUInt().litValue()
 //   inputEvent(device_under_test.io.qin.bits.data -> ((BigInt(0)<<(WIDTH-log2Ceil(AUS))) + 1))
/*    inputEvent(device_under_test.io.qin.bits -> ((BigInt(1)<<(WIDTH-log2Ceil(AUS))) + 2))
    inputEvent(device_under_test.io.qin.bits -> ((BigInt(3)<<(WIDTH-log2Ceil(AUS))) + 3))
    outputEvent(device_under_test.io.qout(0).bits -> ((BigInt(0)<<(WIDTH-log2Ceil(AUS))) + 1))
    outputEvent(device_under_test.io.qout(1).bits -> ((BigInt(1)<<(WIDTH-log2Ceil(AUS))) + 2))
    outputEvent(device_under_test.io.qout(3).bits -> ((BigInt(3)<<(WIDTH-log2Ceil(AUS))) + 3))
    // make bit right next to AUID be 1 as corner case
    inputEvent(device_under_test.io.qin.bits -> ((BigInt(0)<<(WIDTH-log2Ceil(AUS))) + (BigInt(1)<<((WIDTH-log2Ceil(AUS))-1)) + 1))
    inputEvent(device_under_test.io.qin.bits -> ((BigInt(1)<<(WIDTH-log2Ceil(AUS))) + (BigInt(1)<<((WIDTH-log2Ceil(AUS))-1)) + 2))
    inputEvent(device_under_test.io.qin.bits -> ((BigInt(3)<<(WIDTH-log2Ceil(AUS))) + (BigInt(1)<<((WIDTH-log2Ceil(AUS))-1)) + 3))
    outputEvent(device_under_test.io.qout(0).bits -> ((BigInt(0)<<(WIDTH-log2Ceil(AUS))) + (BigInt(1)<<((WIDTH-log2Ceil(AUS))-1)) + 1))
    outputEvent(device_under_test.io.qout(1).bits -> ((BigInt(1)<<(WIDTH-log2Ceil(AUS))) + (BigInt(1)<<((WIDTH-log2Ceil(AUS))-1)) + 2))
    outputEvent(device_under_test.io.qout(3).bits -> ((BigInt(3)<<(WIDTH-log2Ceil(AUS))) + (BigInt(1)<<((WIDTH-log2Ceil(AUS))-1)) + 3))
*/
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
