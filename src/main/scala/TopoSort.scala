package imperative

object TopoSort {
  def genGraph[T]( lst : List[(T,T)]) : Map[T,List[T]] = {
    val m = lst.foldLeft( Map[T,List[T]]()){ 
      case ( m, (s,t)) => m.updated( s, List()).updated( t, List())
    }
    lst.foldLeft( m){ case ( m, (s,t)) => m.updated( s, t :: m( s))}
  }

  def topo[T]( g : Map[T,List[T]]) : List[T] = {
    type B = (Map[T,Boolean],Map[T,Boolean],List[T])
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
