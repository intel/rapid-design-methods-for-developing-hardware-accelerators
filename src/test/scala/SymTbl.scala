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
  it should "return an unmasked variable deeper in the stack" in {
    val sT = new SymTbl().insert( "g", 0.U).push.insert( "f", 1.U)
    sT("g").litValue should be ( 0)
  }
  it should "return all the keys in the stack" in {
    val sT = new SymTbl().insert( "g", 0.U).push.insert( "f", 1.U)
    sT.keys should be ( Set("f","g"))
  }
  it should "update an unmasked variable deeper in the stack" in {
    val sT = new SymTbl().insert( "g", 0.U).push.insert( "f", 1.U).updated( "g", 1.U)
    sT.keys should be ( Set("f","g"))
    sT("g").litValue should be ( 1)
    sT.pop("g").litValue should be ( 1)
  }
  it should "not change a masked variable deeper in the stack" in {
    val sT = new SymTbl().insert( "f", 0.U).push.insert( "f", 1.U).updated( "f", 2.U)
    sT.keys should be ( Set("f"))
    sT("f").litValue should be ( 2)
    sT.pop("f").litValue should be ( 0)
  }
  it should "not change the SymTbl if you update an unknown variable" in {
    val sT = new SymTbl().insert( "f", 0.U).push.updated( "g", 2.U)
    sT.keys should be ( Set("f"))
    sT("f").litValue should be ( 0)
  }

}
