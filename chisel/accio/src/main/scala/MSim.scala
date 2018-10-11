// See LICENSE for license details.
package accio

import chisel3._
import chisel3.util._
import chisel3.iotesters._

import scala.collection.mutable.ArrayBuffer
import scala.collection.mutable.PriorityQueue

trait MSim {
  case class ReadToken(addr: BigInt, tag: BigInt)
  lazy val backedStorage: ArrayBuffer[BigInt] = {
    val inst = new ArrayBuffer[BigInt](initialSize = memSize)
    for (i <- 0 until memSize) {
      inst += BigInt("deadbeef"*16, 16)
    }
    inst
  }
  
  def storage: ArrayBuffer[BigInt] = backedStorage
  def readQueueOrder(timedResp: (Long, ReadToken)) = -timedResp._1
  val readQueue = PriorityQueue[(Long,ReadToken)]()(Ordering.by(readQueueOrder))
  var readCount: Long = 0
  var writeCount: Long = 0
  
  def memStep(rdReq: Option[DecoupledIO[MemRdReq]], 
              rdRsp: Option[DecoupledIO[MemRdResp]], 
              wrReq: Option[DecoupledIO[MemWrReq]], 
              wrRsp: Option[DecoupledIO[MemWrResp]]) = {
    
    //handling read request
    if (!rdReq.isEmpty) {
      if (readQueue.length < reqBufferSize) 
        poke(rdReq.get.ready, 1)
      else 
        poke(rdReq.get.ready, 0)
       
      if(peek(rdReq.get.valid) == 1) {
        require(peek(rdReq.get.bits.addr) < storage.length)
        val readToken = ReadToken(peek(rdReq.get.bits.addr), peek(rdReq.get.bits.tag))
        //println(s"readQueue push ${memLatency.toLong} + $t -> $readToken")
        readQueue.enqueue((memLatency.toLong + t, readToken))
      }
    }
    
    // handling read response
    if (peek(rdRsp.get.ready) == 1 && peek(rdRsp.get.valid) == 1) {
      readQueue.dequeue
      readCount = readCount + 1
    }
    poke(rdRsp.get.valid, 0)
    if(!rdRsp.isEmpty) {
      if (readQueue.length > 0) {
        if (t >= readQueue.head._1) {
          val head = readQueue.head
          poke(rdRsp.get.bits.tag, head._2.tag)
          poke(rdRsp.get.bits.data, storage(head._2.addr.toInt))
          poke(rdRsp.get.valid, 1) 
        } 
      }
    }
    
    //handling write request
    if (!wrReq.isEmpty && peek(wrReq.get.valid) == 1) {
      val addr = peek(wrReq.get.bits.addr)
      val data = peek(wrReq.get.bits.data)
      val tag = peek(wrReq.get.bits.tag)
      storage(addr.toInt) = data
      writeCount = writeCount + 1
    }    
  }
  // user to implement
  def reqBufferSize: Int
  def memSize: Int
  def memLatency: Int
  // PeekPokeTester implements the following
  def poke(signal: Bits, x: BigInt): Unit
  def peek(signal: Bits): BigInt  
  def t: Long
}

