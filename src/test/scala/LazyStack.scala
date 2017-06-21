package imperative

import org.scalatest.{ Matchers, FlatSpec, GivenWhenThen}

// import scala.util.control.Breaks

import scala.annotation.tailrec

import chisel3._
import chisel3.util._
import chisel3.iotesters._

class LazyStackNTester[T <: StackIfc](c:T) extends PeekPokeTester(c) {

//

//  val mybreaks = new Breaks
//  import mybreaks.{break, breakable}

  val timeOut = 10

  def doIn( v : BigInt) {
    poke( c.io("Out").ready, 0)
    poke( c.io("In").valid, 1)
    poke( c.io("In").bits.asInstanceOf[UInt], v)

    @tailrec
    def waitHighWithTimeOut( n : Bool, steps : Int) : Unit = {
      if ( peek( n) == BigInt(0)) {
        step(1)
        if ( steps >= timeOut) {
          println( s"In loop timeOut")
        } else {
          waitHighWithTimeOut( n, steps+1)
        }
      } else {
        println( s"doIn transfer: ${v}")
      }
    }
    waitHighWithTimeOut( c.io("In").ready, 0)

/*
    var steps = 0

    breakable {
      while ( peek( c.io("In").ready) == BigInt(0)) {
        step(1)
        if ( steps >= timeOut) {
          println( s"In loop timeOut")
          break()
        }
        steps += 1
      }
      println( s"doIn transfer: ${v}")
    }
 */
  }

  def doOut( v : BigInt) {
    poke( c.io("Out").ready, 1)
    poke( c.io("In").valid, 0)

    @tailrec
    def waitHighWithTimeOut( n : Bool, steps : Int) : Unit = {
      if ( peek( n) == BigInt(0)) {
        step(1)
        if ( steps >= timeOut) {
          println( s"Out loop timeOut")
        } else {
          waitHighWithTimeOut( n, steps+1)
        }
      } else {
        println( s"doOut transfer: ${peek( c.io("Out").bits.asInstanceOf[UInt])} ${v}")
        expect( c.io("Out").bits.asInstanceOf[UInt], v)
      }
    }
    waitHighWithTimeOut( c.io("Out").valid, 0)

/*
    var steps = 0

    breakable {
      while( peek( c.io("Out").valid) == BigInt(0)) {
        step(1)
        if ( steps >= timeOut) {
          println( s"Out loop timeOut")
          break()
        }
        steps += 1
      }
      println( s"doOut transfer: ${peek( c.io("Out").bits.asInstanceOf[UInt])} ${v}")
      expect( c.io("Out").bits, v)
    }
 */
  }

  poke( c.io("Out").ready, 0)
  poke( c.io("In").valid, 0)

//Mealy
//  expect( c.io("In").ready, 1) /* Don't care */
//  expect( c.io("Out").valid, 1) /* Don't care */

  step(1)

  doIn(  47); step(1)
  doOut( 47); step(1)

  doIn( 100); step(1)
  doIn( 101); step(1)

  doOut( 101); step(1)
  doIn( 102); step(1)
  doIn( 103); step(1)
  doIn( 104); step(1)

  doOut( 104); step(1)
  doOut( 103); step(1)
  doOut( 102); step(1)
  doOut( 100); step(1)
}

class LazyStackNCombTest extends FlatSpec with Matchers {
  behavior of "LazyStackNComb"
  it should "work" in {
    chisel3.iotesters.Driver.execute( Array( /* "--fint-write-vcd", */ "--no-check-comb-loops", "-fct", "imperative.transform.ReportArea", "--backend-name", "firrtl"), () => new LazyStackN(10, () => new LazyStackComb)) { c =>
      new LazyStackNTester( c)
    } should be ( true)
  }
}

import firrtl.{ SeqTransform, LowForm}
import firrtl.transforms.{ CheckCombLoops}
import imperative.transform.ShannonFactor
import reporters.{ ReportArea, InlineAndReportTiming}

class CT extends SeqTransform {
  def inputForm = LowForm
  def outputForm = LowForm
  def transforms = Seq(new ShannonFactor, new ReportArea, new CheckCombLoops, new InlineAndReportTiming)
}

class LazyStackNWaitTest extends FlatSpec with Matchers {
  behavior of "LazyStackNWait"
  it should "work" in {
    chisel3.iotesters.Driver.execute( Array( /* "--fint-write-vcd", */ "--no-check-comb-loops", "-fct", "imperative.CT", "--backend-name", "firrtl"), () => new LazyStackN(10, () => new LazyStackWait)) { c =>
      new LazyStackNTester( c)
    } should be ( true)
  }
}

class LazyStackNWait1Test extends FlatSpec with Matchers {
  behavior of "LazyStackNWait1"
  it should "work" in {
    chisel3.iotesters.Driver.execute( Array( "--no-check-comb-loops", "--backend-name", "verilator"), () => new LazyStackN(10, () => new LazyStackWait1)) { c =>
      new LazyStackNTester( c)
    } should be ( true)
  }
}

class LazyStackNWait2Test extends FlatSpec with Matchers {
  behavior of "LazyStackNWait2"
  it should "work" in {
    chisel3.iotesters.Driver.execute( Array( "--no-check-comb-loops", "-fct", "imperative.CT", "--backend-name", "verilator"), () => new LazyStackN(10, () => new LazyStackWait2)) { c =>
      new LazyStackNTester( c)
    } should be ( true)
  }
}
