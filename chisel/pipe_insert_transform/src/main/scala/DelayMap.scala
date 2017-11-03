// See LICENSE for license details.
package pipe_insert_transform

import collection.mutable

import firrtl.Mappers._

import firrtl.{ WRef, WSubField}
import firrtl.ir.{DefModule,Statement,EmptyStmt,Port,ClockType,DefRegister,DefNode,NoInfo,Block,Connect, Circuit, Expression}

import firrtl.ir.{ DefMemory, UIntType, SIntType, UIntLiteral, SIntLiteral, DefWire, Literal, IsInvalid, Stop, Print, Mux, DoPrim, ValidIf, IntWidth}


object DelayMap {
  
  def constructTimingArcs
    ( tas : mutable.Map[(String,String),(Int,String)],
      regs : mutable.Map[String,(String,String)],
      mems : mutable.Set[DefMemory])
    ( m : DefModule): DefModule = {

    def LogicNode( dummy : String, arg : Any) = arg match {
      case WRef( nm, _, _, _) => nm
      case nm : String => nm
      case _ => throw new Exception("Bad argument to LogicNode")
    }

    def constructTimingArcsStatement( s : Statement): Statement = {

      def extractWidth( x : Any) : Int = x match {
        case UIntType(IntWidth(w)) => w.toInt
        case SIntType(IntWidth(w)) => w.toInt
        case _ => -1
      }

      def inSignals( inp : Seq[Expression]) : List[(String,Int)] =
        inp.toList flatMap {
          case ref@WRef( nm, tpe, knd, gnrd) => 
            val rhsNode = LogicNode( m.name, ref)
            val rhsNode0 = if ( regs.contains( rhsNode)) regs(rhsNode)._1 else rhsNode
            List((rhsNode0,extractWidth(tpe)))
          case sub@WSubField( ref, nm, tpe, gnrd) => 
            val rhsNode = LogicNode( m.name, sub)
            val rhsNode0 = if ( regs.contains( rhsNode)) regs(rhsNode)._1 else rhsNode
            List((rhsNode0,extractWidth(tpe)))
          case UIntLiteral( lit, width) => List()
          case SIntLiteral( lit, width) => List()
          case x@DoPrim( padOrBits, lst, _, _) /*if List("pad","bits","asSInt","asUInt").contains(s"$padOrBits") */ =>
//            println( s"Recursive call: ${x}")
            inSignals( lst)
          case x =>
            println( s"inSignals: Not Yet Implemented: ${x}")
            List()
        }

      s match {
        case _ : Block =>
          s map constructTimingArcsStatement
          ()
        case Connect( info, lhsRef, rhsRef) =>
//          println( s"$s")
          lhsRef match {
            case _: WRef | _: WSubField =>
              val lhsNode = LogicNode( m.name, lhsRef)
              val lhsNode0 = if ( regs.contains( lhsNode)) regs(lhsNode)._2 else lhsNode

              for ( (rhsNode0,width) <- inSignals( List(rhsRef))) {
                tas( ( rhsNode0, lhsNode0)) = (0,"connect")
              }
          }
        case EmptyStmt =>
        case _ : DefRegister => //println( s"$s")
        case _ : DefWire => //println( s"$s")
        case DefNode( info, lhs, rhs) =>
          val (o,lstOfLsts) = rhs match {
            case Mux( cond, te, fe, tpe) =>
              ("mux",List(inSignals(List(cond)),inSignals(List(te,fe))))
            case DoPrim( op, inps, _, tpe) =>
              val opStr = s"${op}"
              ( opStr, List(inSignals(inps)))
            case ValidIf( cond, te, tpe) =>
              ("validif",List(inSignals(List(cond)),inSignals(List(te))))
            case _ : WRef => ("copy",List(inSignals(List(rhs))))
            case _ : Literal => ("literal",List())
            case _ =>
              println( s"constructTimingArcs: DefNode Not Yet Implemented: ${rhs}")
              ("nyi",List())
          }
          
          val widths = (lstOfLsts.map { lst =>
            lst.map( _._2).mkString( "(", ",", ")")
          }).mkString( "(", ",", ")")
          val oo = s"${o},${widths}"
          val delay =
            o match {
              case "pad" => 0
              case "tail" => 0
              case "bits" => 0
              case "cvt" => 0
              case "asSInt" => 0
              case "asUInt" => 0
              case "add" => 3
              case "sub" => 3
              case "mul" => 10
              case "div" => 10
              case "rem" => 10
              case "mux" =>
                if ( widths == "((1),())")
                  0
                else if ( widths.startsWith( "((),"))
                  0
                else
                  1
              case "validif" => 0 
              case "copy" => 0
              case "eq" | "and" | "or" | "lt" | "gt" | "leq" | "geq" =>
                if ( widths == "((1))") 0 else 1
              case _ => 
                println(s"Unknown primitive ${o}. Assign delay to 1")
                1
            }
          for { lst <- lstOfLsts
                (f,width) <- lst} {
            val tgt = LogicNode(m.name,lhs)
            tas( (f,tgt)) = (delay,oo)
          }
        case _ : IsInvalid =>
        case _ : Stop =>
        case _ : Print =>
        case DefMemory( info, nm, dataType, depth, writeLatency, readLatency, readers, writers, readwriters, readUnderWrite) =>
          if ( readLatency == 0) {
            for { rd <- readers} {
              val data = LogicNode( m.name, s"${nm}.${rd}.data")
              val en = LogicNode( m.name, s"${nm}.${rd}.en")
              val addr = LogicNode( m.name, s"${nm}.${rd}.addr")
              tas( (en,data)) = (10,"rden,${extractWidth(dataType)}")
              tas( (addr,data)) = (10,"rd,${extractWidth(dataType)}")
            }
          }
        case _ =>
          println( s"constructTimingArcs: Not Yet Implemented: ${s}")
      }

      s
    }

    println( s"constructTimingArcs: ${m.name}")
    m map constructTimingArcsStatement
  }

    
  def apply( m : DefModule) : Map[String,Int] = {

    val regs = mutable.Map[String,(String,String)]()
    val mems = mutable.Set[DefMemory]()
    val tas = mutable.Map.empty[(String,String),(Int,String)]
  
    constructTimingArcs( tas, regs, mems)(m)
    val delayMap = mutable.Map.empty[String,Int]
    for { ((src,tgt),(delay,o)) <- tas} {
      if ( !delayMap.contains( tgt)) {
        delayMap(tgt) = delay
      } else {
        delayMap(tgt) = delayMap(tgt) max delay
      }
      //println( s"${src},${tgt} -> ${delay},${o}: ${delayMap(tgt)}")
    }

    delayMap.toMap

  }
}
