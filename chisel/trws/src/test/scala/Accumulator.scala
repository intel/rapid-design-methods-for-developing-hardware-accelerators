package trws

import scala.util.Random

import chisel3._
import chisel3.util._
import chisel3.iotesters._

import org.scalatest.{Matchers, FlatSpec}



class AccumulatorTester( c: Accumulator) extends StreamingPeekPokeTester(c) {

  rnd.setSeed( 47L)

  val n = elements_per_cl*cl_per_row
  val n_slices = n*(n-1)/2

  val precision = 12 // 13 fails with 8x3

  val MI0 = IndexedSeq.fill( n_slices, n){
    BigInt( rnd.nextInt( 1<<precision))
  }

  def index( s : Integer, t : Integer) : Integer = {
    t*n-t*(t+1)/2 + s-(t+1)
  }

  val A0 = IndexedSeq.tabulate( n-1){ (t) =>
    MI0.slice( index( t+1, t), index( n-1, t)+1).foldLeft( IndexedSeq.fill( n){BigInt(0)}){ (x,y) =>
      (x zip y) map { case (a,b) => a+b}
    }
  }

  var nsteps = 0

  val mi0_sender = new Sender( "mi0", c.io.mi0, toCL(MI0))
  val a0_receiver = new Receiver( "a0", c.io.a0, toCL(A0))

  while ( !a0_receiver.done) {

    mi0_sender.doPoke
    a0_receiver.doPoke

    mi0_sender.doFire( nsteps)
    a0_receiver.doFire( nsteps)

    step(1)
    nsteps += 1
  }

  mi0_sender.check
  a0_receiver.check
}

class AccumulatorTest extends GenericTest( "Accumulator", () => new Accumulator, (c:Accumulator) => new AccumulatorTester( c))
