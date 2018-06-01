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

  val clalignFactor = log2Ceil(params.CacheLineWidth>>3)
  for (i <- 0 until mod.numReadPorts) {  
    mod.io.mem_rd_resp(i).bits.data := io.spl_rd_resp(i).bits(511,0) 
    mod.io.mem_rd_resp(i).bits.tag := io.spl_rd_resp(i).bits(527,512) 
    io.spl_rd_resp(i).valid <> mod.io.mem_rd_resp(i).valid
    io.spl_rd_resp(i).ready <> mod.io.mem_rd_resp(i).ready
    assert(mod.io.mem_rd_req(i).bits.addr(63, 56) === 0.U, "the address should be in cachelines, so msb should be zeros")
    io.spl_rd_req(i).bits := mod.io.mem_rd_req(i).bits.tag ## ((mod.io.mem_rd_req(i).bits.addr << clalignFactor)(63,0))
    io.spl_rd_req(i).valid <> mod.io.mem_rd_req(i).valid
    io.spl_rd_req(i).ready <> mod.io.mem_rd_req(i).ready
  }  
  for (i <- 0 until mod.numWritePorts) {  
    assert(mod.io.mem_wr_req(i).bits.addr(63, 56) === 0.U, "the address should be in cachelines, so msb should be zeros")
  
    mod.io.mem_wr_resp(i).bits.tag := io.spl_wr_resp(i).bits(16,1)
    io.spl_wr_resp(i).valid <> mod.io.mem_wr_resp(i).valid
    io.spl_wr_resp(i).ready <> mod.io.mem_wr_resp(i).ready
    
    io.spl_wr_req(i).bits := mod.io.mem_wr_req(i).bits.tag ## 0.U(14.W) ## mod.io.mem_wr_req(i).bits.data ## ((mod.io.mem_wr_req(i).bits.addr << clalignFactor)(63,0))    
    io.spl_wr_req(i).valid <> mod.io.mem_wr_req(i).valid
    io.spl_wr_req(i).ready <> mod.io.mem_wr_req(i).ready
  } 
}

object HldAccDriver extends App {
  import HldAccParams._
  chisel3.Driver.execute(args, () => new HldAcceleratorMultiPortWrapper(()=>new HldAcceleratorMultiPortIO(new Bundle{val foo = UInt(4.W)}, 4, 4)))
}
