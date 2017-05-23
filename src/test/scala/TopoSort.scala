package imperative

import org.scalatest.{ Matchers, FlatSpec}

class TopoSortTest extends FlatSpec with Matchers {
  behavior of "TopoSortTest"
  it should "build edges" in {
    val graph = TopoSort.genGraph( List( (0,1), (2,0), (1,3)))
    graph should be ( Map[Int,List[Int]]( 0 -> List(1), 2 -> List(0), 1 -> List(3), 3 -> List()))
  }
  it should "toposort" in {
    val graph = TopoSort.genGraph( List( (0,1), (2,0), (1,3)))
    val topo = TopoSort.topo( graph)
    topo should be ( List( 2,0,1,3))
  }
  it should "throw exception on cyclic graph" in {
    val graph = TopoSort.genGraph( List( (0,1), (1,0)))
    an [Exception] should be thrownBy {
      TopoSort.topo( graph)
    }
  }
}
