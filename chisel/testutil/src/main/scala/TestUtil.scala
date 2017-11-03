// See LICENSE for license details.
package testutil

import chisel3._
import chisel3.util._
import chisel3.iotesters._
import org.scalatest.{Matchers, FlatSpec}

import annotation.tailrec





//Streaming Tester using PeekPokeTester
abstract class DecoupledStreamingTestsUsingAdvTester[T <: Module](dut: T) extends PeekPokeTester(dut){

  def inDecoupledOp [P <: Data, V <: Any](instream: ((DecoupledIO[P],List[V]),Boolean))
  : (DecoupledIO[P],List[V]) ={
    val (( io, lst), delayed_ready) = instream
    if ( !lst.isEmpty && peek(io.valid)==BigInt(1) && delayed_ready) {
       //println("Drop 1 input")
       ( io, lst.tail)
    } else
       ( io, lst)
  }
  
  
  
  def inDecoupledPoke [P <: Data, V <: Any](instream: (DecoupledIO[P],List[V])){
    
    val ( io, lst) = instream
    if (!lst.isEmpty){
      poke( io.valid, 1)
      (io.bits, lst.head)  match{
        case (p: Bits, v: BigInt) => poke(p, v)
        case (p: Bundle, v: Map[String, BigInt]) => poke(p, v)
        case (p: Aggregate, v: IndexedSeq[BigInt]) => poke(p, v)
        case (_,_) => {println("Poke bad type") 
	               fail}
      }   
    } else {
      poke( io.valid, 0)
    }
  }
  
  def manOf[T: Manifest](t: T): Manifest[T] = manifest[T]
    
  def outDecoupledOp [P <: Data, V <: Any](outstream: (((DecoupledIO[P],List[V]), Boolean), Any))
  : (DecoupledIO[P],List[V]) ={
    
    val (((io, lst), delayed_valid), delayed_data) = outstream
    if ( !lst.isEmpty && delayed_valid && peek(io.ready) == BigInt(1)){
        (io.bits, delayed_data, lst.head)  match{
          case (p:Bits, pd: BigInt, v: BigInt) => if(pd!=v) fail
	  case (p:Bundle, pd: scala.collection.mutable.Map[String, BigInt], v: Map[String, BigInt]) => if(!pd.equals(v)) fail
	  case (p:Aggregate, pd: IndexedSeq[BigInt], v: IndexedSeq[BigInt]) => if(pd!=v) {
      println (s"pd = $pd not equal v = $v")
      fail  
    }
	  case (_,_,_) => {println("Expect bad type")
	               fail}
          //println(s"delayed_data: $delayed_data")
        }
	      //println("Drop 1 output")      
        ( io, lst.tail)
    } else
      ( io, lst)
  }
  
  def outDecoupledPoke [P <: Data, V <: Any](outstream: (DecoupledIO[P],List[V])){
    val ( io, lst) = outstream
    poke( io.ready, if (!lst.isEmpty) 1 else 0)
  }
 
  @tailrec
  final def testStreams [P <: Data, V <: Any]( m : Module,
                                               instreams : List[(DecoupledIO[P],List[V])],
                                               outstreams : List[(DecoupledIO[P],List[V])], 
                                               tickcounter : Int, NumMaxTicks: Int) : Unit = {

    if ( tickcounter == 0) reset(1)
    
    if ( instreams.forall{_._2.isEmpty} && outstreams.forall{_._2.isEmpty}){
      instreams.foreach { case (io,_) => poke(io.valid, false)}
      println(s"Finished all streams of the test")
    }else if(tickcounter >= NumMaxTicks){
      println(Console.RED+s"Test exceeded max tick counts: $NumMaxTicks"+Console.BLACK)
      fail
    }else{
      instreams.map{ inDecoupledPoke}
      outstreams.map{ outDecoupledPoke}

      // Want the values of the in channel ready signals before the step
      val inReadys = for( ( io, lst) <- instreams) yield peek(io.ready) == BigInt(1)
      val outValids = for( ( io, lst) <- outstreams) yield peek(io.valid) == BigInt(1)
      val outData = for( ( io, lst) <- outstreams) yield io.bits match { case p:Bits => peek(p) 
									 case p:Bundle => peek(p) 
									 case p:Aggregate => peek(p).reverse 
									 case _ => {println("Peek bad type") 
	                                                                           fail}
										   }
      									 
						       
      
      step(1)
      testStreams( m, (instreams zip inReadys).map{ inDecoupledOp}, (outstreams zip outValids zip outData).map{outDecoupledOp},
        tickcounter + 1, NumMaxTicks)
    }
  }
  
  def listToBigInt (WIDTH: Int, list: List[Int]):BigInt = {
    list.zipWithIndex.map{ case (v,idx) => (BigInt(v) << (WIDTH*idx))}.sum
  }
  
}
