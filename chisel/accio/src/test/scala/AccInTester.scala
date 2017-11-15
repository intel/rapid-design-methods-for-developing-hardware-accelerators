// See LICENSE for license details.
package accio

import chisel3._
import chisel3.util._
import chisel3.iotesters._
import testutil._
import accio.DefaultAccParams._

class MyType extends Bundle {
  val data = UInt(16.W)
}

object MyType {
  def apply(data : UInt) : MyType = {
    val inst = new MyType
    inst.data := data
    inst
  }
}


object BigIntFromUIntBundle {
  def apply(gen : Bundle, values : List[Int]) : BigInt = {
    val widths = gen.elements.map(_._2.getWidth).toList
    BigIntGen(widths,values)
  }
}
object BigIntGen {
  def apply(widths : List[Int], values : List[Int]) : BigInt = {
    assert(values.length > 0)
    assert(values.length == widths.length)
    var result : BigInt = values(0);
    var bundle = widths zip values
    
    for ((width, value) <- bundle.tail) {
      result = (result <<width) + value
    }
    result
  }
}

object MemInit {
  def init_func (in : UInt): UInt = {
    in + 1.U
  }
}
class Top (bufSize: Int) extends Module {

  val accIn = Module(new AccInReg(bufSize))
  val mockmem = Module(new MockMemory(256, Some(MemInit.init_func)))
  val io = IO(new Bundle {
    val acc_in  = Flipped(Decoupled( UInt(accIn.io.acc_in.bits.getWidth.W)) )
    val acc_out = Decoupled( UInt(accIn.io.acc_out.bits.getWidth.W ) )
  })
  val reqQ = Module( new Queue(mockmem.io.mem_rd_in.bits.cloneType, 2))
  val respQ = Module (new Queue(mockmem.io.mem_rd_out.bits.cloneType, 2))
  //val q = new Queue(UInt(), 16)
  //q.io.enq <> producer.io.out
  //consumer.io.in <> q.io.deq

  reqQ.io.enq <> accIn.io.mem_out
  mockmem.io.mem_rd_in <> reqQ.io.deq
  
  respQ.io.enq <> mockmem.io.mem_rd_out
  accIn.io.mem_in <> respQ.io.deq
 
 
  accIn.io.acc_in.valid <> io.acc_in.valid
  accIn.io.acc_in.ready <> io.acc_in.ready

  accIn.io.acc_out.valid <> io.acc_out.valid
  accIn.io.acc_out.ready <> io.acc_out.ready
  
  accIn.io.acc_in.bits :=  accIn.io.acc_in.bits.fromBits(io.acc_in.bits) 
  io.acc_out.bits := accIn.io.acc_out.bits.asUInt
   
}

class TopWithUserAccIn extends Module {
  val accUserIn = Module(new AccUserIn(UInt(32.W), 2, 4))
  val mockmem = Module(new MockMemory(10, Some(MemInit.init_func)))
  val io = IO(new Bundle {
    val acc_in = Flipped(Decoupled(new AccMemBurstRdReq ()))
    val acc_out = Decoupled(accUserIn.io.user_out.bits.cloneType)
  })

  accUserIn.io.mem_out <> mockmem.io.mem_rd_in
  accUserIn.io.mem_in <> mockmem.io.mem_rd_out
  io.acc_in <> accUserIn.io.user_in
  io.acc_out <> accUserIn.io.user_out
}
object AccUserInTests {
  def clToByte(addr: Int): BigInt = {
    BigInt(addr*(params.CacheLineWidth>>3))
  }
}
class AccUserInTestsUsingAdvTester(dut: TopWithUserAccIn) extends DecoupledStreamingTestsUsingAdvTester(dut) {
  val clToByte: Int=>BigInt = AccUserInTests.clToByte
  val in_stream = List(Map("addr"->clToByte(0), "burstSize"->BigInt(4)))
  val instreams = List((dut.io.acc_in, in_stream))
  val outstreams = List()
  testStreams (dut, instreams, outstreams, 0, 1000 )
  poke(dut.io.acc_out.ready, 1)
  val expectedVecs = List(Vector(BigInt(0),BigInt(1)), Vector(BigInt(0),BigInt(2)))
  val expectedCnt = List(2,2)
  (0 until 100).foldLeft(0) {case(o, i) =>
    val next = if(peek(dut.io.acc_out.valid) == 1 && peek(dut.io.acc_out.ready) == 1) {
      val pvec = peek(dut.io.acc_out.bits.vec)
      println(s"iter $o $pvec")
      expect(dut.io.acc_out.bits.vec, expectedVecs(o))
      expect(dut.io.acc_out.bits.cnt, expectedCnt(o))
      o+1
    } else o
    step(1)
    next
  }
}

class AccUserInTestsUsingAdvTesterOddCnt(dut: TopWithUserAccIn)(implicit params: AccParams) extends DecoupledStreamingTestsUsingAdvTester(dut) {
  val x = 1
  val clToByte: Int=>BigInt = AccUserInTests.clToByte
  val in_stream = List(Map("addr"->clToByte(0), "burstSize"->BigInt(5)), Map("addr"->clToByte(2), "burstSize"->BigInt(3)))
  val instreams = List((dut.io.acc_in, in_stream))
  val outstreams = List()
  testStreams (dut, instreams, outstreams, 0, 1000 )
  poke(dut.io.acc_out.ready, 1)
  val expectedVecs = List(Vector(BigInt(0),BigInt(1)), Vector(BigInt(0),BigInt(2)),Vector(BigInt(0),BigInt(3)),
                          Vector(BigInt(0),BigInt(3)),Vector(BigInt(0),BigInt(4)))
  val expectedCnt = List(2,2,1,2,1)
  val outCnt = (0 until 100).foldLeft(0) {case(o, i) =>
    val next = if(peek(dut.io.acc_out.valid) == 1 && peek(dut.io.acc_out.ready) == 1) {
      val pvec = peek(dut.io.acc_out.bits.vec)
      println(s"iter $o $pvec")
      expect(dut.io.acc_out.bits.vec, expectedVecs(o))
      expect(dut.io.acc_out.bits.cnt, expectedCnt(o))
      o+1
    } else o
    step(1)
    next
  }
  assert(outCnt == 5)
}

class AccInTestsUsingAdvTester(dut: Top) extends DecoupledStreamingTestsUsingAdvTester(dut) {
  val inType = dut.accIn.io.acc_in.bits.cloneType
  val outType = dut.accIn.io.acc_out.bits.cloneType
  val inWidthList = List(inType.addr.getWidth,inType.burstSize.getWidth)
  val outWidthList = List(outType.data.getWidth)
  
  val in_stream = List(BigIntGen(inWidthList, List(0,2)), 
                       BigIntGen(inWidthList, List(1,3)), 
                       BigIntGen(inWidthList, List(2,1))
                      )
  val out_stream = List(BigIntGen(outWidthList, List(1)), 
                        BigIntGen(outWidthList, List(2)), 
                        BigIntGen(outWidthList, List(2)), 
                        BigIntGen(outWidthList, List(3)), 
                        BigIntGen(outWidthList, List(4)), 
                        BigIntGen(outWidthList, List(3))
                      )
  val instreams = List((dut.io.acc_in, in_stream))
  val outstreams = List((dut.io.acc_out,out_stream))
  testStreams (dut, instreams, outstreams, 0, 1000 )
}

class AccInTester extends ChiselFlatSpec {
  "AccInTest" should "compile and run without incident" in {
    chisel3.iotesters.Driver(() => new Top(6),"firrtl") { c =>
      new AccInTestsUsingAdvTester(c)
    } should be(true)
  }
}
class AccUserInTester extends ChiselFlatSpec {
  "AccUserInTestWithOddCnt" should "compile and run without incident" in {
    chisel3.iotesters.Driver(() => new TopWithUserAccIn,"verilator") { c =>
      new AccUserInTestsUsingAdvTesterOddCnt(c)
    } should be(true)
  }
//  "AccUserInTest" should "compile and run without incident" in {
//    chisel3.iotesters.Driver(() => new TopWithUserAccIn,"verilator") { c =>
//      new AccUserInTestsUsingAdvTester(c)
//    } should be(true)
//  }

}


