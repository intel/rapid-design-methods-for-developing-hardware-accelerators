package imperative

import chisel3._
import chisel3.util._
import collection.immutable.ListMap

class SymTbl( 
  private var pm : ListMap[String,DecoupledIO[UInt]] = ListMap(),
  private var listOfLM : List[ListMap[String,UInt]] = List(ListMap())
) {

  def updated( s : String, v : UInt) : SymTbl = {
    new SymTbl( pm, List(listOfLM.head.updated( s, v)))
  }
  def keys = listOfLM.head.keys
  def apply( k : String) = listOfLM.head(k)
  def push = new SymTbl( pm, ListMap[String,UInt]() :: listOfLM) 
  def pop = new SymTbl( pm, listOfLM.tail) 

  def pupdated( s : String, v : DecoupledIO[UInt]) : SymTbl = {
    new SymTbl( pm.updated( s, v), listOfLM)
  }
  def pkeys = pm.keys
  def pget( k : String) = pm(k)
}
