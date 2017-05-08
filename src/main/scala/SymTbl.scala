package imperative

import chisel3._
import chisel3.util._
import collection.immutable.ListMap

import scala.annotation.tailrec

class SymTbl( 
  private val pm : ListMap[String,(Bool,Bool,UInt)] = ListMap(),
  private val listOfLM : List[ListMap[String,UInt]] = List(ListMap())
) {

  def contains( s : String) = listOfLM.foldLeft( false){ case (b,lm) => b || lm.contains(s)}

  def insert( s : String, v : UInt) = new SymTbl( pm, listOfLM.head.updated( s, v) :: listOfLM.tail)
  def push = new SymTbl( pm, ListMap[String,UInt]() :: listOfLM) 
  def pop = new SymTbl( pm, listOfLM.tail) 


  def keys = listOfLM.foldLeft( Set[String]()){ case (s,lm) => s union lm.keys.toSet}

  def apply( k : String) : UInt = {
    @tailrec
    def lookup( listOfLM : List[ListMap[String,UInt]], k : String) : Option[UInt] = {
      listOfLM match {
        case hd :: tl if hd.contains( k) => Some(hd(k))
        case hd :: tl if !hd.contains( k) => lookup( tl, k)
        case Nil => None
      }
    }
    lookup( listOfLM, k).get
  }

  def updated( s : String, v : UInt) : SymTbl = {
    def aux( listOfLM : List[ListMap[String,UInt]], k : String, v : UInt) : List[ListMap[String,UInt]] = {
      listOfLM match {
        case hd :: tl if hd.contains( k) => hd.updated( k, v) :: tl
        case hd :: tl if !hd.contains( k) => hd :: aux( tl, k, v)
        case Nil => Nil
      }
    }
    if ( !contains( s)) {
      println( s"updated: ${s} not in SymTbl. Table not updated")
      this
    } else {
      new SymTbl( pm, aux( listOfLM, s, v))
    }
  }


/*
 * Get rid of these eventually
 */
  def pupdated( s : String, r : Bool, v : Bool, d : UInt) : SymTbl = {
    new SymTbl( pm.updated( s, (r,v,d)), listOfLM)
  }
  def pkeys = pm.keys
  def pget( k : String) = pm(k)
}
