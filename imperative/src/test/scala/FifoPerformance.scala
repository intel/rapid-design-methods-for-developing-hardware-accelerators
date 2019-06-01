package imperative

import org.scalatest.{Matchers, FlatSpec}

import chisel3._
import chisel3.util._
import chisel3.iotesters._

import compiler._

class FifoIfc64 extends ImperativeIfc(Compiler.run("process Squash( P : inp UInt(64), Q : out UInt(64)){}"))

class FifoN[T <: ImperativeModule]( n : Int, factory : () => T) extends FifoIfc64 {
  val fifos = (0 until n).toList.map( _ => Module( factory()))
  val ls = io("P") :: fifos.map(_.io("Q"))
  val rs = fifos.map(_.io("P")) :+ io("Q")
  (ls zip rs).foreach { case (l,r) => l <> r}
}

class SplitN( n : Int) extends Module {
  val io = IO(new Bundle {
    val inp = Flipped(Decoupled( UInt( 64.W)))
    val outn = Vec( n, Decoupled( UInt( 64.W)))
  })

  if ( n == 1) {
    io.inp <> io.outn(0)
  } else {
    val n2a = (n+1)/2
    val s0 = Module( new SplitN( n2a))
    val s1 = Module( new SplitN( n-n2a))
    val s = Module( new Split)
    io.inp <> s.io("P")
    s.io("Q0") <> s0.io.inp
    s.io("Q1") <> s1.io.inp
    (0 until n2a).foreach{ i => s0.io.outn(i) <> io.outn(i) }
    (n2a until n).foreach{ i =>  s1.io.outn(i-n2a) <> io.outn(i)}
  }

}

class MergeN( n : Int) extends Module {
  val io = IO(new Bundle {
    val inpn = Vec( n, Flipped(Decoupled( UInt( 64.W))))
    val out = Decoupled( UInt( 64.W))
  })

  if ( n == 1) {
    io.out <> io.inpn(0)
  } else {
    val n2a = (n+1)/2
    val m0 = Module( new MergeN( n2a))
    val m1 = Module( new MergeN( n-n2a))
    val m = Module( new Merge)
    io.out <> m.io("Q")
    m.io("P0") <> m0.io.out
    m.io("P1") <> m1.io.out
    (0 until n2a).foreach{ i => m0.io.inpn(i) <> io.inpn(i) }
    (n2a until n).foreach{ i =>  m1.io.inpn(i-n2a) <> io.inpn(i)}
  }
}


class TreeFifo extends FifoIfc64 {

  val n = 4

  val s = Module( new SplitN( n))
  val m = Module( new MergeN( n))

  io("P") <> s.io.inp
  (0 until n).foreach{ i => {
    s.io.outn(i) <> m.io.inpn(i)
    val q = Module( new FifoN( 1, () => new Squash))
    s.io.outn(i) <> q.io("P")
    q.io("Q") <> m.io.inpn(i)
  }}
  m.io.out <> io("Q")

}

class FifoPerformanceTester[T <: FifoIfc64](c: T) extends PeekPokeTester(c) {
  val max_transfers = 2000
  val max_steps = 100000

  var nsteps = 0
  var nsent = 0
  var nrcvd = 0
  var P_v = false
  var Q_r = false

  var P_fired = false
  var Q_fired = false

  while( nrcvd < max_transfers && nsteps<max_steps) {

     val P_p = rnd.nextInt(16) < 12 & nsent < max_transfers
     val Q_p = rnd.nextInt(16) < 12

// Drive all DUT inputs
// These are P.valid, P.bits, and Q.ready
     P_v = /*P_v && !P_fired ||*/ P_p
     val P_d : BigInt = BigInt(nsent)
     Q_r = /*Q_r && !Q_fired ||*/ Q_p

     poke( c.io("P").valid, if (P_v) BigInt(1) else BigInt(0))
     poke( c.io("P").bits.asInstanceOf[UInt], P_d)
     poke( c.io("Q").ready, if (Q_r) BigInt(1) else BigInt(0))

// Peek at all Mealy DUT outputs
// These are P.ready, Q.valid, and out.bits

     val P_r : Boolean = peek( c.io("P").ready) != 0
     val Q_v : Boolean = peek( c.io("Q").valid) != 0
     val Q_d : BigInt  = peek( c.io("Q").bits.asInstanceOf[UInt])

     if (Q_v & Q_r) {
        println( s"Received $Q_d should be $nrcvd at $nsteps")
        expect( c.io("Q").bits.asInstanceOf[UInt], nrcvd)
        Q_fired = true
        nrcvd += 1
     } else {
        Q_fired = false
     }

     if (P_v & P_r) {
        println( s"Sending $nsent at $nsteps")
        P_fired = true
        nsent += 1
     } else {
        P_fired = false
     }

     step(1)
     nsteps += 1
  }
  assert( nrcvd == max_transfers)
  println("Exited the while loop...")
}

class Channel4PerformanceTest extends FlatSpec with Matchers {
  behavior of "Squash4"

  it should "work" in {
    chisel3.iotesters.Driver( () => new FifoN( 4, () => new Channel), "verilator") { c =>
      new FifoPerformanceTester(c)
    } should be (true)
  }
}

class Squash4PerformanceTest extends FlatSpec with Matchers {
  behavior of "Squash4"

  it should "work" in {
    chisel3.iotesters.Driver( () => new FifoN( 4, () => new Squash), "verilator") { c =>
      new FifoPerformanceTester(c)
    } should be (true)
  }
}

class Moore4PerformanceTest extends FlatSpec with Matchers {
  behavior of "Moore4"

  it should "work" in {
    chisel3.iotesters.Driver( () => new FifoN( 4, () => new Moore), "verilator") { c =>
      new FifoPerformanceTester(c)
    } should be (true)
  }
}

class TreeFifoPerformanceTest extends FlatSpec with Matchers {
  behavior of "TreeFifo"

  it should "work" in {
    chisel3.iotesters.Driver( () => new TreeFifo, "firrtl") { c =>
      new FifoPerformanceTester(c)
    } should be (true)
  }
}
