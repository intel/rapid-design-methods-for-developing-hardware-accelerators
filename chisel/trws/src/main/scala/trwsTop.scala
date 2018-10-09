// See LICENSE for license details.
//
package trws

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
  val dummy2 = UInt(32.W)
  val dummy = UInt(28.W)
  val computeIdx = Bool()
  val loadIdx = Bool()
  val modeCompute = Bool()
  val modeLoad = Bool()
  val n_slices = UInt(32.W)
  val n_cl_per_row = UInt(32.W)
  val lofaddr = UInt(64.W)
  val slcaddr = UInt(64.W)
  val moaddr = UInt(64.W)
  val wiaddr = UInt(64.W)
  val miaddr = UInt(64.W)
  val giaddr = UInt(64.W)

  def n = n_cl_per_row<<log2_elements_per_cl
}

class trwsTop(implicit params: AccParams) extends HldAcceleratorIO(new Config) {
  val trws = Module(new trws)

  val accin0 = Module (new AccIn(8))
  val accin1 = Module (new AccIn(8))
  val accin2 = Module (new AccIn(32))
  val accin3 = Module (new AccIn(8))
  val accin4 = Module (new AccIn(8))
  val accout = Module (new AccOut)

  (io.mem_rd_req, io.mem_rd_resp) <> ReadMemArbiter(List(accin0, accin1, accin2, accin3, accin4))
  (io.mem_wr_req, io.mem_wr_resp) <> (accout.io.mem_out,accout.io.mem_in) 
  
  trws.io.gi  <> TypeRepacker(DecoupledStage(accin0.io.acc_out), to = trws.io.gi.bits) 
  trws.io.wi  <> TypeRepacker(DecoupledStage(accin1.io.acc_out), to = trws.io.wi.bits) 
  trws.io.mi  <> TypeRepacker(DecoupledStage(accin2.io.acc_out), to = trws.io.mi.bits) 
  trws.io.lof <> TypeRepacker(DecoupledStage(accin3.io.acc_out), to = trws.io.lof.bits) 
  trws.io.slc <> TypeRepacker(DecoupledStage(accin4.io.acc_out), to = trws.io.slc.bits) 
  trws.io.start := io.start
  
/*
  def apply[T1 <: Data, T2 <: Data] (in: DecoupledIO[T1], to : T2, moore: Boolean = false): DecoupledIO[T2]  = {
    val tr = Module(new TypeRepacker(in.bits, to))
    tr.io.in.valid := in.valid 
    tr.io.in.bits := in.bits
    in.ready := tr.io.in.ready
    tr.io.flush := Wire(init=false.B)
    if (moore) 
      MooreStage(tr.io.out)
    else 
      DecoupledStage(tr.io.out)
  }
 */

/*
  val foo = Wire(DecoupledIO( USIMD()))
  foo.valid := trws.io.mo.valid
  foo.bits := trws.io.mo.bits.bits
  trws.io.mo.ready := foo.ready
  accout.io.acc_data_in <> TypeRepacker(DecoupledStage(foo/*trws.io.mo*/), to = accout.io.acc_data_in.bits)
 */
// Custom code to flush 

  val dmo = DecoupledStage(trws.io.mo)
  val delayedFlush = Reg( Bool())

  delayedFlush := false.B
  when ( dmo.ready && dmo.valid && dmo.bits.flush) {
    delayedFlush := true.B
    printf( p"${dmo}\n")
  }

  val tr = Module(new TypeRepacker( USIMD(), accout.io.acc_data_in.bits))
  tr.io.in.valid := dmo.valid
  tr.io.in.bits := dmo.bits.bits
  dmo.ready := tr.io.in.ready
  tr.io.flush := delayedFlush
  accout.io.acc_data_in <> DecoupledStage(tr.io.out)
  
  // create single rd/wr requests based on template modules 
  accin0.io.acc_in <> MemRdReqGenSingle(io.config, (conf: Config)=>
    AccMemBurstRdReq(conf.giaddr/(params.CacheLineWidth>>3).U, CacheLine.numCLsFor( trws.io.gi.bits.cloneType, cl_per_row.U)), io.start && io.config.modeCompute)  

  accin1.io.acc_in <> MemRdReqGenSingle(io.config, (conf: Config)=>
    AccMemBurstRdReq(conf.wiaddr/(params.CacheLineWidth>>3).U, CacheLine.numCLsFor( trws.io.wi.bits.cloneType, ((cl_per_row*cl_per_row)<<log2_elements_per_cl).U)), io.start && io.config.modeCompute)  

  accin2.io.acc_in <> MemRdReqGenSingle(io.config, (conf: Config)=>
    AccMemBurstRdReq(conf.miaddr/(params.CacheLineWidth>>3).U, CacheLine.numCLsFor( trws.io.mi.bits.cloneType, (cl_per_row*n_slices).U)), io.start && io.config.modeCompute)  

  accin3.io.acc_in <> MemRdReqGenSingle(io.config, (conf: Config)=>
    AccMemBurstRdReq(conf.lofaddr/(params.CacheLineWidth>>3).U, CacheLine.numCLsFor( trws.io.lof.bits.cloneType, n_slices.U)), io.start && io.config.modeCompute)  

  accin4.io.acc_in <> MemRdReqGenSingle(io.config, (conf: Config)=>
    AccMemBurstRdReq(conf.slcaddr/(params.CacheLineWidth>>3).U, CacheLine.numCLsFor( trws.io.slc.bits.cloneType, ((cl_per_row*cl_per_row)<<(2*log2_elements_per_cl)).U)), io.start && io.config.modeLoad)  



  accout.io.acc_req_in <> MemWrReqGenSingle(io.config, (conf: Config)=>
    AccMemBurstWrReq(conf.moaddr/(params.CacheLineWidth>>3).U, CacheLine.numCLsFor( trws.io.mo.bits.bits.cloneType, (cl_per_row*n_slices).U)), io.start)  


  def delayLine( n : Integer)( x : Bool) : Bool = {
    if ( n == 0) {
      x
    } else {
      RegNext( delayLine( n-1)( x), init=false.B)
    }
  }

  io.done := delayLine( 10)( trws.io.done)
  trws.io.start := io.start
  trws.io.loadIdx := io.config.loadIdx
  trws.io.computeIdx := io.config.computeIdx
  trws.io.modeLoad := io.config.modeLoad
  trws.io.modeCompute := io.config.modeCompute

}



object trwsTopDriver extends App {
  import HldAccParams._
  chisel3.Driver.execute(args, () => new HldAcceleratorWrapper(()=>new trwsTop))
}
