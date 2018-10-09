package trws

import scala.util.Random

import chisel3._
import chisel3.util._
import chisel3.iotesters._
//import treadle.TreadleTester
import firrtl_interpreter.{InterpretiveTester,InterpreterOptionsManager}
import org.scalatest.{Matchers, FlatSpec}

class MACBlackBoxWrapper extends Module with MACAbstractIfc {
  val u = Module( new MAC/*MACBlackBox*/)

  u.io.clock := clock
  u.io.en := io.en
  u.io.a := io.a
  u.io.b := io.b
  u.io.o := io.o
  io.r := u.io.r

}

class MACTester( c: MACBlackBoxWrapper) extends PeekPokeTester(c) {

  poke( c.io.en, 1)
  poke( c.io.a, 47)
  poke( c.io.b, 1)
  poke( c.io.o, 0)

  step( 1)
  poke( c.io.o, 1)

  step( 4)
  expect( c.io.r, 47)
  step( 1)
  expect( c.io.r, 48)

  poke( c.io.o, 0)
  step( 1)
  poke( c.io.o, 1)
  poke( c.io.en, 0)
  step(1)
  poke( c.io.en, 1)

  step( 3)
  expect( c.io.r, 48)
  step( 1)
  expect( c.io.r, 47)
  step( 1)
  expect( c.io.r, 48)

}

class MACTest extends FlatSpec with Matchers {


  val QUARTUS_INSTALL_DIR="/p/atp/tools/altera/16.0.0.211-Pro/quartus"
  val QSYS_SIMDIR="~/ppt4_work_area/stereo/hld/tutorials/trws/chisel/quartus/trws_loaf/mac/sim"

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
    "-v", s"${QUARTUS_INSTALL_DIR}/eda/sim_lib/synopsys/twentynm_hssi_atoms_ncrypt.v",
    "-v", s"${QUARTUS_INSTALL_DIR}/eda/sim_lib/twentynm_hssi_atoms.v",
    "-v", s"${QUARTUS_INSTALL_DIR}/eda/sim_lib/synopsys/twentynm_hip_atoms_ncrypt.v",
    "-v", s"${QUARTUS_INSTALL_DIR}/eda/sim_lib/twentynm_hip_atoms.v",
    "-v", s"${QSYS_SIMDIR}/../altera_a10_native_fixed_point_dsp_160/sim/mac_altera_a10_native_fixed_point_dsp_160_uuzdjcq.v",
    "-v", s"${QSYS_SIMDIR}/mac.v")


  val optionsManager = new TesterOptionsManager {
    testerOptions = testerOptions.copy(
      backendName="vcs",
//      backendName="verilator",
      moreVcsFlags=extra,
      vcsCommandEdits = """s/\+vcs\+initreg\+random //"""
    )
  }

  behavior of "MAC"
  it should "not have expect violations" in {
    chisel3.iotesters.Driver.execute( () => new MACBlackBoxWrapper, optionsManager) { c =>
      new MACTester( c)
    } should be (true)
  }
}
