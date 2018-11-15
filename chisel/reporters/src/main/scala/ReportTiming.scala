package reporters

import scala.annotation.tailrec
import scala.math.Ordered
import scala.math.Ordering.Implicits._

import java.io._

/*
 * To do: replace spray.json with the play version; allows json objects to be send to Ok controller methods.
 * For the server:
 *   Do required modeling
 *   Do tracing through arbitrary net (or timing arc)---not just PIs or POs
 */


import spray.json._
import DefaultJsonProtocol._ // if you don't supply your own Protocol (see below)

import firrtl._
import firrtl.ir._
import firrtl.passes._
import firrtl.annotations._
import firrtl.graph._
import firrtl.analyses.InstanceGraph
import firrtl.Mappers._
import firrtl.WrappedExpression._
import firrtl.Utils.{throwInternalError, toWrappedExpression, kind}
import firrtl.MemoizedHash._
import wiring.WiringUtils.getChildrenMap

import collection.mutable

class ReportTiming( val area_timing : Boolean = false,
                    val interface_paths : Boolean = false,
                    val full_traces : Boolean = true,
                    val startServer : Boolean = false ) extends Transform {
  def inputForm = LowForm
  def outputForm = LowForm

  private type LogicNode = String

  private object LogicNode {

    def apply(moduleName: String, name: String): LogicNode =
      apply(moduleName, WRef(name))

    def apply( moduleName: String, expr: Expression): LogicNode =
      expr match {
        case WRef( nm, _, _, _) => nm
        case WSubField( ref, nm, tpe, gndr) => LogicNode( moduleName, ref) + "." + nm
        case _ =>
          throwInternalError( s"LogicNode::apply ${expr}")
      }

  }


  class Arrival {
    var time : Option[Double] = None
    var u : Option[LogicNode] = None
    var op : Option[String] = None
    var info : Option[Info] = None
  }

  class Required {
    var time : Option[Double] = None
    var v : Option[LogicNode] = None
    var op : Option[String] = None
    var info : Option[Info] = None
  }

  val aMap = mutable.Map[String,mutable.Map[LogicNode,Arrival]]()
  val poMap = mutable.Map[String,Set[LogicNode]]()
  val rMap = mutable.Map[String,mutable.Map[LogicNode,Required]]()
  val piMap = mutable.Map[String,Set[LogicNode]]()


  /** Construct the dependency graph within this module */
  private def setupDepGraph( regs : mutable.Map[LogicNode,(LogicNode,LogicNode)],
                             mems : mutable.Set[DefMemory],
                             depGraph: MutableDiGraph[LogicNode])
                           (mod: DefModule): Unit = {

//    println( s"$mod")

    /** Extract all References and SubFields from a possibly nested Expression */
    def extractRefs(expr: Expression): Seq[Expression] = {
      val refs = mutable.ArrayBuffer.empty[Expression]
      def rec(e: Expression): Expression = {
        e match {
          case ref @ (_: WRef | _: WSubField) => refs += ref
          case nested @ (_: Mux | _: DoPrim | _: ValidIf) => nested map rec
          case ignore @ (_: Literal) => // Do nothing
          case unexpected => throwInternalError("")
        }
        e
      }
      rec(expr)
      refs
    }

    def getDeps( side : String)(expr: Expression): Seq[LogicNode] = {

      def getHierName( e : Expression) : String = e match {
        case WRef( nm, _, _, _) => nm
        case WSubField( expr, nm, tpe, gender) =>
          getHierName( expr) + "." + nm
        case _ =>
          throwInternalError( s"getHierName: $e")
      }


      extractRefs(expr).map { e =>
        if (kind(e) == InstanceKind) {
          throwInternalError( s"getDeps: ${e}")
        } else {
          val nm = getHierName( e)
          if ( regs.contains( nm)) {
            if        ( side == "lhs") {
              LogicNode(mod.name, regs(nm)._2) // ns version
            } else if ( side == "rhs") {
              LogicNode(mod.name, regs(nm)._1) // ps version
            } else {
              throwInternalError( s"getDeps: ${e} ${nm}")
            }
          } else
            LogicNode(mod.name, e)
        }
      }

    }

    def onStmt(stmt: Statement): Unit = stmt match {
      case DefRegister(_, name, tpe, clock, reset, init) =>
        val node = LogicNode(mod.name, name)
        assert( regs.contains( node))
        val (ps,ns) = regs(node)
        depGraph.addVertex(ps)
        depGraph.addVertex(ns)
        Seq( reset, init).flatMap(getDeps("rhs")(_)).foreach(ref => {
          depGraph.addVertex(ref)
          depGraph.addEdge(ns, ref)
        })
      case DefNode(_, name, value) =>
        val node = LogicNode(mod.name, name)
        depGraph.addVertex(node)
        getDeps("rhs")(value).foreach( ref => {
          depGraph.addVertex(ref)
          depGraph.addEdge(node, ref)
        })
      case DefWire(_, name, _) =>
        depGraph.addVertex(LogicNode(mod.name, name))
      case mem : DefMemory =>
        val nm = mem.name
//        
// Seems like we want this: 
//    rType: clk, en, addr, data
//      readLatency = 0
//        en -> data 
//        addr -> data
//      readLatency > 0
//        en -> clk
//        addr -> clk
//        clk -> data
//
//    wType: clk, en, addr, data, mask  
//      writeLatency > 0
//        en -> clk   
//        addr -> clk   
//        data -> clk   
//        mask -> clk   
//
//    rwType: clk, en, addr, rdata, wmode, wdata, wmask
//

        val combinationalArcs = {
//          println( s"DefMemory: readLatency ${mem.readLatency}")
          if ( mem.readLatency == 0) {
//            println( s"DefMemory: ${mem.readers}")
            mem.readers.toList.flatMap{ rd =>
              val en = LogicNode( mod.name, s"${nm}.${rd}.en")
              val data = LogicNode( mod.name, s"${nm}.${rd}.data")
              val addr = LogicNode( mod.name, s"${nm}.${rd}.addr")
              List( ( en, data), ( addr, data))
            }
          } else {
            List()
          }
        }

        for { ( src, tgt) <- combinationalArcs} {
          depGraph.addVertex( src)
          depGraph.addVertex( tgt)
          depGraph.addEdge( tgt, src)
//          println( s"combinationalArc: ${src} -> ${tgt}")
        }




      case Attach(_, exprs) => // Add edge between each expression
/*
        exprs.flatMap(getDeps("rhs")(_)).toSet.subsets(2).map(_.toList).foreach {
          case Seq(a, b) =>
            depGraph.addEdge(a, b)
            depGraph.addEdge(b, a)
        }
 */
        throwInternalError( s"Don't know what to do here: ${stmt}")
      case Connect(_, loc, expr) =>
//        println( s"Connect: ${loc} ${expr}")
        // This match enforces the low Firrtl requirement of expanded connections
        val node = getDeps("lhs")(loc) match { case Seq(elt) => elt }
// only one loc; zero (constant) or one expr
        getDeps("rhs")(expr).foreach{ ref =>
//          println( s"ref: ${ref}")
          depGraph.addVertex(node)
          depGraph.addVertex(ref)
          depGraph.addEdge(node, ref)
        }

      case Block(stmts) => stmts.foreach(onStmt(_))

      case Stop(_,_, clk, en) => // do nothing
      case Print(_, _, args, clk, en) => // do nothing
      case ignore @ (_: IsInvalid | _: WDefInstance | EmptyStmt) => // do nothing
      case other => throwInternalError("")
    }

    // Add all ports as vertices
    mod.ports.foreach {
      case Port(_, name, _, _: GroundType) => depGraph.addVertex(LogicNode(mod.name, name))
      case other => throwInternalError("")
    }

    mod match {
      case m : Module => onStmt(m.body)
      case m : ExtModule => 
    }
  }

  private def createDependencyGraph( regs : mutable.Map[LogicNode,(LogicNode,LogicNode)],
                                     mems : mutable.Set[DefMemory])
                                   ( m : DefModule): DiGraph[LogicNode] = {

    val depGraph = new MutableDiGraph[LogicNode]
    m match {
      case mod : Module => setupDepGraph(regs, mems, depGraph)(mod)
      case mod : ExtModule => setupDepGraph(regs, mems, depGraph)(mod)
      case _ => throwInternalError("")
    }
    DiGraph( depGraph)
  }

  def allRegs( regs : mutable.Map[LogicNode,(LogicNode,LogicNode)],
               modName : String)
             ( s : Statement) : Statement = {
    s map allRegs( regs, modName)
    s match {
      case DefRegister(_, name, tpe, clock, reset, init) =>
        val node = LogicNode( modName, name)
        val nodePS = LogicNode( modName, s"${name}#ps")
        val nodeNS = LogicNode( modName, s"${name}#ns")
        regs(node) = (nodePS,nodeNS)
      case _ => 
    }
    s
  }

  def allMems( mems : mutable.Set[DefMemory],
               modName : String)
             ( s : Statement) : Statement = {
    s map allMems( mems, modName)
    s match {
      case mem : DefMemory => mems += mem
      case _ => 
    }
    s
  }


  def constructTimingArcs
    ( tas : mutable.Map[(LogicNode,LogicNode),(Int,String,Int,Info)],
      regs : mutable.Map[LogicNode,(LogicNode,LogicNode)],
      mems : mutable.Set[DefMemory])
    ( m : DefModule): DefModule = {

    def constructTimingArcsStatement( s : Statement): Statement = {

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

      def inSignals( inp : Seq[Expression]) : List[(LogicNode,Int)] =
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
          case x@DoPrim( padOrBits, lst, _, _) if List("pad","bits","asSInt","asUInt").contains(s"$padOrBits") =>
//            println( s"Recursive call: ${x}")
            inSignals( lst)
          case x@Mux( c, t, e, tpe) =>
//            println( s"inSignals: ${x}")
            inSignals( List(c,t,e))
          case x =>
            println( s"inSignals: Not Yet Implemented: ${x}")
            List()
        }

      def doRHS( rhs:firrtl.ir.Expression) = {
          val (o,lstOfLsts,a) = rhs match {
            case Mux( cond, te, fe, tpe) =>
              ("mux",List(inSignals(List(cond)),inSignals(List(te,fe))), AreaMux( extractWidth(tpe), isConst(cond), isConst(te) + isConst(fe)))
            case DoPrim( op, inps, _, tpe) =>
//              println( s"DoPrim match: ${op}")
              val opStr = s"${op}"
              val inpList = List()

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

              ( opStr, List(inSignals(inps)), AreaOp( opStr, inpSizes, extractWidth(tpe), c))
            case ValidIf( cond, te, tpe) =>
              ("validif",List(inSignals(List(cond)),inSignals(List(te))), AreaNone)
            case _ : WRef => ("copy",List(inSignals(List(rhs))), AreaNone)
            case _ : WSubField => ("copy",List(inSignals(List(rhs))), AreaNone)
            case _ : Literal => ("literal",List(), AreaNone)
            case _ =>
              println( s"constructTimingArcs: DefNode Not Yet Implemented: ${rhs}")
              ("nyi",List(), AreaNone)
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
              case "not" => 0
              case "cat" => 0
              case "asSInt" | "asUInt" => 0
              case "add" | "sub" | "gt" | "lt" | "geq" | "leq" =>
                if ( lstOfLsts.isEmpty || lstOfLsts.head.length < 1)
                  0
                else if ( lstOfLsts.head.length < 2)
                  chisel3.util.log2Floor( lstOfLsts.head.head._2)
                else
                  chisel3.util.log2Floor( lstOfLsts.head.head._2) + 1
              case "mul" =>
// assumes only shifting if one or both is a constant
                if ( lstOfLsts.isEmpty || lstOfLsts.head.length < 2)
                  0
                else
                  2*chisel3.util.log2Floor( lstOfLsts.head.head._2)
              case "div" | "rem" =>
// assumes only shifting if one or both is a constant
                if ( lstOfLsts.isEmpty || lstOfLsts.head.length < 2)
                  0
                else
                  lstOfLsts.head.head._2
              case "shr" => 0
              case "dshlw" => 3
              case "shlw" => 0
              case "mux" =>
                if ( widths == "((1),())")
                  0
                else if ( widths.startsWith( "((),"))
                  0
                else
                  1
              case "validif" => 0 
              case "copy" => 0
              case "literal" => 0
              case "eq" | "neq" | "and" | "or" | "xor" | "xnor" =>
//                println( s"${o} ${oo}  ${lstOfLsts}")
                if ( lstOfLsts.isEmpty || lstOfLsts.head.length < 1)
                  0
                else if ( lstOfLsts.head.length < 2)
                  chisel3.util.log2Floor( lstOfLsts.head.head._2)
                else
                  chisel3.util.log2Floor( lstOfLsts.head.head._2) + 1
              case _ => 
                println(s"Unknown primitive ${o}. Assign delay to 1")
                1
            }
          val area = ComputeArea( a, mutable.Map[String,Int]())
          (area,delay,oo,lstOfLsts)

      }
      s match {
        case _ : Block =>
          s map constructTimingArcsStatement
          ()
        case Connect( info, lhsRef, rhsRef) =>
//          println( s"$s: ${rhsRef}")
//          println( s"$info")
          lhsRef match {
            case _: WRef | _: WSubField =>
              val lhsNode = LogicNode( m.name, lhsRef)
              val lhsNode0 = if ( regs.contains( lhsNode)) regs(lhsNode)._2 else lhsNode

/*
              for ( (rhsNode0,width) <- inSignals( List(rhsRef))) {
//                println( s"${rhsNode0}")
                tas( ( rhsNode0, lhsNode0)) = (0,"connect",0,info)
              }
 */
              val (area,delay,oo,lstOfLsts) = doRHS( rhsRef)
              for { lst <- lstOfLsts
                    (f,width) <- lst} {
//                println( s"${f} ${tgt} ${delay} ${oo}")
                tas( (f,lhsNode0)) = (delay,oo,area,info)
              }

          }
        case EmptyStmt =>
        case _ : DefRegister => //println( s"$s")
        case _ : DefWire => //println( s"$s")
        case DefNode( info, lhs, rhs) =>
          val (area,delay,oo,lstOfLsts) = doRHS( rhs)
          for { lst <- lstOfLsts
                (f,width) <- lst} {
            val tgt = LogicNode(m.name,lhs)
//            println( s"${f} ${tgt} ${delay} ${oo}")
            tas( (f,tgt)) = (delay,oo,area,info)
          }
        case _ : IsInvalid =>
        case _ : Stop =>
        case _ : Print =>
        case mem : DefMemory =>
          val nm = mem.name
          if ( mem.readLatency == 0) {
            for { rd <- mem.readers} {
              val data = LogicNode( m.name, s"${nm}.${rd}.data")
              val en = LogicNode( m.name, s"${nm}.${rd}.en")
              val addr = LogicNode( m.name, s"${nm}.${rd}.addr")
              tas( (en,data)) = (10,s"rden,${extractWidth(mem.dataType)}",100,mem.info)
              tas( (addr,data)) = (10,s"rd,${extractWidth(mem.dataType)}",100,mem.info)
            }
          }
        case _ =>
          println( s"constructTimingArcs: Not Yet Implemented: ${s}")
      }

      s
    }

//    println( s"constructTimingArcs: ${m.name}")
    m map constructTimingArcsStatement
  }

    def worstSinkSet( arrivals : mutable.Map[LogicNode,Arrival], poSet : Set[LogicNode]) : Option[ (LogicNode,Double)] = {
      val sinks =
        for { v <- poSet
              if arrivals.contains( v)
              t <- arrivals(v).time
            } yield (v,t)
      sinks.toSeq.sortBy{ case (v,t) => (t,v)}.lastOption
    }

    def worstSourceSet( requireds : mutable.Map[LogicNode,Required], piSet : Set[LogicNode]) : Option[ (LogicNode,Double)] = {
      val sources =
        for { u <- piSet
              if requireds.contains( u)
              t <- requireds(u).time
            } yield (u,t)
      sources.toSeq.sortBy{ case (u,t) => (t,u)}.lastOption
    }

    def worstThrough( arrivals : mutable.Map[LogicNode,Arrival],
                      requireds : mutable.Map[LogicNode,Required],
                      uv : LogicNode) : Option[ (LogicNode,Double)] = {
      if ( arrivals.contains( uv) && requireds.contains( uv)) {
        for { at <- arrivals(uv).time
              rt <- requireds(uv).time} yield ( uv, at+rt)
      } else {
        None
      }
    }

    def jsonTraces( arrivals : mutable.Map[LogicNode,Arrival], poSet : Set[LogicNode]) : String = {
      @tailrec
      def backTrace( accum : List[LogicNode], n : Option[LogicNode]) : List[LogicNode] =
        n match {
          case None => accum
          case Some( nn) => backTrace( nn :: accum, arrivals(nn).u)
        }

      val sinks =
        for { v <- poSet
              t <- arrivals(v).time
            } yield (v,t)
      val sortedSinks = sinks.toSeq.sortBy{ case (v,t) => (t,v)}

      val paths = for { (v,t) <- sortedSinks} yield {
        val trc = backTrace( List.empty[LogicNode], Some(v))
        val header = JsObject( "delay" -> JsNumber( arrivals(v).time.get),
          "source" -> JsString( trc.head),
          "sink" -> JsString( v))
        
        val trace = mutable.ArrayBuffer[JsObject]()
        for { v <- trc} {
          val a = arrivals(v)
          if ( !a.u.isEmpty) {
//            val fanout = reverseDepGraph.getEdges( v).size
            val incr = if (a.u.isEmpty) 0 else a.time.getOrElse( 0.0) - arrivals(a.u.get).time.getOrElse( 0.0)
            val op = a.op.getOrElse( "<unknown>")
            val info = a.info.getOrElse( NoInfo)
            trace.append( JsObject( "op" -> JsString( op), "incr" -> JsNumber(incr)/*, "fanout" -> JsNumber( fanout)*/, "info" -> JsString( s"${info}")))
          }
          trace.append( JsObject( "arrival" -> JsNumber( a.time.getOrElse( 0.0)), "net" -> JsString(v)))
        }
        JsObject( "header" -> header, "trace" -> JsArray( trace:_*))
      }

      JsArray( paths:_*).prettyPrint
    }

    def jsonRequiredTraces( requireds : mutable.Map[LogicNode,Required], piSet : Set[LogicNode]) : String = {
      @tailrec
      def forwardTrace( accum : List[LogicNode], n : Option[LogicNode]) : List[LogicNode] =
        n match {
          case None => accum
          case Some( nn) => forwardTrace( nn :: accum, requireds(nn).v)
        }

      val sources =
        for { u <- piSet
              t <- requireds(u).time
            } yield (u,t)
      val sortedSources = sources.toSeq.sortBy{ case (u,t) => (t,u)}

      val paths = for { (u,t) <- sortedSources} yield {
        val trc = forwardTrace( List.empty[LogicNode], Some(u)).reverse
        val header = JsObject( "delay" -> JsNumber( requireds(u).time.get),
          "source" -> JsString( u),
          "sink" -> JsString( trc.head))
        
        val trace = mutable.ArrayBuffer[JsObject]()
        for { u <- trc} {
          val r = requireds(u)
          trace.append( JsObject( "required" -> JsNumber( r.time.getOrElse( 0.0)), "net" -> JsString(u)))
          if ( !r.v.isEmpty) {
//            val fanout = reverseDepGraph.getEdges( v).size
            val incr = if (r.v.isEmpty) 0 else r.time.getOrElse( 0.0) - requireds(r.v.get).time.getOrElse( 0.0)
            val op = r.op.getOrElse( "<unknown>")
            val info = r.info.getOrElse( NoInfo)
            trace.append( JsObject( "op" -> JsString( op), "incr" -> JsNumber(incr)/*, "fanout" -> JsNumber( fanout)*/, "info" -> JsString( s"${info}")))
          }
        }
        JsObject( "header" -> header, "trace" -> JsArray( trace:_*))
      }

      JsArray( paths:_*).prettyPrint
    }

  def executePerModule(m : DefModule): DefModule = {

// open up output file

    val fp = new File( s"${m.name}.rpt")
    val pw = new PrintWriter(new BufferedWriter(new FileWriter( fp)))

    val regs = mutable.Map[LogicNode,(LogicNode,LogicNode)]()
    val mems = mutable.Set[DefMemory]()

    m map allRegs( regs, name)
    m map allMems( mems, name)

    val depGraph = createDependencyGraph( regs, mems)( m)

    val reverseDepGraph = depGraph.reverse

    val tas = mutable.Map.empty[(LogicNode,LogicNode),(Int,String,Int,Info)]

    constructTimingArcs( tas, regs, mems)( m)

    val sccs = reverseDepGraph.findSCCs
    val cyclicComponents = sccs.filter( _.size > 1).sortBy( _.size)
    if ( cyclicComponents.size > 0) {
      pw.println( s"Total SCCs: ${sccs.size} Cyclic SCC sizes: ${cyclicComponents.map(_.size)}")
      pw.println( s"Smallest cycle:")
      for ( v <- cyclicComponents(0)) {
        pw.println( s"\t$v forward: ${reverseDepGraph.getEdges(v)} backward: ${depGraph.getEdges(v)}")
      }
      throw new Exception( "Trying to perform timing analysis on cyclic graph.")
    }



    val topoOrder = reverseDepGraph.linearize
    val arrivals = mutable.Map.empty[LogicNode,Arrival]
    aMap(m.name) = arrivals
   
    def assignArrivals : Unit =
      for { v <- topoOrder} {
        for { u <- depGraph.getEdges( v)
              a <- arrivals( u).time} {

          val (delay,op,area,info) = tas.getOrElse( ( u, v), (0,"unknown",0,NoInfo))

          val newA = a + delay

          val t = arrivals(v).time
          if ( t.isEmpty || t.get < newA) {
            arrivals(v).time = Some( newA)
            arrivals(v).u = Some( u)
            arrivals(v).op = Some( op)
            arrivals(v).info = Some( info)
          }
        }
      }

    val backwardTopoOrder = depGraph.linearize
    val requireds = mutable.Map.empty[LogicNode,Required]
    rMap(m.name) = requireds

    def assignRequireds : Unit =
      for { u <- backwardTopoOrder} {
        for { v <- reverseDepGraph.getEdges( u)
              r <- requireds( v).time} {

          val (delay,op,area,info) = tas.getOrElse( ( u, v), (0,"unknown",0,NoInfo))

          val newR = r + delay

          val t = requireds(u).time
          if ( t.isEmpty || t.get < newR) {
            requireds(u).time = Some( newR)
            requireds(u).v = Some( v)
            requireds(u).op = Some( op)
            requireds(u).info = Some( info)
          }
        }
      }

    def printTraces( poSet : Option[Set[LogicNode]], printHeader : Boolean, printTrace : Boolean) : Unit = {
      @tailrec
      def backTrace( accum : List[LogicNode], n : Option[LogicNode]) : List[LogicNode] =
        n match {
          case None => accum
          case Some( nn) => backTrace( nn :: accum, arrivals(nn).u)
        }

      val sinks =
        for { v <- topoOrder
              if !poSet.isDefined && reverseDepGraph.getEdges( v).isEmpty || poSet.isDefined && poSet.get.contains( v)
              t <- arrivals(v).time
            } yield (v,t)
      val sortedSinks = sinks.sortBy{ case (v,t) => (t,v)}

      for { (v,t) <- sortedSinks} {
        val trc = backTrace( List.empty[LogicNode], Some(v))
        if ( printHeader) {
          pw.println( f"Path: ${arrivals(v).time.get}%8.2f  ${trc.head} -> ${v}")
        }
        if ( printTrace) {
          pw.println( s"Node Type            Incr  Arrival    Required   Fanout  Node Name")
          pw.println( s"-------------------- ----- ---------  ---------  ------  ------------------------------")
          for { v <- trc} {
            val a = arrivals(v)
            if ( !a.u.isEmpty) {
              val fanout = reverseDepGraph.getEdges( v).size
              val incr = if (a.u.isEmpty) 0 else a.time.getOrElse( 0.0) - arrivals(a.u.get).time.getOrElse( 0.0)
              val op = a.op.getOrElse( "<unknown>")
              pw.println( f"${op}%-20s ${incr}%5.1f                       ${fanout}%6d")
            }
            pw.println( f"                           ${a.time.getOrElse( 0.0)}%9.1f                     ${v}%-30s")
          }
          if ( printHeader) {
            pw.println( s"==================== ===== =========  =========  ======  ==============================")
          }
        }
      }
    }

    def printTracesJson( bw : BufferedWriter, poSet : Option[Set[LogicNode]]) : Unit = {
      @tailrec
      def backTrace( accum : List[LogicNode], n : Option[LogicNode]) : List[LogicNode] =
        n match {
          case None => accum
          case Some( nn) => backTrace( nn :: accum, arrivals(nn).u)
        }

      val sinks =
        for { v <- topoOrder
              if !poSet.isDefined && reverseDepGraph.getEdges( v).isEmpty || poSet.isDefined && poSet.get.contains( v)
              t <- arrivals(v).time
            } yield (v,t)
      val sortedSinks = sinks.sortBy{ case (v,t) => (t,v)}

      val paths = for { (v,t) <- sortedSinks} yield {
        val trc = backTrace( List.empty[LogicNode], Some(v))
        val header = JsObject( "delay" -> JsNumber( arrivals(v).time.get),
          "source" -> JsString( trc.head),
          "sink" -> JsString( v))
        
        val trace = mutable.ArrayBuffer[JsObject]()
        for { v <- trc} {
          val a = arrivals(v)
          if ( !a.u.isEmpty) {
            val fanout = reverseDepGraph.getEdges( v).size
            val incr = if (a.u.isEmpty) 0 else a.time.getOrElse( 0.0) - arrivals(a.u.get).time.getOrElse( 0.0)
            val op = a.op.getOrElse( "<unknown>")
            trace.append( JsObject( "op" -> JsString( op), "incr" -> JsNumber(incr), "fanout" -> JsNumber( fanout)))
          }
          trace.append( JsObject( "arrival" -> JsNumber( a.time.getOrElse( 0.0)), "net" -> JsString(v)))
        }
        JsObject( "header" -> header, "trace" -> JsArray( trace:_*))
      }

      bw.write( JsArray( paths:_*).prettyPrint)
      bw.write( "\n")
    }

    def printRequiredTraces( piSet : Option[Set[LogicNode]], printHeader : Boolean, printTrace : Boolean) : Unit = {
      @tailrec
      def forwardTrace( accum : List[LogicNode], n : Option[LogicNode]) : List[LogicNode] =
        n match {
          case None => accum
          case Some( nn) => forwardTrace( nn :: accum, requireds(nn).v)
        }

      val sources =
        for { u <- backwardTopoOrder
              if !piSet.isDefined && depGraph.getEdges( u).isEmpty || piSet.isDefined && piSet.get.contains( u)
              t <- requireds(u).time
            } yield (u,t)
      val sortedSources = sources.sortBy{ case (u,t) => (t,u)}

      for { (u,t) <- sortedSources} {
        val trc = forwardTrace( List.empty[LogicNode], Some(u)).reverse

        if ( printHeader) {
          pw.println( f"Path: ${requireds(u).time.get}%8.2f  ${u} -> ${trc.last}")
        }
        if ( printTrace) {
          if ( printHeader) {
            pw.println( s"Node Type            Incr  Arrival    Required   Fanout  Node Name")
            pw.println( s"-------------------- ----- ---------  ---------  ------  ------------------------------")
          }
          for { u <- trc} {
            val r = requireds(u)

            pw.println( f"                                      ${r.time.getOrElse( 0.0)}%9.1f          ${u}%-30s")
            if ( !r.v.isEmpty) {
              val fanout = if (r.v.isEmpty) 0 else reverseDepGraph.getEdges( r.v.get).size
              val incr = if (r.v.isEmpty) 0 else r.time.getOrElse( 0.0) - requireds(r.v.get).time.getOrElse( 0.0)
              val op = r.op.getOrElse( "<unknown>")
              pw.println( f"${op}%-20s ${incr}%5.1f                       ${fanout}%6d")
            }

          }
          pw.println( s"==================== ===== =========  =========  ======  ==============================")
        }
      }
    }



// collect PIs
    val pis = mutable.ArrayBuffer.empty[String]
    val pos = mutable.ArrayBuffer.empty[String]
    m.ports.foreach {
      case Port( _, name, Input, _: GroundType) => pis += name
      case Port( _, name, Output, _: GroundType) => pos += name
      case Port( _, name, pt, g) =>
        println( s"Warning: unknown point direction ${pt} ${g}")
    }

    val piSet = Set( (pis.map(LogicNode(m.name,_))): _*) - LogicNode(m.name,"clock")
    val poSet = Set( (pos.map(LogicNode(m.name,_))): _*)

    val psSet0 = Set( (regs.toSeq.map{ case (v,(ps,ns)) => ps}): _*)
    val nsSet0 = Set( (regs.toSeq.map{ case (v,(ps,ns)) => ns}): _*)

    val psSet1 = mutable.Set[LogicNode]()
    val nsSet1 = mutable.Set[LogicNode]()

    for { mem <- mems} {
      val nm = mem.name
      for { rd <- mem.readers} {
        if ( mem.readLatency > 0) {
          nsSet1 += LogicNode( m.name, s"${nm}.${rd}.en")
          nsSet1 += LogicNode( m.name, s"${nm}.${rd}.addr")
          psSet1 += LogicNode( m.name, s"${nm}.${rd}.data")
        }
      }
      for { wr <- mem.writers} {
        nsSet1 += LogicNode( m.name, s"${nm}.${wr}.en")
        nsSet1 += LogicNode( m.name, s"${nm}.${wr}.data")
        nsSet1 += LogicNode( m.name, s"${nm}.${wr}.addr")
        nsSet1 += LogicNode( m.name, s"${nm}.${wr}.mask")
      }
    }

    val psSet = psSet0 ++ psSet1
    val nsSet = nsSet0 ++ nsSet1

    arrivals.clear
    for { v <- topoOrder} {
      arrivals(v) = new Arrival
      if ( depGraph.getEdges( v).isEmpty && (piSet.contains(v) || psSet.contains(v))) {
        arrivals(v).time = Some(0)
        arrivals(v).op = Some("No incoming edges")
      }
    }
    assignArrivals

    requireds.clear
    for { u <- backwardTopoOrder} {
      requireds(u) = new Required
      if ( reverseDepGraph.getEdges( u).isEmpty && (poSet.contains(u) || nsSet.contains(u))) {
        requireds(u).time = Some(0)
        requireds(u).op = Some("No outgoing edges")
      }
    }
    assignRequireds

    println( s"Worst path delay to any PO or Reg: ${worstSinkSet( arrivals, poSet ++ nsSet)}")
    println( s"Worst path delay from any PI or Reg: ${worstSourceSet( requireds, piSet ++ psSet)}")


/*
    def computeMaxCycleRatio( useRatio : Boolean = true) : Double = {
// Primal Dual Max Cycle Ratio
      val pd = new PD[Int]
      val vertexMap = mutable.Map.empty[LogicNode,Int]

      for { v <- Seq( "##source##", "##sink##") ++ depGraph.getVertices} {
        vertexMap += v -> vertexMap.size
        pd.addVertex( vertexMap(v))
      }
      for { v <- depGraph.getVertices
            u <- depGraph.getEdges( v)} {
        val (delay,op,area,info) = tas.getOrElse( ( u, v), (0,"unknown",0,NoInfo))
        pd.addEdge( vertexMap( u), vertexMap( v), delay.toDouble, 0, area.toDouble)
      }

      for { (_,(ps,ns)) <- regs} {
        if ( useRatio) {
          pd.addEdge( vertexMap(ns), vertexMap(ps), 0.0, 1, 0.0)
        } else {
          pd.addEdge( vertexMap(ns), vertexMap("##sink##"), 0.0, 0, 0.0)
          pd.addEdge( vertexMap("##source##"), vertexMap(ps), 0.0, 0, 0.0)
        }
      }

      for { mem <- mems} {
        val nm = mem.name
        for { rd <- mem.readers} {
          val en = LogicNode( m.name, s"${nm}.${rd}.en")
          val data = LogicNode( m.name, s"${nm}.${rd}.data")
          val addr = LogicNode( m.name, s"${nm}.${rd}.addr")
          if ( mem.readLatency > 0) {
            for { v <- List( en, data, addr)}  {
              if ( !vertexMap.contains(v)) {
                vertexMap += v -> vertexMap.size
                pd.addVertex( vertexMap(v))
              }
            }
            pd.addEdge( vertexMap(en),   vertexMap("##sink##"), 0.0, 0, 1.0)
            pd.addEdge( vertexMap(addr), vertexMap("##sink##"), 0.0, 0, 1.0)
            pd.addEdge( vertexMap( "##source##"), vertexMap(data), 0.0, 0, 1.0)
          }
        }
        for { wr <- mem.writers} {
          val en   = LogicNode( m.name, s"${nm}.${wr}.en")
          val data = LogicNode( m.name, s"${nm}.${wr}.data")
          val addr = LogicNode( m.name, s"${nm}.${wr}.addr")
          val mask = LogicNode( m.name, s"${nm}.${wr}.mask")
          for { v <- List( en, data, addr, mask)}  {
            if ( !vertexMap.contains(v)) {
              vertexMap += v -> vertexMap.size
              pd.addVertex( vertexMap(v))
            }
          }
          pd.addEdge( vertexMap(en),   vertexMap("##sink##"), 0.0, 0, 1.0)
          pd.addEdge( vertexMap(addr), vertexMap("##sink##"), 0.0, 0, 1.0)
          pd.addEdge( vertexMap(data), vertexMap("##sink##"), 0.0, 0, 1.0)
          pd.addEdge( vertexMap(mask), vertexMap("##sink##"), 0.0, 0, 1.0)
        }
      }

      for { pi <- Seq( "clock") ++ piSet} pd.addEdge( vertexMap("##source##"), vertexMap(pi), 0.0, 0, 0.0)
      for { po <- poSet} pd.addEdge( vertexMap( po), vertexMap("##sink##"), 0.0, 0, 0.0)
      pd.addEdge( vertexMap( "##sink##"), vertexMap( "##source##"), 0.0, 1, 0.0)

      println(s"Running the PrimalDual version:")
      //val p = pd.maxCycleRatio
      val Seq( (p,_)) = pd.maxCycleRatioWithLP( targetTheta=Some(Seq(1.0)))
      println(s"maxCycleRatio: $p")


      if ( area_timing) {

        println(s"Running the LP version:")

        val fractions = (0 to 20) map ( _ / 20.0)

        val results1 = pd.maxCycleRatioWithLP( inverterDelaysPerDevice=None, targetCyclePeriod=Some(fractions.map( (f : Double) => p*(1.0+f))) )
        val maps1 = for ( (fraction,(pWithLP,areaSum)) <- fractions zip results1) yield {
          println(s"maxCycleRatio: $pWithLP areaSum: $areaSum")
          Map( "fraction" -> fraction, "period" -> pWithLP, "area" -> areaSum)
        }

        val thetas = (0 to 20) map ( _ / 20.0 + 1.0)

        val results2 = pd.maxCycleRatioWithLP( targetTheta=Some(thetas))
        val maps2 = for ( (theta,(pWithLP,areaSum)) <- thetas zip results2) yield {
          println(s"maxCycleRatio: $pWithLP areaSum: $areaSum")
          Map( "theta" -> theta, "period" -> pWithLP, "area" -> areaSum)
        }

        val jsonAST =
          JsArray(
            JsObject(
              "key" -> JsString( if ( useRatio) "FixedRatio" else "FixedDelay"),
              "color" -> JsString( if ( useRatio) "#0000ff" else "#ff00ff"),
              "values" -> maps1.toJson
            ),
            JsObject(
              "key" -> JsString( if ( useRatio) "ThetaRatio" else "ThetaDelay"),
              "color" -> JsString( if ( useRatio) "#00ff00" else "#ffff00"),
              "values" -> maps2.toJson
            )
          )

        {
          val fp = new File( if ( useRatio) "tradeoffRatio.json" else "tradeoffPeriod.json")
          val bw = new BufferedWriter(new FileWriter( fp))
          bw.write( jsonAST.prettyPrint)
          bw.close()
        }
      }

      p
    }
 */

    poMap(m.name) = (poSet ++ nsSet).toSet
    piMap(m.name) = (piSet ++ psSet).toSet

    if ( !startServer) {

      val sortedPairs = (for { v <- topoOrder
                               if !requireds(v).time.isEmpty && !arrivals(v).time.isEmpty} yield {
        val delay = requireds(v).time.get + arrivals(v).time.get
        (-delay, v)
      }).sortBy( x => x)

      val histo = mutable.Map[Int,Int]()

      if ( !sortedPairs.isEmpty) {
        val period = -sortedPairs.head._1
        for { (t,v) <- sortedPairs} {
          val i = scala.math.rint(t+period).toInt
          if ( !histo.contains( i)) histo(i) = 0
          histo(i) += 1
        }
      }

      val cumulativeGraph = collection.mutable.ArrayBuffer.empty[(Int,Int)]
      histo.toSeq.sortBy{ x => x}.foldLeft(0){ case (sum,(i,c)) =>
        cumulativeGraph.append( (i,sum))
        val newsum = sum + c
        cumulativeGraph.append( (i,newsum))
        newsum
      }

      {
        val fp = new File( "cumulativeGraph.json")
        val bw = new BufferedWriter(new FileWriter( fp))
        val tmp = JsArray( (for( (p0,p1) <- cumulativeGraph) yield JsArray( JsNumber(p0), JsNumber(p1))):_*)
        bw.write( JsObject( "cumulativeGraph" -> tmp).prettyPrint)
        bw.write( "\n")
        bw.close()
      }

      {
        val fp = new File( "sinkPaths.json")
        val bw = new BufferedWriter(new FileWriter( fp))
        printTracesJson( bw, Some( poSet ++ nsSet))
        bw.close()
      }

      pw.println( s"Worst paths to each PO or Reg:")
      pw.println( s"============================================================================")
      printTraces( Some(poSet ++ nsSet), true, full_traces)
      pw.println( s"============================================================================")

      pw.println( s"Worst paths from each PI or Reg:")
      pw.println( s"============================================================================")
      printRequiredTraces( Some(piSet ++ psSet), true, full_traces)
      pw.println( s"============================================================================")

      pw.println( s"Worst paths to and from each node:")
      pw.println( s"============================================================================")
      for { v <- sortedPairs.map{ case (t,v) => v} } {
        printTraces( Some( Set(v)), false, full_traces)
        printRequiredTraces( Some( Set(v)), false, full_traces)
      }
      pw.println( s"============================================================================")
    }

    if ( interface_paths) {
      pw.println( s"Interface paths:")
      pw.println( s"============================================================================")
      for { pi <- piSet.toList.sortBy( v => v)} {
        arrivals.clear
        for { v <- topoOrder} {
          arrivals(v) = new Arrival
          if ( depGraph.getEdges( v).isEmpty && LogicNode( m.name, pi) == v) {
            arrivals(v).time = Some(0)
            arrivals(v).op = Some("No incoming edges")
          }
        }
        assignArrivals
        pw.println( s"Worst paths from $pi to each PO or Reg:")
        printTraces( Some(poSet ++ nsSet), true, full_traces)
        pw.println( s"============================================================================")
      }
    }

/*
    if ( false) {
      println( s"Setting up Max Period Problem")
      val p = computeMaxCycleRatio( false)
      println( f"Max period: ${p}%8.2f")
      pw.println( s"Worst paths to each PO or Reg (LP generated):")
      pw.println( s"============================================================================")
      printTraces( Some(poSet ++ nsSet), true, full_traces)
      pw.println( s"============================================================================")
    }
    if ( false) {
      println( s"Setting up Max Cycle Ratio Problem")
      val p = computeMaxCycleRatio()
      println( f"Max cycle ratio: ${p}%8.2f")

    }
 */

    pw.close()

    m
  }

  def hasDefInstance( m : DefModule) : Boolean = {
//    println( s"Trying to find instances in ${m.name}")

    var value = false

    def onStmt( s : Statement) : Statement = {
      s map onStmt
      s match {
        case WDefInstance( info, instanceName, templateName, _) =>
          println( s"\tfound ${instanceName} ${templateName}")
          value = true
        case _ =>
      }
      s
    }

    m map onStmt
    value
  }

  def execute(state: CircuitState): CircuitState = {

    println( s"Going to run ReportTiming on these modules...")
    state.circuit.modules map {
      case m : ExtModule =>
        println( s"-W- ReportTiming: Found ExtModule by structure ${m.name} ...")
        m
      case m : Module =>
        if ( m.name == "BlackBoxAsyncFF" || m.name == "BlackBoxAsyncFF_1") {
          println( s"-W- ReportTiming: Found BlackBox by name")
        }
        m
    }

    val mods = state.circuit.modules map {
      case m : ExtModule =>
        println( s"-W- ReportTiming: Skipping BlackBoxes by structure ${m.name} ...")
        m
      case m : Module =>
        if ( hasDefInstance( m)) {
          println( s"-W- ReportTiming: Skipping processing on hierarchical module ${m.name} ...")
        } else if ( m.name == "BlackBoxAsyncFF" || m.name == "BlackBoxAsyncFF_1") {
          println( s"-W- ReportTiming: Skipping BlackBoxes by name ${m.name} ...")
        } else {
          println( s"Running ReportTiming on module ${m.name} ...")
          executePerModule( m)
          println( s"Finished running ReportTiming on module ${m.name} ...")
        }
        m
    }

    if ( startServer) {

      import play.api.mvc.{Action, AnyContent, RequestHeader, Results, Result}
      import play.api.routing.sird._


      val modules = JsArray( poMap.keys.toSeq.map{ JsString( _)}:_*).prettyPrint

      def endpoints( module : String) = JsArray(
        (for { po <- poMap( module).toSeq
               (po0,delay) <- worstSinkSet( aMap( module), Set(po))}  yield {
          JsObject( "po" -> JsString( po0), "slack" -> JsNumber( -delay))
        }):_*).prettyPrint

      def startpoints( module : String) = JsArray(
        (for { pi <- piMap( module).toSeq
               (pi0,delay) <- worstSourceSet( rMap( module), Set(pi))}  yield {
          JsObject( "pi" -> JsString( pi0), "slack" -> JsNumber( -delay))
        }):_*).prettyPrint

      def allpoints( module : String) = JsArray(
        (for { uv <- aMap(module).keys.toSeq
               (uv0,delay) <- worstThrough( aMap( module), rMap( module), uv)}  yield {
          JsObject( "uv" -> JsString( uv0), "slack" -> JsNumber( -delay))
        }):_*).prettyPrint

      import java.nio.file.{ Files, FileSystems, Path}
      def findFile( basename : String) : Option[Path] = {
        val paths = List( "src/main/scala", "../designutils/src/main/scala", "../dataflow_building_blocks/src/main/scala", "../accio/src/main/scala", "../chisel3/src/main/scala/chisel3/util")

        val defaultFS = FileSystems.getDefault()
        val separator = defaultFS.getSeparator()

        @annotation.tailrec
        def aux( paths : List[String]) : Option[Path] = paths match {
          case Nil => None
          case hd::tl =>
            val filename = s"${hd}${separator}${basename}"
            val path = defaultFS.getPath( filename)
            if ( Files.exists(path)) Some( path) else aux( tl)
        }

        aux( paths)
      }

      val routes: PartialFunction[RequestHeader, Action[AnyContent]] = {
        case GET(p"/modules") => Action {
          Results.Ok( modules)
        }
        case GET(p"/endpoints" ? q"module=$module") => Action {
          Results.Ok( endpoints( module))
        }
        case GET(p"/startpoints" ? q"module=$module") => Action {
          Results.Ok( startpoints( module))
        }
        case GET(p"/allpoints" ? q"module=$module") => Action {
          Results.Ok( allpoints( module))
        }
        case GET(p"/path" ? q"module=$module" & q"endpoint=$endpoint") => Action {
          Results.Ok( jsonTraces( aMap( module), Set( endpoint)))
        }
        case GET(p"/requiredpath" ? q"module=$module" & q"startpoint=$startpoint") => Action {
          Results.Ok( jsonRequiredTraces( rMap( module), Set( startpoint)))
        }
        case GET(p"/source/$file") => Action {
          val pathO = findFile( file)
          if ( pathO.isDefined) {
            val fileContents = scala.io.Source.fromFile( pathO.get.toString).getLines.mkString("\n")
            Results.Ok( fileContents)
          } else {
            println( s"Couldn't find: ${file}")
            Results.NotFound
          }
        }
        case GET(p"/") => Action {
          try {
            val fileContents = scala.io.Source.fromFile( "public/index.html").getLines.mkString("\n")
            Results.Ok( fileContents).as( "text/html")
          } catch {
            case e : java.io.FileNotFoundException => 
              Results.NotFound
          }
        }
        case GET(p"/assets/javascript/$file*") => Action {
          try {
            val fileContents = scala.io.Source.fromFile( s"public/javascript/$file").getLines.mkString("\n")
            Results.Ok( fileContents).as( "text/javascript")
          } catch {
            case e : java.io.FileNotFoundException => 
              Results.NotFound
          }
        }
        case GET(p"/assets/css/$file*") => Action {
          try {
            val fileContents = scala.io.Source.fromFile( s"public/css/$file").getLines.mkString("\n")
            Results.Ok( fileContents).as( "text/css")
          } catch {
            case e : java.io.FileNotFoundException => 
              Results.NotFound
          }
        }
      }

      play.core.server.NettyServer.fromRouter()( routes)

      println( s"Server started...")

    }

    state
  }

}

class ReportAreaTimingTradeoff extends ReportTiming( area_timing = true)

class ReportTimingCompact extends ReportTiming( full_traces = false)
class ReportTimingServer extends ReportTiming( full_traces = false, startServer = true)
class ReportTimingFull extends ReportTiming( full_traces = true)
class ReportTimingInterfacePaths extends ReportTiming( interface_paths = true)
class ReportTimingInterfacePathsCompact extends ReportTiming( interface_paths = true, full_traces = false)



class InlineAllModules extends InlineInstances {
  override def execute( state: CircuitState): CircuitState = {
    val c = state.circuit
    println( s"Calling InlineAllModules... working on ${c.main}")
    val modSet = c.modules.collect {
      case Module(_, name, _, _) if name != c.main => ModuleName(name, CircuitName(c.main))
      case ExtModule(_, name, _, _, _) if name != c.main => ModuleName(name, CircuitName(c.main))
    }.toSet
    println( s"modSet: ${modSet}")
    run( c, modSet, Set.empty[ComponentName], state.annotations)
  }
}

abstract class LowFormSeqTransform extends SeqTransform {
  def inputForm = LowForm
  def outputForm = LowForm
}

class InlineAnd( t : Transform) extends LowFormSeqTransform {
  def transforms = Seq( new InlineAllModules, t)
}

class OptInlineAnd( t : Transform) extends LowFormSeqTransform {
  def transforms = Seq( new LowFirrtlOptimization, new InlineAllModules, t)
}

class OptAnd( t : Transform) extends LowFormSeqTransform {
  def transforms = Seq( new LowFirrtlOptimization, t)
}


class InlineAndReportTiming extends InlineAnd( new ReportTiming)
class InlineAndReportTimingServer extends InlineAnd( new ReportTimingServer)
class InlineAndReportTimingFull extends InlineAnd( new ReportTimingFull)
class InlineAndReportTimingCompact extends InlineAnd( new ReportTimingCompact)
class InlineAndReportTimingInterfacePaths extends InlineAnd( new ReportTimingInterfacePaths)
class InlineAndReportTimingInterfacePathsCompact extends InlineAnd( new ReportTimingInterfacePathsCompact)
class InlineAndReportAreaTimingTradeoff extends InlineAnd( new ReportAreaTimingTradeoff)

class InlineAndReportArea extends InlineAnd( new ReportArea)

class ReportAreaThenInlineAndReportTiming extends LowFormSeqTransform {
  def transforms = Seq( new ReportArea, new InlineAndReportTiming)
}

class OptInlineAndReportTiming extends OptInlineAnd( new ReportTiming)
class OptInlineAndReportTimingServer extends OptInlineAnd( new ReportTimingServer)
class OptInlineAndReportTimingFull extends OptInlineAnd( new ReportTimingFull)
class OptInlineAndReportTimingCompact extends OptInlineAnd( new ReportTimingCompact)
class OptInlineAndReportTimingInterfacePaths extends OptInlineAnd( new ReportTimingInterfacePaths)
class OptInlineAndReportTimingInterfacePathsCompact extends OptInlineAnd( new ReportTimingInterfacePathsCompact)
class OptInlineAndReportAreaTimingTradeoff extends OptInlineAnd( new ReportAreaTimingTradeoff)

class OptInlineAndReportArea extends OptInlineAnd( new ReportArea)

class OptAndReportArea extends OptAnd( new ReportArea)

class OptReportAreaThenInlineAndReportTiming extends LowFormSeqTransform {
  def transforms = Seq( new OptAndReportArea, new OptInlineAndReportTiming)
}
