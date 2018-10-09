package trws

import scala.util.Random

import chisel3._
import chisel3.util._
import chisel3.iotesters._

import org.scalatest.{Matchers, FlatSpec}

class StreamingPeekPokeTester[M <: Module]( c : M)  extends PeekPokeTester( c) {

  abstract class Port {
    def doPoke : Unit
    def doFire( nsteps : Integer) : Boolean
    def check : Unit
  }

  abstract class SenderBase[T <: Data, S <: Any]( val tag : String,
                                                  val decoupled : DecoupledIO[T],
                                                  val s : IndexedSeq[S] )  extends Port {

    def specificPoke( t : T, ss : S) : Unit  


    var fired : Boolean = false
    var k : Integer = 0
    var v : Boolean = false

    poke( decoupled.valid, 0)

    def doPoke = {
      val num = 12
      val den = 16
      val p = rnd.nextInt(den) < num && k < s.size
      v = v && !fired || p
//      v = p
      poke( decoupled.valid, if (v) BigInt(1) else BigInt(0))
      if ( v) {
        specificPoke( decoupled.bits, s(k))
      }
    }

    def doFire( nsteps : Integer) : Boolean = {
      val r = peek( decoupled.ready) != 0
      fired = v && r
      if ( fired) {
        println( s"Sending $tag $k at $nsteps")
        k += 1
      }
      fired
    }

    def check = {
      if ( k != s.size) {
        println( s"The number of ${tag} messages sent (${k}) is not equal to ${s.size}")
      }
    }

  }

  class Sender[T <: Aggregate]( tag : String,
                                decoupled : DecoupledIO[T],
                                s : IndexedSeq[IndexedSeq[BigInt]]) extends SenderBase( tag, decoupled, s) {
    override def specificPoke( t : T, ss : IndexedSeq[BigInt]) {
      poke( t, ss)
    }
  }

  class SenderBundle[T <: Bundle]( tag : String,
                                   decoupled : DecoupledIO[T],
                                   s : IndexedSeq[scala.collection.immutable.Map[String,BigInt]]) extends SenderBase( tag, decoupled, s) {
    override def specificPoke( t : T, ss : scala.collection.immutable.Map[String,BigInt]) {
      poke( t, ss)
    }
  }


  abstract class ReceiverBase[T <: Data, S <: Any]( val tag : String,
                                                    val decoupled : DecoupledIO[T],
                                                    val s : IndexedSeq[S] ) extends Port {
    var fired : Boolean = false
    var k : Integer = 0
    var r : Boolean = false

    poke( decoupled.ready, 0)

    def specificExpect( t : T, ss : S) : Unit

    def doPoke = {
      val num = 12
      val den = 16
      val p = rnd.nextInt(den) < num && k < s.size
      r = r && !fired || p
//      r = p
      poke( decoupled.ready, if (r) BigInt(1) else BigInt(0))
    }

    def doFire( nsteps : Integer) : Boolean = {
      val v = peek( decoupled.valid) != 0
      fired = v & r
      if ( fired) {
        println( s"Receiving $tag $k at $nsteps")
        specificExpect( decoupled.bits, s(k))
        k += 1
      }
      fired
    }

    def done : Boolean = k == s.size

    def check = {
      if ( !done) {
        println( s"The number of ${tag} messages received (${k}) is not equal to ${s.size}")
      }
    }
  }

  class Receiver[T <: Aggregate]( tag : String,
                                  decoupled : DecoupledIO[T],
                                  s : IndexedSeq[IndexedSeq[BigInt]] ) extends ReceiverBase( tag, decoupled, s) {

    override def specificExpect( t : T, ss : IndexedSeq[BigInt]) {
      expect( t, ss)
    }

  }

  class ReceiverBits[T <: Bits]( tag : String,
                                 decoupled : DecoupledIO[T],
                                 s : IndexedSeq[BigInt] ) extends ReceiverBase( tag, decoupled, s) {

    override def specificExpect( t : T, ss : BigInt) {
      expect( t, ss)
    }
  }

  def toCL( s : IndexedSeq[IndexedSeq[BigInt]]) : IndexedSeq[IndexedSeq[BigInt]] = {
// reverse needed because chisel3.Vec are in descending order (Why?)
    for { v <- s
          jj <- 0 until cl_per_row} yield v.slice( elements_per_cl*jj, elements_per_cl*(jj+1)).reverse
  }
}


class GenericTest[T <: Module, P <: PeekPokeTester[T]]( tag : String, dutFactory : () => T, testerFactory : (T) => P) extends FlatSpec with Matchers {

  val QUARTUS_INSTALL_DIR="/p/atp/tools/altera/16.0.0.211-Pro/quartus"
  val QSYS_SIMDIR="../../quartus/trws_loaf/mac/sim"

  val extra = Array( 
    "+lint=TFIPC-L",
    "+lint=PCWM",
    "-lca",
    "-sverilog",
    "-v", s"${QUARTUS_INSTALL_DIR}/eda/sim_lib/altera_primitives.v",
    "-v", s"${QUARTUS_INSTALL_DIR}/eda/sim_lib/220model.v",
    "-v", s"${QUARTUS_INSTALL_DIR}/eda/sim_lib/sgate.v",
    "-v", s"${QUARTUS_INSTALL_DIR}/eda/sim_lib/altera_mf.v",
//    s"${QUARTUS_INSTALL_DIR}/eda/sim_lib/altera_lnsim.sv",
    "-v", s"${QUARTUS_INSTALL_DIR}/eda/sim_lib/twentynm_atoms.v",
    "-v", s"${QUARTUS_INSTALL_DIR}/eda/sim_lib/synopsys/twentynm_atoms_ncrypt.v",
//    "-v", s"${QUARTUS_INSTALL_DIR}/eda/sim_lib/synopsys/twentynm_hssi_atoms_ncrypt.v",
//    "-v", s"${QUARTUS_INSTALL_DIR}/eda/sim_lib/twentynm_hssi_atoms.v",
//    "-v", s"${QUARTUS_INSTALL_DIR}/eda/sim_lib/synopsys/twentynm_hip_atoms_ncrypt.v",
//    "-v", s"${QUARTUS_INSTALL_DIR}/eda/sim_lib/twentynm_hip_atoms.v",
    "-v", s"${QSYS_SIMDIR}/../altera_a10_native_fixed_point_dsp_160/sim/mac_altera_a10_native_fixed_point_dsp_160_uuzdjcq.v",
    "-v", s"${QSYS_SIMDIR}/mac.v")

  val optionsManager = new TesterOptionsManager {
    testerOptions = testerOptions.copy(
      backendName="vcs",
//      backendName="verilator",
//      backendName="firrtl",
      moreVcsFlags=extra,
      isVerbose=true,
      vcsCommandEdits = """s/\+vcs\+initreg\+random //"""
    )
  }

  behavior of tag
  it should "not have expect violations" in {
    chisel3.iotesters.Driver.execute( dutFactory, optionsManager)(testerFactory)    should be (true)
  }
}

