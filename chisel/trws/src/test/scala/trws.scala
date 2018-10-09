package trws

import scala.util.Random

import chisel3._
import chisel3.util._
import chisel3.iotesters._

import org.scalatest.{Matchers, FlatSpec}

class trwsWrapper extends Module {
  val io = IO(new Bundle {
    val start = Input( Bool())
    val done = Output( Bool())

    val modeLoad = Input( Bool())
    val modeCompute = Input( Bool())

    val loadIdx = Input( UInt(log2_ncontexts.W))
    val computeIdx = Input( UInt(log2_ncontexts.W))

    val gi = Flipped( DecoupledIO( USIMD()))
    val wi = Flipped( DecoupledIO( SSIMD()))
    val mi = Flipped( DecoupledIO( USIMD()))

    val slc = Flipped( DecoupledIO( new Pair))
    val lof = Flipped( DecoupledIO( new Pair))

    val mo = DecoupledIO( USIMD)
  })

  val m = Module( new trws)
  m.io.start := io.start
  io.done := m.io.done

  m.io.modeLoad := io.modeLoad
  m.io.modeCompute := io.modeCompute

  m.io.loadIdx := io.loadIdx
  m.io.computeIdx := io.computeIdx

  m.io.gi <> io.gi
  m.io.wi <> io.wi
  m.io.mi <> io.mi

  m.io.slc <> io.slc
  m.io.lof <> io.lof

  io.mo.valid := m.io.mo.valid
  io.mo.bits := m.io.mo.bits.bits

  m.io.mo.ready := io.mo.ready

}


class trwsTester( c: trwsWrapper) extends StreamingPeekPokeTester(c) {

  rnd.setSeed( 47L)

  val n = elements_per_cl*cl_per_row
  val n_slices = n*(n-1)/2

  val precision = 8

  val SLC = IndexedSeq.fill( n*n){
    Map( "a" -> BigInt(rnd.nextInt( 1<<precision)), "e" -> BigInt( rnd.nextInt( 1<<precision)))
  }
  val LOF = IndexedSeq.fill( n_slices){
    Map( "a" -> BigInt(rnd.nextInt( 1<<precision)), "e" -> BigInt( rnd.nextInt( 1<<precision)))
  }

  val GI = IndexedSeq.fill( 1, n){ BigInt( rnd.nextInt( 1<<precision))}

  val WI = IndexedSeq.fill( n-1, n){ BigInt( rnd.nextInt( 1<<precision)-(1<<(precision-1)))}

  val MI = IndexedSeq.fill( n_slices, n){ BigInt( rnd.nextInt( 1<<precision))  }

  def index( s : Integer, t : Integer) : Integer = {
    t*n-t*(t+1)/2 + s-(t+1)
  }

  val GIcl = toCL(GI)
  val WIcl = toCL(WI)
  val MIcl = toCL(MI)

  val OFF = Array.fill( n_slices, n){ BigInt(0)}
  val OUT = Array.fill( n_slices, n){ BigInt(0)}
  val RES = Array.fill( n_slices, n){ BigInt(0)}

  def computeOUT( islice : Integer, j : Integer) : BigInt = {
    val b = LOF(islice)("a") // LOF is b,f
    val f = LOF(islice)("e")
    var best : BigInt = 0
    for ( i<-0 until n) {
      val a = SLC(i*n+j)("a") // SLC is a,e
      val e = SLC(i*n+j)("e")
      val o = OFF(islice)(i)
      val mx = if ( e < f) f else e
      val ab = if ( a < b) b-a else a-b
      val lmt = BigInt(204)
      val mn = if ( ab < lmt) ab else lmt
      val cand = (( mx * mn ) >> radixPoint) + o
      if ( i == 0 || cand < best) {
        best = cand
      }
    }
    println( s"islice: $islice j: $j best: $best")
    best
  }

  def computeRES( islice : Integer) : IndexedSeq[BigInt] = {
    val v = OUT(islice)
    val best = v.reduceLeft{ (a,b) => if ( a<b) a else b}
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
      val res = computeRES( k)
      for ( j<-0 until n) {
        RES(k)(j) = res(j)
      }
    }

  }

  val MOcl = toCL( (RES map {_.toIndexedSeq}).toIndexedSeq)

  def compute( modeLoad : Boolean, modeCompute : Boolean, loadIdx : BigInt, computeIdx : BigInt) {

    reset(1)

    poke( c.io.start, 1)

    val gi_sender = new Sender( "gi", c.io.gi, GIcl)
    val wi_sender = new Sender( "wi", c.io.wi, WIcl)
    val mi_sender = new Sender( "mi", c.io.mi, MIcl)
    val mo_receiver = new Receiver( "mo", c.io.mo, MOcl)

    val slc_sender = new SenderBundle( "slc", c.io.slc, SLC)
    val lof_sender = new SenderBundle( "lof", c.io.lof, LOF)

    var nsteps = 0

    poke( c.io.modeLoad, if ( modeLoad) BigInt(1) else BigInt(0))
    poke( c.io.modeCompute, if ( modeCompute) BigInt(1) else BigInt(0))
    poke( c.io.loadIdx, loadIdx)
    poke( c.io.computeIdx, computeIdx)

    while ( peek( c.io.done) == 0) {

      println( s"gi,wi,mi,slc,lof,mo: ${gi_sender.k},${wi_sender.k},${mi_sender.k},${slc_sender.k},${lof_sender.k},${mo_receiver.k}: ${gi_sender.s.size},${wi_sender.s.size},${mi_sender.s.size},${slc_sender.s.size},${lof_sender.s.size},${mo_receiver.s.size}")

// Do pokes first (all of them)      

      if ( modeLoad) {
        slc_sender.doPoke
      }

      if ( modeCompute) {
        lof_sender.doPoke
        gi_sender.doPoke
        wi_sender.doPoke
        mi_sender.doPoke
        mo_receiver.doPoke
      }

// Do peeks and make decisions now

      if ( modeLoad) {
        slc_sender.doFire( nsteps)
      }

      if ( modeCompute) {
        lof_sender.doFire( nsteps)
        gi_sender.doFire( nsteps)
        wi_sender.doFire( nsteps)
        mi_sender.doFire( nsteps)
        mo_receiver.doFire( nsteps)
      }

      step(1)
      nsteps += 1

    }

    if ( modeLoad) {
      slc_sender.check
    }

    if ( modeCompute) {
      lof_sender.check
      gi_sender.check
      wi_sender.check
      mi_sender.check
      mo_receiver.check
    }

    poke( c.io.start, 0)
    step( 100)

  }

  compute( true, false, 0, 0)
  compute( false, true, 0, 0)

}

class trwsTest extends GenericTest( "trws", () => new trwsWrapper, (c:trwsWrapper) => new trwsTester( c))
