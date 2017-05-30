package imperative

import org.scalatest.{ Matchers, FlatSpec}

class TopoSortTest extends FlatSpec with Matchers {
  behavior of "TopoSortTest"
  it should "toposort integers" in {
    val topo = TopoSort( List( 0, 1, 2, 3), List( (0,1), (2,0), (1,3)))
    topo should be ( List( 2,0,1,3))
  }
  it should "toposort strings" in {
    val topo = TopoSort( List( "steve", "burns", "is", "a"), List( ("steve","burns"), ("is","steve"), ("burns","a")))
    topo should be ( List( "is","steve","burns","a"))
  }
  it should "throw exception on cyclic graph" in {
    an [Exception] should be thrownBy {
      TopoSort( List( 0, 1), List( (0,1), (1,0)))
    }
  }
}
