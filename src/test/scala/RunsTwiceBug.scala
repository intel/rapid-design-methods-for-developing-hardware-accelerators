package twice

import org.scalatest.{ Matchers, FlatSpec}

import chisel3._
import chisel3.iotesters._

import firrtl.{Transform, LowForm, CircuitState, WRef, PortKind, MALE}
import firrtl.ir.{Circuit, DefModule, Statement, Expression, UIntLiteral, IntWidth, Block, Print, NoInfo, StringLit, ClockType}
import firrtl.Mappers._

class AddPrintf extends Transform {
  def inputForm = LowForm
  def outputForm = LowForm
  def execute( state : CircuitState) : CircuitState = {
    println( s"Running FIRRTL transform AddPrintf...")

    def onStmt( m : DefModule)( s : Statement ) : Statement = {

      s match {
        case Block( lst) =>
          val prnt =
            Print(
              NoInfo,
              StringLit( s"Running module ${m.name}\n"),
              Seq[Expression](),
              WRef( "clock", ClockType, PortKind, MALE),
              UIntLiteral(1,IntWidth(1))
            )
          Block( lst ++ List( prnt))
      }

    }

    val mods = state.circuit.modules map { case m : DefModule =>
      m map onStmt(m)
    }

    state.copy( circuit = state.circuit.copy( modules = mods))
  }
}

class RunsTwiceBug extends Module {
  println( s"Building RunsTwiceBug...")
  val io = IO(new Bundle{})
}

class Tester(c:RunsTwiceBug) extends PeekPokeTester(c) {
  step(1)
}

class FirrtlTest extends FlatSpec with Matchers {
  behavior of "RunsTwiceBug"
  it should "work" in {
    chisel3.iotesters.Driver.execute( Array( "-fct", "twice.AddPrintf", "--backend-name", "firrtl"), () => new RunsTwiceBug) { c =>
      new Tester( c)
    } should be ( true)
  }
}

class VerilatorTest extends FlatSpec with Matchers {
  behavior of "RunsTwiceBug"
  it should "work" in {
    chisel3.iotesters.Driver.execute( Array( "-fct", "twice.AddPrintf", "--backend-name", "verilator"), () => new RunsTwiceBug) { c =>
      new Tester( c)
    } should be ( true)
  }
}
