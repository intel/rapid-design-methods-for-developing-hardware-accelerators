package trws

import scala.util.Random

import chisel3._
import chisel3.util._
import chisel3.iotesters._

import org.scalatest.{Matchers, FlatSpec}

class ClamperTester( c: Clamper) extends StreamingPeekPokeTester(c) {

  rnd.setSeed( 47L)

  val n = elements_per_cl*cl_per_row
  val n_slices = n*(n-1)/2
  println( s"n: ${n} n_slices: ${n_slices} elements_per_cl: ${elements_per_cl} cl_per_row: ${cl_per_row}")

  val OUT = IndexedSeq.fill( n_slices){
    IndexedSeq.fill( n){ BigInt( rnd.nextInt( 1<<13) - (1<<12))}
  }

  val RES = OUT map { v =>
    val best = v.reduceLeft{ (a,b) => if ( a<b) a else b}
    v map ( _ - best)
  }

  var nsteps = 0

  poke( c.io.start, 1)

  val out_sender = new Sender( "out", c.io.out, toCL( OUT))
  val res_receiver = new Receiver( "res", c.io.res, toCL( RES))

  while ( !res_receiver.done) {
    println( s"${out_sender.k},${res_receiver.k}: ${out_sender.s.size},${res_receiver.s.size}")

    out_sender.doPoke
    res_receiver.doPoke

    out_sender.doFire( nsteps)
    res_receiver.doFire( nsteps)

    step(1)
    nsteps += 1
  }

  out_sender.check
  res_receiver.check
}

class ClamperTest extends GenericTest( "Clamper", () => new Clamper, (c:Clamper) => new ClamperTester( c))
