// See LICENSE for license details.

package reporters

import firrtl.{Transform, LowForm, CircuitState, Utils, WRef, WSubField, WDefInstance}
import firrtl.ir.{Circuit, Module, DefModule, DefRegister, Statement, Expression, Mux, UIntLiteral, SIntLiteral, DoPrim, UIntType, SIntType, IntWidth, Connect, Block, EmptyStmt, IsInvalid, Field, DefNode, DefWire, DefMemory, Stop, Print}
import firrtl.Mappers._

import scala.collection.mutable
import scala.math.Ordering.Implicits._


import spray.json._
import DefaultJsonProtocol._
import java.io._

class Ledger {

  private var moduleName: Option[String] = None

  private val moduleOpMap = mutable.Map[String,mutable.Map[Area,Int]]()

  def foundOp( a : Area): Unit = {
    val innerMap = moduleOpMap(getModuleName)
    innerMap(a) = innerMap.getOrElse( a, 0) + 1
  }
  def getModuleName: String = moduleName match {
    case None => Utils.error("Module name not defined in Ledger!")
    case Some(name) => name
  }
  def setModuleName(myName: String): Unit = {
    moduleOpMap(myName) = moduleOpMap.getOrElse( myName, mutable.Map())
    moduleName = Some(myName)
  }
  def report() : Unit = {
    val arcs = for { (tgt,m) <- moduleOpMap
                     (AreaModule(src,_),_) <- m
    } yield (src,tgt)

    val tbl = mutable.Map[String,Int]()
    for{ nm <- TopoSort( moduleOpMap.keys.toSeq, arcs.toSeq)} {
      val area = ComputeArea(moduleOpMap(nm),tbl)
      val areas = for{ (k,v) <- moduleOpMap(nm)} yield (s"$k", v, ComputeArea(k,tbl))

      val sortedAreas = areas.toList.sortWith{ case (x,y) => (-x._2*x._3,x._1) < (-y._2*y._3,y._1)}

      var cumulative : Int = 0

      println( f"${nm}%-30s     Area    Count         Total      Cumulative")
      println( f"-------------------------------    ----    -----     -------------  ----------")
      for{ (k,v,larea) <- sortedAreas} {
        val total = larea*v
        cumulative += total
        println( f"${k}%-30s ${larea}%8d ${v}%8d ${total}%9d ${total*100.0/area}%6.1f%%   ${cumulative*100.0/area}%8.1f%%")
      }
      println( f"-------------------------------    ----    -----     -------------  ----------")
      println( f"Sum                                              ${area}%9d")
      println( f"==============================================================================")

      tbl(nm) = area
    }
  }

  def reportJson( circuit : Circuit, fn : String) : Unit = {

    val tbl = mutable.Map[String,Int]()

    def aux( nm : String, instanceName : String) : JsObject = {

      val children = mutable.ArrayBuffer[(String,String)]()
      val objs = mutable.ArrayBuffer[JsObject]()

      for{ (k,v) <- moduleOpMap(nm)} {
        k match {
          case AreaModule( templateName, instanceName) => children.append( (templateName, instanceName))
          case a => objs.append( JsObject( "name" -> JsString(s"$a $v"), "size" -> JsNumber(v*ComputeArea( a, tbl))))
        }
      }

      val seq = for ( (child,childInstanceName) <- children) yield {
        aux( child, childInstanceName)
      }

      JsObject( "name" -> JsString(instanceName), "children" -> JsArray( (objs ++ seq) : _*))
    }

    val jsonAST = aux( circuit.main, "top")

    val fp = new File( fn)
    val bw = new BufferedWriter(new FileWriter( fp))
    bw.write( jsonAST.prettyPrint)
    bw.close()

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
    ledger.reportJson( circuit, "areas.json")

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
        ledger.foundOp( AreaModule( s"$templateName", s"$instanceName"))
      case DefRegister( info, lhs, tpe, clock, reset, init) =>
        ledger.foundOp( AreaRegister( extractWidth( tpe)))
      case DefMemory( info, nm, tpe, sz, wrLat, rdLat, readers, writers, readWriters, _) =>
        ledger.foundOp( AreaMemory( extractWidth( tpe), sz, readers.length, writers.length, readWriters.length))
      case _ : Block => ()
      case _ : DefNode => ()
      case _ : DefWire => ()
      case _ : Connect => ()
      case _ : Print => ()
      case _ : Stop => ()
      case EmptyStmt => () // EmptyStmt is an object
      case _ => 
        println( s"Missed this statement: $s")
        ()
    }
    s
  }

  def walkExpression(ledger: Ledger)(e: Expression): Expression = {
    e map walkExpression(ledger)
    e match {
      case Mux(cond, tval, fval, tpe) => 
        ledger.foundOp( AreaMux( extractWidth(tpe), isConst(cond), isConst(tval)+isConst(fval)))
      case DoPrim( op, inps, _, tpe) =>

        val inpSizes : List[Int] = inps.map{
          case WRef( _, inpTpe, _, _) => extractWidth(inpTpe)
          case UIntLiteral( _, IntWidth( w)) => w.toInt
          case SIntLiteral( _, IntWidth( w)) => w.toInt
          case DoPrim( _, _, _, inpTpe) => extractWidth(inpTpe)
          case WSubField( expr, nm, inpTpe, gndr) =>
            extractWidth(inpTpe)
          case x =>
            println( s"inputSizes: Unimplemented match ${x}")
            0
        }.toList

        val c = inps.foldLeft( 0){ case (l,r) => l + isConst(r)}
        ledger.foundOp( AreaOp( s"${op}", inpSizes, extractWidth(tpe), c))
      case _ => ()
    }
    e
  }
}
