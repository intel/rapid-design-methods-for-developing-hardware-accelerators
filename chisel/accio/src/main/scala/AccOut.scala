// See LICENSE for license details.
package accio

import chisel3._
import chisel3.util._
import designutils.DecoupledStage
import designutils.DebugMacros._
import type_repacker._
/** A store unit that takes a burst write request and produce N memory write requests.
 *  It doesn't support partial writes. It drops memory responses (acknowledgements)    
 */
class AccOut ()(implicit params : AccParams) extends Module with AccioDebug {
  val io = IO(new Bundle {
    val acc_req_in  = Flipped(Decoupled( new AccMemBurstWrReq()))
    val acc_data_in = Flipped(Decoupled( new AccMemWrData))
    val mem_in  = Flipped(Decoupled( new MemWrResp()))
    val mem_out = Decoupled( new MemWrReq() )
  })
  
  val remainReqCnt = RegInit(init=0.U)
  val nextReqAddr = RegInit(init=0.U)
  
  val w_hasReq = WireInit(remainReqCnt =/= 0.U)
  
  io.acc_req_in.nodeq()
  io.acc_data_in.nodeq()
  io.mem_in.nodeq()
  io.mem_out.noenq()

  
  val w_canGetNextReq = WireInit(remainReqCnt === 0.U)
  
  val w_wr_data = WireInit(io.acc_data_in.bits)

  when(remainReqCnt =/= 0.U) {
    when (io.mem_out.ready) {
      io.acc_data_in.deq
    }
    when (io.acc_data_in.valid) {
      pf("remainReqCnt=%d memWrReq addr = %d data - %x\n",remainReqCnt, nextReqAddr, w_wr_data.data)
      io.mem_out.enq(MemWrReq(nextReqAddr, w_wr_data.data, nextReqAddr))
      when (io.mem_out.ready) {
        nextReqAddr := nextReqAddr + 1.U
        remainReqCnt := remainReqCnt - 1.U
        when (remainReqCnt === 1.U) {
          w_canGetNextReq := true.B
        }
      }
    }
  }  

  when (w_canGetNextReq) {
    val req = io.acc_req_in.deq
    when(io.acc_req_in.valid) {
      pf("REQ addr = %d burst = %d \n",req.addr, req.burstSize)
      nextReqAddr := req.addr
      remainReqCnt := req.burstSize
    }
  }
   
  when (io.mem_in.valid) {
    // drop response
    io.mem_in.deq    
  }
}

      
class AccOutReg ()(implicit params : AccParams) extends Module {
  val io = IO(new Bundle {
    val acc_req_in  = Flipped(Decoupled( new AccMemBurstWrReq()))
    val acc_data_in = Flipped(Decoupled( new AccMemWrData))
    val mem_in  = Flipped(Decoupled( new MemWrResp()))
    val mem_out = Decoupled( new MemWrReq() )
  })
  
  val acc_out = Module(new AccOut)
  
  acc_out.io.acc_req_in <> io.acc_req_in
  acc_out.io.acc_data_in <> io.acc_data_in
  acc_out.io.mem_in <> io.mem_in

  io.mem_out <> acc_out.io.mem_out
}


class AccUserOut[T<:Data] (gen: T, vecSize: Int)(implicit params: AccParams) extends Module {
  val io = IO(new Bundle {
    val user_req_in = Flipped(Decoupled(new AccMemBurstWrReq ()))
    val user_data_in = Flipped(Decoupled(Vec(vecSize, gen)))
    val mem_in  = Flipped(Decoupled(new MemRdResp()))
    val mem_out = Decoupled(new MemRdReq)
  })
  
  if (gen.getWidth*vecSize != params.CacheLineWidth) {
    assert(false, "Not implemented") 
  }

  val accOut = Module(new AccOut)
  accOut.io.acc_req_in.enq(AccMemBurstWrReq(io.user_req_in.bits.addr, io.user_req_in.bits.burstSize/vecSize.U + Mux(io.user_req_in.bits.burstSize%vecSize.U === 0.U, 0.U, 1.U)))
  accOut.io.acc_req_in.valid := io.user_req_in.valid
  io.user_req_in.ready := accOut.io.acc_req_in.ready
  
  io.mem_out <> TypeRepacker(io.user_data_in, to = io.mem_out.bits)
}