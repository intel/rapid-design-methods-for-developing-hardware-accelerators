// See LICENSE for license details.
package designutils
 
import language.experimental.macros
import reflect.macros.Context
 
import chisel3._
 
object DebugMacros {
  
  def pf(format : String, params: Bits*): Unit = macro printf_impl
  def pf(pable : Printable): Unit = macro printf_printable_impl

  
  def printf_impl(c: Context)(format : c.Expr[String], params: c.Expr[Bits]*): c.Expr[Unit] = {
    import c.universe._
    
    val printf_func = Select(Ident(TermName("printf")), TermName("apply"))
    val paramtree : Seq[Tree]= params.map(param => param.tree) 

    val printftree = Apply(printf_func, format.tree :: paramtree.toList)

    val condprintf = If(Ident(TermName("DEBUG_PRINT")), printftree, Literal(Constant(())))

    c.Expr[Unit](condprintf)
  }

  def printf_printable_impl(c: Context)(pable : c.Expr[Printable]): c.Expr[Unit] = {
    import c.universe._
    val condprintf = If(Ident(TermName("DEBUG_PRINT")), reify(printf(pable.splice)).tree, Literal(Constant(())))
     
    c.Expr[Unit](condprintf)
  }

}
