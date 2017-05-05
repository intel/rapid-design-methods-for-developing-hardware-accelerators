package imperative

import org.scalatest.{ Matchers, FlatSpec, GivenWhenThen}
import org.scalacheck.{ Properties, Gen, Arbitrary}
import org.scalacheck.Prop.{ forAll, AnyOperators, collect}

import chisel3._
import firrtl_interpreter.InterpretiveTester

import collection.immutable.ListMap

abstract class AST

abstract class Command extends AST
abstract class Expression extends AST
abstract class BExpression extends Expression

case class Port( nm : String) extends Expression
case class Variable( nm : String) extends Expression
case class ConstantInteger( c : Int) extends Expression
case class ConstantTrue() extends BExpression

case class Assignment( lhs : Variable, rhs : Expression) extends Command
case class While( cond : BExpression, body : Command) extends Command
case class IfThenElse( cond : BExpression, bodyT : Command, bodyF : Command) extends Command
case class SequentialComposition( seq : Seq[Command]) extends Command
case class AddExpression( l : Expression, r : Expression) extends Expression
case class AndBExpression( l : BExpression, r : BExpression) extends BExpression
case class NotBExpression( e : BExpression) extends BExpression
case class Wait() extends Command
case class NBCanGet( p : Port) extends BExpression
case class NBCanPut( p : Port) extends BExpression
case class NBGet( p : Port) extends Expression
case class NBPut( p : Port, e : Expression) extends Command




object G {
  val width = 8

  val ast : AST = While(
    ConstantTrue(),
    SequentialComposition(
      List( Assignment( Variable( "o"), AddExpression( Variable( "a"), Variable( "b"))),
            Assignment( Variable( "o"), AddExpression( Variable( "a"), Variable( "o"))),
            Assignment( Variable( "o"), AddExpression( Variable( "b"), Variable( "o"))))))
}

class CustomUIntBundle(elts: (String, UInt)*) extends Record {
  val elements = ListMap(elts map { case (field, elt) => field -> elt.chiselCloneType }: _*)
  def apply(elt: String): UInt = elements(elt)
  override def cloneType = (new CustomUIntBundle(elements.toList: _*)).asInstanceOf[this.type]
}

class AddIO extends CustomUIntBundle( ("a", Input(UInt(G.width.W))),
  ("b", Input(UInt(G.width.W))),
  ("o", Output(UInt(G.width.W))))

class Add extends Module {
  val io = IO(new AddIO)

  val inps = io.elements.filter{ case (k,v) => v.dir == core.Direction.Input}
  val outs = io.elements.filter{ case (k,v) => v.dir == core.Direction.Output}

  type SymTbl = ListMap[String,UInt]

  def wireCopy( st : SymTbl) = st

  def evalExpression( ast : Expression, sT : SymTbl) : UInt = ast match {
    case Variable( s) => sT( s)
    case AddExpression( l, r) => evalExpression( l, sT) + evalExpression( r, sT)
  }

  def evalBExpression( ast : BExpression, sT : SymTbl) : Bool = ast match {
    case ConstantTrue() => true.B
    case AndBExpression( l, r) => evalBExpression( l, sT) && evalBExpression( r, sT)
    case NotBExpression( e) => !evalBExpression( e, sT)
  }

  def evalCommand( ast : AST, sT : SymTbl) : SymTbl = ast match {
    case While( ConstantTrue(), b) => evalCommand( b, sT)
    case SequentialComposition( seq) => seq.foldLeft(sT){ case (s,a) => evalCommand(a,s) }
    case Assignment( Variable( s), r) => sT.updated( s, evalExpression( r, sT))
    case IfThenElse( b, t, e) => {
      val bb = evalBExpression( b, sT)
      val tSymTbl = wireCopy( sT)
      val eSymTbl = wireCopy( sT)
      tSymTbl
    }
  }

  val sT : SymTbl = ListMap()
  val sTInps = inps.foldLeft( sT    ){ case (s,(k,v)) => s.updated( k, io(k))}
  val sTOuts = outs.foldLeft( sTInps){ case (s,(k,v)) => s.updated( k, io(k).cloneType)}
  val sTLast = evalCommand( G.ast, sTOuts)
  outs.foreach{ case (k,v) => io(k) := sTLast(k) }

}

class AddTester {
  val s = chisel3.Driver.emit( () => new Add)
  val tester = new InterpretiveTester( s)
  def run( a : Int, b : Int) = {
    val result = (a + b + a + b) & ((1 << G.width)-1)
    tester.poke( s"io_a", a)
    tester.poke( s"io_b", b)
    tester.peek( s"io_o") ?= result
  }
}

object AddTest extends Properties("Add") {
  val t = new AddTester
  val gen = Gen.choose(0,(1 << G.width)-1)
  property("Add") = forAll( gen, gen) {
    case (a:Int,b:Int) => t.run( a, b)
  }
}
