
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


  def executePerModule( ns : Namespace, srcName : String, tgtName : String)( m : DefModule): DefModule = {

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

    def transformCone( modName : String, srcName : String, tgtName : String, cone : Set[LogicNode])( m : DefModule) : DefModule = {

      val visitedLogicNodes = mutable.Set[LogicNode]()

      val newNameMap = mutable.Map[LogicNode,String]()

      def upNm( nm : String) : Unit = {
        val n = LogicNode( modName, nm)
        if ( cone.contains( n)) {
          if ( !newNameMap.contains( n)) {
            newNameMap(n) = ns.newName( "_SHANNON")
//            println( s"Generating new name: ${n} ${newNameMap(n)}")
          }
        }
      }

      def auxE( e : Expression) : Expression = {
        val ex = e map auxE

        ex match {
          case WRef( nm, tpe, knd, gender) => 
            if ( nm == srcName) {
              UIntLiteral(1,IntWidth(1))
            } else {
              upNm( nm)
              val n = LogicNode( modName, nm)
              if ( cone.contains( n)) {
                WRef( newNameMap(n), tpe, knd, gender)
              } else {
                ex
              }
            }
          case (_ : Mux | _ : DoPrim | _ : UIntLiteral | _ : SIntLiteral) => ex
          case _ => 
            println( s"Not Yet Implemented Expressions: ${ex}")
            throwInternalError
        }
      }

      def auxS( sx : Statement) : Statement = {
        sx match {
          case _ : Block => sx map auxS
          case EmptyStmt => sx
          case _ : DefRegister => sx
// DefNode and Connect should contain all driven nets
          case DefNode( info, lhs, rhs) =>
            val lhsNode = LogicNode( modName, lhs)
//            println( s"auxS: ${lhs}")
            if ( cone.contains( lhsNode)) {
              visitedLogicNodes += lhsNode
              upNm( lhs)
              rhs match {
                case ( _ : Mux | _ : DoPrim) =>
                  val rhs0 = auxE( rhs)
//                  println( s"Duplicating ${lhs} <= ${rhs}")
//                  println( s"         as ${newNameMap(lhsNode)} <= ${rhs0}")
                  Block( List( sx, DefNode( info, newNameMap(lhsNode), rhs0)))
                case _ =>
                  println( s"-E- Need to duplicate unknown ${lhs} <= ${rhs}")
                  sx
              }
            } else {
              sx
            }
          case Connect( info, lhsRef, rhsRef) =>
            val WRef( lhs, tpe_l, knd_l, gnd_l) = lhsRef
            val lhsNode = LogicNode( modName, lhs)
            if ( cone.contains( lhsNode)) {
              visitedLogicNodes += lhsNode
              if ( lhs == tgtName) {
                rhsRef match {
                  case w@WRef( rhs, tpe, knd, gnd) =>
                    val rhs0 = auxE( rhsRef)
                    if ( !cone.contains( LogicNode( modName, rhs))) {
                      println( s"-E- Expected rhs of connect to be in cone: ${rhs}")
                    }
                    Connect( info, lhsRef, rhs0)
                  case _ =>
                    println( s"-E- Need to duplicate unknown ${lhs} <= ${rhsRef}")
                    sx
                }
              } else {
                upNm( lhs)
                rhsRef match {
                  case w@WRef( rhs, tpe_r, knd_r, gnd_r) =>
//                    println( s"Duplicating ${lhsRef} <= ${rhsRef}")
                    val rhs0 = auxE( rhsRef)
                    if ( !cone.contains( LogicNode( modName, rhs))) {
                      println( s"-E- Expected rhs of connect to be in cone: ${rhs}")
                    }
                    val lhs0 = WRef( newNameMap(lhsNode), tpe_l, knd_l, gnd_l)
//                    println( s"         as ${lhs0} <= ${rhs0}")
                    Block( List( sx, Connect( info, lhs0, rhs0)))
                  case _ =>
                    println( s"-E- Need to duplicate unknown ${lhs} <= ${rhsRef}")
                    sx
                }
              }
            } else {
              sx
            }
          case IsInvalid( info, WRef( lhs, tpe, knd, gnd)) => 
            val lhsNode = LogicNode( modName, lhs)
            if ( cone.contains( lhsNode)) {
              println( s"-E- Unimplemented statement in logic cone ${sx}")
            }
            sx
          case DefWire( info, lhs, tpe) => 
            val lhsNode = LogicNode( modName, lhs)
            if ( cone.contains( lhsNode)) {
              upNm( lhs)
//              println( s"Duplicating DefWire ${lhs}")
//              println( s"         as DefWire ${newNameMap( lhsNode)}")
              Block( List( sx, DefWire( info, newNameMap( lhsNode), tpe)))
            } else {
              sx
            }
          case _ =>
            println( s"-E- Unknown statement: ${sx}")
            sx
        }
      }

      val mx = m map auxS

      println( s"Set difference: ${cone -- visitedLogicNodes}")

      mx
    }


    val srcNode = LogicNode( m.name, srcName)
    val fromInReachableNodes = reverseDepGraph.reachableFrom( srcNode) + srcNode
    val tgtNode = LogicNode( m.name, tgtName)
    val toOutReachableNodes = depGraph.reachableFrom( tgtNode) + tgtNode

    val cone = toOutReachableNodes intersect fromInReachableNodes

/*
    {
      println( s"Between ${srcName} and ${tgtName}")
      for { v <- reverseDepGraph.linearize if cone.contains(v) } {
        println( s"\t${v}")
      }
    }
 */

    transformCone( m.name, srcName, tgtName, cone)( m)
  }

  def hasDefInstance( m : DefModule) : Boolean = {
    println( s"Trying to find instances in ${m.name}")

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

    println( s"Running ShannonFactor ...")

    val mods = state.circuit.modules map { case m : DefModule =>
      val mx =
        if ( hasDefInstance( m)) {
          println( s"Skipping processing on hierarchical module ${m.name}...")
          m
        } else {
          // Add all ports as vertices
          val re_ready = """io_(.*)_ready""".r
          val re_valid = """io_(.*)_valid""".r

          val tuples =
            m.ports.flatMap {
              case p@Port( info, name, Input, tpe : GroundType) => {
//                println( s"Ground Input: ${p}")
                name match {
                  case re_valid( nm) =>
                    println( s"Input with _valid suffix: ${p}")
                    List( ( s"io_${nm}_valid", s"io_${nm}_ready"))
                  case re_ready( nm) =>
                    println( s"Input with _ready suffix: ${p}")
                    List( ( s"io_${nm}_ready", s"io_${nm}_valid"), ( s"io_${nm}_ready", s"io_${nm}_bits"))
                  case _ =>
                    List()
                }
              }
              case p@Port( info, name, Output, tpe : GroundType) =>
//                println( s"Ground Output: ${p}")
                List()
              case p@Port( info, name, _, tpe : BundleType) =>
//                println( s"Bundle: ${p}")
                List()
              case other =>
                println( s"${other}")
                throwInternalError
            }

          println( s"${m.name} ${tuples}")

          val ns = Namespace( m)
          tuples.foldLeft( m){ case (m0, ( srcName, tgtName)) =>
            println( s"Running ShannonFactor ${m0.name} ${srcName} ${tgtName} ...")
            val m1 = executePerModule( ns, srcName, tgtName)( m0)
            println( s"Finishing ShannonFactor ${srcName} ${tgtName} ...")
            m1
          }
        }
      mx
    }

    println( s"Finishing ShannonFactor ...")

    state.copy( circuit = state.circuit.copy( modules = mods))
  }

  def execute(state: CircuitState): CircuitState = {
    val state0 = execute0(state)
// This just runs it again to make sure that the cycles were removed
// We throw the result away.
    val state1 = execute0(state0)
    state0
  }
}
