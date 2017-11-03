// See LICENSE for license details.
package pipe_insert_transform

import chisel3._
import chisel3.experimental.ChiselAnnotation
import chisel3.internal.InstanceId
import firrtl.ir.{DefModule,Statement,EmptyStmt,Port,ClockType,DefRegister,DefNode,NoInfo,Block,Connect, Circuit, Expression}
import firrtl.{CircuitForm, CircuitState, LowForm, Transform, WRef, Namespace}
import firrtl.annotations.{Annotation, ModuleName, Named, ComponentName, CircuitName}
import firrtl.annotations.AnnotationUtils._
import firrtl.Utils.{one, zero, BoolType}
import firrtl.Mappers._
import collection.mutable
import firrtl.passes.{Pass, InferTypes, ResolveKinds, ResolveGenders}
import firrtl.PortKind
import firrtl.ir.Mux
import chisel3.util.Decoupled
import netlist_graph._
import firrtl.analyses.InstanceGraph
import firrtl.graph.DiGraph
import firrtl.graph._
import netlist_graph._
import firrtl.passes.DeadCodeElimination
import java.io.File
import java.io.PrintWriter
import firrtl.ir.DoPrim
import firrtl.ir.ValidIf
import firrtl.flattenType
import firrtl.ir.UnknownType
import firrtl.ir.Type

import firrtl.ir.Module
import firrtl.passes.InlineInstances
import firrtl.ir.Width
import firrtl.ir.IntWidth
import firrtl.EmptyExpression
import java.util.EmptyStackException
import firrtl.ir.Literal
import scala.collection.mutable.ListBuffer
import firrtl.ir.UIntLiteral


class InlineAllModules (info : Seq[PipeInsertInfo]) extends InlineInstances {
  override def execute( state: CircuitState): CircuitState = {
    //TODO: inline only modules that specify inline in annotation. currently inline everything if see at least one inline
    val inline = info.map(_.shouldInline).reduceLeft(_||_) 
    if (inline) {
      val c = state.circuit
      println( s"Calling InlineAllModules... working on ${c.main}")
      val modSet = c.modules.collect {
        case firrtl.ir.Module(_, name, _, _) if name != c.main => ModuleName(name, CircuitName(c.main))
      }.toSet
      run( c, modSet, Set.empty[ComponentName], state.annotations)
    } else { 
      state  
    }
  }
}

trait HldRetimer {
  self: chisel3.Module =>
    def hld_retime(component: InstanceId, clockPeriod : Int = 30, shouldInline : Boolean = true) : Unit = {
      annotate(ChiselAnnotation(component, classOf[PipeInsertTransform], s"$clockPeriod;$shouldInline"))
    }
}

object PipeInsertAnnotation {
  def apply(target: ModuleName, value: String): Annotation = Annotation(target, classOf[PipeInsertTransform], value)

  def unapply(a: Annotation): Option[(ModuleName, String)] = a match {
    case Annotation(ModuleName(name, cirname), t, value) if t == classOf[PipeInsertTransform] => Some((ModuleName(name, cirname), value))
    case _ => None
  }
}

class PipeInsertInfo (val firElem : ModuleName, val clockPeriod : Int, val shouldInline : Boolean)


class PipeInsertTransform extends Transform {
  override def inputForm: CircuitForm = LowForm
  override def outputForm: CircuitForm = LowForm
  
  def transforms(info : Seq[PipeInsertInfo]) =
    Seq(//new PipeInsertPass(info),
        //DeadCodeElimination,
        new InlineAllModules(info),
        new PipeRetimePass(info),
        InferTypes,
        ResolveKinds,
        ResolveGenders
        )
        
  override def execute(state: CircuitState): CircuitState = {
    val pinfo = getMyAnnotations(state) match {
      case Nil => 
        println("getMyAnnotations -> Nil")
        Seq.empty
      case myAnnotations =>
        
        val pipeInsertInfos = myAnnotations.map(_ match {
          case PipeInsertAnnotation(cname, value) => value.split(";").toList match {
            case clockPeriod :: shouldInline :: Nil =>
              println(s"ANNO: ${cname} clockPeriod=$clockPeriod") 
              new PipeInsertInfo(cname, clockPeriod.toInt, shouldInline.toBoolean)
        }})
        pipeInsertInfos      
//        val newst = transforms(pipeInsertInfos).foldLeft(state) { (in, xform) => xform.runTransform(in) ;}
//        newst
    }
    val newst = transforms(pinfo).foldLeft(state) { (in, xform) => xform.runTransform(in) ;}
    newst

  }
}

class PipeRetimePass (info : Seq[PipeInsertInfo]) extends Pass {
  case class SuperNode (name : String) extends NetlistNode 
  
  def run(c: Circuit): Circuit = {

     val mods = c.modules map { m =>
       
       if (info.size > 0 && m.name == info(0).firElem.name) { 
         println (s"!!!PipeRetimePass on module ${m.name}!!!")
      	 pipeRetime(m)
       } else {
         m
       }
     }
          
     // update state with updated modules          
     c.copy( modules = mods)
  }
  // TODO: move to a tester (see imperative/.../F.scala)
  def buildTestNetlist () : DiGraph[NetlistNode] = {
    val net = new MutableDiGraph[NetlistNode]
    val in = new InputNode("in")
    val out = new OutputNode("out")
    val g1 = new CombNode("g1")
    val g2 = new CombNode("g2")
    val r1 = new RegNode("r1")
    val r2 = new RegNode("r2")
    
    net.addVertex(in)
    net.addVertex(out)
    net.addVertex(g1)
    net.addVertex(g2)
    net.addVertex(r1)
    net.addVertex(r2)
    net.addEdge(in, g1)    
    net.addEdge(g1, g2)    
    net.addEdge(g2, r1)    
    net.addEdge(r1, r2)    
    net.addEdge(r2, out)    
    DiGraph(net)
  }

  
  def processRecirculMuxes(m: DefModule, enname: String): (Boolean, DefModule) = {
    val enmuxes = mutable.HashMap.empty[String, Mux]
    val regsrc = mutable.HashMap.empty[String, Expression]
    val regsinks = mutable.HashMap.empty[String, Set[Expression]]
    
    def sss (s: Statement): Statement = s match {
      case DefNode(_, name, value) => value match {
        case Mux(cond, con, alt,tp) => cond match {
          case WRef(en,_,_,_) =>
            if (en == enname) {
              enmuxes += name->Mux(cond, con, alt, tp)
            }
            s
          case _ => s
        }
        case _ =>  
          //println(s"DEFNODE $name $value")
          s
      }
      case DefRegister(_, name, _, clock, reset, init) =>
        //println(s"DEFREG $name ")
        regsrc(name) = EmptyExpression
        s
//      case DefWire(_, name, _) =>
//        s
      case Connect(_, loc, expr) => loc match {
        case WRef(lhs,_,_,_) => 
          if (regsrc.contains(lhs)) {
            regsrc(lhs) = expr
          }
        case _ => s
          
        }
        //println(s"CONNECT $loc $expr ")
        s
      
      case _: Block =>
        s map sss

      case _ => s
    }
    m map sss
    
    
    //println(s"ENMUXES: $enmuxes")
    val allRegsEnabled = regsrc.foldLeft(true) {case (allEn,(reg, exp)) => exp match {
      case WRef(nm,_,_,_) =>
        if( enmuxes.contains(nm) ) {
          allEn
        } else {
          println (s"Reg $reg is driven not by mux but $nm")
          false
        }
      case _ =>
        println (s"Reg $reg is driven not by mux but $exp")
        false
    }}
      
    //println(s"REGSRC: $regsrc")
    
    def removeRecircMuxes(s: Statement): Statement = s match {
      case DefNode(_, name, value) => if (enmuxes.contains(name)) {
        //println(s"REMOVING MUX $name")
        EmptyStmt 
      } else s
      case Connect(info, loc, expr) => loc match {
        case WRef(lhs,_,_,_) => 
          if (regsrc.contains(lhs)) {
            regsrc(lhs) match {
              case WRef(muxName,_,_,_)=>
                assert(enmuxes.contains(muxName))
                enmuxes(muxName) match {
                  case Mux(cond, con, alt, tp) => 
                    //println(s"RECONNECT REG $loc to MUX IN $con")
                    Connect(info, loc, con)
                }
            }
          } else s
        case _ => s
          
        }
      case _: Block =>
        s map removeRecircMuxes

      case _ => s
      
    }
    if (allRegsEnabled) {
      (true, m map removeRecircMuxes)
    } else {
      println("NOT ALL REGISTERS ENABLED WITH THE SAME ENABLE. LAUNCHING SIMPLE RETIMING")
      (false, m)
    }
  }
  
  def introduceEnableMuxes(m: DefModule, enable: String): (DefModule, Statement) = {
    var enableStmt : Statement = EmptyStmt
    val regs = mutable.Map.empty[String,DefRegister]
    def rewrite(s: Statement): Statement = s match {
      case DefNode(_, name, value) => if (name == enable) enableStmt = s
        s
      case Connect(info, loc, expr) => loc match {
        case WRef(lhs,_,_,_) => if (regs.contains(lhs)) {
          val conReg = Connect(NoInfo, WRef(lhs), Mux(WRef(enable), expr, WRef(lhs), expr.tpe))
    			//val rewireToReg = Connect(NoInfo, out, WRef(regName))
    			conReg
        } 
        else 
          s 
        }
      case DefRegister(info,name,tpe,clk,rst,exp) =>
        regs += name->DefRegister(info, name, tpe, clk, rst, exp)
        s
      case _: Block =>
        s map rewrite
      case _ => s
        
    }
    (m map rewrite, enableStmt)
  }
  
  def pipeRetime (inM : DefModule) : DefModule = {
    val gBuilder = new NetlistGraphBuilder
    val enname = "io_enable"
    // get netlist for module
    val (muxRemoved, m) = processRecirculMuxes(inM, enname)
    val netlist = gBuilder.createNetlistGraph(m)
    // or use a simple test one 
    //val netlist = buildTestNetlist()
    netlist.getVertices map {node => 
    		val edges = netlist.getEdges(node)
    		//println (s"NETLIST EDGE for node $node $edges")
    }
    val resetSignals = TransformUtils.getResetSignals(m, netlist)
    println (s"reset signals - $resetSignals")

    // generate graph for matrices W, D computation  
    // currently just merge inputs and outputs 
    val wdGraph = convertNetlistToWDGraph (netlist)
    
    gBuilder.dumpDot("before.dot", netlist)
    //println (s"WD GRAPH!")
    wdGraph.getVertices map {node => 
    	val edges = wdGraph.getEdges(node)
    	//println (s"EDGE for node $node $edges")
    }
    gBuilder.dumpDot("wd.dot", wdGraph)
    
//    val (useDelayMap,defaultDelay) = (false,10)
    val (useDelayMap,defaultDelay) = (true,1)
    val delayMap = DelayMap( m)

    // assign arbitrary delays
    val delays = (wdGraph.getVertices.map {vtx => vtx match {
      case InputNode(_) => vtx -> 0
      case OutputNode(_) => vtx -> 0
      case RegNode(_) => vtx -> 0
      case CombNode(tgt) =>
        if ( useDelayMap) {
          if ( delayMap.contains( tgt)) {
            vtx -> delayMap( tgt)
          } else {
            //println( s"${tgt} using defaultDelay ${defaultDelay}")
            vtx -> defaultDelay
          }
        } else {
          vtx -> defaultDelay
        }
      case _ =>
        //println( s"Unknown case ${vtx} using defaultDelay ${defaultDelay}")
        vtx -> defaultDelay
    }}) toMap
    
    
    gBuilder.dumpDot("before.dot", netlist, Some(delays))
    println (s"Computing WD matrices...")
    wdGraph.getVertices map {node => 
    	val edges = wdGraph.getEdges(node)
    	//println (s"EDGE for node $node $edges")
    }
    gBuilder.dumpDot("wd.dot", wdGraph, Some(delays))

    // init W,D and solve
    val W = mutable.HashMap(wdGraph.getVertices.flatMap {v1 => wdGraph.getVertices.map {v2 => (v1,v2)-> wdGraph.getVertices.size}}.toSeq : _* ) 
    val D = mutable.HashMap(wdGraph.getVertices.flatMap {v1 => wdGraph.getVertices.map {v2 => (v1,v2)-> 10000}}.toSeq : _* )
    val b4wd = System.nanoTime()
    floydWarshall(wdGraph, delays, W, D)
    val a4wd = System.nanoTime()
    println(s"Computing WD took "+ (a4wd-b4wd)/1000000.0 + "ms")
    
    
    // build retiming graph from wdGraph using W,D and clock cycle threshold 
    val retWeights = mutable.HashMap.empty[(NetlistNode,NetlistNode), Int]
    val retGraph = generateRetimingGraph(wdGraph, info(0).clockPeriod, W, D, retWeights)
    val distances = mutable.HashMap.empty[NetlistNode, Int]
    
    println (s"Retiming...")
    val b4ret =  System.nanoTime()   
    val success = computeRetimingBF(retGraph, retWeights, distances)
    val a4ret = System.nanoTime()
    println(s"Retiming took "+ (a4ret-b4ret)/1000000.0 + "ms")

    if (success) {
      println ("SUCCESS")
      val newRegCount = extractRegChangeCount(netlist, W.toMap, distances.toMap).toList
      val (deleteRegs, insertRegs) = newRegCount.partition(_._2 < 0)
      // sort to insert before delete as well as topo sort (important for deletion)
      val topoDeleteRegs = topoSort(deleteRegs)
      
      //println (s"OLD => $m")
      
      val regDefs = mutable.Map.empty[String, Statement]
      val retimedMod = (insertRegs++topoDeleteRegs).foldLeft(m) {modifyModuleDef(resetSignals,regDefs,_, _)}
      
      //println (s"NEW => $newModWithDefsInFront")
      val (retimedModWithMuxes, enableStmt) = if (muxRemoved) {
        introduceEnableMuxes(retimedMod, enname)
      } else (retimedMod, EmptyStmt) 
      
      val newModWithDefsInFront = rewriteDefs(retimedModWithMuxes, regDefs.toMap, Map(enname->enableStmt)) 
        
      val after = gBuilder.createNetlistGraph(newModWithDefsInFront)
      gBuilder.dumpDot("after.dot", after, Some(delays))
      newModWithDefsInFront
    } else {
      println ("FAILURE")
      m
    }
  }
  
  def topoSort(edges: List[((NetlistNode, NetlistNode), Int)]): List[((NetlistNode, NetlistNode), Int)] = {
    val g = new MutableDiGraph[NetlistNode]
    val map = edges.map { case ((n1,n2), w) =>
      g.addEdge(n1, n2)
    }
    val edgeMap = edges.toMap 
    try {
      DiGraph(g).linearize.flatMap{n => g.getEdges(n).map(out => (n,out)->edgeMap((n,out)))}.toList
    } catch {
      case c: firrtl.graph.DiGraph$CyclicException => 
        println("Cyclic graph for edge creation/deletion. Cannot topo sort it")
        edges
    } 
  }
  
  def generateRetimingGraph(wdGraph: DiGraph[NetlistNode], clockCycle : Int, W : mutable.Map[(NetlistNode,NetlistNode), Int], D : mutable.Map[(NetlistNode,NetlistNode), Int], retimingWeights : mutable.Map[(NetlistNode,NetlistNode), Int]) : DiGraph[NetlistNode] = {
    val retGraph = new MutableDiGraph[NetlistNode]
    val superNode = new SuperNode("super_node")
    retGraph.addVertex(superNode)
    wdGraph.getVertices foreach {vtx =>  
      retGraph.addVertex(vtx)
      retGraph.addEdge(superNode,vtx)
      retimingWeights += (superNode,vtx)->0    

      wdGraph.getEdges(vtx).foreach {toVtx =>
        retGraph.addEdge(toVtx, vtx)
        // add weights for all existing edges
        retimingWeights += (toVtx,vtx)->W((vtx,toVtx))
      }
      // foreach other vertex create edge and add distance if the distance is above clockCycle
      wdGraph.getVertices foreach {
        toVtx => 
          val dist = D((vtx, toVtx))
          if (dist > clockCycle) {
            val weight = W((vtx,toVtx))
            //println (s"add negative path edge: $toVtx -> $vtx dist = $dist weight = $weight")
            retGraph.addEdge(toVtx, vtx)
            retimingWeights += (toVtx,vtx)->(weight - 1)
          } 
      }
    }
//    println (s"RETIMING GRAPH!")
//    retGraph.getVertices map {node => 
//    		val edges = retGraph.getEdges(node)
//    		println (s"EDGE for node $node $edges")
//    }

    DiGraph(retGraph)

  }
  
  def computeRetimingBF(graph : DiGraph[NetlistNode], weights : mutable.Map[(NetlistNode,NetlistNode), Int], distances : mutable.Map[NetlistNode, Int]) : Boolean = {
    // set initial distances
    graph.getVertices.foreach {vtx => vtx match {
      case SuperNode(_) => distances(vtx) = 0
      case _ => distances(vtx) = graph.getVertices.size // infinity?
    }}
    
    // run relaxation iterations of bellman ford 
    val vtxs = graph.getVertices 
    0 until vtxs.size foreach {_ =>
      vtxs.foreach {
        fromVtx => graph.getEdges(fromVtx) foreach {toVtx => 
          val weight = if(weights.contains((fromVtx, toVtx))) weights((fromVtx, toVtx)) else 0
          if (distances(fromVtx) + weight < distances(toVtx)) {
            distances(toVtx) = distances(fromVtx) + weight
          }
        }
      }
    }
    //check for negative cycles
    val negcycles = vtxs.flatMap {
    	fromVtx => graph.getEdges(fromVtx) map {toVtx =>
      	val weight = if(weights.contains((fromVtx, toVtx))) weights((fromVtx, toVtx)) else 0
      	//list.any will short-circuit, or foldleft
      	if (distances(toVtx) > weight + distances(fromVtx)) {
      		//println (s"The graph contains NEGATIVE-WEIGHT CYCLE")
      		true
      	} else {
      	  false
      	}
    	}
    }
      
    
    distances.foreach {d =>
      //println (s"Distance $d")
    }
    !negcycles.contains(true)
  }
  
  def floydWarshall(netlist : DiGraph[NetlistNode], delays : Map[NetlistNode, Int], W : mutable.Map[(NetlistNode,NetlistNode), Int], D : mutable.Map[(NetlistNode,NetlistNode), Int]) : Unit = {
    val vtxs = netlist.getVertices.toList
    
    vtxs.foreach {vtx =>
      W((vtx,vtx)) = 0
      D((vtx,vtx)) = 0
      
    }
    
    vtxs foreach {fromVtx => fromVtx match {
      case RegNode(_) => netlist.getEdges(fromVtx) foreach {toVtx =>
        D((fromVtx,toVtx)) = 0 
        W((fromVtx,toVtx)) = 1
      }
      case _ => netlist.getEdges(fromVtx) foreach {toVtx => 
        //println(s"${vidMap(fromVtx)},${vidMap(toVtx)}")
        D((fromVtx,toVtx)) = -delays(fromVtx)
        W((fromVtx,toVtx)) = 0
      }
    }}
    
    vtxs.foreach {vtxK =>
      vtxs.foreach {vtxI =>
        vtxs.foreach {vtxJ =>
          if (W((vtxI,vtxJ)) == W((vtxI,vtxK)) + W((vtxK,vtxJ)) && D((vtxI,vtxJ)) > D((vtxI,vtxK)) + D((vtxK,vtxJ))) {
        	  D((vtxI,vtxJ)) = D((vtxI,vtxK)) + D((vtxK,vtxJ))
          }
          if (W((vtxI,vtxJ)) > W((vtxI,vtxK)) + W((vtxK,vtxJ))) {
            W((vtxI,vtxJ)) = W((vtxI,vtxK)) + W((vtxK, vtxJ)) 
            D((vtxI,vtxJ)) = D((vtxI,vtxK)) + D((vtxK,vtxJ))
          }
        }
      }
    }
    
    // print distance matrix
    for (vtx1 <- vtxs; vtx2 <- vtxs) {
      //case (vtx1, vtx2) => {
        //print (s"$vtx1 $vtx2 -> ")
        //print(W((vtx1,vtx2)))
        //print (s" delays = ")
        D((vtx1,vtx2)) = delays(vtx2) - D((vtx1,vtx2))
        
        //println(D((vtx1,vtx2)))
        //if (W((vtx1,vtx2)) == 0) D((vtx1,vtx2)).toFloat -> (vtx1,vtx2) else  D((vtx1,vtx2)).toFloat/W((vtx1,vtx2)) -> (vtx1,vtx2)
    }
  }
  
  def wdOutNode : NetlistNode = {
    NetlistNode("out", "Out")
  }
  def wdInNode : NetlistNode = {
    NetlistNode("in", "In")
  }
  // merge all input terminals into one called "in" and the same for output with a merged output called "out" 
  def convertNetlistToWDGraph(netlist: DiGraph[NetlistNode]) : DiGraph[NetlistNode] = {
    val wdGraph = new MutableDiGraph[NetlistNode]
    val inTerm = wdInNode
    wdGraph.addVertex(inTerm)
    val outTerm = wdOutNode
    wdGraph.addVertex(outTerm)
    wdGraph.addEdge(outTerm, inTerm)
    
    netlist.getVertices foreach { vtx => vtx match {
      // merged all inputs into interm so rewire to interm for all nodes driven by an InputNode 
      case InputNode(name) => {
        netlist.getEdges(vtx) foreach {nextVtx => nextVtx match {
          case OutputNode(oname) => {
            //println(s"INOUT EDGE $name $oname")
            wdGraph.addEdge(inTerm, outTerm)
          }
          case _ => {
            wdGraph.addVertex(nextVtx)
            wdGraph.addEdge(inTerm, nextVtx)
          }
        }}
      }
      // skip output nodes, don't add them to the final graph
      case OutputNode(name) => {
      }
      case _ => {
        // forall comb and reg nodes we add them to the graph
        if (netlist.getEdges(vtx).size != 0) wdGraph.addVertex(vtx)
        netlist.getEdges(vtx) foreach {nextVtx => nextVtx match {
          // for nodes that drive primary outputs, we rewire to drive merged outTerm
          case OutputNode(name) => {
            wdGraph.addEdge(vtx, outTerm)
          }
          case _ => {
            wdGraph.addVertex(nextVtx)
            wdGraph.addEdge(vtx, nextVtx)
          }
        }}
      }
    }}
    DiGraph(wdGraph)
  }

  def insertRegisters(m : DefModule, regDelete : ((NetlistNode, NetlistNode),Int), regDefs : mutable.Map[String,Statement]) : DefModule = {
    println (s"-T- ------ Inserting register ------ at $regDelete")
    var rewritten = false
    var fromType : Type = UnknownType
    var fromNodeIsReset = false

    val namespace = Namespace(m)
    val (clkRef, rstRef) = TransformUtils.getClkRstRefs(m)

    val (from, to, count) = (regDelete._1._1, regDelete._1._2, regDelete._2);
    //if (count != 1) throw new Exception("Retiming of more than 1 register at once is not supported")
    
    def printIfRelevant (str: String)=
      if (str.indexOf(from.name) != -1 && str.indexOf(to.name) != -1) {println (s"statement -> $str")}
    
    def rewireExp (x : Expression,withExp : Expression) : Expression = { 
      //println (s"NESTED EXPRESSION -> $x")
      val new_x = x match {
        case Mux( cond, te, fe, tpe) => 
          Mux(rewireExp(cond, withExp), rewireExp(te, withExp), rewireExp(fe, withExp), tpe)
        case DoPrim( op, inps, z, tpe) => {
          DoPrim(op,inps.map(rewireExp(_,withExp)), z, tpe)
        }
        case ValidIf( cond, te, tpe) => {
          ValidIf(rewireExp(cond, withExp), rewireExp(te, withExp), tpe) 
        }
        case WRef(name, tpe, kind, gender) => 
          //println(s"-T- WREF $name to ${from.name}") 
          if (name == from.name) {
            //println(s"-T- REWIRING $name to $withExp") 
            rewritten = true
            fromType = tpe
            withExp
          } else {
            x
          }
        case z : Literal => x  
        case _ => {
          println (s"-E- insertRegisters:UNSUPPORTED EXPRESSION $x")
          x
        }
      }
      //println (s" NEW NESTED EXPRESSION -> $new_x")
      new_x
    }
    def DefRegWithReset (regName : String) : DefRegister = fromType match {
      case UnknownType => DefRegister(NoInfo, regName, fromType, clkRef, rstRef, WRef(regName))
      case _ =>
        if(flattenType(fromType).width == new IntWidth(1))
      	  DefRegister(NoInfo, regName, fromType, clkRef, rstRef, zero)
      	else 
      	  DefRegister(NoInfo, regName, fromType, clkRef, rstRef, WRef(regName))
    }
    
    def genDefRegChain (regNames : Seq[String]) : Block = {
      //  register definitions
      val regDefs = regNames.map(DefRegWithReset(_))
      //  connecting chain of them if more than 1
      val connStmts : Seq[Statement] = regNames.size match {
        case 1 => Seq(EmptyStmt)
        case _ => regNames.sliding(2).map {case Seq(n1,n2) =>
                    Connect(NoInfo, WRef(n2), WRef(n1))
                  }.toSeq
      }
      val pr = Block(regDefs ++ connStmts)
      Block(regDefs ++ connStmts)
    }

    def txStmt(x : Statement) : Statement = {
      x match {
      case _: Block =>
        x map txStmt
        
      case EmptyStmt =>
        x
      case DefRegister(_,name,tpe,clk,rst,exp) => 
        x
      case DefNode(_,name,exp) => 
        if (name == to.name) {
          //printIfRelevant(s"FOUND DEFNODE $name with expression ${exp.toString}")
          val regNames = (0 until count) map {v => namespace newName s"reg_${from.name}"}
          val newDefNode = DefNode(NoInfo, name, rewireExp(exp,WRef(regNames.last)));
    			//val reg = DefRegWithReset(regName)
          val conReg = Connect(NoInfo, WRef(regNames.head), WRef(from.name))
          val regDef = genDefRegChain(regNames)
          regDefs++= (regNames.toSeq zip regDef.stmts) toMap
          val stmts = Seq(regDef, newDefNode, conReg)
          //println (s" DefNode $name $exp INSERTED STMTS: $stmts")
          Block(stmts)
        } else {
          x
        }
      case Connect(_,loc, expr) => 
        //printIfRelevant(x.toString)
        loc match {
          case WRef(name,tpe,kind,gender) => 
            if (name == to.name) {
              //println (s"FOUND NODE $x expr type ${expr.tpe}")
              val regNames = (0 until count) map {v => namespace newName s"reg_${from.name}"}
              val newConnectNode = Connect(NoInfo, WRef(name), rewireExp(expr,WRef(regNames.last)));
              val conReg = Connect(NoInfo, WRef(regNames.head), WRef(from.name))
              val regDef = genDefRegChain(regNames)
              regDefs++= (regNames.toSeq zip regDef.stmts) toMap
              val stmts = Seq(regDef, newConnectNode, conReg)
              //println (s" Connect $loc $expr INSERTED STMTS: $stmts")
              Block(stmts)
              //TransformUtils.genRegWithEnable(namespace, name,clkRef, rstRef, expr, WRef(pinfo.enable), loc)
              
            } else {
              x
            }
            
          case _ =>
            x
        }
      case _ =>
        x 
      } 
    } 
    
    val mx = m map txStmt 
    if (fromNodeIsReset) {
      println (s"-I- Skipping retiming on reset")
    } else if (!rewritten) {
      println (s"-E- COULD NOT INSERT REGISTER FOR  $from, $to ")
    }
    mx
  }
  
  def deleteRegisters(m : DefModule, regDelete : ((NetlistNode, NetlistNode),Int)) : DefModule = {
    println (s"-T- ------ Deleting register ------ at $regDelete")
    var rewritten = false

    val namespace = Namespace(m)
    val (clkRef, rstRef) = TransformUtils.getClkRstRefs(m)


    val (from, to, count) = (regDelete._1._1, regDelete._1._2, regDelete._2);
    def printIfRelevant (str: String)=
      if (str.indexOf(from.name) != -1 && str.indexOf(to.name) != -1) {println (s"statement -> $str")}
    var regLhsExpr : Option[Expression] = None
    
    def rewireExp (x : Expression) : Expression = { 
      //println (s"NESTED EXPRESSION -> $x")
      val new_x = x match {
        case Mux( cond, te, fe, tpe) => 
          Mux(rewireExp(cond), rewireExp(te), rewireExp(fe), tpe)
        case DoPrim( op, inps, z, tpe) => {
          DoPrim(op,inps.map(rewireExp(_)), z, tpe)
        }
        case ValidIf( cond, te, tpe) => {
          ValidIf(rewireExp(cond), rewireExp(te), tpe) 
        }
        case WRef(name, tpe, kind, gender) => 
          //println(s"-T- WREF $name to ${from.name}") 
          if (name == from.name) {
            //println(s"-T- REWIRING $name to $withExp") 
            rewritten = true
            regLhsExpr.get
          } else {
            x
          }
        case z : Literal => x  
          
        case _ => {
          println (s"-E- deleteRegisters:UNSUPPORTED EXPRESSION $x")
          x
        }
      }
      //println (s" NEW NESTED EXPRESSION -> $new_x")
      new_x
    }
    
    def findRegExpr(x : Statement) : Statement = x match {
      case _: Block =>
        x map findRegExpr
        
      case Connect(_,loc, expr) =>
        loc match {
          case WRef(name,tpe,kind,gender) => 
            if (name == from.name) {
              regLhsExpr = Some(expr)
              x
            } else {
              x
            }
          case _ =>
            x
        }
      case _ =>
        x 
    }
    
    def txStmt (x : Statement) : Statement = {
      x match {
      case _: Block =>
        x map txStmt
        
      case EmptyStmt =>
        x
      case DefRegister(_,name,tpe,clk,rst,exp) =>
        x
      case DefNode(_,name,exp) =>
        if(name == to.name) {
          val dn = DefNode(NoInfo,name,rewireExp(exp))
          //println (s"Deleting register: DefNode$name $exp -> new node $dn")
          dn 
        } else {
          x
        }
      case Connect(_,loc, exp) =>
        loc match {
          case WRef(name,tpe,kind,gender) => 
            if (name == to.name) {
              val nc = Connect(NoInfo, loc,rewireExp(exp))
              //println (s"Deleting register: COnnect $loc $exp -> new node $nc")
              nc//Connect(NoInfo, loc,rewireExp(exp)) 
            } else {
              x
            }
            
          case _ =>
            x
        }
      case _ =>
        x 
      } 
    } 
    
    
    m map findRegExpr

    if (!regLhsExpr.isDefined) {
      println (s"-E- COULD NOT FIND LHS OF THE REG  $from ")
      return m
    } 
    val mx = m map txStmt 
    
    if (!rewritten) {
      println (s"-E- COULD NOT DELETE REGISTER FOR  $from, $to ")
    }
    
    mx
  }
  
  def modifyModuleDef(rsigs : Set[NetlistNode], regDefs : mutable.Map[String,Statement], m : DefModule, regChange : ((NetlistNode, NetlistNode),Int)) : DefModule = {
    regChange._2 match {
      case 0 => m
      case x if x<0 => {
        deleteRegisters(m, regChange)
      }
      case x => {
        if (!rsigs.contains(regChange._1._1))
          insertRegisters(m, regChange, regDefs)
        else {
          println (s"Skipping reset retiming ${regChange._1}")
          m
        }
      }
    }
  }

  def extractRegChangeCount(netlist: DiGraph[NetlistNode], W: Map[(NetlistNode,NetlistNode),Int], distances: Map[NetlistNode,Int]) = {
    val vtxs = netlist.getVertices
    
    vtxs flatMap {fromVtx => netlist.getEdges(fromVtx) map {toVtx => 
      val weight = if (W.contains((fromVtx, toVtx))) W((fromVtx, toVtx)) else 0 
      (fromVtx, toVtx) match {
        case (InputNode(_), OutputNode(_)) => (fromVtx, toVtx) -> (distances(wdOutNode) - distances(wdInNode))
        case (_, OutputNode(_)) => (fromVtx, toVtx) -> (distances(wdOutNode) - distances(fromVtx))
        case (InputNode(_), _)  => (fromVtx, toVtx) -> (distances(toVtx) - distances(wdInNode))
        case _ => (fromVtx, toVtx) -> (distances(toVtx) - distances(fromVtx))
      }
    }}
  }

  def rewriteDefs(m: DefModule, regDefs: Map[String,Statement], nodeDefs: Map[String,Statement]) : DefModule = {
    // node has to go after reg defs as nodes have definitions references in them
    val defsList = regDefs.values.toList ++ nodeDefs.values.toList
    var added = false

    def txStmt (x : Statement) : Statement = {
      x match {
      case _: Block =>
        x map txStmt
        
      case EmptyStmt =>
        x
        
      case DefRegister(_,name,tpe,clk,rst,exp) => (added,regDefs.contains(name)) match {
        case (false, true) => { 
          added = true
          Block (defsList)
        }
        case (false, false) => {
          added = true
          Block (defsList :+ x)
        }
        case (true, true) => EmptyStmt
        case (true, false) => x
      }
        
        
      case DefNode(_,name,exp) =>  added match {
        case false => {
          added = true
          Block (defsList :+ x)
        }
        case true => x
      }
      case _ =>
        x 
      
      }
    } 
    val mx = m map txStmt
    mx
  }
}


  object TransformUtils {

    def portRef(op : Option[Port]) : Expression = op match {
      case Some(p) => WRef(p.name, p.tpe)
      case None =>  
        throw new Exception("port does not exist...");
        //zero
    }
    def getResetSignals(m: DefModule, netlist : DiGraph[NetlistNode]) : Set[NetlistNode]= {
      val resetNodes = mutable.HashSet.empty[NetlistNode]
      val (clk,rst) = getClkRstRefs(m)
      
      def recFanout (n : NetlistNode) : Unit = {
        netlist.getEdges(n) foreach {out => out match {
          case CombNode(_) => 
            resetNodes += out
            recFanout(out)
          case _ => Unit
        }}
      }
      
      rst match {
        case WRef(name,_,_,_) => {
          recFanout(InputNode(name)) 
          resetNodes += InputNode(name)
        }
        case UIntLiteral(_,_) => {
          
        }
      }
      resetNodes.toSet
    }
    
    def getClkRstRefs(m: DefModule) = {
        val clk_rst = m.ports.foldLeft((None, None) : (Option[Port], Option[Port])) {
          (tuple, p)=> p match {
            case Port(info, name, direction,tpe) =>
              tpe match {
                case ClockType => 
                  tuple.copy(_1 = Some(p))
                case _ =>  name match {
                  case "reset" => tuple.copy(_2 = Some(p))
                  case _ => tuple
              }
  
              }
          }
        }
        
        val clkRef = portRef(clk_rst._1)
        
        val rstRef = if (clk_rst._2 == None) UIntLiteral(0, new IntWidth(1)) else portRef(clk_rst._2)
        (clkRef, rstRef)
    }
    
  	def genRegWithEnable(n : Namespace,  basename : String, clk : Expression, rst : Expression, in : Expression, en : Expression, out : Expression) : Block =  {
  			val regName = n newName s"reg_$basename"
  			val muxName = n newName s"mux_$basename"
  			
        val reg = DefRegister(NoInfo, regName, in.tpe, clk, rst, WRef(regName))
  			val conReg = Connect(NoInfo, WRef(regName), Mux(en, in, out, in.tpe))
  			val rewireToReg = Connect(NoInfo, out, WRef(regName))
  			 
  			println (s"REGEN BLOCK ${Block(Seq(reg, conReg, rewireToReg))}")
  			Block(Seq(reg, conReg, rewireToReg))
  	}
  	
  	def genReg(n : Namespace,  basename : String, clk : Expression, rst : Expression, in : Expression, out : Expression) : Block =  {
  			val regName = n newName s"reg_$basename"
  
  			val reg = DefRegister(NoInfo, regName, in.tpe, clk, rst, WRef(regName))
  			val conReg = Connect(NoInfo, WRef(regName), in)
  			val rewireToReg = Connect(NoInfo, out, WRef(regName))
  			
  			//println (s"REGEN BLOCK ${Block(Seq(reg, conReg, rewireToReg))}")
  			Block(Seq(reg, conReg, rewireToReg))
  	}

  }
