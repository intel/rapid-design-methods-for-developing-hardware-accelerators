// See LICENSE for license details.
//
package dataflow
import chisel3._
import chisel3.util._
import scala.reflect.runtime.universe._
import scala.collection.mutable.HashMap
import accio._
import type_repacker._
import designutils._
import accio.MemRdReqRespTuple._  
import accio.MemWrReqRespTuple._  

class Config extends Bundle {
  val in0addr = UInt(32.W)
  val out0addr = UInt(32.W)
  val vecCount = UInt(32.W)
}


class VectorAdd[T <: UInt] (gen : T, vecSize : Int) extends SDFActor(1, 1) {
  val vecType = Vec(vecSize, gen)
  override lazy val io = IO(new ActorIO {
   val in = In(Vec(2*vecSize, gen), 1) 
   val out = Out(Vec(vecSize, gen), 1) 
  })
    
  override def func = {
    val add = VecInit(Seq.tabulate(vecSize) {i => io.in.bits(i%vecSize) + io.in.bits(vecSize + i%vecSize) })
    io.out.bits := add
    printf("VectorAdd IN = "); io.in.bits.foreach {printf("%d ", _)}; printf("\n")
    printf("VectorAdd OUT = ");add.foreach {printf("%d ", _)};printf("\n")
  }
}



class VectorAdd2[T <: UInt] (gen : T, vecSize : Int) extends SDFActor(2,1) {
    override lazy val io = IO(new ActorIO {
      val in1 = In(Vec(vecSize, gen), 1) 
      val in2 = In(Vec(vecSize, gen), 1) 
      val out = Out(Vec(vecSize, gen), 1) 
    })
    
    override def func = {
      io.out.bits := VecInit(Seq.tabulate(vecSize) {i => io.in1.bits(i) + io.in2.bits(i) })
    }
}

class MemCtrl()(implicit params : AccParams) extends Module {
	val io = IO(new Bundle {
		val config = Flipped(Decoupled(new Config)) 
	  val acc_rd_req = Decoupled(new AccMemBurstRdReq)
		val acc_wr_req = Decoupled(new AccMemBurstWrReq)
	})

	val conf = Reg(new Config)
	val initState = RegInit(init = true.B)

  io.config.nodeq

	io.acc_rd_req.noenq
  io.acc_wr_req.noenq
  
	when (initState && io.config.valid && io.acc_rd_req.ready && io.acc_wr_req.ready) {
		printf("REQ MADE at in0addr = %d\n", conf.in0addr)
		conf := io.config.deq
		io.acc_rd_req.enq(AccMemBurstRdReq(io.config.bits.in0addr, io.config.bits.vecCount))
		io.acc_wr_req.enq(AccMemBurstWrReq(io.config.bits.out0addr, io.config.bits.vecCount/2.U))
		initState := false.B
	}.otherwise {

	}
}

//*****************************************************************************
class VectorAddTop[T <: UInt] (gen : T, vecSize : Int)(implicit params : AccParams) extends Module {
  val io = IO(new Bundle {
    val config = Flipped(Decoupled(new Config)) 
    val mem_rd_req  = Decoupled( new MemRdReq) 
    val mem_rd_resp  = Flipped(Decoupled( new MemRdResp))
    val mem_wr_req  = Decoupled( new MemWrReq)
    val mem_wr_resp  = Flipped(Decoupled( new MemWrResp))
  })

  val vecadd = Module (new VectorAdd(gen, vecSize))
  val accin = Module (new AccInReg(4))
  val accout = Module (new AccOutReg)
  val mem_ctrl = Module(new MemCtrl)
  
  vecadd.io.in <> TypeRepacker(accin.io.acc_out, to = vecadd.io.in.bits.cloneType)
  accout.io.acc_data_in <> TypeRepacker(DecoupledStage(vecadd.io.out), to = accout.io.acc_data_in.bits.cloneType)


  accin.io.acc_in <> DecoupledStage(mem_ctrl.io.acc_rd_req) 
  accout.io.acc_req_in <> DecoupledStage(mem_ctrl.io.acc_wr_req)
    
  accin.io.mem_in <> io.mem_rd_resp
  accin.io.mem_out <> io.mem_rd_req
  accout.io.mem_in <> io.mem_wr_resp
  accout.io.mem_out <> io.mem_wr_req

  mem_ctrl.io.config <> io.config
}
class VecAddNoConfigIO[T <: UInt](val gen : T, val vecLen : Int)(implicit params : AccParams) extends Module {
  val io = IO(new Bundle {
    val acc_rd_req1 = Flipped(Decoupled(new AccMemBurstRdReq))
    val acc_rd_req2 = Flipped(Decoupled(new AccMemBurstRdReq))
    val acc_wr_req = Flipped(Decoupled(new AccMemBurstWrReq))
    val mem_rd_req  = Decoupled( new MemRdReq) 
    val mem_rd_resp  = Flipped(Decoupled( new MemRdResp))
    val mem_wr_req  = Decoupled( new MemWrReq)
    val mem_wr_resp  = Flipped(Decoupled( new MemWrResp))
  })

}
class VecAddNoConfig[T <: UInt](gen : T, vecLen : Int)(implicit params : AccParams) extends VecAddNoConfigIO(gen, vecLen){
  val va = Module(new SDFActor(2,1) {
    override lazy val io = IO(new ActorIO {
      val in1 = In(Vec(vecLen, gen), 1) 
      val in2 = In(Vec(vecLen, gen), 1) 
      val out = Out(Vec(vecLen, gen), 1) 
    })
    
    override def func = {
      io.out.bits := VecInit(Seq.tabulate(vecLen) {i => io.in1.bits(i) + io.in2.bits(i) })
    }
  })
  val accin1 = Module (new AccIn(32))
  val accin2 = Module (new AccIn(32))
  val accout = Module (new AccOut)

  
  (io.mem_rd_req, io.mem_rd_resp) <> ReadMemArbiter(List(accin1, accin2))
  (io.mem_wr_req, io.mem_wr_resp) <> (accout.io.mem_out,accout.io.mem_in) 
  
  va.io.in1 <> TypeRepacker(DecoupledStage(accin1.io.acc_out), to = va.io.in1.bits) 
  va.io.in2 <> TypeRepacker(DecoupledStage(accin2.io.acc_out), to = va.io.in2.bits)
  
  accout.io.acc_data_in <> TypeRepacker(DelayModel(DecoupledStage(va.io.out),10, 2), to = accout.io.acc_data_in.bits) 
  //accout.io.acc_data_in <> TypeRepacker(DecoupledStage(va.io.out), to = accout.io.acc_data_in.bits) 
  
  accin1.io.acc_in <> io.acc_rd_req1
  accin2.io.acc_in <> io.acc_rd_req2
  accout.io.acc_req_in <> io.acc_wr_req

}

class VecAddMulDP[T <: UInt](gen : T, vecLen : Int)(implicit params : AccParams) extends SDFActor(2,1) {
    override lazy val io = IO(new ActorIO {
      val in1 = In(Vec(vecLen, gen), 1) 
      val in2 = In(Vec(vecLen, gen), 1) 
      val out = Out(Vec(vecLen, gen), 1) 
    })
    
    override def func = {
      val add = VecInit(Seq.tabulate(vecLen) {i => io.in1.bits(i) + io.in2.bits(i) })
      val mul1 = VecInit(Seq.tabulate(vecLen) {i => io.in1.bits(i) * add(i) })
      val mul2 = VecInit(Seq.tabulate(vecLen) {i => io.in2.bits(i) * add(i) })
      val add2 = VecInit(Seq.tabulate(vecLen) {i => mul1(i) + mul2(i) })
      val mul3 = VecInit(Seq.tabulate(vecLen) {i => mul1(i) * add2(i) })
      val mul4 = VecInit(Seq.tabulate(vecLen) {i => mul2(i) * add2(i) })
      val add3 = VecInit(Seq.tabulate(vecLen) {i => mul3(i) + mul4(i) })
      io.out.bits := add3
    }
}

class VecAddMulDPWStages[T <: UInt](gen : T, vecLen : Int)(implicit params : AccParams) extends Module {
  
  val io = IO(new ActorIO {
    val in0 = In(UInt((gen.getWidth*vecLen).W), 1) 
    val in1 = In(UInt((gen.getWidth*vecLen).W), 1) 
    val out = Out(UInt((gen.getWidth*vecLen).W), 1) 
  })

  val va = Module(new VecAddMulDP(gen, vecLen))
    
  va.io.in1 <> TypeRepacker(io.in0, to = va.io.in1.bits) 
  va.io.in2 <> TypeRepacker(io.in1, to = va.io.in2.bits)
  io.out <> TypeRepacker(DecoupledPipe(va.io.out, 2), to=io.out.bits)
}


class VecAddMulNoConfig[T <: UInt](gen : T, vecLen : Int)(implicit params : AccParams) extends VecAddNoConfigIO(gen, vecLen){
  val va = Module(new VecAddMulDP(gen, vecLen))
  val accin1 = Module (new AccIn(32))
  val accin2 = Module (new AccIn(32))
  val accout = Module (new AccOut)

  
  (io.mem_rd_req, io.mem_rd_resp) <> ReadMemArbiter(List(accin1, accin2))
  (io.mem_wr_req, io.mem_wr_resp) <> (accout.io.mem_out,accout.io.mem_in) 
  
  va.io.in1 <> TypeRepacker(DecoupledStage(accin1.io.acc_out), to = va.io.in1.bits) 
  va.io.in2 <> TypeRepacker(DecoupledStage(accin2.io.acc_out), to = va.io.in2.bits)
  
  accout.io.acc_data_in <> TypeRepacker(DelayModel(DecoupledStage(va.io.out),10, 2), to = accout.io.acc_data_in.bits) 
  //accout.io.acc_data_in <> TypeRepacker(DecoupledStage(va.io.out), to = accout.io.acc_data_in.bits) 
  
  accin1.io.acc_in <> io.acc_rd_req1
  accin2.io.acc_in <> io.acc_rd_req2
  accout.io.acc_req_in <> io.acc_wr_req
}

class VecAddNoConfigPpt[T <: UInt](val gen : T, val vecLen : Int)(implicit params : AccParams) extends Module {
  val io = IO(new Bundle {
    val acc_rd_req1 = Flipped(Decoupled(new AccMemBurstRdReq))
    val acc_rd_req2 = Flipped(Decoupled(new AccMemBurstRdReq))
    val acc_wr_req = Flipped(Decoupled(new AccMemBurstWrReq))
    
    val mem_rd_req  = Decoupled( new MemRdReq) 
    val mem_rd_resp  = Flipped(Decoupled( new MemRdResp))
    val mem_wr_req  = Decoupled( new MemWrReq)
    val mem_wr_resp  = Flipped(Decoupled( new MemWrResp))
  })
  
  val va = Module(new SDFActor(2,1) {
    override lazy val io = IO(new ActorIO {
      val in1 = In(Vec(vecLen, gen), 1) 
      val in2 = In(Vec(vecLen, gen), 1) 
      val out = Out(Vec(vecLen, gen), 1) 
    })
    override def func = {
      io.out.bits := VecInit(Seq.tabulate(vecLen) {i => io.in1.bits(i) + io.in2.bits(i) })
    }
  })
  val accin1 = Module (new AccIn(32))
  val accin2 = Module (new AccIn(32))
  val accout = Module (new AccOut)
  
  (io.mem_rd_req, io.mem_rd_resp) <> ReadMemArbiter(List(accin1, accin2))
  (io.mem_wr_req, io.mem_wr_resp) <> (accout.io.mem_out,accout.io.mem_in) 
  
  va.io.in1 <> TypeRepacker(DecoupledStage(accin1.io.acc_out), to = va.io.in1.bits) 
  va.io.in2 <> TypeRepacker(DecoupledStage(accin2.io.acc_out), to = va.io.in2.bits)
  accout.io.acc_data_in <> TypeRepacker(DelayModel(DecoupledStage(va.io.out),10, 2), to = accout.io.acc_data_in.bits) 
  
  accin1.io.acc_in <> io.acc_rd_req1
  accin2.io.acc_in <> io.acc_rd_req2
  accout.io.acc_req_in <> io.acc_wr_req
}


class MultiVecAddNoConfig[T <: UInt](val gen : T, val vecLen : Int, val N : Int)(implicit params : AccParams) extends Module {
  val io = IO(new Bundle {
    val acc_rd_req1 = Vec(N, Flipped(Decoupled(new AccMemBurstRdReq)))
    val acc_rd_req2 = Vec(N, Flipped(Decoupled(new AccMemBurstRdReq)))
    val acc_wr_req = Vec(N, Flipped(Decoupled(new AccMemBurstWrReq)))
    val mem_rd_req  = Decoupled( new MemRdReq) 
    val mem_rd_resp  = Flipped(Decoupled( new MemRdResp))
    val mem_wr_req  = Decoupled( new MemWrReq)
    val mem_wr_resp  = Flipped(Decoupled( new MemWrResp))

  })
  
  
  val vas = Array.fill(N) {Module(new VectorAdd2(gen, vecLen))}
    
  val accin1 = Array.fill(N) {Module (new AccIn(32))}
  val accin2 = Array.fill(N) {Module (new AccIn(32))}
  val accout = Array.fill(N) {Module (new AccOut)}

  (io.mem_rd_req, io.mem_rd_resp) <> ReadMemArbiter(accin1 ++ accin2)
  (io.mem_wr_req, io.mem_wr_resp) <> WriteMemArbiter(accout) 

  
  for (i <-0 until N) {
    vas(i).io.in1 <> TypeRepacker(DecoupledStage(accin1(i).io.acc_out), to = vas(i).io.in1.bits)
    vas(i).io.in2 <> TypeRepacker(DecoupledStage(accin2(i).io.acc_out), to = vas(i).io.in2.bits)
    //accout(i).io.acc_data_in <> TypeRepacker(DelayModel(DecoupledStage(vas(i).io.out),lat = 10, ii = 2), to = accout(i).io.acc_data_in.bits) 
    accout(i).io.acc_data_in <> TypeRepacker(DelayModel(ShiftRegisterPipeReg(vas(i).io.out, 2), 1, 4), to = accout(i).io.acc_data_in.bits) 
    
    accin1(i).io.acc_in <> io.acc_rd_req1(i)
    accin2(i).io.acc_in <> io.acc_rd_req2(i)
    accout(i).io.acc_req_in <> io.acc_wr_req(i)
  }
}

class DecoupledPipe[T<:Data](gen : T,N: Int) extends Module {
  assert(N % 2 == 0)
  val io = IO(new Bundle {
    val in = Flipped(Decoupled(gen))
    val out = Decoupled(gen)
  })
  
  def doN[T](n: Int, func: T => T, in: T): T =
      (0 until n).foldLeft(in)((last, _) => func(last))

  io.out <> doN(N, (last: DecoupledIO[T]) => DecoupledStage(last), io.in)
  
}

object DecoupledPipe {
  def apply[T<:Data](in : DecoupledIO[T], N: Int) = {
    val pipe = Module(new DecoupledPipe(in.bits, N))
    pipe.io.in <> in
    pipe.io.out
  }
}

class ShiftRegisterPipe[T<:Data] (gen : T, N : Int) extends Module {
  val io = IO(new Bundle {
    val in = Flipped(Decoupled(gen))
    val out = Decoupled(gen)
  })
  io.out.valid := ShiftRegister(io.in.valid, N, io.out.ready)
  io.out.bits := ShiftRegister(io.in.bits, N, io.out.ready)
  io.in.ready := io.out.ready
}

class ShiftRegisterPipeReg[T<:Data] (gen : T, N : Int, stage_in_front : Boolean = false) extends Module {
  val io = IO(new Bundle {
    val in = Flipped(Decoupled(gen))
    val out = Decoupled(gen)
  })
  val stage = Module(new DecoupledStage(io.in.bits.cloneType))
  
  if (stage_in_front) {
    stage.io.inp.valid := io.in.valid
    stage.io.inp.bits := io.in.bits
    io.in.ready := stage.io.inp.ready
    
    io.out.valid := ShiftRegister(stage.io.out.valid, N, io.out.ready)
    io.out.bits := ShiftRegister(stage.io.out.bits, N, io.out.ready)
    stage.io.out.ready := io.out.ready

  } else {
    stage.io.inp.valid := ShiftRegister(io.in.valid, N, stage.io.inp.ready)
    stage.io.inp.bits := ShiftRegister(io.in.bits, N, stage.io.inp.ready)
    io.in.ready := stage.io.inp.ready
    
    io.out.valid := stage.io.out.valid
    io.out.bits := stage.io.out.bits
    stage.io.out.ready := io.out.ready
  }

}

object ShiftRegisterPipeReg {
  def apply[T<:Data](in : DecoupledIO[T], N: Int, stage_in_front : Boolean = false) = {
    val srp = Module(new ShiftRegisterPipeReg(in.bits.cloneType, N))
    srp.io.in <> in
    srp.io.out
  }
}


object VecAddGen extends App {
  import accio.DefaultAccParams._
  chisel3.Driver.execute(args, () => new VecAddMulDPWStages(UInt(16.W), 8))

}
