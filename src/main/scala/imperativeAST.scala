package parser

import scala.util.parsing.input.Positional

sealed trait Command extends Positional
sealed trait Expression extends Positional
sealed trait BExpression extends Positional

case class Port( nm : String) extends Expression
case class Variable( nm : String) extends Expression
case class ConstantInteger( c : Int) extends Expression
case object ConstantTrue extends BExpression

case class Assignment( lhs : Variable, rhs : Expression) extends Command
case class While( cond : BExpression, body : Command) extends Command
case class IfThenElse( cond : BExpression, bodyT : Command, bodyF : Command) extends Command
case class SequentialComposition( seq : Seq[Command]) extends Command
case class AddExpression( l : Expression, r : Expression) extends Expression
case class MulExpression( l : Expression, r : Expression) extends Expression
case class EqBExpression( l : Expression, r : Expression) extends BExpression
case class AndBExpression( l : BExpression, r : BExpression) extends BExpression
case class NotBExpression( e : BExpression) extends BExpression
case object Wait extends Command
case class NBCanGet( p : Port) extends BExpression
case class NBCanPut( p : Port) extends BExpression
case class NBGetData( p : Port) extends Expression
case class NBGet( p : Port) extends Command
case class NBPut( p : Port, e : Expression) extends Command
