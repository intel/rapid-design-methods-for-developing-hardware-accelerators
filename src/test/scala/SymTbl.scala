package imperative

import org.scalatest.{ Matchers, FlatSpec}

import chisel3._
import chisel3.util._

class SymTblTest extends FlatSpec with Matchers {
  behavior of "SymTblTest"
  it should "initially have no variables" in {
    val sT = new SymTbl()
    sT.keys should be ( Set())
  }
  it should "initially have no ports" in {
    val sT = new SymTbl()
    sT.pkeys should be ( Set())
  }
  it should "have one var if we add one" in {
    val sT = new SymTbl().insert( "f", 0.U)
    sT.keys should be ( Set("f"))
  }
  it should "have one port if we add one" in {
    val sT = new SymTbl().pupdated( "P", false.B, false.B, 0.U)
    sT.pkeys should be ( Set("P"))
  }
  it should "push, insert, pop should leave no keys" in {
    val sT = new SymTbl().push.insert( "f", 0.U).pop
    sT.keys should be ( Set())
  }
  it should "updated should not changed anything if the key is missing" in {
    val sT = new SymTbl().updated( "f", 0.U)
    sT.keys should be ( Set())
  }
  it should "have one var if we update it" in {
    val sT = new SymTbl().insert( "f", 0.U)
    sT.keys should be ( Set("f"))
    sT("f").litValue should be ( 0)
    val sT2 = sT.updated( "f", 1.U)
    sT2.keys should be ( Set("f"))
    sT2("f").litValue should be ( 1)
  }

}
