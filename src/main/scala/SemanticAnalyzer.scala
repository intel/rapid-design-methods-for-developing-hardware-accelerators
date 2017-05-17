package imperative

import compiler._

object SemanticAnalyzer {

  def apply( ast : Process) : Either[CompilationError, Process] = {
    ast match {
      case Process( _, Blk( _, _)) => {
//        println( s"Performing semantic analysis ${ast}")
        Right( ast)
      }
      case _ => {
        Left(SemanticAnalyzerError("Can't have while at top level"))
      }
    }
  }

}
