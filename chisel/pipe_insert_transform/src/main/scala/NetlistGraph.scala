// See LICENSE for license details.
package netlist_graph


import firrtl._
import firrtl.ir._
import firrtl.passes._
import firrtl.annotations._
import firrtl.graph._
import firrtl.analyses.InstanceGraph
import firrtl.Mappers._
import firrtl.WrappedExpression._
import firrtl.Utils.{toWrappedExpression, kind}
import firrtl.MemoizedHash._
import wiring.WiringUtils.getChildrenMap

import collection.mutable
import java.io.PrintWriter
import java.io.File



trait NetlistNode {
  val name: String
}

case class InputNode (name : String) extends NetlistNode 
case class OutputNode (name : String) extends NetlistNode 
case class CombNode (name : String) extends NetlistNode 
case class RegNode (name : String) extends NetlistNode 
case class UndefinedTypeNode (name : String) extends NetlistNode 

object NetlistNode {

    def apply(name: String, tpe : String): NetlistNode = tpe match {
      case "In" => InputNode(name)
      case "Out" => OutputNode(name)
      case "Comb" => CombNode(name)
      case "Reg" => RegNode(name)
      case _ => UndefinedTypeNode(name)

//    def apply( moduleName: String, expr: Expression): LogicNode =
//      expr match {
//        case WRef( nm, _, _, _) => nm
//        case WSubField( ref, nm, tpe, gndr) => LogicNode( moduleName, ref) + "." + nm
//        case _ =>
//          println( s"LogicNode::apply ${expr}")
//          throwInternalError
//      }
  }

}

class NetlistGraphBuilder {
  def inputForm = LowForm
  def outputForm = LowForm

    

  /** Extract all References and SubFields from a possibly nested Expression */
  def extractRefs(expr: Expression): Seq[Expression] = {
    val refs = mutable.ArrayBuffer.empty[Expression]
    def rec(e: Expression): Expression = {
      e match {
        case ref @ (_: WRef | _: WSubField) => refs += ref
        case nested @ (_: Mux | _: DoPrim | _: ValidIf) => nested map rec
        case ignore @ (_: Literal) => // Do nothing
        case unexpected => throw new RuntimeException(s"Unknown expression $expr")
      }
      e
    }
    rec(expr)
    refs
  }

  // Gets all dependencies and constructs LogicNodes from them
  private def getDepsImpl(mname: String)
                         (expr: Expression): Seq[NetlistNode] =
    extractRefs(expr).flatMap { e =>
      if (kind(e) == InstanceKind) {
        println( s"getDepsImpl: ${e}")
        throw new RuntimeException("Hierarchical designs are not supported. Inline it prior to this")
      } else e match {
          case WRef( nm, tp, _, _) => tp match {
            // skipping clock dependency
            case ClockType => None
            case _ =>  Some(UndefinedTypeNode(nm))
          }
          case WSubField( ref, nm, tpe, gndr) => Some(UndefinedTypeNode(mname + "." + nm))
          case _ =>
            println( s"Netlist::apply ${expr}")
            throw new RuntimeException(s"Unknown expression $e")
      }
    }

  /** Construct the dependency graph within this module */
  private def setupDepGraph(depGraph: MutableDiGraph[NetlistNode])
                           (mod: DefModule): Unit = {

    def getDeps(expr: Expression): Seq[NetlistNode] = {
      getDepsImpl(mod.name)(expr) map {node =>
        
        depGraph.getVertices.find(_.name == node.name) match {
          case Some(node) => node
          case _ => 
            println( s"Vertex ${node.name} was not found when referred ${expr} in graph: ")
            depGraph.getVertices.foreach {n => println(n.name)}
            throw new RuntimeException("Node was not found in graph")
        }
      }
    }
    // Add all ports as vertices
    mod.ports.foreach {
      case Port(_, name, dir : Direction, _: GroundType) => dir match  {
        case Input => depGraph.addVertex(NetlistNode(name, "In"))
        case Output => depGraph.addVertex(NetlistNode(name, "Out"))
      }
      case other => throw new RuntimeException(s"Unexpected argument $other")
    }

    def onStmt(stmt: Statement): Unit = stmt match {
      case DefRegister(_, name, _, clock, reset, init) =>
        val node = NetlistNode(name, "Reg")
        depGraph.addVertex(node)
        //XXX: not sure if we need init
        //println (s" RESET $reset INIT $init")
        //Seq(clock, reset, init).flatMap(getDeps(_)).foreach(ref => depGraph.addEdge(ref, node))
        Seq(clock, reset).flatMap(getDeps(_)).foreach(ref => depGraph.addEdge(ref, node))
        //Seq(clock, init).flatMap(getDeps(_)).foreach(ref => depGraph.addEdge(ref, node))
      case DefNode(_, name, value) =>
        val node = NetlistNode(name, "Comb")
        depGraph.addVertex(node)
        getDeps(value).foreach(ref => depGraph.addEdge(ref, node))
      case DefWire(_, name, _) =>
        depGraph.addVertex(NetlistNode(name, "Comb"))
      case mem: DefMemory =>
        // Treat DefMems as a node with outputs depending on the node and node depending on inputs
        // From perpsective of the module or instance, MALE expressions are inputs, FEMALE are outputs
        val memRef = WRef(mem.name, MemPortUtils.memType(mem), ExpKind, FEMALE)
        val exprs = Utils.create_exps(memRef).groupBy(Utils.gender(_))
        val sources = exprs.getOrElse(MALE, List.empty).flatMap(getDeps(_))
        val sinks = exprs.getOrElse(FEMALE, List.empty).flatMap(getDeps(_))
        val memNode = getDeps(memRef) match { case Seq(node) => node }
        depGraph.addVertex(memNode)
        println( s"DefMemory: sources: ${sources} sinks: ${sinks}")
// arbitrarily breaking this loop
//        sinks.foreach(sink => depGraph.addEdge(sink, memNode))
        sources.foreach(source => depGraph.addEdge(source, memNode))
      case Attach(_, exprs) => // Add edge between each expression
        exprs.flatMap(getDeps(_)).toSet.subsets(2).map(_.toList).foreach {
          case Seq(a, b) =>
            depGraph.addEdge(a, b)
            depGraph.addEdge(b, a)
        }
      case Connect(_, loc, expr) =>
        // This match enforces the low Firrtl requirement of expanded connections
        val foo = getDeps(loc)
        foo match {
          case Seq(node) =>
            getDeps(expr).foreach(ref => depGraph.addEdge(ref, node))
          case Seq() =>
        }
      case Block(stmts) => stmts.foreach(onStmt(_))

      case Stop(_,_, clk, en) => // do nothing
      case Print(_, _, args, clk, en) => // do nothing
      case ignore @ (_: IsInvalid | _: WDefInstance | EmptyStmt) => // do nothing
      case other => new Exception(s"Not supported statement in netlist $stmt")
    }


    mod match {
      case m : Module => onStmt(m.body)
      case m : ExtModule => 
    }
  }

  def createNetlistGraph(m: DefModule): DiGraph[NetlistNode] = {
    val depGraph = new MutableDiGraph[NetlistNode]
    setupDepGraph(depGraph)(m)
    DiGraph(depGraph)
  }

  def dumpDot(fname : String, g : DiGraph[NetlistNode], delays : Option[Map[NetlistNode,Int]] = None) = {
    def q( s : String) = if ( s.contains('$')) s""""$s"""" else s

    val pw = new PrintWriter(new File(fname))
    val edgeList = (g.getVertices.flatMap(fromVtx => 
      g.getEdges(fromVtx) map {toVtx => fromVtx match {
      case InputNode(_) => s"${q(fromVtx.name)} -> ${q(toVtx.name)}"
      case RegNode(_) => s"${q(fromVtx.name)} -> ${q(toVtx.name)}"
      case _ => s"${q(fromVtx.name)} -> ${q(toVtx.name)}"
      
    }})) toList
    
    def lbl( vtx : NetlistNode) : String = {
      if ( delays.isDefined) {
        val m = delays.get
        val default = "Unknown"
        if ( m.contains(vtx)) {
          ",label=\"" + s"${vtx.name},${m(vtx)}" + "\""
        } else {
          ",label=\"" + s"${vtx.name},unknown" + "\""
        }
      } else {
        ""
      }
    }

    val shapeList = g.getVertices.map {vtx => vtx match {
      case InputNode(_) => s"${q(vtx.name)}" + " [shape=\"invtriangle\"]"
      case RegNode(_) => s"${q(vtx.name)}" + " [shape=\"box\"]"
      case OutputNode(_) => s"${q(vtx.name)}" + " [shape=\"invtriangle\"]"
      case _ => s"${q(vtx.name)}" + " [shape=\"oval\"" + lbl(vtx) + "]"
    }} toList
    
    pw.write("digraph Netlist {\n" + (edgeList++shapeList).mkString(";\n") + " \n}")
    pw.close
  }

}
