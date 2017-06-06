// See LICENSE for license details.

package imperative.transform

import imperative.{ TopoSort}

// Compiler Infrastructure
import firrtl.{Transform, LowForm, CircuitState, Utils, WRef, WSubField, WDefInstance}
// Firrtl IR classes
import firrtl.ir.{Circuit, DefModule, DefRegister, Statement, Expression, Mux, UIntLiteral, SIntLiteral, DoPrim, UIntType, SIntType, IntWidth, Connect, Block, EmptyStmt, IsInvalid}
// Map functions
import firrtl.Mappers._
// Scala's mutable collections
import scala.collection.mutable

/** Ledger tracks [[Circuit]] statistics
  *
  * In this lesson, we want to count the number of muxes in each
  *  module in our design.
  *
  * This [[Ledger]] class will be passed along as we walk our
  *  circuit, and help us count each [[Mux]] we find.
  *
  * See [[lesson1.AnalyzeCircuit]]
  */

sealed abstract class Area()
case class AreaModule( nm : String) extends Area
case class AreaMux( w : Int, cConds : Int, cExprs : Int) extends Area
case class AreaRegister( w : Int) extends Area
case class AreaOp( nm : String, ninps : List[Int], w : Int, cExprs : Int) extends Area

object ComputeArea {
  val cMux = 8
  val cMaj = 12
  val cXor = 10
  val cReg = 30
  val cNand2 = 4
  val cNand3 = 6
  def cNand( n : Int) : Int = 
    if ( n < 2) 0 else if ( n % 2 == 0) cNand2 + (n-2)/2*cNand3 else cNand3 + (n-3)/2*cNand3

  def apply( a : Area, tbl : Map[String,Int]) : Int = a match {
    case AreaOp( "add", List(w0,w1), w, 0) => w*(cMaj+2*cXor)
    case AreaOp( "add", List(w0,w1), w, 1) => w*(cNand( 2)+cXor)
    case AreaOp( "add", List(w0,w1), w, 2) => 0
    case AreaOp( "and", inpSizes, w, 0) => w*cNand( inpSizes.size)
    case AreaOp( "or", inpSizes, w, 0) => w*cNand( inpSizes.size)
    case AreaOp( "eq", List(w0,w1), w, 0) => w0*cXor+cNand(w0)
    case AreaOp( "eq", inpSizes@List(w0,w1), w, 1) => apply( AreaOp( "and", inpSizes, w0, 0),tbl)
    case AreaOp( "eq", List(w0,w1), w, 2) => 0
    case AreaOp( "mul", List(w0,w1), w, 0) => w0*apply(AreaOp("add", List(w1,w), w, 0),tbl)+apply( AreaOp( "add", List(w,w), w, 0),tbl)
    case AreaOp( "lt", List(w0,w1), w, c) => apply( AreaOp( "add", List(w0,w1), w0, c),tbl)
    case AreaOp( "bits", _, _, _) => 0
    case AreaOp( "pad", _, _, _) => 0
    case AreaOp( "tail", _, _, _) => 0
    case AreaRegister( w) => w*cReg
    case AreaMux( w, 0, 0) => w*cMux
    case AreaMux( w, 1, _) => 0
    case AreaMux( w, 0, 2) => 0
    case AreaMux( w, 0, 1) => apply( AreaOp( "and", List(w,w), w, 0),tbl)
    case AreaModule( nm) => tbl(nm)
    case _ => println( s"unknown op ${a}"); 0
  }

  def apply( m : Map[Area,Int], tbl : Map[String,Int]) : Int =
    m.toList.foldLeft( 0){ case (s,(k : Area,v : Int)) => s + v * apply(k,tbl)}

}

class Ledger {

  private var moduleName: Option[String] = None
  private var moduleOpMap = Map[String,Map[Area,Int]]()
  def foundOp( a : Area): Unit = moduleName match {
    case None => Utils.error("Module name not defined in Ledger!")
    case Some(name) => {
      val m = moduleOpMap(name)
      moduleOpMap = moduleOpMap.updated( name, m.updated( a, m.getOrElse( a, 0) + 1))
    }
  }
  def getModuleName: String = moduleName match {
    case None => Utils.error("Module name not defined in Ledger!")
    case Some(name) => name
  }
  def setModuleName(myName: String): Unit = {
    moduleOpMap = moduleOpMap.updated( myName, moduleOpMap.getOrElse( myName, Map()))
    moduleName = Some(myName)
  }
  def report() : Unit = {
    val arcs = for { (tgt,m) <- moduleOpMap
                     (AreaModule(src),_) <- m
    } yield (src,tgt)

    var tbl : Map[String,Int] = Map()
    for{ nm <- TopoSort( moduleOpMap.keys.toSeq, arcs.toSeq)} {
      val area = ComputeArea(moduleOpMap(nm),tbl)

      println( f"${nm}%-30s     Area    Count   T. Area  Percent")
      println( f"-------------------------------    ----    -----   -------  -------")
      for{ (k,v) <- moduleOpMap(nm)} {
        val larea = ComputeArea(k,tbl)
        println( f"${k}%-30s ${larea}%8d ${v}%8d ${larea*v}%9d ${larea*v*100.0/area}%8.1f")
      }
      println( f"-------------------------------    ----    -----   -------  -------")
      println( f"Sum                                              ${area}%9d")
      println( f"===================================================================")

      tbl = tbl.updated( nm, area)
    }
  }

}

class ReportArea extends Transform {

  def extractWidth( x : Any) : Int = x match {
    case UIntType(IntWidth(w)) => w.toInt
    case SIntType(IntWidth(w)) => w.toInt
    case _ => -1
  }

  def isConst( x : Any) : Int = x match {
    case _ : UIntLiteral => 1
    case _ : SIntLiteral => 1
    case _ => 0
  }

  def inputForm = LowForm
  def outputForm = LowForm

  def execute(state: CircuitState): CircuitState = {
    val ledger = new Ledger()
    val circuit = state.circuit
    circuit map walkModule(ledger)
    ledger.report
    state
  }

  def walkModule(ledger: Ledger)(m: DefModule): DefModule = {
    ledger.setModuleName(m.name)
    m map walkStatement(ledger)
    m
  }

  def walkStatement(ledger: Ledger)(s: Statement): Statement = {
    s map walkExpression(ledger) map walkStatement(ledger)
    s match {
      case WDefInstance( info, instanceName, templateName, _) =>
        ledger.foundOp( AreaModule( s"$templateName"))
      case DefRegister( info, lhs, tpe, clock, reset, init) =>
        ledger.foundOp( AreaRegister( extractWidth( tpe)))
      case _ => ()
    }
    s
  }

  def walkExpression(ledger: Ledger)(e: Expression): Expression = {
    e map walkExpression(ledger)
    e match {
      case Mux(cond, tval, fval, tpe) => 
        ledger.foundOp( AreaMux( extractWidth(tpe), isConst(cond), isConst(tval)+isConst(fval)))
      case DoPrim( op, inps, _, tpe) =>

        val inpSizes : List[Int] = inps.map{ x => x match {
          case WRef( _, inpTpe, _, _) => extractWidth(inpTpe)
          case UIntLiteral( _, IntWidth( w)) => w.toInt
          case SIntLiteral( _, IntWidth( w)) => w.toInt
          case _ => throw new Exception( s"inputSizes: Unimplemented match ${x}")
        }}.toList

        val c = inps.foldLeft( 0){ case (l,r) => l + isConst(r)}
        ledger.foundOp( AreaOp( s"${op}", inpSizes, extractWidth(tpe), c))
      case _ => ()
    }
    e
  }
}
