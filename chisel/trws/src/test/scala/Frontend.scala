package trws

import scala.util.Random

import chisel3._
import chisel3.util._
import chisel3.iotesters._

import org.scalatest.{Matchers, FlatSpec}


class FrontendTester( c: Frontend) extends StreamingPeekPokeTester(c) {

  //rnd.setSeed( 47L)

  val n = elements_per_cl*cl_per_row
  val n_slices = n*(n-1)/2

  val precision = 10 // 12 fails with radixPoint 16 8x3, 11 fails with radixPoint 10 8x3

  val GI = IndexedSeq.fill( 1, n){ BigInt( rnd.nextInt( 1<<precision))}
  val WI = IndexedSeq.fill( n-1, n){ BigInt( rnd.nextInt( 1<<precision)-(1<<(precision-1)))}

  val MI1 = IndexedSeq.fill( n_slices, n){
    BigInt( rnd.nextInt( 1<<precision))
  }

  val ADJ = IndexedSeq.tabulate( n-2, n){ (i,j) =>
    BigInt( rnd.nextInt( 1<<precision))
  }

  val A0 = IndexedSeq.tabulate( n-1){ (t) =>
    MI1.slice( index( t+1, t), index( n-1, t)+1).foldLeft( IndexedSeq.fill( n){BigInt(0)}){ (x,y) =>
      (x zip y) map { case (a,b) => a+b}
    }
  }

  def index( s : Integer, t : Integer) : Integer = {
    t*n-t*(t+1)/2 + s-(t+1)
  }

  val OFF = Array.fill( n_slices, n){ BigInt(0)}

  for ( t<-0 until n) {
    def wADJ( tt : Integer)( jj : Integer) : BigInt = {
      if ( tt == 0) 0 else ADJ(tt-1)(jj)
    }
    for ( s<-t+1 until n) {
      val k = index( s, t)
      for ( j<-0 until n) {
        OFF(k)(j) = ((GI(0)(s)*(A0(t)(j) + wADJ(t)(j) + WI(t)(j)))>>radixPoint) - MI1(k)(j)
      }
    }
  }

  val OFFcl = toCL( (OFF map {_.toIndexedSeq}).toIndexedSeq)

  def toUInt16( x : BigInt) : BigInt = {
    var xx = x
    while ( xx < 0) {
      xx += (1<<16)
    }
    xx % (1<<16)
  }

  poke( c.io.start, 1)

  val gi_sender = new Sender( "gi", c.io.gi, toCL( GI))
  val wi_sender = new Sender( "wi", c.io.wi, toCL( WI))
  val mi1_sender = new Sender( "mi1", c.io.mi1, toCL( MI1))
  val a0_sender = new Sender( "a0", c.io.a0, toCL( A0))
  val adj_sender = new Sender( "adj", c.io.adj, toCL( ADJ))

  val off_receiver = new Receiver( "off", c.io.off, OFFcl)

  var nsteps = 0

  while ( !off_receiver.done) {
// All pokes
    gi_sender.doPoke
    wi_sender.doPoke
    mi1_sender.doPoke
    a0_sender.doPoke
    adj_sender.doPoke
    off_receiver.doPoke
// All decisions
    gi_sender.doFire( nsteps)
    wi_sender.doFire( nsteps)
    mi1_sender.doFire( nsteps)
    a0_sender.doFire( nsteps)
    adj_sender.doFire( nsteps)
    off_receiver.doFire( nsteps)

    step(1)
    nsteps += 1
  }

  gi_sender.check
  wi_sender.check
  mi1_sender.check
  a0_sender.check
  adj_sender.check
  off_receiver.check

}

class FrontendTest extends GenericTest( "Frontend", () => new Frontend, (c:Frontend) => new FrontendTester( c))
