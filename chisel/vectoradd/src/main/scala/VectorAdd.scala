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
import hld_interface_wrapper._

class Config extends Bundle {
  val vecCount = UInt(32.W)
  val out0addr = UInt(64.W)
  val in1addr = UInt(64.W)
  val in0addr = UInt(64.W)
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
    val cnt_computed = Output(chiselTypeOf(Int.MaxValue.U))
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
      io.out.bits := VecInit.tabulate(vecLen) {i => io.in1.bits(i) + io.in2.bits(i) }
    }
  })
  val accin1 = Module (new AccIn(32))
  val accin2 = Module (new AccIn(32))
  val accout = Module (new AccOut)

  // counts transactions we computed
  val (numComputed,_) = Counter(va.io.out.fire, /*some large number*/Int.MaxValue)

  (io.mem_rd_req, io.mem_rd_resp) <> ReadMemArbiter(List(accin1, accin2))
  (io.mem_wr_req, io.mem_wr_resp) <> (accout.io.mem_out,accout.io.mem_in) 
  
  va.io.in1 <> TypeRepacker(DecoupledStage(accin1.io.acc_out), to = va.io.in1.bits) 
  va.io.in2 <> TypeRepacker(DecoupledStage(accin2.io.acc_out), to = va.io.in2.bits)
  
  accout.io.acc_data_in <> TypeRepacker(DelayModel(DecoupledStage(va.io.out),10, 2), to = accout.io.acc_data_in.bits) 
  //accout.io.acc_data_in <> TypeRepacker(DecoupledStage(va.io.out), to = accout.io.acc_data_in.bits) 
  
  accin1.io.acc_in <> io.acc_rd_req1
  accin2.io.acc_in <> io.acc_rd_req2
  accout.io.acc_req_in <> io.acc_wr_req

  io.cnt_computed := numComputed
}

class VectorAddTop[T <: UInt](gen: T, vecLen: Int)(implicit params: AccParams) extends HldAcceleratorIO(new Config) {
  val va = Module(new VecAddNoConfig(gen, vecLen))

  // create single rd/wr requests based on template modules 
  va.io.acc_rd_req1 <> MemRdReqGenSingle(io.config, (conf: Config)=>
    AccMemBurstRdReq(conf.in0addr/(params.CacheLineWidth>>3).U, CacheLine.numCLsFor(Vec(vecLen, gen), conf.vecCount)), io.start)  
  va.io.acc_rd_req2 <> MemRdReqGenSingle(io.config, (conf: Config)=>
    AccMemBurstRdReq(conf.in1addr/(params.CacheLineWidth>>3).U, CacheLine.numCLsFor(Vec(vecLen, gen), conf.vecCount)), io.start)  
  va.io.acc_wr_req <> MemWrReqGenSingle(io.config, (conf: Config)=>
    AccMemBurstWrReq(conf.out0addr/(params.CacheLineWidth>>3).U, CacheLine.numCLsFor(Vec(vecLen, gen), conf.vecCount)), io.start)  

  // wait 20 cycles after the computation is done and consumers are all ready - assert done signal
  val (_,done) = RichCounter(io.start&&(va.io.cnt_computed >= io.config.vecCount) && io.mem_wr_req.ready && io.mem_rd_req.ready, 20, true)
    
  io.done := done
  va.io.mem_rd_req <> io.mem_rd_req
  va.io.mem_rd_resp <> io.mem_rd_resp
  va.io.mem_wr_req <> io.mem_wr_req
  va.io.mem_wr_resp <> io.mem_wr_resp
}



object VecAddGen extends App {
  import HldAccParams._
  chisel3.Driver.execute(args, () => new HldAcceleratorWrapper(()=>new VectorAddTop(UInt(32.W), 16)))

}
