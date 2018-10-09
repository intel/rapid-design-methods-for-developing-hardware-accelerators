package trws

import scala.util.Random

import chisel3._
import chisel3.util._
import chisel3.iotesters._

import org.scalatest.{Matchers, FlatSpec}


class BackendWrapper extends Module {
  val io = IO(new Bundle {
    val start = Input( Bool())
    val done = Output( Bool())

    val res = Flipped( DecoupledIO( USIMD()))

//    val mo = DecoupledIO( new USIMDWithFlush)
    val mo = DecoupledIO( USIMD())
    val adj = DecoupledIO( USIMD())
  })


  val m = Module( new Backend)
  m.io.start := io.start
  io.done := m.io.done

  io.res <> m.io.res
  io.adj <> m.io.adj

  io.mo.valid := m.io.mo.valid
  io.mo.bits := m.io.mo.bits.bits

  m.io.mo.ready := io.mo.ready


}




class BackendTester( c: BackendWrapper) extends StreamingPeekPokeTester(c) {

  //rnd.setSeed( 47L)

  val n = elements_per_cl*cl_per_row
  val n_slices = n*(n-1)/2

  val precision = 12 // 13 fails for 24x24

  val RES = IndexedSeq.fill( n_slices, n){
    BigInt( rnd.nextInt( 1<<precision))
  }

  def index( s : Integer, t : Integer) : Integer = {
    t*n-t*(t+1)/2 + s-(t+1)
  }

  val ADJ = Array.fill( n-1, n){ BigInt(0)}

  for ( t<-0 until n) {
    for ( s<-t+1 until n) {
      val k = index( s, t)
      for ( j<-0 until n) {
        ADJ(s-1)(j) += RES(k)(j)
      }
    }
  }

  val REScl = toCL( RES)
  val ADJcl = toCL( (ADJ map {_.toIndexedSeq}).toIndexedSeq)

  poke( c.io.start, 1)

  val res_sender = new Sender( "res", c.io.res, REScl)

  val mo_receiver = new Receiver( "mo", c.io.mo, REScl)
  val adj_receiver = new Receiver( "adj", c.io.adj, ADJcl)

  var nsteps = 0

  while ( peek( c.io.done) == 0) {
// All pokes
    res_sender.doPoke
    mo_receiver.doPoke
    adj_receiver.doPoke
// All decisions
    res_sender.doFire( nsteps)
    mo_receiver.doFire( nsteps)
    adj_receiver.doFire( nsteps)

    step(1)
    nsteps += 1
  }

  res_sender.check
  mo_receiver.check
  adj_receiver.check

}

class BackendTest extends GenericTest( "Backend", () => new BackendWrapper, (c:BackendWrapper) => new BackendTester( c))
