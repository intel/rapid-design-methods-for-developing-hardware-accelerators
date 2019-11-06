// See LICENSE for license details.
package accio

import chisel3._
import chisel3.util._
import chisel3.iotesters._
import testutil._
import designutils._
import accio.DefaultAccParams._

class TopAccOut extends Module { 
  val AccOut = Module(new AccOut())
  val mockmem = Module(new MockMemory(128, Some(MemInit.init_func)))
  val io = IO(new Bundle {
    val acc_req_in  = Flipped(Decoupled( UInt(AccOut.io.acc_req_in.bits.getWidth.W)) )
    val acc_data_in = Flipped(Decoupled( UInt(AccOut.io.acc_data_in.bits.getWidth.W ) ))
  	val mem_probe = Input((mockmem.clSize-1).U.cloneType)
  	val mem_monitor = Output(UInt(params.CacheLineWidth.W))
  })
  val reqQ = Module( new Queue(mockmem.io.mem_wr_in.bits.cloneType, 2))
  //val reqQ = Module (new DecoupledStage(mockmem.io.mem_wr_in.bits))
  val respQ = Module (new Queue(mockmem.io.mem_wr_out.bits.cloneType, 2))
  
  mockmem.io.mem_rd_out.ready := DontCare
  mockmem.io.mem_rd_in.valid := DontCare
  mockmem.io.mem_rd_in.bits := DontCare

  reqQ.io.enq.bits := DontCare
  reqQ.io.enq.valid := DontCare
  reqQ.io.deq.ready := DontCare
  reqQ.io.deq.ready := DontCare

  //printf("mem req in top reqQ.io.enq (v/r) %d/%d addr - %d data - %x - widths %d\n", reqQ.io.enq.valid,reqQ.io.enq.ready,reqQ.io.enq.bits.addr, reqQ.io.enq.bits.data, reqQ.io.enq.bits.getWidth.U)  
  //printf("mem req in top reqQ.io.deq (v/r) %d/%d addr - %d data - %x - widths %d\n", reqQ.io.deq.valid,reqQ.io.deq.ready,reqQ.io.deq.bits.addr, reqQ.io.deq.bits.data, reqQ.io.deq.bits.getWidth.U)  
  
  io.mem_monitor := mockmem.io.mem_monitor  
  mockmem.io.mem_probe := io.mem_probe   
  
  //reqQ.io.enq <> AccOut.io.mem_out
  //mockmem.io.mem_wr_in <> reqQ.io.deq
  
  mockmem.io.mem_wr_in <> DecoupledStage(AccOut.io.mem_out)
  
  //reqQ.io.inp <> AccOut.io.mem_out
  //mockmem.io.mem_wr_in <> reqQ.io.out
  //mockmem.io.mem_wr_in <> AccOut.io.mem_out  
  
  respQ.io.enq <> mockmem.io.mem_wr_out
  AccOut.io.mem_in <> respQ.io.deq
  //AccOut.io.mem_in <> mockmem.io.mem_wr_out
 
  AccOut.io.acc_req_in.valid <> io.acc_req_in.valid
  AccOut.io.acc_req_in.ready <> io.acc_req_in.ready

  AccOut.io.acc_data_in.valid <> io.acc_data_in.valid
  AccOut.io.acc_data_in.ready <> io.acc_data_in.ready
  
  AccOut.io.acc_req_in.bits :=  io.acc_req_in.bits.asTypeOf(AccOut.io.acc_req_in.bits)
  AccOut.io.acc_data_in.bits := io.acc_data_in.bits.asTypeOf(AccOut.io.acc_data_in.bits)
  
  //printf("acc data in (v/r) %d/%d  data - %x - widths %d\n", io.acc_data_in.valid,io.acc_data_in.ready,io.acc_data_in.bits, io.acc_data_in.bits.getWidth.U)
   
}

class AccOutTestsUsingAdvTester(dut: TopAccOut) extends DecoupledStreamingTestsUsingAdvTester(dut) {
  val reqType = dut.AccOut.io.acc_req_in.bits
  val dataType = dut.AccOut.io.acc_data_in.bits
  val reqWidthList = List(reqType.addr.getWidth,reqType.burstSize.getWidth)
  val dataWidthList = List(dataType.data.getWidth)
  val req_in_stream = List(
                       BigIntGen(reqWidthList, List(1,2)), 
                       BigIntGen(reqWidthList, List(10,3)), 
                       BigIntGen(reqWidthList, List(20,1))
                      )
  val data_in_stream = List(
                       BigIntGen(dataWidthList, List(12)), 
                       BigIntGen(dataWidthList, List(14)), 
                       BigIntGen(dataWidthList, List(23)), 
                       BigIntGen(dataWidthList, List(24)), 
                       BigIntGen(dataWidthList, List(25)), 
                       BigIntGen(dataWidthList, List(31))
                      )
                      
  val instreams = List((dut.io.acc_req_in, req_in_stream), (dut.io.acc_data_in, data_in_stream))
  //val outstreams = List((dut.io.acc_out,out_stream))
  testStreams (dut, instreams, List.empty, 0, 200 )
  step(200)
  poke(dut.io.mem_probe, 0)
  expect(dut.io.mem_monitor, 1)
  poke(dut.io.mem_probe, 1)
  expect(dut.io.mem_monitor, 12)
  poke(dut.io.mem_probe, 2)
  expect(dut.io.mem_monitor, 14)
  poke(dut.io.mem_probe, 10)
  expect(dut.io.mem_monitor, 23)
  poke(dut.io.mem_probe, 11)
  expect(dut.io.mem_monitor, 24)
  poke(dut.io.mem_probe, 12)
  expect(dut.io.mem_monitor, 25)
  poke(dut.io.mem_probe, 20)
  expect(dut.io.mem_monitor, 31)
  poke(dut.io.mem_probe, 21)
  expect(dut.io.mem_monitor, 22)
}

class AccOutTester extends ChiselFlatSpec {
   val optionsManager = new TesterOptionsManager {
    interpreterOptions = interpreterOptions.copy(
      writeVCD = true
    )
  }
 
  "AccOutTest" should "compile and run without incident" in {
    chisel3.iotesters.Driver(() => new TopAccOut,"firrtl") { c =>
      new AccOutTestsUsingAdvTester(c)
    } should be(true)
  }

}
