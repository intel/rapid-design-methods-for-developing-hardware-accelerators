// See LICENSE for license details.
package accio

import chisel3._
import chisel3.util._
import designutils._

/** A memory request MemArbiter arbiter and memory response steering logic to arbitrate beween multiple AccIO units
 *  The arbiter is based on RRArbiter from Chisel utility
 *  The steering logic is coded in SteerReged. The steerBit indexing starts with 0 being LSB
 */

class MemSteerReged[T<:Data with HasMetadata]( gen : T, ways: Int, steerBit : Int ) extends Module {
  val io = IO(new Bundle {
    val qin  = Flipped(Decoupled( gen ))
    val qout = Vec(ways, Decoupled( gen ))
  })
  
  val stage_out_N = Array.fill(ways)(Module(new DecoupledStage( gen )))
  val out_stage_inps = Vec(stage_out_N.map(_.io.inp))
  for (i <- 0 until ways) {
    stage_out_N(i).io.inp  <> out_stage_inps(i)
  }

  //printf("out_stage_inps = %d, stage_out_N(0).io.inp.valid = %d\n", out_stage_inps(0).valid, stage_out_N(0).io.inp.valid)
  val out_stage_outs = stage_out_N.map(_.io.out)
  io.qin.nodeq()

  for (out_stage <- out_stage_inps) {
    out_stage.noenq()
  }
  
  //assert(isPow2(ways))
  val pendData = Reg( gen )
  val pendValid = RegInit(init=false.B)

  val pendAuId = pendData.tag(steerBit+log2Ceil(ways)-1, steerBit)


  val pendCopy = Wire(init = pendData)
  pendCopy.tag := UIntUtils.replace(pendData.tag, steerBit, 0.U(log2Ceil(ways).W))

  //printf("In data %x %x\n", pendData.asUInt(), pendCopy.asUInt())
  //printf("Tag %d %d\n", pendData.tag, pendCopy.tag)
  //printf("Pending valid %d, auid=%d\n", pendValid, pendAuId)

  when (pendValid && out_stage_inps(pendAuId).ready) {
    out_stage_inps(pendAuId).enq(pendCopy)
  }
    
  when (pendValid && out_stage_inps(pendAuId).ready || !pendValid) {
    pendValid := io.qin.valid
    when (io.qin.valid) {
      pendData := Wire(init = io.qin.deq())
    }
  }

  // connect state_out_N outputs to primary outputs
  
  stage_out_N.zipWithIndex.foreach { case (stage, i) =>stage.io.out<> io.qout(i) }
}

class RRArbiterWithTag [T<:Data with HasMetadata](gen : T, ways: Int, steerBit : Int) extends RRArbiter(gen, ways) {
  
  val data = Wire(init = io.in(io.chosen).bits)
  when (io.out.valid) {
    data.tag := UIntUtils.replace(io.in(io.chosen).bits.tag, steerBit, io.chosen.asTypeOf(ways.U))
  }
  io.out.bits := data
}


class MemArbiterReged[T<:Data with HasMetadata]( gen : T, ways: Int, steerBit : Int ) extends Module {
  val io = IO(new Bundle {
    val qin  = Flipped(Vec(ways, Decoupled( gen )))
    val qout = Decoupled( gen )
  })
 
  val arb = Module(new RRArbiterWithTag( gen, ways, steerBit))
  
  
  for ((qin, arb_in) <- io.qin.zip(arb.io.in)) {
    arb_in <> DecoupledStage(qin)
  }

  val dataWithTag = arb.io.out.bits
/*  when (arb.io.out.valid) {
    
    arb.io.out.bits.tag := UIntUtils.replace(arb.io.out.bits.tag, steerBit, arb.io.chosen.asTypeOf(ways.U))
  }
  * 
  */
  io.qout <> DecoupledStage(arb.io.out)
}

class MemArbiter [T1 <: Data with HasMetadata, T2 <: Data with HasMetadata] (ReqType : T1, RespType : T2, N: Int, TagSteerBit: Int) extends Module {
  val io = IO(new Bundle {
    val req_in  = Flipped(Vec(N, Decoupled( ReqType ))) 
    val req_out = Decoupled( ReqType ) 
    val resp_in = Flipped(Decoupled( RespType ) )
    val resp_out  = Vec(N, Decoupled( RespType ))
  })
  
  val arbiter = Module( new MemArbiterReged(ReqType, N, steerBit=TagSteerBit) )
  //val steer_read = Module( new SteerReged(UInt(width = RD_RESP_W), ways = rd_ports, steerBit=RD_RESP_W-TAG_W-log2Ceil(rd_ports)) )
  val steer = Module( new MemSteerReged(RespType, ways = N, steerBit=TagSteerBit))
  
  io.req_in <> arbiter.io.qin
  io.req_out <> arbiter.io.qout
  DecoupledStage(io.resp_in) <> steer.io.qin
  io.resp_out <> steer.io.qout
}

trait ReadMemArbiterCompatible {
  def mem_out: DecoupledIO[MemRdReq]
  def mem_in: DecoupledIO[MemRdResp]
  def bufSize: Int
}

object ReadMemArbiter  {
  def apply[T<:ReadMemArbiterCompatible](accins : Seq[T])(implicit params : AccParams) = {
    val maxBufSize = accins.map(_.bufSize) reduceLeft (_ max _)
    val steerBit = log2Ceil(maxBufSize)
    val rdArb = Module(new MemArbiter(new MemRdReq, new MemRdResp, accins.length, steerBit))
    assert(steerBit+log2Ceil(accins.size) < rdArb.arbiter.dataWithTag.tag.getWidth)
    
    
    accins.zipWithIndex.foreach { case(accin, ind) => { 
      accin.mem_out <> rdArb.io.req_in(ind)
      accin.mem_in <> rdArb.io.resp_out(ind)
    }}
    
    (rdArb.io.req_out,rdArb.io.resp_in) 
  }
  
  def apply(accins: Seq[(DecoupledIO[MemRdReq], DecoupledIO[MemRdResp])], maxBufSize: Int)(implicit params : AccParams) = {
    val steerBit = log2Ceil(maxBufSize)
    val rdArb = Module(new MemArbiter(new MemRdReq, new MemRdResp, accins.length, steerBit))
    assert(steerBit+log2Ceil(accins.size) < rdArb.arbiter.dataWithTag.tag.getWidth)
    
    
    accins.zipWithIndex.foreach { case(accin, ind) => { 
      accin._1 <> rdArb.io.req_in(ind)
      accin._2 <> rdArb.io.resp_out(ind)
    }}
    
    (rdArb.io.req_out,rdArb.io.resp_in) 
   
  }
}

object WriteMemArbiter  {
  def apply(accouts : Seq[AccOut])(implicit params : AccParams) = {
    val steerBit = 0
    val wrArb = Module(new MemArbiter(new MemWrReq, new MemWrResp, accouts.length, steerBit))
    assert(steerBit+log2Ceil(accouts.size) < wrArb.arbiter.dataWithTag.tag.getWidth)
    
    
    accouts.zipWithIndex.foreach { case(accout, ind) => { 
      accout.io.mem_out <> wrArb.io.req_in(ind)
      accout.io.mem_in <> wrArb.io.resp_out(ind)
    }}
    
    (wrArb.io.req_out,wrArb.io.resp_in) 
  }
}

