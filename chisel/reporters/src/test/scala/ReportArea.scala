// See LICENSE for license details.

package reporters.firrtlTests

import firrtl._
import firrtlTests._
import firrtl.ir._
import firrtl.Mappers._
import FirrtlCheckers._

import collection.mutable

import reporters._

class ReportAreaSpec extends FirrtlFlatSpec {
  def compile(input: String): CircuitState =
    (new LowFirrtlCompiler).compileAndEmit(CircuitState(parse(input), ChirrtlForm), List.empty)
  def compileBody(body: String) = {
    val str = """
      |circuit Test :
      |  module Test :
      |""".stripMargin + body.split("\n").mkString("    ", "\n    ", "")
    compile(str)
  }

  def check( state : CircuitState, area : Int) = {
    state.annotations map ( x => x match {
      case AreaAnnotation( a) =>
           println( s"Found area annotation: ${a}")
      	   a should be (area)
	   ()
      case _ =>
           println( s"Found other annotation: ${x}")
	   ()
    })

  }


  "Report Area" should "run on wires and return 0 area" in {
    val result = compileBody(s"""
      |input a : UInt<8>
      |output z : UInt<8>
      |z <= a""".stripMargin
    )
    val state = (new ReportArea).execute(result)
    check( state, 0)
  }

  it should "run on 8-bit and2 and return 4*8=32" in {
    val result = compileBody(s"""
      |input a : UInt<8>
      |input b : UInt<8>
      |output z : UInt<8>
      |node w = and(a,b)
      |z <= w""".stripMargin
    )
    val state = (new ReportArea).execute(result)
    check( state, 32)
  }

  it should "run on 8-bit eq to constant and return (6+6+4)+6=22" in {
    val result = compileBody(s"""
      |input a : UInt<8>
      |output z : UInt<1>
      |node w = eq(a,UInt<8>("hff"))
      |z <= w""".stripMargin
    )
    val state = (new ReportArea).execute(result)
    check( state, 22)
  }

  it should "run on 8-bit add (output 9-bits) and return 288" in {
    val result = compileBody(s"""
      |input a : UInt<8>
      |input b : UInt<8>
      |output z : UInt<8>
      |node w = add(a,b)
      |z <= w""".stripMargin
    )
    val state = (new ReportArea).execute(result)
    check( state, 288)
  }
}
