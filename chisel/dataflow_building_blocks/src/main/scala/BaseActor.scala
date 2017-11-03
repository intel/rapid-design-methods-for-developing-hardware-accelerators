// See LICENSE for license details.
//
package dataflow
import chisel3._
import chisel3.util._
import scala.reflect.runtime.universe._
import scala.collection.mutable.ListBuffer
import chisel3.core.DataMirror
import chisel3.core.ActualDirection

//*****************************************************************************
class ActorIO extends Bundle {

  var ratesIn = new ListBuffer[Int]()
  var ratesOut = new ListBuffer[Int]()

  def In[T <: Data](dataType: T, rate: Int = 0) = {
  	ratesIn += rate
  	Flipped(Decoupled(dataType))
  }

  def Out[T <: Data](dataType: T, rate: Int = 0) = {
  	ratesOut += rate
  	Decoupled(dataType)
  }

  def RuntimeParamIn[T <: Data](dataType: T) = {
	Input(dataType)
  }

  def getIOs() = elements flatMap {
    case (_ : Any , elem : DecoupledIO[_]) => Some(elem) 
    case (_, v : Vec[_])  => v flatMap {
      case elem : DecoupledIO[_] => Some(elem)
    }
    case _ => None
    
  }

  def getInps = {
    getIOs.filter(x => DataMirror.directionOf(x.valid) == ActualDirection.Input).toList
  }
  def getOuts = {
    getIOs.filter(x => DataMirror.directionOf(x.valid) == ActualDirection.Output).toList
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
	  noActions()
		when (reset.toBool || ShiftRegister(reset.toBool, 2, true.B)) {
			init()
		} .otherwise {
			when (fire()) {
				actions()
			}
		}
	}
  def print() {
    
  }
}

//*****************************************************************************
