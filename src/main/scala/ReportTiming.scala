
package imperative.transform

//import firrtl.transforms.{ DontTouchAnnotation, OptimizableExtModuleAnnotation}

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
        Seq(clock, reset, init).flatMap(getDeps(_)).foreach(ref => depGraph.addEdge(ps, ref))
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

    val topoOrder = reverseDepGraph.linearize

    val arrivalTimes = mutable.Map.empty[LogicNode,Option[Int]]

    for { v <- topoOrder} {
      arrivalTimes(v) = None

      if ( depGraph.getEdges( v).isEmpty) {
        arrivalTimes(v) = Some(0)
      }
    }

// backward
    for { v <- topoOrder} {
      for { e <- depGraph.getEdges( v)
            a <- arrivalTimes( e)} {
        arrivalTimes( v) = arrivalTimes(v) match {
          case None => Some( a + 1)
          case Some( b) => Some( math.max( b, a + 1))
        }
      }
    }


    {
      for { v <- topoOrder} {
        println( s"\t${v} ${arrivalTimes(v)}")
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
