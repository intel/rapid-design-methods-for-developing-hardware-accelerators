package imperative

import org.scalatest.{Matchers, FlatSpec}

import chisel3._
import chisel3.util._
import chisel3.iotesters._

class FifoIfc64 extends Module {
  val io = IO(new Bundle { 
    val inp = Flipped(Decoupled( UInt( 64.W)))
    val out = Decoupled( UInt( 64.W))
  })
}

class SquashN( n : Int) extends FifoIfc64 {
  val fifos = (0 until n).toList.map( _ => Module( new Squash))
  val ls = io.inp :: fifos.map(_.io("Q"))
  val rs = fifos.map(_.io("P")) :+ io.out
  (ls zip rs).foreach { case (l,r) => l <> r}
}

class ChannelN( n : Int) extends FifoIfc64 {
  val fifos = (0 until n).toList.map( _ => Module( new Channel))
  val ls = io.inp :: fifos.map(_.io("Q"))
  val rs = fifos.map(_.io("P")) :+ io.out
  (ls zip rs).foreach { case (l,r) => l <> r}
}

class MooreN( n : Int) extends FifoIfc64 {
  val fifos = (0 until n).toList.map( _ => Module( new Moore))
  val ls = io.inp :: fifos.map(_.io("Q"))
  val rs = fifos.map(_.io("P")) :+ io.out
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
    val n2b = n - n2a
    val s0 = Module( new SplitN( n2a))
    val s1 = Module( new SplitN( n2b))
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
    val n2b = n - n2a
    val m0 = Module( new MergeN( n2a))
    val m1 = Module( new MergeN( n2b))
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

  io.inp <> s.io.inp
  (0 until n).foreach{ i => {
    s.io.outn(i) <> m.io.inpn(i)
/*
    val q = Module( new SquashN( 1))
    s.io.outn(i) <> q.io.inp
    q.io.out <> m.io.inpn(i)
 */
  }}
  m.io.out <> io.out

}

class FifoPerformanceTester[T <: FifoIfc64](c: T) extends PeekPokeTester(c) {
  val max_transfers = 2000
  val max_steps = 100000

  var nsteps = 0
  var nsent = 0
  var nrcvd = 0
  var inp_v = false
  var out_r = false

  var inp_fired = false
  var out_fired = false

  while( nrcvd < max_transfers && nsteps<max_steps) {

     val inp_p = rnd.nextInt(16) < 12 & nsent < max_transfers
     val out_p = rnd.nextInt(16) < 12

// Drive all DUT inputs
// These are inp.valid, inp.bits, and out.ready
     inp_v = /*inp_v && !inp_fired ||*/ inp_p
     val inp_d : BigInt = BigInt(nsent)
     out_r = /*out_r && !out_fired ||*/ out_p

     poke( c.io.inp.valid, if (inp_v) BigInt(1) else BigInt(0))
     poke( c.io.inp.bits.asInstanceOf[UInt], inp_d)
     poke( c.io.out.ready, if (out_r) BigInt(1) else BigInt(0))

// Peek at all Mealy DUT outputs
// These are inp.ready, out.valid, and out.bits

     val inp_r : Boolean = peek( c.io.inp.ready) != 0
     val out_v : Boolean = peek( c.io.out.valid) != 0
     val out_d : BigInt  = peek( c.io.out.bits.asInstanceOf[UInt])

     if (out_v & out_r) {
        println( s"Received $out_d should be $nrcvd at $nsteps")
        expect( c.io.out.bits.asInstanceOf[UInt], nrcvd)
        out_fired = true
        nrcvd += 1
     } else {
        out_fired = false
     }

     if (inp_v & inp_r) {
        println( s"Sending $nsent at $nsteps")
        inp_fired = true
        nsent += 1
     } else {
        inp_fired = false
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
    chisel3.iotesters.Driver( () => new ChannelN( 4), "verilator") { c =>
      new FifoPerformanceTester(c)
    } should be (true)
  }
}

class Squash4PerformanceTest extends FlatSpec with Matchers {
  behavior of "Squash4"

  it should "work" in {
    chisel3.iotesters.Driver( () => new SquashN( 4), "verilator") { c =>
      new FifoPerformanceTester(c)
    } should be (true)
  }
}

class Moore4PerformanceTest extends FlatSpec with Matchers {
  behavior of "Moore4"

  it should "work" in {
    chisel3.iotesters.Driver( () => new MooreN( 4), "verilator") { c =>
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
