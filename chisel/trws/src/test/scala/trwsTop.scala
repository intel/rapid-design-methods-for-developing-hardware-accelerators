package trws

import scala.util.Random

import chisel3._
import chisel3.util._
import chisel3.iotesters._

import org.scalatest.{Matchers, FlatSpec}

class trwsTopTester( c: trwsTop) extends StreamingPeekPokeTester(c) {

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
//    println( s"islice: $islice j: $j best: $best")
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

// pack into a 32 short (512 bit) cache line
  def packCL( s : IndexedSeq[IndexedSeq[BigInt]]) : IndexedSeq[IndexedSeq[BigInt]] = {
    val l0 = s.size
    val l1 = s(0).size
    val l = ((l0*l1+31)/32)*32
    val ss = (s.flatten) ++ IndexedSeq.fill( l - l0*l1){ BigInt( "dead", 16)}

//    println( s"l: ${l} l0: ${l0} l1: ${l1} s: ${s} ss: ${ss}")  

    val r = (0 until ss.size by 32) map { j => ss.slice(j,j+32)}
//    println( s"r: ${r}")
    r
  }

  def packPairCL( s : IndexedSeq[Map[String,BigInt]]) : IndexedSeq[IndexedSeq[BigInt]] = {
//    println( s"pair s: $s")
    packCL( s map { m => IndexedSeq( m("a"), m("e"))})
  }

  val MO = (RES map {_.toIndexedSeq}).toIndexedSeq

  val GIcl = packCL(GI)
  val WIcl = packCL(WI)
  val MIcl = packCL(MI)
  val MOcl = packCL( MO)
  val SLCcl = packPairCL(SLC)
  val LOFcl = packPairCL(LOF)

  val GIsz = (n+31)/32
  val WIsz = (n*(n-1)+31)/32
  val MIsz = (n_slices*n+31)/32
  val MOsz = (n_slices*n+31)/32
  val SLCsz = (2*n*n+31)/32
  val LOFsz = (2*n_slices+31)/32

  println( s"szs: ${GIsz} ${WIsz} ${MIsz} ${MOsz} ${SLCsz} ${LOFsz}")

  val GIoff = 0
  val WIoff = GIoff + GIsz
  val MIoff = WIoff + WIsz
  val MOoff = MIoff + MIsz
  val SLCoff = MOoff + MOsz
  val LOFoff = SLCoff + SLCsz
  val finaloff = LOFoff + LOFsz

  val sz = finaloff - GIoff
  val mem = Array.fill( sz){ BigInt(0)}

  def convertToCL( v : IndexedSeq[BigInt]) : BigInt = {
    assert( v.size == 32)
    (0 until v.size) map { j => 
      val cand = v(j)
      val cand2 =
        if ( cand < 0) {
          (1<<16) + cand
        } else {
          cand
        }
      assert( cand2 >= 0)
      cand2 << (16*(j))
    } reduceLeft { _ | _}
  }

//
// r=3
// 100   110   000 010
//    101   111  001 011
// -4 -3 -2 -1 0 1 2 3

  def convertFromCL( line : BigInt, signed : Boolean = false) : IndexedSeq[BigInt] = {
    val res =
    for ( j <- 0 until 32) yield {
      val v = (line >> (16*(j))) & ((1<<16)-1)
      val r =
      if ( signed && ( v >= (1<<15))) {
        v - (1<<16)
      } else {
        v
      }
//      println( s"$v $r")
      r
    }

    val line2 = convertToCL( res)
    assert( line == line2)
    res
  }

  def fillMem( offset : BigInt, s : IndexedSeq[IndexedSeq[BigInt]], fill : Option[BigInt] = None) {
    for( i <- 0 until s.size) {
      val v =
        if ( fill.isDefined) {
          IndexedSeq.fill(32){ fill.get}
        } else {
          s(i)
        }
      mem( offset.toInt + i) = convertToCL( v)
    }
  }

  def checkMem( tag : String,
                offset : BigInt,
                s : IndexedSeq[IndexedSeq[BigInt]],
                signed : Boolean = false,
                element_count : Option[Integer] = None) {
    var n_elements : Integer = 0
    for( i <- 0 until s.size) {
      val line = mem( offset.toInt + i)
      val v = convertFromCL( line, signed)
      val cand = s(i)

      for ( j <- 0 until 32) {
        val c = 
        if ( signed && (cand(j) >= (1<<15))) {
          cand(j) - (1<<16)
        } else {
          cand(j)
        }

        val g = !element_count.isDefined || n_elements < element_count.get
        if ( v(j) != c) {
          println( s"${tag} cache line $i element $j differ: ${v(j)} != ${c} check=${g}")
        }
        assert( !g || v(j) == c)
        n_elements += 1
      }
    }
  }

  fillMem( BigInt( GIoff), GIcl)
  fillMem( BigInt( WIoff), WIcl)
  fillMem( BigInt( MIoff), MIcl)
  fillMem( BigInt( MOoff), MOcl, Some(BigInt( "dead", 16)))
  fillMem( BigInt( SLCoff), SLCcl)
  fillMem( BigInt( LOFoff), LOFcl)

  checkMem( "GI", BigInt( GIoff), GIcl)
  checkMem( "WI", BigInt( WIoff), WIcl, true)
  checkMem( "MI", BigInt( MIoff), MIcl)

  checkMem( "SLC", BigInt( SLCoff), SLCcl)
  checkMem( "LOF", BigInt( LOFoff), LOFcl)


  def compute( modeLoad : Boolean, modeCompute : Boolean, loadIdx : BigInt, computeIdx : BigInt) {

    reset(1)

    poke( c.io.start, 1)
    poke( c.io.config.modeLoad, modeLoad)
    poke( c.io.config.modeCompute, modeCompute)
    poke( c.io.config.loadIdx, loadIdx)
    poke( c.io.config.computeIdx, computeIdx)

    poke( c.io.config.n_slices, n_slices)
    poke( c.io.config.n_cl_per_row, cl_per_row)

    poke( c.io.config.lofaddr, BigInt( LOFoff) << 6)
    poke( c.io.config.slcaddr, BigInt( SLCoff) << 6)
    poke( c.io.config.moaddr, BigInt( MOoff) << 6)
    poke( c.io.config.wiaddr, BigInt( WIoff) << 6)
    poke( c.io.config.miaddr, BigInt( MIoff) << 6)
    poke( c.io.config.giaddr, BigInt( GIoff) << 6)

    while( peek( c.io.done) == 0) {
      poke( c.io.mem_rd_req.ready, 1)
      poke( c.io.mem_wr_req.ready, 1)

      {
        poke( c.io.mem_rd_resp.valid, 0)

        val valid = peek( c.io.mem_rd_req.valid)

        if ( valid != 0) {
          val addr = peek( c.io.mem_rd_req.bits.addr)
          val tag = peek( c.io.mem_rd_req.bits.tag)

/*
          println( s"mem_rd_req.valid: ${valid}")
          println( s"mem_rd_req.bits.addr: ${addr.toString(16)}")
          println( s"mem_rd_req.bits.tag: ${tag.toString(2)}")
 */

          poke( c.io.mem_rd_resp.valid, 1)
          poke( c.io.mem_rd_resp.bits.data, mem(addr.toInt))
          poke( c.io.mem_rd_resp.bits.tag, tag)
        }

      }

      {
        poke( c.io.mem_wr_resp.valid, 0)

        val valid = peek( c.io.mem_wr_req.valid)

        if ( valid != 0) {
          val addr = peek( c.io.mem_wr_req.bits.addr)
          val data = peek( c.io.mem_wr_req.bits.data)
          val tag = peek( c.io.mem_wr_req.bits.tag)

          mem(addr.toInt) = data

/*
          println( s"mem_wr_req.valid: ${valid}")
          println( s"mem_wr_req.bits.addr: ${addr.toString(16)}")
          println( s"mem_wr_req.bits.data: ${data.toString(16)}")
          println( s"mem_wr_req.bits.tag: ${tag.toString(2)}")
 */

          poke( c.io.mem_wr_resp.valid, 1)
          poke( c.io.mem_wr_resp.bits.tag, tag)
        }

      }

      step(1)
    }

    if ( modeCompute) {
      checkMem( "MO", BigInt( MOoff), MOcl, element_count=Some(n_slices*n))
    }

  }

//  compute( true, false, 0, 0)
  compute( true, true, 0, 0)

}

import hld_interface_wrapper.HldAccParams._
class trwsTopTest extends GenericTest( "trwsTop", () => new trwsTop, (c:trwsTop) => new trwsTopTester( c))
