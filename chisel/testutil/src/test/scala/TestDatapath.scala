// See LICENSE for license details.
package testutil

import chisel3._
import chisel3.util._
import chisel3.iotesters._
import scala.reflect.runtime.universe._
import scala.collection.mutable.HashMap
import org.scalatest.{Matchers, FlatSpec}


class ActorIO extends Bundle {

  var numOut = 0
  var numIn = 0
  val ratesIn = new HashMap[Int, Int] // TODO: consider making a list
  val ratesOut = new HashMap[Int, Int]

  def In[T <: Data](dataType: T, rate: Int) = { // TODO: reduce length? 
  	ratesIn(numIn) = rate
  	numIn = numIn + 1
  	Flipped(Decoupled(dataType))
  }

  def Out[T <: Data](dataType: T, rate: Int) = {
  	ratesOut(numOut) = rate
  	numOut = numOut + 1
  	Decoupled(dataType)
  }

  def getEls() = {
  	val i = scala.reflect.runtime.universe.runtimeMirror(this.getClass.getClassLoader).reflect(this)
  	val j = 0
  	//for (j <- 0 to numIn) {
		val sym = i.symbol.typeSignature.decls.toVector(j).asInstanceOf[reflect.runtime.universe.TermSymbol]
		val b: DecoupledIO[_] = i.reflectField(sym).get.asInstanceOf[DecoupledIO[_]]
		b.bits
	//}
  }
}
//*****************************************************************************
abstract class DataflowActor extends Module { // TODO : make trait

	def fire(): Bool
	def init(): Unit
	def actions(): Unit
	def noActions() {
		//printf(name + ": No action taken\n")
	}

	def actLikeDataflow() { 
		when (reset.toBool || ShiftRegister(reset.toBool, 2, true.B)) {
			init()
			noActions()
		} .otherwise {
			when (fire()) {
				actions()
			} .otherwise {
				noActions()
			}
		}
	}
}

class Datapath(N: Int, W: Int, Wout: Int, Mode: Int) extends DataflowActor {

  val io = IO(new ActorIO { 
  	val in0 = In(Vec(N, UInt(W.W)), 1)
  	val in1 = In(Vec(N, UInt(W.W)), 1) 
  	val out = Out(UInt(Wout.W), 1)
  	})

  override def fire() = {io.in0.valid & io.in1.valid & io.out.ready }
  override def init() = {
  }

  override def actions() = {
  	val in0Vec = io.in0.deq()
  	val in1Vec = io.in1.deq()
  	var outVal = 0.U // TODO: make a wire when mode is runtime param
  	if (Mode == 2) { // L2
		outVal = ((in0Vec zip in1Vec) map (v => ((v._1.zext -& v._2.zext) * (v._1.zext -& v._2.zext)).asUInt)).reduceLeft(_ +& _)
  	} else if (Mode == 1) { // L1
  		outVal = ((in0Vec zip in1Vec) map (v => (v._1.zext -& v._2.zext).abs().asUInt)).reduceLeft(_ +& _)
  	} else { // Linf
  	  	outVal = ((in0Vec zip in1Vec) map (v => (v._1.zext -& v._2.zext).abs().asUInt)).reduceLeft(_ max _)
  	}
  	io.out.enq(outVal)
  	printf(p"Saw $in0Vec and $in1Vec and produced output: $outVal\n")
  	//val els = io.getEls()
  	//printf(p"Signature is: $els\n")
  } 

  override def noActions() = {
  	io.in0.nodeq()
  	io.in1.nodeq()
  	io.out.noenq() // don't send anything
  	super.noActions()
  }

  actLikeDataflow() // todo: leverage early with super, early definition
}

class DecoupledDatapathTestsUsingAdvTester(dut: Datapath) extends DecoupledStreamingTestsUsingAdvTester(dut) {
  def VtoIndexedSeq(value: Int, N: Int): IndexedSeq[BigInt] = {
    (for (i <- 0 until N)
      yield BigInt(value)
    ).toIndexedSeq
  }
  
  val stream0 = List(VtoIndexedSeq(1,4),VtoIndexedSeq(2,4),VtoIndexedSeq(3,4))
  println(s"Stream0 is $stream0")
  val stream1 = List(VtoIndexedSeq(2,4),VtoIndexedSeq(4,4),VtoIndexedSeq(6,4))
  val instreams = List((dut.io.in0, stream0), (dut.io.in1,stream1))
  val outstreams = List((dut.io.out, List(BigInt(4), BigInt(16), BigInt(36))))
  testStreams (dut, instreams, outstreams, 0, 1000)
}


// *****************************************************************************
class DecoupledDatapathTesterVerilator  extends ChiselFlatSpec{
  "decoupled datapath" should "calculate L2 norms" in {
    chisel3.iotesters.Driver(() => new Datapath(4,8,44,2),"verilator"){ c =>
      new DecoupledDatapathTestsUsingAdvTester(c)
    }should be(true)
  }
}

object WithVCSBackend extends org.scalatest.Tag("WithVCSBackend")

class DecoupledDatapathTesterVCS  extends ChiselFlatSpec{
  "decoupled datapath" should "calculate L2 norms" taggedAs(WithVCSBackend) in {
    chisel3.iotesters.Driver(() => new Datapath(4,8,44,2),"vcs"){ c =>
      new DecoupledDatapathTestsUsingAdvTester(c)
    }should be(true)
  }
}

class DecoupledDatapathTesterFirrtl  extends ChiselFlatSpec{
  "decoupled datapath" should "calculate L2 norms" in {
    chisel3.iotesters.Driver(() => new Datapath(4,8,44,2),"firrtl"){ c =>
      new DecoupledDatapathTestsUsingAdvTester(c)
    }should be(true)
  }
}
