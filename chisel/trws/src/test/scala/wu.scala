package trws

import scala.util.Random

import chisel3._
import chisel3.util._
import chisel3.iotesters._

import org.scalatest.{Matchers, FlatSpec}


class wuWrapper extends Module {

  val io = IO(new Bundle {
    val start = Input( Bool())
    val done = Output( Bool())

    val gi = Flipped( DecoupledIO( USIMD()))
    val wi = Flipped( DecoupledIO( SSIMD()))
    val mi = Flipped( DecoupledIO( USIMD()))

    val mo = DecoupledIO( USIMD())
  })

  val m = Module( new wu)

  m.io.start := io.start
  io.done := m.io.start

  io.gi <> m.io.gi
  io.wi <> m.io.wi
  io.mi <> m.io.mi

  io.mo.valid := m.io.mo.valid
  io.mo.bits := m.io.mo.bits.bits
  m.io.mo.ready := io.mo.ready



}



class wuTester( c: wuWrapper) extends StreamingPeekPokeTester(c) {

  //rnd.setSeed( 47L)

  val n = elements_per_cl*cl_per_row
  val n_slices = n*(n-1)/2

  // 12 fails radixpoint 16 8x3, 11 sometimes fails also
  // 9 fails radixpoint 10 8x3

  val precision = 8

  val GI = IndexedSeq.fill( 1, n){ BigInt( rnd.nextInt( 1<<precision))}
  val WI = IndexedSeq.fill( n-1, n){ BigInt( rnd.nextInt( 1<<precision)-(1<<(precision-1)))}

  val MI = IndexedSeq.fill( n_slices, n){
    BigInt( rnd.nextInt( 1<<precision))
  }

  def index( s : Integer, t : Integer) : Integer = {
    t*n-t*(t+1)/2 + s-(t+1)
  }

  val OFF = Array.fill( n_slices, n){ BigInt(0)}
  val OUT = Array.fill( n_slices, n){ BigInt(0)}
  val RES = Array.fill( n_slices, n){ BigInt(0)}

  def computeOUT( islice : Integer, j : Integer) : BigInt = {
    OFF(islice)(j)
  }

  def computeRES( islice : Integer) : IndexedSeq[BigInt] = {
    val v = OUT(islice)
    val best = v.tail.foldLeft( v.head){ (a,b) => if ( a<b) a else b}
    v map ( _ - best)
  }

  for ( t<-0 until n-1) {
    val weights = Array.tabulate(n){ j => WI(t)(j)}

    // Forward
    for ( s<-t+1 until n) {
      val k = index( s, t)
      for ( j<-0 until n) {
        weights(j) += MI(k)(j)
      }
    }
    // Backward
    for ( s<-0 until t) {
      val k = index( t, s) // swap
      for ( j<-0 until n) {
        weights(j) += RES(k)(j)
      }
    }
    // Gen OFF
    for ( s<-t+1 until n) {
      val k = index( s, t)
      for ( j<-0 until n) {
        OFF(k)(j) = ((GI(0)(s)*weights(j))>>radixPoint) - MI(k)(j)
      }
      for ( j<-0 until n) {
        OUT(k)(j) = computeOUT( k, j)
      }
      for ( j<-0 until n) {
        RES(k)(j) = computeRES( k)(j)
      }
    }

  }

  val REScl = toCL( (RES map {_.toIndexedSeq}).toIndexedSeq)

  def toUInt16( x : BigInt) : BigInt = {
    var xx = x
    while ( xx < 0) {
      xx += (1<<16)
    }
    xx % (1<<16)
  }

  val MOcl = REScl // map { v => v map toUInt16}

  poke( c.io.start, 1)

  val gi_sender = new Sender( "gi", c.io.gi, toCL( GI))
  val wi_sender = new Sender( "wi", c.io.wi, toCL( WI))
  val mi_sender = new Sender( "mi", c.io.mi, toCL( MI))

  val mo_receiver = new Receiver( "mo", c.io.mo, MOcl)

  var nsteps = 0

  while ( peek( c.io.done) == 0) {
// All pokes
    gi_sender.doPoke
    wi_sender.doPoke
    mi_sender.doPoke
    mo_receiver.doPoke
// All decisions
    gi_sender.doFire( nsteps)
    wi_sender.doFire( nsteps)
    mi_sender.doFire( nsteps)
    mo_receiver.doFire( nsteps)

    step(1)
    nsteps += 1
  }

  gi_sender.check
  wi_sender.check
  mi_sender.check

  mo_receiver.check

}

class wuTest extends GenericTest( "wu", () => new wuWrapper, (c:wuWrapper) => new wuTester( c))
