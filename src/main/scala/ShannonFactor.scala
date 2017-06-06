
package imperative.transform

import firrtl.transforms.{ DontTouchAnnotation, OptimizableExtModuleAnnotation}

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

class ShannonFactor extends Transform {
  def inputForm = LowForm
  def outputForm = LowForm

  /** Based on LogicNode ins CheckCombLoops, currently kind of faking it */
//  private type LogicNode = MemoizedHash[WrappedExpression]
  private type LogicNode = WrappedExpression
  private object LogicNode {
    def apply(moduleName: String, expr: Expression): LogicNode = {
//      println( s"Constructing LogicNode ${moduleName} ${expr}")
      WrappedExpression(Utils.mergeRef(WRef(moduleName), expr))
    }
    def apply(moduleName: String, name: String): LogicNode = apply(moduleName, WRef(name))
    def apply(component: ComponentName): LogicNode = {
      // Currently only leaf nodes are supported TODO implement
      val loweredName = LowerTypes.loweredName(component.name.split('.'))
      apply(component.module.name, WRef(loweredName))
    }
    /** External Modules are representated as a single node driven by all inputs and driving all
      * outputs
      */
    def apply(ext: ExtModule): LogicNode = LogicNode(ext.name, ext.name)
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
  private def getDepsImpl(mname: String,
                          instMap: collection.Map[String, String])
                         (expr: Expression): Seq[LogicNode] =
    extractRefs(expr).map { e =>
//      println( s"genDepsImpl: ${e}")
      if (kind(e) == InstanceKind) {
        val (inst, tail) = Utils.splitRef(e)
        LogicNode(instMap(inst.name), tail)
      } else {
        LogicNode(mname, e)
      }
    }

// names of registers
  val regs = mutable.Map[LogicNode,(LogicNode,LogicNode)]()

  /** Construct the dependency graph within this module */
  private def setupDepGraph(depGraph: MutableDiGraph[LogicNode],
                            instMap: collection.Map[String, String])
                           (mod: Module): Unit = {
    def getDeps(expr: Expression): Seq[LogicNode] = getDepsImpl(mod.name, instMap)(expr)

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
            println( s"Left-hand side reg: ${node}")
            regs(node)._2
          } else {
            node
          }
        getDeps(expr).foreach{ ref =>
          val ref0 =
            if ( regs.contains( ref)) {
              println( s"Right-hand side reg: ${ref}")
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

  private def createDependencyGraph( c: Circuit): MutableDiGraph[LogicNode] = {

    val iGraph = new InstanceGraph(c)
    val moduleDeps = iGraph.graph.edges.map { case (k,v) =>
      k.module -> v.map(i => i.name -> i.module).toMap
    }

    val depGraph = new MutableDiGraph[LogicNode]
    c.modules.foreach {
      case mod: Module => setupDepGraph(depGraph, moduleDeps(mod.name))(mod)
      case ext: ExtModule => throw new Exception(s"ExtModule not supported.")
    }

    depGraph
  }

  def allRegs( modName : String)( s : Statement) : Statement = {
    s map allRegs( modName)

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


  def executePerModule( c : Circuit, m : DefModule): DefModule = {

    regs.clear

    m match {
      case Module(info, name, _, body) =>
        body map allRegs( name)
    }

    println( s"${regs}")

    val depGraph = DiGraph( createDependencyGraph( c))

    println( s"depGraph built.")

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

    val forwardOrder = reverseDepGraph.linearize


    def printPrimInCone( modName : String, cone : Set[LogicNode]) : Unit = {

      val visitedLogicNodes = mutable.Set[LogicNode]()

      def auxE( e : Expression) : Expression = {
        e map auxE

        e match {
          case Mux( cond, tval, fval, tpe) =>
          case DoPrim( op, inps, _, tpe) =>
//            println( s"auxE: ${inps} ${tpe}")
          case _ =>
        }

        e
      }

      def auxS( s : Statement) : Statement = {
        s map auxS map auxE

        s match {
          case Block( _) =>
          case EmptyStmt =>
// DefNode and Connect should contain all driven nets
          case DefNode( info, lhs, rhs) =>
            val lhsNode = LogicNode( modName, lhs)
            if ( cone.contains( lhsNode)) {
              rhs match {
                case m : Mux =>
                // println( s"Need to duplicate ${lhs} <= ${m}")
                case p : DoPrim =>
                // println( s"Need to duplicate ${lhs} <= ${p}")
                case _ => println( s"-E- Need to duplicate unknown ${lhs} <= ${rhs}")
              }
              visitedLogicNodes += lhsNode
            }
          case Connect( info, lhsRef, rhsRef) =>
            val WRef( lhs, _, _, _) = lhsRef
            val lhsNode = LogicNode( modName, lhs)
            if ( cone.contains( lhsNode)) {
              rhsRef match {
                case w@WRef( rhs, tpe, knd, gnd) => {
//                  println( s"Need to duplicate ${lhs} <= ${w}")
                  if ( !cone.contains( LogicNode( modName, rhs))) {
                    println( s"-E- Expected rhs of connect to be in cone: ${rhs}")
                  }
                }
                case _ => println( s"-E- Need to duplicate unknown ${lhs} <= ${rhsRef}")
              }
              visitedLogicNodes += lhsNode
            }
          case _ =>
        }

        s
      }

      for { mod <- c.modules} {
        mod match {
          case Module(info, name, _, body) if name == modName =>
            body map auxS
        }
      }

      println( s"Set difference: ${cone -- visitedLogicNodes}")

    }


    def printCone( modName : String, srcName : String, tgtName : String) : Unit = {

      val tgtNode = LogicNode( modName, tgtName)
      val toOutReachableNodes = depGraph.reachableFrom( tgtNode) + tgtNode

/*
      {
        println( s"Reachable from ${tgtName}")
        for { v <- toOutReachableNodes} {
          println( s"\t${v}")
        }
      }
 */

      val srcNode = LogicNode( modName, srcName)
      val fromInReachableNodes = reverseDepGraph.reachableFrom( srcNode) + srcNode

/*
      {
        println( s"Reachable from ${srcName} (reverse graph)")
        for { v <- fromInReachableNodes} {
          println( s"\t${v}")
        }
      }
 */

      val cone = toOutReachableNodes intersect fromInReachableNodes

      printPrimInCone( modName, cone)

      {
        println( s"Between ${srcName} and ${tgtName}")
        for { v <- forwardOrder if cone.contains(v) } {
          println( s"\t${v}")
        }
      }
    }

    // Add all ports as vertices
    val re_ready = """io_(.*)_ready""".r
    val re_valid = """io_(.*)_valid""".r

    m.ports.foreach {
      case p@Port( info, name, Input, tpe : GroundType) => {
        name match {
          case re_valid( nm) =>
            println( s"Input with _valid suffix: ${p}")
            printCone( m.name, s"io_${nm}_valid", s"io_${nm}_ready")
            printCone( m.name, s"io_${nm}_bits", s"io_${nm}_ready")
          case re_ready( nm) =>
            println( s"Input with _ready suffix: ${p}")
            printCone( m.name, s"io_${nm}_ready", s"io_${nm}_valid")
            printCone( m.name, s"io_${nm}_ready", s"io_${nm}_bits")
          case _ =>
        }
      }
      case p@Port( info, name, Output, tpe : GroundType) => 
      case other => throwInternalError
    }

    println( s"Finishing ShannonFactor...")

    m 
  }

  def execute(state: CircuitState): CircuitState = {

    println( s"Running ShannonFactor...")
    val c = state.circuit

    for { mod <- c.modules} {
      executePerModule( c, mod)
    }
    state
  }


}
