package imperative

object TopoSort {
  def apply[T]( nodes : Seq[T], arcs : Seq[(T,T)]) : List[T] = {
    type B = (Map[T,Boolean],Map[T,Boolean],List[T])
    val m0 = nodes.foldLeft( Map[T,List[T]]()){ case ( m, s) => m.updated( s, List())}
    val g = arcs.foldLeft( m0){ case ( m, (s,t)) => m.updated( s, t :: m( s))}

    def dfs( b : B, s : T) : B = {
      val b0 = (b._1.updated( s, true),b._2.updated( s, true),b._3)
      val (l,v,a) = g(s).foldLeft( b0){ case ( b@(l,v,a), t) =>
        if ( l(t)) throw new Exception("Cycle found")
        else if ( !v(t)) dfs( b, t) else b
      }
      ( l.updated( s, false), v, s :: a)
    }

    val blank = g.keys.foldLeft( Map[T,Boolean]()){ case ( m, s) => m.updated( s, false)}
    g.keys.foldLeft( (blank,blank,List[T]())){ case ( (l,v,a), s) =>
      if ( !v(s)) dfs( (l,v,a), s) else (l,v,a)
    }._3
  }
}
