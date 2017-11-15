// See LICENSE for license details.
package type_repacker

import chisel3._
import chisel3.util._
import chisel3.iotesters._
import testutil._
import accio._
import accio.DefaultAccParams._


//Adapter module
class BitsToVec[T <: Data](gen : T, N : Int) extends Module {
  val io = IO(
    new Bundle{
      val in = Flipped(Decoupled(UInt((N*gen.getWidth).W)))
      val out = Decoupled(Vec(N,gen))
    }
  )
  val WIDTH = gen.getWidth
  
  for (i <- 0 until N){
    io.out.bits(i) := io.out.bits(i).fromBits(io.in.bits(i*WIDTH+WIDTH-1,i*WIDTH))
  }
  
  //printf("BitsToVec %d converted to %d (valid=%d, %d) ready = %d\n", io.in.bits(0).asUInt, io.out.bits(0).asUInt, io.in.valid, io.out.valid, io.out.ready)
	io.out.valid := io.in.valid
	io.in.ready := io.out.ready
}

class VecToBits[T <: Data](gen : T, N : Int) extends Module {
  val io = IO(new Bundle{
    val in = Flipped(Decoupled(Vec(N,gen)))
    val out = Decoupled(UInt((N*gen.getWidth).W))
  })
  
  val WIDTH = gen.getWidth
 
  val out = Wire(UInt((N*gen.getWidth).W))
  
  io.out.bits := io.in.bits.asUInt
  
  //printf("VecToBits %d converted to %d (valid=%d, %d)\n", io.in.bits(0).asUInt, io.out.bits, io.in.valid, io.out.valid)
  io.out.valid := io.in.valid
  io.in.ready := io.out.ready
}

class TopIf [T1 <: Data, T2<: Data] (gen1 : T1, gen2 : T2) extends Module {
  val io = IO(new Bundle {
    val in_flat  = Flipped(Decoupled(UInt((gen1.getWidth).W)))
    val out_flat = Decoupled(UInt((gen2.getWidth).W))
  })
  
}
//class Top[T1 <: Vec[_ <: Data], T2<: Vec[_ <: Data]] (gen1 : T1, gen2 : T2) extends TopIf(gen1, gen2) {
class Top[T1 <: Data, T2<: Data] (vgen1 : T1, vgen2 : T2) extends TopIf(vgen1, vgen2) {
  val gen1 = vgen1.asInstanceOf[Vec[_ <: Data]]
  val gen2 = vgen2.asInstanceOf[Vec[_ <: Data]]
  
  val dut = Module (new TypeRepackerVecReg(gen1(0), gen1.length, gen2(0), gen2.length))  
  val in_adapter = Module (new BitsToVec(gen1(0), gen1.length))
  val out_adapter = Module (new VecToBits(gen2(0), gen2.length)) 
  
  io.in_flat <> in_adapter.io.in
  dut.io.in <> in_adapter.io.out

  io.out_flat <> out_adapter.io.out 
  out_adapter.io.in <> dut.io.out
}

class TopSimple[T1 <: Data, T2<: Data] (gen1 : T1, gen2 : T2) extends TopIf(gen1, gen2) {
  val dut = Module (new TypeRepacker(gen1, gen2))  

  dut.io.in.valid := io.in_flat.valid
  dut.io.in.ready <> io.in_flat.ready
  dut.io.in.bits := dut.io.in.bits.fromBits(io.in_flat.bits.asUInt())
  
  io.out_flat.valid := dut.io.out.valid 
  io.out_flat.ready <> dut.io.out.ready 
  io.out_flat.bits := io.out_flat.bits.fromBits(dut.io.out.bits.asUInt()) 
}


class SimpleTypeRepackerTester[T1 <: Data, T2 <: Data](dut : TopIf[T1, T2]) extends DecoupledStreamingTestsUsingAdvTester(dut) {
  val in_stream = List(BigIntGen(List(16,16,16,16), List(0,1,2,3)) 
                      )
  val out_stream = List(BigIntGen(List(16, 16), List(2,3)),
                        BigIntGen(List(16, 16), List(0,1))
                      )
  val instreams = List((dut.io.in_flat, in_stream))
  val outstreams = List((dut.io.out_flat,out_stream))
  testStreams (dut, instreams, outstreams, 0, 100 )
} 

class AccumTypeRepackerTester[T1 <: Data, T2 <: Data](dut : TopIf[T1, T2]) extends DecoupledStreamingTestsUsingAdvTester(dut) {
  val in_stream = List(BigIntGen(List(16), List(1)),
                        BigIntGen(List(16), List(1)),
                        BigIntGen(List(16), List(1)),
                        BigIntGen(List(16), List(2)),
                        BigIntGen(List(16), List(2)),
                        BigIntGen(List(16), List(2))
                      )
  val out_stream = List(BigIntGen(List(16, 16, 16), List(1,1,1)),
                        BigIntGen(List(16, 16, 16), List(2,2,2))
                      )
  val instreams = List((dut.io.in_flat, in_stream))
  val outstreams = List((dut.io.out_flat,out_stream))
  testStreams (dut, instreams, outstreams, 0, 100 )
} 


class BiggerTypeRepackerTester[T1 <: Data, T2 <: Data](dut : TopIf[T1, T2]) extends DecoupledStreamingTestsUsingAdvTester(dut) {
  val t1gen = new BiggerType()
  val t2gen = new MyType()
  
  val in_stream = List(BigIntGen(List(t1gen.getWidth), List(1)),
                       BigIntGen(List(t1gen.getWidth), List(65536)) 
                      )
  val out_stream = List(BigIntGen(List(16,16), 
                                  List(0,1)),
                        BigIntGen(List(16,16), 
                                  List(1,0))
                      )
  val instreams = List((dut.io.in_flat, in_stream))
  val outstreams = List((dut.io.out_flat,out_stream))
  testStreams (dut, instreams, outstreams, 0, 100 )
} 

class MergeTypeRepackerTester[T1 <: Data, T2 <: Data](dut : TopIf[T1, T2]) extends DecoupledStreamingTestsUsingAdvTester(dut) {
  val t1gen = new MyType()
  val t2gen = new BiggerType()
  
  val in_stream = List(BigIntGen(List(16), 
                                  List(1)),
                        BigIntGen(List(16), 
                                  List(0)),
                        BigIntGen(List(16), 
                                  List(0)),
                        BigIntGen(List(16), 
                                  List(1)),
                        BigIntGen(List(16), 
                                  List(2)),
                        BigIntGen(List(16), 
                                  List(0)),
                        BigIntGen(List(16), 
                                  List(3)),
                        BigIntGen(List(16), 
                                  List(0))
                      )
  val out_stream = List(BigIntGen(List(t1gen.getWidth), List(1))
                     , BigIntGen(List(t1gen.getWidth), List(65536)) 
                     , BigIntGen(List(t1gen.getWidth), List(2)) 
                     , BigIntGen(List(t1gen.getWidth), List(3)) 
                     )
  val instreams = List((dut.io.in_flat, in_stream))
  val outstreams = List((dut.io.out_flat,out_stream))
  testStreams (dut, instreams, outstreams, 0, 100 )

} 

class SplitTypeRepackerTester[T1 <: Data, T2 <: Data](dut : TopIf[T1, T2]) extends DecoupledStreamingTestsUsingAdvTester(dut) {
  val t1gen = new BiggerType()
  val t2gen = new MyType()
  
  val in_stream = List(BigIntGen(List(t1gen.getWidth), List(1))
                     , BigIntGen(List(t1gen.getWidth), List(65536)) 
                     , BigIntGen(List(t1gen.getWidth), List(2)) 
                     , BigIntGen(List(t1gen.getWidth), List(3)) 
                     )
  val out_stream = List(BigIntGen(List(16), 
                                  List(1)),
                        BigIntGen(List(16), 
                                  List(0)),
                        BigIntGen(List(16), 
                                  List(0)),
                        BigIntGen(List(16), 
                                  List(1)),
                        BigIntGen(List(16), 
                                  List(2)),
                        BigIntGen(List(16), 
                                  List(0)),
                        BigIntGen(List(16), 
                                  List(3)),
                        BigIntGen(List(16), 
                                  List(0))
                      )
  val instreams = List((dut.io.in_flat, in_stream))
  val outstreams = List((dut.io.out_flat,out_stream))
  testStreams (dut, instreams, outstreams, 0, 100 )
} 


class BiggerType extends Bundle {
  val data = UInt(32.W)
}


class TypeRepackerTester extends ChiselFlatSpec {
   val optionsManager = new TesterOptionsManager {
    interpreterOptions = interpreterOptions.copy(
      writeVCD = true
    )
  }
  def createDut[T1 <: Data, T2 <: Data] (gen1 : T1, gen2 : T2) : TopIf[T1, T2] = new Top(gen1, gen2)
  "SimpleTypeRepackerTester" should "compile and run without incident" in {
    chisel3.iotesters.Driver(() => createDut(Vec(4, new MyType()), Vec(2, new MyType())),"firrtl") { c =>
      new SimpleTypeRepackerTester(c)
    } should be(true)
  }


  "BiggerTypeRepackerTester" should "compile and run without incident" in {
    chisel3.iotesters.Driver(() => createDut(Vec(1,new BiggerType()), Vec(2,  new MyType())),"firrtl") { c =>
      new BiggerTypeRepackerTester(c)
    } should be(true)
  }
  "SplitTypeRepackerTester" should "compile and run without incident" in {
    chisel3.iotesters.Driver(() => createDut(Vec(1,new BiggerType()), Vec(1,  new MyType())),"firrtl") { c =>
      new SplitTypeRepackerTester(c)
    } should be(true)
  }

  "MergeTypeRepackerTester" should "compile and run without incident" in {
    def createDut[T1 <: Data, T2 <: Data] (gen1 : T1, gen2 : T2) : TopIf[T1, T2] = new Top(gen1, gen2)
    chisel3.iotesters.Driver(() => createDut(Vec(1,new MyType()),Vec(1, new BiggerType())),"firrtl") { c =>
      new MergeTypeRepackerTester(c)
    } should be(true)
  }
}


class TypeRepackerSimpleTester extends ChiselFlatSpec {
   val optionsManager = new TesterOptionsManager {
    interpreterOptions = interpreterOptions.copy(
      writeVCD = true
    )
  }
  def createDut[T1 <: Data, T2 <: Data] (gen1 : T1, gen2 : T2) : TopIf[T1, T2] = new TopSimple(gen1, gen2)
  
  "AccumTypeRepackerTester" should "compile and run without incident" in {
    chisel3.iotesters.Driver(() => createDut(new MyType(), Vec(3, new MyType())),"verilator") { c =>
      new AccumTypeRepackerTester(c)
    } should be(true)
  }
  "SimpleTypeRepackerTester" should "compile and run without incident" in {
    chisel3.iotesters.Driver(() => createDut(Vec(4, new MyType()), Vec(2, new MyType())),"firrtl") { c =>
      new SimpleTypeRepackerTester(c)
    } should be(true)
  }



  "BiggerTypeRepackerTester" should "compile and run without incident" in {
    chisel3.iotesters.Driver(() => createDut(Vec(1,new BiggerType()), Vec(2,  new MyType())),"firrtl") { c =>
      new BiggerTypeRepackerTester(c)
    } should be(true)
  }
  "SplitTypeRepackerTester" should "compile and run without incident" in {
    chisel3.iotesters.Driver(() => createDut(Vec(1,new BiggerType()), Vec(1,  new MyType())),"firrtl") { c =>
      new SplitTypeRepackerTester(c)
    } should be(true)
  }

  "MergeTypeRepackerTester" should "compile and run without incident" in {
    def createDut[T1 <: Data, T2 <: Data] (gen1 : T1, gen2 : T2) : TopIf[T1, T2] = new Top(gen1, gen2)
    chisel3.iotesters.Driver(() => createDut(Vec(1,new MyType()),Vec(1, new BiggerType())),"firrtl") { c =>
      new MergeTypeRepackerTester(c)
    } should be(true)
  }
}
