// See LICENSE for license details.
package accio

import chisel3._
import chisel3.util._
import designutils._
import designutils.DebugMacros._
import type_repacker._

/** A load unit that is cache addressable and takes a burst read request and produce N memory read requests.
 *  It writes responses back in order.
 *  
 *  acc_in - accelerator burst read request.  
 *  acc_out - accelerator read response (responses will come in-order)
 *  mem_out - memory read request 
 *  mem_in - memory read response (cacheline)
 *
 *  @param bufSize size of the buffer for outstanding memory requests        
 */
class AccIn(val bufSize : Int )(implicit params : AccParams) extends Module with AccioDebug with ReadMemArbiterCompatible {
  val io = IO(new Bundle {
    val acc_in  = Flipped(Decoupled( new AccMemBurstRdReq()))
    val acc_out = Decoupled( new AccMemRdResp() )
    val mem_in  = Flipped(Decoupled( new MemRdResp()))
    val mem_out = Decoupled( new MemRdReq() )
  })
  
  def mem_in = io.mem_in
  def mem_out = io.mem_out
  // create a type for tag and pass it in
  //type TagType = (bufSize-1).U.cloneType
  val tagWidth = (bufSize-1).U.getWidth.W
  val creditQ = Module(new Queue(UInt(tagWidth), bufSize))
  val orderedReqsQ = Module(new Queue(UInt(tagWidth), bufSize))
  val respBuffer = SyncReadMem(bufSize, io.acc_out.bits.cloneType)
  val respBufValids = RegInit(init = Vec.fill(bufSize){false.B})
  
  val s_resetCreditQ :: s_main :: Nil = Enum(2)
  val state = RegInit(init = s_resetCreditQ)
  val resetCreditCnt = RegInit(init=(bufSize-1).U)
  
  val rdEnReg = RegInit(init=false.B)
  val memReadValid = RegInit(init=false.B)
  val memReadData = Reg(new AccMemRdResp)

  val remainReqCnt = RegInit(init=0.U)
  val nextReqAddr = RegInit(init=0.U)
   
  val reqTagReg = RegInit(init = InvalidToken(UInt(tagWidth)))
  val memRdReqTag = RegInit(init = InvalidToken(UInt(tagWidth)))
 
   
  io.mem_out.noenq()
  io.acc_out.noenq()
  io.acc_in.nodeq()
  io.mem_in.nodeq()
  creditQ.io.enq.noenq()
  creditQ.io.deq.nodeq()
  orderedReqsQ.io.enq.noenq()
  orderedReqsQ.io.deq.nodeq()
  
   
  switch (state) {
    is(s_resetCreditQ) {
      resetCreditCnt := resetCreditCnt - 1.U
      creditQ.io.enq.enq(resetCreditCnt)
      when(resetCreditCnt === 0.U) {
        state := s_main
      }
    }
    
    is (s_main) {
      //printf("credit available = %d\n", creditQ.io.deq.valid)
      val w_hasReq = Wire(init = (remainReqCnt != 0.U))
      val w_nextag = Wire(init = creditQ.io.deq.bits)

      when (remainReqCnt != 0.U) {
        when (io.mem_out.ready && orderedReqsQ.io.enq.ready) {
      	  w_nextag := creditQ.io.deq.deq
        }
  
        //when (remainReqCnt != 0.U && io.mem_out.ready && creditQ.io.deq.valid && orderedReqsQ.io.enq.ready) {
        when (creditQ.io.deq.valid) {
      	  val nextag = w_nextag//creditQ.io.deq.deq()
      	  when (orderedReqsQ.io.enq.ready) {
      		  io.mem_out.enq(MemRdReq(addr = nextReqAddr, tag = nextag))
      			//MemRdReq(addr = nextReqAddr, tag = nextag).print
      	  }
      	  when (io.mem_out.ready) {
      		  orderedReqsQ.io.enq.enq(nextag)
      	  }
      	  when(io.mem_out.ready && orderedReqsQ.io.enq.ready) {
      		  assert(orderedReqsQ.io.enq.fire)
      		  remainReqCnt := remainReqCnt - 1.U
      		  nextReqAddr := nextReqAddr + 1.U
      		  when (remainReqCnt === 1.U) {
      			  w_hasReq := false.B;
      		  }
      	  }
        }
      }


      when (!w_hasReq && io.acc_in.valid) {
        val acc_in_tx = io.acc_in.deq()
        //acc_in_tx.print
        assert(acc_in_tx.burstSize != 0.U)
        remainReqCnt := acc_in_tx.burstSize
        nextReqAddr := acc_in_tx.addr
      }
      
      
      when(io.mem_in.valid) {
        val mem_in_tx = io.mem_in.deq()
        assert(mem_in_tx.tag < bufSize.U)
        respBuffer(mem_in_tx.tag) := AccMemRdResp(mem_in_tx.data)
        assert(respBufValids(mem_in_tx.tag) === false.B)
        respBufValids(mem_in_tx.tag) := true.B
        //printf("mem_in %d  %x\n",mem_in_tx.tag , mem_in_tx.data)
      }
       
      //when (io.acc_out.ready) {
        val rdAddr = reqTagReg.token
        val rdEn = io.acc_out.ready && reqTagReg.valid && respBufValids(reqTagReg.token)
        val memRd = respBuffer.read(rdAddr,rdEn)
        val memRdReg = Reg(memRd.cloneType)
        
        rdEnReg := rdEn
        when (rdEnReg) {
          memRdReg := memRd
        }
        when (reqTagReg.valid || rdEn || memRdReqTag.valid) {
          pf("reqTagReg(%d)=%d, rdEn = %d, memRdReg(%d)=%d memData = %x\n",  reqTagReg.valid, reqTagReg.token, rdEn, memRdReqTag.valid, memRdReqTag.token, memRd.data)
        }
        val w_memRdReqTag = Wire(init = memRdReqTag)
        when(memRdReqTag.valid) {
          val rdResp = Mux(rdEnReg, memRd.data, memRdReg.data)
          io.acc_out.enq(AccMemRdResp(rdResp))
          when(io.acc_out.ready) {
            respBufValids(memRdReqTag.token) := false.B
            reqTagReg.reset()
            creditQ.io.enq.enq(memRdReqTag.token)
            w_memRdReqTag.valid := false.B
          }
        }
        
        val w_reqTagReg = Wire(init = reqTagReg)
        when (!w_memRdReqTag.valid && rdEn) {
          memRdReqTag := reqTagReg
          w_reqTagReg.valid := false.B
        }.otherwise {
          memRdReqTag := w_memRdReqTag
        }
        
        when (!w_reqTagReg.valid && orderedReqsQ.io.deq.valid) {
          reqTagReg.set(orderedReqsQ.io.deq.deq())
        }.otherwise {
          reqTagReg := w_reqTagReg
        }
      //}
    }
  }
}

class AccInReg(val bufSize : Int )(implicit params : AccParams) extends Module with ReadMemArbiterCompatible {
  val io = IO(new Bundle {
    val acc_in  = Flipped(Decoupled( new AccMemBurstRdReq()))
    val acc_out = Decoupled( new AccMemRdResp() )
    val mem_in  = Flipped(Decoupled( new MemRdResp()))
    val mem_out = Decoupled( new MemRdReq() )
  })

  def mem_in = io.mem_in
  def mem_out = io.mem_out

  val accin = Module(new AccIn(bufSize))

  accin.io.acc_in <> io.acc_in
  accin.io.mem_in <> io.mem_in
  
  io.acc_out <> DecoupledStage(accin.io.acc_out)
  io.mem_out <> DecoupledStage(accin.io.mem_out)
}

/**
 * Load unit that takes byte addressable requests of objects of user type T that the user wants to be packed in Vec(vecSize, T) vectors. 
 */
class AccUserIn[T<:Data] (gen: T, vecSize: Int, val bufSize: Int)(implicit params: AccParams) extends Module with ReadMemArbiterCompatible {
  val io = IO(new Bundle {
    val user_in = Flipped(Decoupled(new AccMemBurstRdReq ()))
    val user_out = Decoupled(new AccMemUserRdResp(gen, vecSize))
    val mem_in  = Flipped(Decoupled(new MemRdResp()))
    val mem_out = Decoupled(new MemRdReq)
  })
  // need extra slot in the queue due to one extra latency we have on the reading from accIn.acc_out
  val burstTrackingQ = Module(new Queue(UInt(params.DefBurstSizeWidth.W), bufSize+1))
  val outstandingBurst = RegInit(init = 0.U(params.DefBurstSizeWidth.W))
  val accIn = Module(new AccIn(bufSize))
  
  def mem_in = io.mem_in
  def mem_out = io.mem_out
  
  io.mem_in.nodeq
  io.mem_out.noenq
  io.user_in.nodeq
  io.user_out.noenq
  burstTrackingQ.io.enq.noenq
  burstTrackingQ.io.deq.nodeq
  accIn.io.acc_in.noenq
  
  if (gen.getWidth*vecSize != params.CacheLineWidth) {
    println(s"${gen.getWidth} times $vecSize not equal to ${params.CacheLineWidth}") 
    assert(false, "Not implemented") 
  }
  // request handler
  when(io.user_in.valid) {
    assert(io.user_in.bits.addr%(params.CacheLineWidth>>3).U === 0.U, "Data is not cache aligned")
    accIn.io.acc_in.enq(AccMemBurstRdReq(io.user_in.bits.addr/(params.CacheLineWidth>>3).U, io.user_in.bits.burstSize/vecSize.U + Mux(io.user_in.bits.burstSize%vecSize.U === 0.U, 0.U, 1.U)))
  }
  io.user_in.ready := accIn.io.acc_in.ready
  when(io.user_in.fire) {
    assert(burstTrackingQ.io.enq.ready)
    burstTrackingQ.io.enq.enq(io.user_in.bits.burstSize)
  }
  //response handler
  val repackedIO = TypeRepacker(DecoupledStage(accIn.io.acc_out), to=Vec(vecSize, gen))
  assert(params.CacheLineWidth%gen.getWidth == 0)
  
  
  when(outstandingBurst != 0.U) {
    val userOutData = Wire(io.user_out.bits.cloneType)
    userOutData.vec := repackedIO.bits
    userOutData.cnt := Mux(outstandingBurst >= vecSize.U, vecSize.U, outstandingBurst)
    io.user_out.bits := userOutData
    io.user_out.valid := repackedIO.valid
    repackedIO.ready := io.user_out.ready
    when(repackedIO.fire) {
	    val newBurst = Mux(outstandingBurst > vecSize.U, outstandingBurst - vecSize.U, 0.U)
	    //if sent the last chunk read new one
      when(newBurst === 0.U && burstTrackingQ.io.deq.valid) {
        outstandingBurst := burstTrackingQ.io.deq.deq
	    }.otherwise {
	      outstandingBurst := newBurst
	    }
    }
  }.otherwise {
    repackedIO.ready := false.B
    when(burstTrackingQ.io.deq.valid) {
      outstandingBurst := burstTrackingQ.io.deq.deq
	  }
  }
  accIn.io.mem_in <> io.mem_in
  accIn.io.mem_out <> io.mem_out
}

object AccInDriver extends App {
  import accio.DefaultAccParams._
  chisel3.Driver.execute(args, () => new AccIn(4))
}
