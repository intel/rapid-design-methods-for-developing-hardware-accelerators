package imperative.transform

import scala.annotation.tailrec

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

class ReportTiming extends Transform {
  def inputForm = LowForm
  def outputForm = LowForm

//  private type LogicNode = MemoizedHash[WrappedExpression]
  private type LogicNode = WrappedExpression
  private object LogicNode {
    def apply(moduleName: String, expr: Expression): LogicNode = {
//      println( s"Constructing LogicNode ${moduleName} ${expr}")
      WrappedExpression(Utils.mergeRef(WRef(moduleName), expr))
    }
    def apply(moduleName: String, name: String): LogicNode = apply(moduleName, WRef(name))
  }

  /** Extract all References and SubFields from a possibly nested Expression */
  def extractRefs(expr: Expression): Seq[Expression] = {
    val refs = mutable.ArrayBuffer.empty[Expression]
    def rec(e: Expression): Expression = {
      e match {
        case ref @ (_: WRef | _: WSubField) => refs += ref
        case nested @ (_: Mux | _: DoPrim | _: ValidIf) => nested map rec
        case ignore @ (_: Literal) => // Do nothing
        case unexpected => throwInternalError
      }
      e
    }
    rec(expr)
    refs
  }

  // Gets all dependencies and constructs LogicNodes from them
  private def getDepsImpl(mname: String)
                         (expr: Expression): Seq[LogicNode] =
    extractRefs(expr).map { e =>
      if (kind(e) == InstanceKind) {
        println( s"getDepsImpl: ${e}")
        throwInternalError
      } else
        LogicNode(mname, e)
    }

  /** Construct the dependency graph within this module */
  private def setupDepGraph( regs : mutable.Map[LogicNode,(LogicNode,LogicNode)],
                             depGraph: MutableDiGraph[LogicNode])
                           (mod: Module): Unit = {

    def getDeps(expr: Expression): Seq[LogicNode] = getDepsImpl(mod.name)(expr)

    def onStmt(stmt: Statement): Unit = stmt match {
      case DefRegister(_, name, tpe, clock, reset, init) =>
        val node = LogicNode(mod.name, name)
        assert( regs.contains( node))
        val (ps,ns) = regs(node)
        depGraph.addVertex(ps)
        depGraph.addVertex(ns)
        Seq( reset, init).flatMap(getDeps(_)).foreach(ref => depGraph.addEdge(ns, ref))
      case DefNode(_, name, value) =>
        val node = LogicNode(mod.name, name)
        depGraph.addVertex(node)
        getDeps(value).foreach(ref => depGraph.addEdge(node, ref))
      case DefWire(_, name, _) =>
        depGraph.addVertex(LogicNode(mod.name, name))
      case mem: DefMemory =>
        // Treat DefMems as a node with outputs depending on the node and node depending on inputs
        // From perpsective of the module or instance, MALE expressions are inputs, FEMALE are outputs
        val memRef = WRef(mem.name, MemPortUtils.memType(mem), ExpKind, FEMALE)
        val exprs = Utils.create_exps(memRef).groupBy(Utils.gender(_))
        val sources = exprs.getOrElse(MALE, List.empty).flatMap(getDeps(_))
        val sinks = exprs.getOrElse(FEMALE, List.empty).flatMap(getDeps(_))
        val memNode = getDeps(memRef) match { case Seq(node) => node }
        depGraph.addVertex(memNode)
        sinks.foreach(sink => depGraph.addEdge(sink, memNode))
        sources.foreach(source => depGraph.addEdge(memNode, source))
      case Attach(_, exprs) => // Add edge between each expression
        exprs.flatMap(getDeps(_)).toSet.subsets(2).map(_.toList).foreach {
          case Seq(a, b) =>
            depGraph.addEdge(a, b)
            depGraph.addEdge(b, a)
        }
      case Connect(_, loc, expr) =>
        // This match enforces the low Firrtl requirement of expanded connections
//        println( s"${loc} ${getDeps(loc)} ${expr} ${getDeps(expr)}")
        val node = getDeps(loc) match { case Seq(elt) => elt }
// only one loc; zero (constant) or one expr
        val node0 =
          if ( regs.contains( node)) {
//            println( s"Left-hand side reg: ${node}")
            regs(node)._2
          } else {
            node
          }
        getDeps(expr).foreach{ ref =>
          val ref0 =
            if ( regs.contains( ref)) {
//              println( s"Right-hand side reg: ${ref}")
              regs(ref)._1
            } else {
              ref
            }
          depGraph.addEdge(node0, ref0)
        }

      case Block(stmts) => stmts.foreach(onStmt(_))

      case Stop(_,_, clk, en) => // do nothing
      case Print(_, _, args, clk, en) => // do nothing
      case ignore @ (_: IsInvalid | _: WDefInstance | EmptyStmt) => // do nothing
      case other => throwInternalError
    }

    // Add all ports as vertices
    mod.ports.foreach {
      case Port(_, name, _, _: GroundType) => depGraph.addVertex(LogicNode(mod.name, name))
      case other => throwInternalError
    }
    onStmt(mod.body)
  }

  private def createDependencyGraph( regs : mutable.Map[LogicNode,(LogicNode,LogicNode)])
                                   ( m : DefModule): DiGraph[LogicNode] = {

    val depGraph = new MutableDiGraph[LogicNode]
    m match {
      case mod : Module => setupDepGraph(regs, depGraph)(mod)
      case _ => throwInternalError
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


  def constructTimingArcs
    ( tas : mutable.Map[(LogicNode,LogicNode),(Int,String)],
      regs : mutable.Map[LogicNode,(LogicNode,LogicNode)])
    ( m : DefModule): DefModule = {

    def constructTimingArcsStatement( s : Statement): Statement = {

      def extractWidth( x : Any) : Int = x match {
        case UIntType(IntWidth(w)) => w.toInt
        case SIntType(IntWidth(w)) => w.toInt
        case _ => -1
      }

      def inSignals( inp : Seq[Expression]) : List[(LogicNode,Int)] =
        inp.toList flatMap {
          case WRef( nm, tpe, knd, gnrd) => 
            val rhsNode = LogicNode( m.name, nm)
            val rhsNode0 = if ( regs.contains( rhsNode)) regs(rhsNode)._1 else rhsNode
            List((rhsNode0,extractWidth(tpe)))
          case UIntLiteral( lit, width) => List()
          case SIntLiteral( lit, width) => List()
          case x@DoPrim( padOrBits, lst, _, _) =>
            println( s"Recursive call: ${x}")
            inSignals( lst)
          case x =>
            println( s"Not Yet Implemented: ${x}")
            List()
        }

      s match {
        case _ : Block =>
          s map constructTimingArcsStatement
          ()
        case Connect( info, lhsRef, rhsRef) =>
//          println( s"$s")
          lhsRef match {
            case WRef( lhs, l_tpe, l_knd, l_gndr) =>
              val lhsNode = LogicNode( m.name, lhs)
              val lhsNode0 = if ( regs.contains( lhsNode)) regs(lhsNode)._2 else lhsNode

              for ( (rhsNode0,width) <- inSignals( List(rhsRef))) {
                tas( ( rhsNode0, lhsNode0)) = (0,"connect")
              }
          }
        case EmptyStmt =>
        case _ : DefRegister => //println( s"$s")
        case _ : DefWire => //println( s"$s")
        case DefNode( info, lhs, rhs) =>
          val (o,lst) = rhs match {
            case Mux( cond, te, fe, tpe) =>
              ("mux",inSignals(List(cond,te,fe)))
            case DoPrim( op, inps, _, tpe) =>
              val opStr = s"${op}"
              ( opStr, inSignals(inps))
            case _ =>
              println( s"Not Yet Implemented: ${s}")
              ("nyi",List())
          }
          
          val widths = lst.map( _._2).mkString(",")
          val oo = s"${o},(${widths})"
          val delay =
            o match {
              case "pad" => 0
              case "tail" => 0
              case "bits" => 0
              case "add" => 3
              case "mul" => 10
              case "mux" => if ( widths == "(1)") 0 else 1
              case _ => 1
            }
          for { (f,width) <- lst} {
            tas( (f,LogicNode(m.name,lhs))) = (delay,oo)
          }
        case _ : IsInvalid =>
        case _ =>
          println( s"Not Yet Implemented: ${s}")
      }

      s
    }

    println( s"constructTimingArcs: ${m.name}")
    m map constructTimingArcsStatement
  }

  def executePerModule(m : DefModule): DefModule = {

    val regs = mutable.Map[LogicNode,(LogicNode,LogicNode)]()

    m match {
      case Module(info, name, _, body) =>
        body map allRegs( regs, name)
    }

    val depGraph = createDependencyGraph( regs)( m)

    val reverseDepGraph = {
      val reverseDepGraph = new MutableDiGraph[LogicNode]
      for{ v <-depGraph.getVertices} {
        reverseDepGraph.addVertex( v)
      }
      for{ v <- depGraph.getVertices} {
        for{ e <- depGraph.getEdges( v)} {
          reverseDepGraph.addEdge( e, v)
        }
      }
      DiGraph( reverseDepGraph)
    }

    val tas = mutable.Map.empty[(LogicNode,LogicNode),(Int,String)]

    constructTimingArcs( tas, regs)( m)

    val topoOrder = reverseDepGraph.linearize

    class Arrival {
      var time : Option[Int] = None
      var u : Option[LogicNode] = None
      var op : Option[String] = None
    }

    val arrivals = mutable.Map.empty[LogicNode,Arrival]

    for { v <- topoOrder} {
      arrivals(v) = new Arrival

      if ( depGraph.getEdges( v).isEmpty) {
        println( s"Setting arrival to zero on vertex: ${v}")
        arrivals(v).time = Some(0)
        arrivals(v).op = Some("No incoming edges")
      }
    }

// backward
    for { v <- topoOrder} {
      for { e <- depGraph.getEdges( v)
            a <- arrivals( e).time} {

        val (delay,op) = tas.getOrElse( ( e, v), (0,"unknown"))
        val newA = a + delay

        arrivals(v).time match {
          case None =>
            arrivals(v).time = Some( newA)
            arrivals(v).u = Some( e)
            arrivals(v).op = Some( op)
          case Some( b) =>
            if ( b < newA) {
              arrivals(v).time = Some( newA)
              arrivals(v).u = Some( e)
              arrivals(v).op = Some( op)
            }
        }
      }
    }


    println( s"Sinks:")

    {
      @tailrec
      def backTrace( accum : List[LogicNode], n : Option[LogicNode]) : List[LogicNode] =
        n match {
          case None => accum
          case Some( nn) => backTrace( nn :: accum, arrivals(nn).u)
        }

      val sinks =
        for { v <- topoOrder
              if reverseDepGraph.getEdges( v).isEmpty
              t <- arrivals(v).time
            } yield (v,t)
      val sortedSinks = sinks.sortBy{ case (v,t) => t}

      for { (v,t) <- sortedSinks} {
        for { u <- backTrace( List.empty[LogicNode], Some(v))} {
          println( s"\t${u} ${arrivals(u).time} ${arrivals(u).op} ${arrivals(u).u}")
        }
        println( s"===========================")
      }
    }

    m
  }

  def hasDefInstance( m : DefModule) : Boolean = {
//    println( s"Trying to find instances in ${m.name}")

    class Flag { var value = false}

    def onStmt( f : Flag)( s : Statement) : Statement = {
      s map onStmt( f)
      s match {
        case _ : Block =>
        case _ : Connect =>
        case WDefInstance( info, instanceName, templateName, _) =>
          println( s"\tfound ${instanceName} ${templateName}")
          f.value = true
        case _ =>
      }
      s
    }

    val f = new Flag
    m map onStmt( f)
    f.value
  }

  def execute0(state: CircuitState): CircuitState = {

    val mods = state.circuit.modules map { case m : DefModule =>
      if ( hasDefInstance( m)) {
        println( s"-W- ReportTiming: Skipping processing on hierarchical module ${m.name} ...")
        m
      } else {
        println( s"Running ReportTiming on module ${m.name} ...")
        executePerModule( m)
        println( s"Finished running ReportTiming on module ${m.name} ...")
      }
    }

    state
  }

  def execute(state: CircuitState): CircuitState = {
    val state0 = execute0(state)
    state
  }
}
