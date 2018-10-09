package trws

import scala.util.Random

import chisel3._
import chisel3.util._
import chisel3.iotesters._

import org.scalatest.{Matchers, FlatSpec}

class ComputeBestTester( c: ComputeBest) extends StreamingPeekPokeTester(c) {

  rnd.setSeed( 47L)

  val n = elements_per_cl*cl_per_row
  val n_slices = n*(n-1)/2

  val OUT0 = IndexedSeq.fill( n_slices){
//    IndexedSeq.tabulate( n){ i => BigInt(i)}
    IndexedSeq.fill( n){ BigInt( rnd.nextInt( 1<<13) - (1<<12))}
  }

  val OBEST = OUT0 map { v =>
    v.tail.foldLeft( v.head){ (a,b) => if ( a<b) a else b}
  }

  var nsteps = 0

  poke( c.io.start, 1)

  val out0_sender = new Sender( "out", c.io.out0, toCL( OUT0))
  val obest_receiver = new ReceiverBits( "obest", c.io.obest, OBEST)

  while ( !obest_receiver.done) {
    println( s"${out0_sender.k},${obest_receiver.k}: ${out0_sender.s.size},${obest_receiver.s.size}")

    out0_sender.doPoke
    obest_receiver.doPoke

    out0_sender.doFire( nsteps)
    obest_receiver.doFire( nsteps)

    step(1)
    nsteps += 1
  }

  out0_sender.check
  obest_receiver.check
}

class ComputeBestTest extends GenericTest( "ComputeBest", () => new ComputeBest, (c:ComputeBest) => new ComputeBestTester( c))
