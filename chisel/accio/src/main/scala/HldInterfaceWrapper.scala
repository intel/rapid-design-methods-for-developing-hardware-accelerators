// See LICENSE for license details.
package hld_interface_wrapper

import chisel3._
import chisel3.util._
import accio._

// one need to use these parameters for hld accelerator
object HldAccParams { 
  implicit val params = new AccParams {
    val AddrWidth = 64
    val CacheLineWidth = 512
    val DefMemTagWidth = 16
    val DefBurstSizeWidth = 32
  }
}
// one needs to have their accelerator derive from this IO to help us wrap it around for RTL integration
class HldAcceleratorIO[T<:Data](conf: T)(implicit params: AccParams) extends Module {
  val io=IO(new Bundle {
    val config = Input(conf.cloneType)
    val start = Input(Bool())
    val done = Output(Bool())
  
    val mem_rd_resp = Flipped(Decoupled(new MemRdResp))
    val mem_rd_req = Decoupled(new MemRdReq)
    val mem_wr_resp = Flipped(Decoupled(new MemWrResp))
    val mem_wr_req = Decoupled(new MemWrReq)
  })    
}

// produces the right interface and connections for RTL integration with HLD FPGA methodology 
class HldAcceleratorWrapper[T<:Data] (dutGen: ()=>HldAcceleratorIO[T]) extends Module {
  import HldAccParams._
  val io = IO(new Bundle{
    val config = Input(UInt(320.W))
    val start = Input(Bool())
    val done = Output(Bool())
  
    val spl_rd_resp = Flipped(Decoupled(UInt(528.W)))
    val spl_rd_req = Decoupled(UInt(80.W))
    val spl_wr_resp = Flipped(Decoupled(UInt(17.W)))
    val spl_wr_req = Decoupled(UInt(606.W))
  })  
  
  val mod = Module(dutGen())
  
  require (mod.io.config.getWidth.toInt <= 320, s"Config width should be under 320 bits but ${mod.io.config.getWidth.toInt} is detected")
  require (mod.io.mem_rd_req.bits.getWidth == (new MemRdReq).getWidth, s"mod.io.mem_rd_req.getWidth = ${mod.io.mem_rd_req.getWidth} not equal to ${(new MemRdReq).getWidth}")
  require (mod.io.mem_wr_req.bits.getWidth == (new MemWrReq).getWidth)
  require (mod.io.mem_rd_resp.bits.getWidth == (new MemRdResp).getWidth)
  require (mod.io.mem_wr_resp.bits.getWidth == (new MemWrResp).getWidth)
  
  mod.io.config := io.config(mod.io.config.getWidth - 1, 0).asTypeOf(mod.io.config.cloneType)
  io.start <> mod.io.start
  io.done <> mod.io.done

  mod.io.mem_rd_resp.bits.data := io.spl_rd_resp.bits(511,0) 
  mod.io.mem_rd_resp.bits.tag := io.spl_rd_resp.bits(527,512) 
  io.spl_rd_resp.valid <> mod.io.mem_rd_resp.valid
  io.spl_rd_resp.ready <> mod.io.mem_rd_resp.ready
  
  val clalignFactor = log2Ceil(params.CacheLineWidth>>3)
  assert(mod.io.mem_rd_req.bits.addr(63, 56) === 0.U, "the address should be in cachelines, so msb should be zeros")
  assert(mod.io.mem_wr_req.bits.addr(63, 56) === 0.U, "the address should be in cachelines, so msb should be zeros")
  io.spl_rd_req.bits := mod.io.mem_rd_req.bits.tag ## ((mod.io.mem_rd_req.bits.addr << clalignFactor)(63,0))
  io.spl_rd_req.valid <> mod.io.mem_rd_req.valid
  io.spl_rd_req.ready <> mod.io.mem_rd_req.ready
  
  mod.io.mem_wr_resp.bits.tag := io.spl_wr_resp.bits(16,1)
  io.spl_wr_resp.valid <> mod.io.mem_wr_resp.valid
  io.spl_wr_resp.ready <> mod.io.mem_wr_resp.ready
  
  io.spl_wr_req.bits := mod.io.mem_wr_req.bits.tag ## 0.U(14.W) ## mod.io.mem_wr_req.bits.data ## ((mod.io.mem_wr_req.bits.addr << clalignFactor)(63,0))    
  io.spl_wr_req.valid <> mod.io.mem_wr_req.valid
  io.spl_wr_req.ready <> mod.io.mem_wr_req.ready
  
}


// one needs to have their accelerator derive from this IO to help us wrap it around for RTL integration
class HldAcceleratorMultiPortIO[T<:Data](conf: T, val numReadPorts: Int, val numWritePorts: Int)(implicit params: AccParams) extends Module {
  val io=IO(new Bundle {
    val config = Input(conf.cloneType)
    val start = Input(Bool())
    val done = Output(Bool())
  
    val mem_rd_resp = Vec(numReadPorts, Flipped(Decoupled(new MemRdResp)))
    val mem_rd_req = Vec(numReadPorts, Decoupled(new MemRdReq))
    val mem_wr_resp = Vec(numWritePorts, Flipped(Decoupled(new MemWrResp)))
    val mem_wr_req = Vec(numWritePorts, Decoupled(new MemWrReq))
  })    
}

// produces the right interface and connections for RTL integration with HLD FPGA methodology 
class HldAcceleratorMultiPortWrapper[T<:Data] (dutGen: ()=>HldAcceleratorMultiPortIO[T], name: String = "HldAcceleratorMultiPortWrapper") extends Module {
  import HldAccParams._

  val mod = Module(dutGen())

  val io = IO(new Bundle{
    val config = Input(UInt(320.W))
    val start = Input(Bool())
    val done = Output(Bool())
  
    val spl_rd_resp = Vec(mod.numReadPorts, Flipped(Decoupled(UInt(528.W))))
    val spl_rd_req = Vec(mod.numReadPorts, Decoupled(UInt(80.W)))
    val spl_wr_resp = Vec(mod.numWritePorts, Flipped(Decoupled(UInt(17.W))))
    val spl_wr_req = Vec(mod.numWritePorts, Decoupled(UInt(606.W)))
  })  

  require (mod.io.config.getWidth.toInt <= 320, s"Config width should be under 320 bits but ${mod.io.config.getWidth.toInt} is detected")
  for (i <- 0 until mod.numReadPorts) {  
    require (mod.io.mem_rd_req(i).bits.getWidth == (new MemRdReq).getWidth, s"mod.io.mem_rd_req.getWidth = ${mod.io.mem_rd_req.getWidth} not equal to ${(new MemRdReq).getWidth}")
    require (mod.io.mem_rd_resp(i).bits.getWidth == (new MemRdResp).getWidth)
  }
  for (i <- 0 until mod.numWritePorts) {  
    require (mod.io.mem_wr_req(i).bits.getWidth == (new MemWrReq).getWidth)
    require (mod.io.mem_wr_resp(i).bits.getWidth == (new MemWrResp).getWidth)
  }
  mod.io.config := io.config(mod.io.config.getWidth - 1, 0).asTypeOf(mod.io.config.cloneType)
  io.start <> mod.io.start
  io.done <> mod.io.done

  def tagToSpl(tag: UInt, auid: Int, auCnt: Int) = {
    // spl tag: (15,7)(6,0)
    //    ld/st tag^    ^auid+memarbid
    // (6,0) are shared between auid and memarbid, with memarbid taking LSBs and auid are MSB
    // chisel tag (15,0) - shared ld/st tag and memarb with ld/st tag is LSBs and right after it there's memarbid
    // thus chisel tag(8,0) should go to splTag(15,7) and the rest tag(15,9) to splTag(6,0)
    // then, MSB of splTag(6,0) gets auid, based of log2(auid), for 4 AUs, splTag(6,5)=auid
 log2Ceil(auCnt)   val splTag = Wire(UInt(params.DefMemTagWidth.W))
    //splTag(15, 7) := tag(8,0)
    //splTag(6,0) := tag(15,9)
    //splTag(6, 6-log2Ceil(auCnt)+1) := auid.U
    splTag := tag(8,0) ## auid.U(log2Ceil(auCnt).W) ## tag(15-log2Ceil(auCnt),9)
    splTag
  }
  def splToTag(splTag: UInt, auCnt: Int) = {
    val tag = Wire(UInt(params.DefMemTagWidth.W))
    //tag(8,0) := splTag(15, 7)
    //tag(15-log2Ceil(auCnt),9) := splTag(6-log2Ceil(auCnt),0) // dropping auid
    //tag(15, 15-log2Ceil(auCnt)+1) := 0.U
    tag := 0.U(log2Ceil(auCnt).W)##splTag(6-log2Ceil(auCnt),0)##splTag(15, 7)
    tag
  }
  
  val clalignFactor = log2Ceil(params.CacheLineWidth>>3)
  for (i <- 0 until mod.numReadPorts) {  
    mod.io.mem_rd_resp(i).bits.data := io.spl_rd_resp(i).bits(511,0) 
    //AA: support for multiported AUs. need to modify the tag to add id for port number
    mod.io.mem_rd_resp(i).bits.tag := splToTag(io.spl_rd_resp(i).bits(527,512), mod.numReadPorts)
    io.spl_rd_resp(i).valid <> mod.io.mem_rd_resp(i).valid
    io.spl_rd_resp(i).ready <> mod.io.mem_rd_resp(i).ready
    assert(mod.io.mem_rd_req(i).bits.addr(63, 56) === 0.U, "the address should be in cachelines, so msb should be zeros")
    //AA: support for multiported AUs. need to modify the tag to add id for port number
    io.spl_rd_req(i).bits := tagToSpl(mod.io.mem_rd_req(i).bits.tag, i, mod.numReadPorts) ## ((mod.io.mem_rd_req(i).bits.addr << clalignFactor)(63,0))
    io.spl_rd_req(i).valid <> mod.io.mem_rd_req(i).valid
    io.spl_rd_req(i).ready <> mod.io.mem_rd_req(i).ready
  }  
  for (i <- 0 until mod.numWritePorts) {  
    assert(mod.io.mem_wr_req(i).bits.addr(63, 56) === 0.U, "the address should be in cachelines, so msb should be zeros")
  
    //AA: support for multiported AUs. need to modify the tag to add id for port number
    mod.io.mem_wr_resp(i).bits.tag := splToTag(io.spl_wr_resp(i).bits(16,1), mod.numWritePorts)
    io.spl_wr_resp(i).valid <> mod.io.mem_wr_resp(i).valid
    io.spl_wr_resp(i).ready <> mod.io.mem_wr_resp(i).ready
    //AA: support for multiported AUs. need to modify the tag to add id for port number
    io.spl_wr_req(i).bits := tagToSpl(mod.io.mem_wr_req(i).bits.tag, i, mod.numWritePorts) ## 0.U(14.W) ## mod.io.mem_wr_req(i).bits.data ## ((mod.io.mem_wr_req(i).bits.addr << clalignFactor)(63,0))    
    io.spl_wr_req(i).valid <> mod.io.mem_wr_req(i).valid
    io.spl_wr_req(i).ready <> mod.io.mem_wr_req(i).ready
  } 
}

object HldAccDriver extends App {
  import HldAccParams._
  chisel3.Driver.execute(args, () => new HldAcceleratorMultiPortWrapper(()=>new HldAcceleratorMultiPortIO(new Bundle{val foo = UInt(4.W)}, 4, 4)))
}
