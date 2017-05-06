package imperative

import chisel3._
import chisel3.util._
import collection.immutable.ListMap

class SymTbl( 
  private val pm : ListMap[String,(Bool,Bool,UInt)] = ListMap(),
  private val listOfLM : List[ListMap[String,UInt]] = List(ListMap())
) {

  def updated( s : String, v : UInt) : SymTbl = {
    new SymTbl( pm, List(listOfLM.head.updated( s, v)))
  }
  def keys = listOfLM.head.keys
  def apply( k : String) = listOfLM.head(k)
  def push = new SymTbl( pm, ListMap[String,UInt]() :: listOfLM) 
  def pop = new SymTbl( pm, listOfLM.tail) 

  def pupdated( s : String, r : Bool, v : Bool, d : UInt) : SymTbl = {
    new SymTbl( pm.updated( s, (r,v,d)), listOfLM)
  }
  def pkeys = pm.keys
  def pget( k : String) = pm(k)
}
