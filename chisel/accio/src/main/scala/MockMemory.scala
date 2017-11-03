// See LICENSE for license details.
package accio

import chisel3._
import chisel3.util._
import chisel3.iotesters._
import testutil._
import designutils.DecoupledStage
  
import org.scalatest.{Matchers, FlatSpec}
import designutils.LatencyPipe
import designutils.DebugMacros._

class MockMemoryComb (clSize : Int = 128, mem_init_function: Option[UInt => UInt], testPushBack : Boolean = false)(implicit params : AccParams) extends Module with AccioDebug
{ 
  val io = IO(new Bundle {
    val mem_rd_in  = Flipped(Decoupled( new MemRdReq))
    val mem_rd_out = Decoupled( new MemRdResp)
    val mem_wr_in  = Flipped(Decoupled( new MemWrReq))
    val mem_wr_out = Decoupled( new MemWrResp)
    val mem_probe = Input((clSize-1).U.cloneType)
    val mem_monitor = Output(UInt(params.CacheLineWidth.W))
  })
  val s_init :: s_main :: Nil = Enum(2)
  val s_state = RegInit(init = if (mem_init_function.isDefined) s_init else s_main)

  val initCnt = Counter(s_state === s_init, clSize)
  
  //val clines = RegInit(init = Vec.tabulate (clSize) { mem_init_function })
  val clines = Mem(clSize,UInt(params.CacheLineWidth.W))

  io.mem_rd_in.nodeq()
  io.mem_rd_out.noenq()
  io.mem_wr_in.nodeq()
  io.mem_wr_out.noenq()
  
  io.mem_monitor := clines(io.mem_probe)

  val init_index = initCnt._1
  when (s_state === s_init) {
    clines.write(init_index, mem_init_function.getOrElse((v: UInt)=>0.U)(init_index))
    when (init_index === (clSize - 1).U) {
      s_state := s_main
    }
  }
  
  when(s_state === s_main) {

    val cnt = Counter(8)
    cnt.inc()
    val out_p = cnt.value 
    
    //io.mem_rd_in.ready := io.mem_rd_out.ready
    //io.mem_rd_in.ready := Wire(init = !io.mem_rd_in.valid || io.mem_rd_out.ready)
    //io.mem_wr_in.ready := Wire(init = !io.mem_wr_in.valid ||  !(cnt.value % 4.U) && io.mem_wr_out.ready)
  
    when (!testPushBack.B || !(cnt.value % 3.U)) {
      val w_mem_rd_req = Wire(init = io.mem_rd_in.bits)
      when (io.mem_rd_out.ready) {
        w_mem_rd_req := io.mem_rd_in.deq     //just setting ready to true (bits were taken before)
      }
      when (io.mem_rd_in.valid) {
        pf("MockMemory read: mem_req.addr = %d data = %x tag = %d\n",w_mem_rd_req.addr, clines(w_mem_rd_req.addr), w_mem_rd_req.tag)
        io.mem_rd_out.enq(MemRdResp(clines(w_mem_rd_req.addr), w_mem_rd_req.tag))
      }
    }
    
    when (!testPushBack.B || !(cnt.value % 4.U)) {
      val w_mem_wr_in = Wire(init = io.mem_wr_in.bits)
      when (io.mem_wr_out.ready) {
        w_mem_wr_in := io.mem_wr_in.deq
      }
      when (io.mem_wr_in.valid) {
        io.mem_wr_out.enq(MemWrResp(w_mem_wr_in.tag))
        when(io.mem_wr_out.ready) {    
          pf("MockMemory write: mem_req.addr = %d data = %x tag = %d\n",w_mem_wr_in.addr, w_mem_wr_in.data, w_mem_wr_in.tag)
          assert(w_mem_wr_in.addr < clSize.U)
          clines(w_mem_wr_in.addr) := w_mem_wr_in.data
        }
      }
    }
  }
} 

class MockMemoryPerfCounters extends Bundle {
	val read_idle_cycles = UInt(64.W)
  val write_idle_cycles = UInt(64.W)
	val total_cycles = UInt(64.W)

	override def cloneType = (new MockMemoryPerfCounters).asInstanceOf[this.type]
}

class MockMemory(val clSize : Int = 128, mem_init_function : Option[UInt => UInt] = None, latency : Int = 2, testPushBack : Boolean = false)(implicit params : AccParams) extends Module with AccioDebug {
  val combMem = Module(new MockMemoryComb (clSize, mem_init_function, testPushBack)) 
  val io = IO(new Bundle {
    val mem_rd_in  = Flipped(Decoupled( new MemRdReq ))
    val mem_rd_out = Decoupled( new MemRdResp)
    val mem_wr_in  = Flipped(Decoupled( new MemWrReq))
    val mem_wr_out = Decoupled( new MemWrResp)
    val mem_probe = Input((clSize-1).U.cloneType)
    val mem_monitor = Output(UInt(params.CacheLineWidth.W))
    val perf_monitor = Output(new MockMemoryPerfCounters)
  })

  io.mem_rd_out <> LatencyPipe(combMem.io.mem_rd_out, latency)
  io.mem_rd_in <> combMem.io.mem_rd_in
  io.mem_wr_out <> LatencyPipe(combMem.io.mem_wr_out, latency)
  io.mem_wr_in <> combMem.io.mem_wr_in
  io.mem_probe <> combMem.io.mem_probe
  io.mem_monitor <> combMem.io.mem_monitor
  
  val firstRWCycle = RegInit(init = 0.U(64.W))
  val lastRWCycle = RegInit(init = 0.U(64.W))

  val rdNonFirings = Counter(!io.mem_rd_out.fire, 1 << 30)
  val wrNonFirings = Counter(!io.mem_wr_out.fire, 1 << 30)
  val clkFirings = Counter(true.B, 1<<30)
  
  when((io.mem_rd_out.fire || io.mem_wr_out.fire)) {
    lastRWCycle := clkFirings._1
  }
  
  when(firstRWCycle === 0.U && (io.mem_rd_in.fire || io.mem_wr_in.fire)) {
    firstRWCycle := clkFirings._1
  }
  
  io.perf_monitor.read_idle_cycles := rdNonFirings._1 - (clkFirings._1 - (lastRWCycle - firstRWCycle))
  io.perf_monitor.write_idle_cycles := wrNonFirings._1 - (clkFirings._1 - (lastRWCycle - firstRWCycle))
  io.perf_monitor.total_cycles := lastRWCycle - firstRWCycle

}
