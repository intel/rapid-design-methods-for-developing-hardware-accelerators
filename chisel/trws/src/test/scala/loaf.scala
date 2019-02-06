package trws

import scala.util.Random

import chisel3._
import chisel3.util._
import chisel3.iotesters._
import org.scalatest.{Matchers, FlatSpec}

class LoafTester( c: Loaf) extends StreamingPeekPokeTester(c) {


  rnd.setSeed( 47L)

  val n = elements_per_cl*cl_per_row

  val n_slices = n*(n-1)/2
  val max_steps = 1000000

  val precision = 13

  val AE = IndexedSeq.fill( n*n){
    Map( "a" -> BigInt( rnd.nextInt( 1<<precision)), "e" -> BigInt( rnd.nextInt( 1<<precision)))
  }
  val BF = IndexedSeq.fill( n_slices){
    Map( "a" -> BigInt( rnd.nextInt( 1<<precision)), "e" -> BigInt( rnd.nextInt( 1<<precision)))
  }

  val Off = IndexedSeq.tabulate( n_slices, n){ (islice, i) =>
    BigInt( rnd.nextInt( 1<<precision) - (1<<(precision-1)))
  }

  val Out = IndexedSeq.tabulate( n_slices, n){ (islice,j) =>
    val b = BF(islice)("a")
    val f = BF(islice)("e")
    var best : BigInt = 0
    for ( i<-0 until n) {
      val a = AE(i*n+j)("a")
      val e = AE(i*n+j)("e")
      val o = Off(islice)(i)
      val mx = if ( e < f) f else e
      val ab = if ( a < b) b-a else a-b
      val lmt = BigInt(204)
      val mn = if ( ab < lmt) ab else lmt
      val cand = (( mx * mn ) >> radixPoint) + o
      if ( i == 0 || cand < best) {
        best = cand
      }
    }
    best
  }



    def compute( modeLoad : Boolean, modeCompute : Boolean, loadIdx : BigInt, computeIdx : BigInt) {

      val startMillis = System.currentTimeMillis

      reset(1)

      poke( c.io.start, BigInt(1))

      var nsteps = 0

      val maxsteps = 1000

      poke( c.io.modeLoad, if ( modeLoad) BigInt(1) else BigInt(0))
      poke( c.io.modeCompute, if ( modeCompute) BigInt(1) else BigInt(0))
      poke( c.io.loadIdx, loadIdx)
      poke( c.io.computeIdx, computeIdx)

      val off_sender = new Sender( "off", c.io.off, toCL( Off))
      val out_receiver = new Receiver( "out", c.io.out, toCL( Out))

      val slc_sender = new SenderBundle( "slc", c.io.slc, AE)
      val lof_sender = new SenderBundle( "lof", c.io.lof, BF)

      val lst = (if ( modeLoad) List( slc_sender) else List()) ++ (if ( modeCompute) List( lof_sender, off_sender, out_receiver) else List())

      var done = false
      while( !done) {
// Do pokes first (all of them)
        lst.foreach( _.doPoke)

	val notDone = if ( modeLoad) peek( c.io.done) == 0 else if (modeCompute) !out_receiver.done else false
	if (!notDone/* || nsteps > maxsteps*/) {
	   done = true
	} else {

// Then all peeks and expects
           lst.foreach( _.doFire( nsteps))

           step(1)
	   nsteps += 1
        }
      }

      poke( c.io.start, 0)
      step(1)

      lst.foreach( _.check)

      println( s"Exited the while loop: ${System.currentTimeMillis-startMillis}")
    }

    compute( true,  false, 0, 0)
    compute( false,  true,  1, 0)
//    compute( true,  true,  1, 0)
//    compute( false, true,  0, 1)
//    compute( false, true,  1, 0)



}

class LoafTest extends GenericTest( "Loaf", () => new Loaf, (c:Loaf) => new LoafTester( c))
