// See LICENSE for license details.
//
package dataflow
import chisel3._
import chisel3.util._
import scala.reflect.runtime.universe._
import scala.collection.mutable.HashMap


class FillableQueue [T <: Data] (gen: T, entries: Int, init_entries : Int, pipe: Boolean = false, flow: Boolean = false) extends Module {

  val io = IO(new QueueIO(gen, entries) {
    val fenq = Flipped(Decoupled(gen.cloneType))
  })
  
  val q = Module(new Queue(gen, entries, pipe, flow)) 
  
    
  val fillStage = RegInit(init = init_entries.U) 

  
  when (fillStage =/= 0.U) {
    fillStage := fillStage - 1.U
  }

  
  io.deq <> q.io.deq
  io.count <> q.io.count
  
  io.enq.ready := Mux (fillStage === 0.U, q.io.enq.ready, false.B)
  io.fenq.ready := Mux (fillStage =/= 0.U, q.io.enq.ready, false.B)
  
  q.io.enq.valid := Mux (fillStage === 0.U, io.enq.valid, io.fenq.valid)
  q.io.enq.bits := Mux (fillStage === 0.U, io.enq.bits, io.fenq.bits)
}

//*****************************************************************************
abstract class SDFActor (numInps : Int = 1, numOuts : Int = 1, latency : Int = 1) extends DataflowActor { 

  lazy val io : ActorIO = new ActorIO {}
  
  def func : Unit = {}
  
  lazy val ins = io.getInps
  lazy val outs = io.getOuts
  
  override def actLikeDataflow() { 
	  noActions()
	  when (fire()) {
		  actions()
		}
	}

  def fire(): Bool = {
    val otherOutReady = outs.map(out => (out, outs.foldLeft(true.B) {(allReady, nextOut) => if (out != nextOut) allReady && nextOut.ready else allReady})).toMap 
    val outsReady = outs.foldLeft(true.B) {(allReady, nextOut) => allReady && nextOut.ready}
	  val otherInValid = ins.map(in => (in, ins.foldLeft(true.B) {(allValid, nextIn) => if (in != nextIn) allValid && nextIn.valid else allValid})).toMap
	  val insValid = ins.foldLeft(true.B) {(allValid, nextIn) => allValid && nextIn.valid}
    ins.foreach(in => when (otherInValid(in)) {in.ready := outsReady} otherwise {in.ready := false.B})
    outs.foreach(out => when(otherOutReady(out)) {out.valid := insValid} otherwise {out.valid := false.B})
    outsReady&insValid
  }
	def init(): Unit = {}
	def actions(): Unit = {
    func
	}
	
	override def noActions(): Unit = {
	  ins.foreach(_.nodeq())
	  outs.foreach(_.noenq())	  
	}
	actLikeDataflow()
}

abstract class SDFActorWithState[T<:Data] (sgen: T, numInps : Int = 1, numOuts : Int = 1, latency : Int = 1) extends SDFActor(numInps, numOuts, latency) { 
  lazy val state = RegInit(sgen, init=0.U.asTypeOf(sgen))
}
//*****************************************************************************
