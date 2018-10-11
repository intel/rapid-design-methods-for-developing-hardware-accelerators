// See LICENSE for license details.
package accio

import chisel3._
import chisel3.util._
import scala.language.implicitConversions

//object Params {
//  val AddrWidth = 64
//  val CacheLineWidth = 512
//  
//  val DefMemTagWidth = 12
//  val DefBurstSizeWidth = 32
//}

abstract class AccParams {
  val AddrWidth : Int
  val CacheLineWidth : Int
  
  val DefMemTagWidth : Int
  val DefBurstSizeWidth : Int
}

object DefaultAccParams { 
  implicit val params = new AccParams {
    val AddrWidth = 64
    val CacheLineWidth = 64
    
    val DefMemTagWidth = 12
    val DefBurstSizeWidth = 32
  }
}


trait HasMetadata {
    val tag: UInt
}

class AccMemBurstRdReq()(implicit params : AccParams) extends Bundle {
  val addr = UInt(params.AddrWidth.W)
  val burstSize = UInt(params.DefBurstSizeWidth.W)
  
  def print() {
    printf("AccMemBurstRdReq addr = %x, burst size = %x \n", addr, burstSize)
  }
  override def cloneType = (new AccMemBurstRdReq).asInstanceOf[this.type]
}

object AccMemBurstRdReq {
  def apply (addr : UInt, burst : UInt)(implicit params : AccParams) = {
    val inst = Wire(new AccMemBurstRdReq)
    inst.addr := addr;
    inst.burstSize := burst
    inst
  }
}

class AccMemRdResp ()(implicit params : AccParams) extends Bundle {
  val data = UInt(params.CacheLineWidth.W)
 
  override def cloneType = (new AccMemRdResp).asInstanceOf[this.type]
}

object AccMemRdResp {
  def apply (data : UInt)(implicit params : AccParams) = {
    val inst = Wire(new AccMemRdResp)
    inst.data := data
    inst
  }
}

class AccMemUserRdResp[T<:Data](gen: T,vecSize: Int)(implicit params : AccParams) extends Bundle {
  val vec = Vec(vecSize, gen.cloneType)
  val cnt = UInt(vecSize.U.cloneType.getWidth.W)
  override def cloneType = (new AccMemUserRdResp(gen, vecSize)).asInstanceOf[this.type]
}

object AccMemUserRdResp {
  def apply[T<:Data] (vec : Vec[T], cnt: UInt)(implicit params : AccParams) = {
    val result = Wire(new AccMemUserRdResp(vec(0).cloneType, vec.length))
    result.vec := vec
    result.cnt := cnt
    result
  }
}

class MemRdReq ()(implicit params : AccParams) extends Bundle with HasMetadata {
  val addr = UInt(params.AddrWidth.W)
  val tag = UInt(params.DefMemTagWidth.W)
  
  def print() {
    printf("MemRdReq addr = %x, tag = %x \n", addr, tag)
  }

  override def cloneType = (new MemRdReq()).asInstanceOf[this.type]
}

object MemRdReq {
  def apply (addr : UInt, tag : UInt)(implicit params : AccParams) : MemRdReq = {
    val inst = Wire(new MemRdReq())
    inst.addr := addr
    inst.tag := tag
    inst
  }
}

class MemRdResp ()(implicit params : AccParams) extends Bundle with HasMetadata {
  val data = UInt(params.CacheLineWidth.W)
  val tag = UInt(params.DefMemTagWidth.W)
  override def cloneType = (new MemRdResp).asInstanceOf[this.type]
}

object MemRdResp {
  def apply (data : UInt, tag : UInt)(implicit params : AccParams) : MemRdResp = {
    val inst = Wire(new MemRdResp())
    inst.data := data
    inst.tag := tag
    inst
  }
}

class MemRdReqRespTuple (a : (DecoupledIO[MemRdReq], DecoupledIO[MemRdResp])) {
  def <> (b : (DecoupledIO[MemRdReq], DecoupledIO[MemRdResp])) {
    a._1 <> b._1
    a._2 <> b._2
  }
}

object MemRdReqRespTuple {
  implicit def convert_to_mem_rd_req_resp_tuple (a : (DecoupledIO[MemRdReq], DecoupledIO[MemRdResp])) = {
    new MemRdReqRespTuple(a)
  }
}

class AccMemBurstWrReq()(implicit params : AccParams) extends Bundle {
  val addr = UInt(params.AddrWidth.W)
  val burstSize = UInt(params.DefBurstSizeWidth.W)

  def print() {
    printf("AccMemBurstWrReq addr = %x, burst size = %x \n", addr, burstSize)
  }
  override def cloneType = (new AccMemBurstWrReq).asInstanceOf[this.type]
}


object AccMemBurstWrReq {
  def apply (addr : UInt, burst : UInt) (implicit params : AccParams) = {
    val inst = Wire(new AccMemBurstWrReq)
    inst.addr := addr;
    inst.burstSize := burst
    inst
  }
}


class AccMemWrData ()(implicit params : AccParams) extends Bundle {
  val data = UInt(params.CacheLineWidth.W)

  def print() {
    printf("AccMemBurstWrData data = %x, burst size = %x \n", data.asUInt())
  }
  override def cloneType = (new AccMemWrData).asInstanceOf[this.type]
}

object AccMemWrData {
  def apply (data : UInt)(implicit params : AccParams) = {
    val inst = Wire(new AccMemWrData)
    inst.data := data
    inst
  }
  
}

class MemWrReq ()(implicit params : AccParams) extends Bundle with HasMetadata {
  val addr = UInt(params.AddrWidth.W)
  val data = UInt(params.CacheLineWidth.W)
  val tag = UInt(params.DefMemTagWidth.W)

  def print() {
    printf("MemWrReq addr = %d, data = %x, tag = %d \n", addr, data, tag)
  }
    
  override def cloneType = (new MemWrReq).asInstanceOf[this.type]
}

object MemWrReq {
  def apply (addr : UInt, data : UInt, tag : UInt)(implicit params : AccParams) = {
    val inst = Wire(new MemWrReq)
    inst.addr := addr
    inst.data := data
    inst.tag := tag
    inst
  }
}

class MemWrResp ()(implicit params : AccParams) extends Bundle with HasMetadata {
  val tag = UInt(params.DefMemTagWidth.W)

  override def cloneType = (new MemWrResp).asInstanceOf[this.type]
}

class MemWrReqRespTuple (a : (DecoupledIO[MemWrReq], DecoupledIO[MemWrResp])) {
  def <> (b : (DecoupledIO[MemWrReq], DecoupledIO[MemWrResp])) {
    a._1 <> b._1
    a._2 <> b._2
  }
}

object MemWrReqRespTuple {
  implicit def convert_to_mem_wr_req_resp_tuple (a : (DecoupledIO[MemWrReq], DecoupledIO[MemWrResp])) = {
    new MemWrReqRespTuple(a)
  }
}

object MemWrResp {
  def apply(tag : UInt)(implicit params : AccParams) = {
    val inst = Wire(new MemWrResp)
    inst.tag := tag
    inst
  }  
}


class ValidToken [T<:Data](gen : T) extends Bundle {
  val valid = Bool()
  val token = gen.cloneType
  
  def reset() = {
    valid := false.B
    token := DontCare
  }
  
  def set(t : T) = {
    valid := true.B
    token := t
  }
  
  override def cloneType = (new ValidToken(gen)).asInstanceOf[this.type]
}

trait PackedWithCount {
  def packed: Data
  def count: UInt
}


object InvalidToken {
  def apply[T<:Data] (token : T) : ValidToken[T] = {
    val inst = Wire(new ValidToken(token))
    inst.reset
    inst
  }
}


class MemRdReqGen[T<:Data, CONFT<:Data] (confGen: CONFT, inGen: T, genFunc: (CONFT, T) => AccMemBurstRdReq)(implicit params: AccParams) extends Module {
  val io = IO(new Bundle {
    val config = Input(confGen.cloneType)
    val in = Flipped(Decoupled(inGen.cloneType))
    val req = Decoupled(new AccMemBurstRdReq)
  })
  
  io.req.valid := io.in.valid
  io.in.ready := io.req.ready
  io.req.bits := genFunc(io.config, io.in.bits)
}

object MemRdReqGen {
  def apply[T<:Data, CONFT<:Data](conf: CONFT, genFunc: (CONFT, T) => AccMemBurstRdReq, in: DecoupledIO[T])(implicit params: AccParams): DecoupledIO[AccMemBurstRdReq] = {
    val mod = Module(new MemRdReqGen(conf, in.bits.cloneType, genFunc))
    mod.io.config := conf
    mod.io.in <> in
    mod.io.req
  }
}

class MemRdReqGenSingle[T<:Data, CONFT<:Data] (confGen: CONFT, genFunc: (CONFT) => AccMemBurstRdReq)(implicit params: AccParams) extends Module {
  val io = IO(new Bundle {
    val start = Input(Bool())
    val config = Input(confGen.cloneType)
    val req = Decoupled(new AccMemBurstRdReq)
  })
  
  val isSent = RegInit(init = false.B)
  
  io.req.noenq
  when (io.start) {
    io.req.valid := !isSent 
    isSent := isSent || io.req.ready
    io.req.bits := genFunc(io.config)
  }
}

object MemRdReqGenSingle {
  def apply[CONFT<:Data](conf: CONFT, genFunc: (CONFT) => AccMemBurstRdReq, start: Bool = true.B)(implicit params: AccParams): DecoupledIO[AccMemBurstRdReq] = {
    val mod = Module(new MemRdReqGenSingle(conf, genFunc))
    mod.io.config := conf
    mod.io.start := start
    mod.io.req
  }
}

class MemWrReqGen[T<:Data, CONFT<:Data] (confGen: CONFT, inGen: T, genFunc: (CONFT, T) => AccMemBurstWrReq)(implicit params: AccParams) extends Module {
  val io = IO(new Bundle {
    val config = Input(confGen.cloneType)
    val in = Flipped(Decoupled(inGen.cloneType))
    val req = Decoupled(new AccMemBurstWrReq)
  })
  
  io.req.valid := io.in.valid
  io.in.ready := io.req.ready
  io.req.bits := genFunc(io.config, io.in.bits)
}

object MemWrReqGen {
  def apply[T<:Data, CONFT<:Data](conf: CONFT, genFunc: (CONFT, T) => AccMemBurstWrReq, in: DecoupledIO[T])(implicit params: AccParams): DecoupledIO[AccMemBurstWrReq] = {
    val mod = Module(new MemWrReqGen(conf, in.bits.cloneType, genFunc))
    mod.io.config := conf
    mod.io.in <> in
    mod.io.req
  }
}

class MemWrReqGenSingle[T<:Data, CONFT<:Data] (confGen: CONFT, genFunc: (CONFT) => AccMemBurstWrReq)(implicit params: AccParams) extends Module {
  val io = IO(new Bundle {
    val start = Input(Bool())
    val config = Input(confGen.cloneType)
    val req = Decoupled(new AccMemBurstRdReq)
  })
  
  val isSent = RegInit(init = false.B)
  
  io.req.noenq
  when (io.start) {
    io.req.valid := !isSent
    isSent := io.req.ready
    io.req.bits := genFunc(io.config)
  }
}

object MemWrReqGenSingle {
  def apply[CONFT<:Data](conf: CONFT, genFunc: (CONFT) => AccMemBurstWrReq, start: Bool = true.B)(implicit params: AccParams): DecoupledIO[AccMemBurstRdReq] = {
    val mod = Module(new MemWrReqGenSingle(conf, genFunc))
    mod.io.config := conf
    mod.io.start := start
    mod.io.req
  }
}

object CacheLine {
  def numCLsFor[T<:Data](gen: T, size: UInt)(implicit params: AccParams) = {
    if(gen.getWidth <= params.CacheLineWidth && params.CacheLineWidth%gen.getWidth == 0) {
      val rem = size % (params.CacheLineWidth/gen.getWidth).U
      size / (params.CacheLineWidth/gen.getWidth).U + Mux(rem != 0.U,1.U,0.U) 
    } else {
      assert(false, "Not supported")
      size
    }
  }
}

trait AccioDebug {
  var DEBUG_PRINT = false
}
trait AccioDebugDisable {
  var DEBUG_PRINT = false
}
trait AccioDebugEnable {
  var DEBUG_PRINT = true
}
