package reporters.decoder

import chisel3._
import chisel3.util._
import chisel3.iotesters._
import org.scalatest.{Matchers, FlatSpec}

class Tester(c: DecoderWithValidIfc) extends PeekPokeTester(c) {

  val golden = for { valid <- 0 until 2
                     idx   <- 0 until c.n} yield {

    def toBigInt( s : IndexedSeq[Boolean]) : IndexedSeq[BigInt] =
      s map (x=>if (x) BigInt(1) else BigInt(0))

    val idxVec = (0 until c.k) map (i=>(idx & (1<<i)) != 0)
    val onehot = (0 until c.n) map (j=>j==idx && valid != 0)

    ( valid, toBigInt(idxVec), toBigInt(onehot))
  }

  for { (valid,idx,onehot) <- golden} {
    println( s"${valid} ${idx}")
    poke( c.io.valid, valid)
// Funny reverse here
    poke( c.io.idx, idx.reverse)
    step(1)
    expect( c.io.onehot, onehot.reverse)
  }

}


class GenericTest( tag : String, factory : () => DecoderWithValidIfc, backendStr : String)
    extends FlatSpec with Matchers {
  behavior of tag

  it should "work" in {
    chisel3.iotesters.Driver( factory, backendStr) { c =>
      new Tester(c)
    } should be (true)
  }
}

class TestNested2 extends GenericTest( "Test", () => new DecoderWithValidNested(2), "treadle")
class TestNested3 extends GenericTest( "Test", () => new DecoderWithValidNested(3), "treadle")
class TestNested4 extends GenericTest( "Test", () => new DecoderWithValidNested(4), "treadle")
class TestNested5 extends GenericTest( "Test", () => new DecoderWithValidNested(5), "treadle")

class TestFlat1 extends GenericTest( "Test", () => new DecoderWithValidFlat(1), "treadle")

class TestFlat2 extends GenericTest( "Test", () => new DecoderWithValidFlat(2), "treadle")

class TestFlat3 extends GenericTest( "Test", () => new DecoderWithValidFlat(3), "treadle")
