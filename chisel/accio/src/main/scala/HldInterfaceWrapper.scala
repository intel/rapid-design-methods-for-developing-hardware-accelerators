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

