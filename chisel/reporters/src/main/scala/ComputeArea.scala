// See LICENSE for license details.
package reporters

import collection.mutable

sealed abstract class Area()
case object AreaNone extends Area
case class AreaModule( templateName : String, instanceName : String) extends Area
case class AreaMux( w : Int, cConds : Int, cExprs : Int) extends Area
case class AreaRegister( w : Int) extends Area
case class AreaMemory( bitWidth : Int, words : Int, rdPorts : Int, wrPorts : Int, rdWrPorts : Int) extends Area
case class AreaOp( nm : String, ninps : List[Int], w : Int, cExprs : Int) extends Area {
  override def toString() : String = {
    val ninpsStr = ninps.mkString( "(", ",", ")")
    s"AreaOp(${nm},${ninpsStr},${w},${cExprs})"
  }
}

object ComputeArea {
  val cBitCell = 6
  val cMux = 8
  val cMux4 = 20
  val cMaj = 12
  val cXor = 10
  val cReg = 30
  val cNand2 = 4
  val cNand3 = 6
  def cNand( n : Int) : Int = 
    if ( n < 2) 0 else if ( n % 2 == 0) cNand2 + (n-2)/2*cNand3 else cNand3 + (n-3)/2*cNand3

  def apply( a : Area, tbl : mutable.Map[String,Int]) : Int = a match {
    case AreaOp( "add", List(w0,w1), w, 0) => w*(cMaj+2*cXor)
    case AreaOp( "add", List(w0,w1), w, 1) => w*(cNand( 2)+cXor)
    case AreaOp( "add", List(w0,w1), w, 2) => 0
    case AreaOp( "sub", List(w0,w1), w, 0) => w*(cMaj+2*cXor)
    case AreaOp( "sub", List(w0,w1), w, 1) => w*(cNand( 2)+cXor)
    case AreaOp( "sub", List(w0,w1), w, 2) => 0
    case AreaOp( op, List(w0), w, c) if List("shl","shr","shlw").contains( op) => 0
    case AreaOp( "dshl", List(w0,w1), w, 0) => cMux4*w0*((w1+1)/2)
    case AreaOp( "dshlw", List(w0,w1), w, 0) => cMux4*w0*((w1+1)/2)
    case AreaOp( "dshl", List(w0,w1), w, c) if c > 0 => 0
    case AreaOp( "dshr", List(w0,w1), w, 0) => cMux4*w0*((w1+1)/2)
    case AreaOp( "dshrw", List(w0,w1), w, 0) => cMux4*w0*((w1+1)/2)
    case AreaOp( "dshr", List(w0,w1), w, c) if c > 0 => 0
    case AreaOp( op, inpSizes, w, c) if List("and","or").contains( op) =>
      w*cNand( inpSizes.size - c)
    case AreaOp( "xor", List(w0,w1), w, 0) => w*cXor
    case AreaOp( "xor", List(w0,w1), w, c) if c > 0 => 0
    case AreaOp( "not", List(w0), w, c) => 0
    case AreaOp( "neq", List(w0,w1), w, c) => apply( AreaOp( "eq", List(w0,w1), w, c), tbl)
    case AreaOp( "eq", List(w0,w1), w, 0) => w0*cXor+cNand(w0)
    case AreaOp( "eq", inpSizes@List(w0,w1), w, 1) => apply( AreaOp( "and", inpSizes, w0, 0),tbl)
    case AreaOp( "eq", List(w0,w1), w, 2) => 0
    case AreaOp( "mul", List(w0,w1), w, 0) => w0*apply(AreaOp("add", List(w1,w), w, 0),tbl)+apply( AreaOp( "add", List(w,w), w, 0),tbl)
    case AreaOp( "mul", List(w0,w1), w, 1) => 0 // only if constant a power of 2 
    case AreaOp( "mul", List(w0,w1), w, 2) => 0
    case AreaOp( "div", List(w0,w1), w, 0) => 2*apply( AreaOp( "mul", List(w0,w1), w, 0), tbl)
    case AreaOp( "div", List(w0,w1), w, 1) => 0 // only if constant a power of 2
    case AreaOp( "div", List(w0,w1), w, 2) => 0
    case AreaOp( "rem", List(w0,w1), w, c) => apply( AreaOp( "div", List(w0,w1), w, c), tbl)
    case AreaOp( op, List(w0,w1), w, c) if List("lt","gt","leq","geq").contains( op) => apply( AreaOp( "add", List(w0,w1), w0, c),tbl)
    case AreaOp( "cat", _, _, _) => 0
    case AreaOp( "bits", _, _, _) => 0
    case AreaOp( "pad", _, _, _) => 0
    case AreaOp( "tail", _, _, _) => 0
    case AreaOp( "asUInt", _, _, _) => 0
    case AreaOp( "asSInt", _, _, _) => 0
    case AreaOp( "cvt", _, _, _) => 0
    case AreaRegister( w) => w*cReg
    case AreaMemory( bitWidth, words, _, _, _) => bitWidth*words*cBitCell
    case AreaMux( w, 0, 0) => w*cMux
    case AreaMux( w, 1, _) => 0
    case AreaMux( w, 0, 2) => 0
    case AreaMux( w, 0, 1) => apply( AreaOp( "and", List(w,w), w, 0),tbl)
    case AreaModule( templateName, instanceName) =>
      if ( tbl.contains(templateName))
        tbl(templateName)
      else {
        println( s"Unknown module template name: ${templateName}")
        0
      }
    case AreaNone => 0
    case _ => println( s"unknown op ${a}"); 0
  }

  def apply( m : mutable.Map[Area,Int], tbl : mutable.Map[String,Int]) : Int =
    m.toList.foldLeft( 0){ case (s,(k : Area,v : Int)) => s + v * apply(k,tbl)}

}
