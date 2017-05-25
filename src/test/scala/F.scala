package imperative

import org.scalatest.{ Matchers, FlatSpec, FreeSpec}

import org.scalameter.api._

import scala.annotation.tailrec

class F
case class FSeq( lst : Seq[F]) extends F
case class FPrim( x : Int) extends F

object F {
  def e( s : FSeq) = FSeq.unapply( s).get
  def flatten( f : F) : FSeq = f match {
    case FSeq( lst) => lst.foldLeft( FSeq( List[F]())){ case ( l,x) =>
      FSeq( e( l) ++ e( flatten( x)))
    }
    case FPrim( _) => FSeq( List(f))
  }

  def doit = flatten( FSeq( List( FSeq( List( FSeq( List( FPrim(0))), FPrim(1))), FPrim(2))))

}

class FTest extends FreeSpec with Matchers {
  "F" - {
    "F.doit works" in {
      F.doit should be ( FSeq(List(FPrim(0), FPrim(1), FPrim(2))))
    }
    "F slugbug shouldn't take too much time" in {
      val n = 30000
      F.flatten( FSeq( (for( i <- 0 until n) yield FSeq( List( FPrim(i)))))) should be (
         FSeq( (for( i <- 0 until n) yield FPrim( i))))
    }
  }
}

object RangeBenchmark extends Bench.LocalTime {
  val sizes = Gen.exponential("size")( 128, 8*1024, 2)
  val ranges = for {
    n <- sizes
  } yield FSeq( (for( i <- 0 until n) yield FSeq( List( FPrim(i)))))
  println( s"${ranges}")
  performance of "Range" in {
    measure method "F.flatten" in {
      using(ranges) in { r => { F.flatten( r); ()}}
    }
  }
}
