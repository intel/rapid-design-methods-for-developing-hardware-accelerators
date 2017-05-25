package imperative

import org.scalatest.{ Matchers, FlatSpec, FreeSpec}

import org.scalameter.api._

import scala.annotation.tailrec

class F
case class FSeq( lst : List[F]) extends F
case class FPrim( x : Int) extends F

object F {
  def e( s : FSeq) = FSeq.unapply( s).get
  def flatten( f : F) : FSeq = f match {
    case FSeq( lst) => lst.foldLeft( FSeq( List[F]())){ case ( l,x) =>
      FSeq( e( l) ++ e( flatten( x)))
    }
    case FPrim( _) => FSeq( List(f))
  }

  def flatten2( f : F) : FSeq = {
    def aux( accum : List[F], f : F) : List[F] = f match {
      case FPrim( _) => f :: accum
      case FSeq( hd::tl) => aux( aux( accum, hd), FSeq( tl))
      case FSeq( Nil) => accum
    }
    FSeq( aux( List(), f).reverse)
  }


  def doit( func : F => FSeq) =
    func( FSeq( List( FSeq( List( FSeq( List( FPrim(0))), FPrim(1))), FPrim(2))))

}

class FTest extends FreeSpec with Matchers {
  "F" - {
    "F.flatten works" in {
      F.doit{ F.flatten} should be ( FSeq(List(FPrim(0), FPrim(1), FPrim(2))))
    }
    "F.flatten2 works" in {
      F.doit{ F.flatten2} should be ( FSeq(List(FPrim(0), FPrim(1), FPrim(2))))
    }
    "flatten shouldn't take too much time" in {
      val n = 3000
      F.flatten( FSeq( (for( i <- 0 until n) yield FSeq( List( FPrim(i)))).toList)) should be (
         FSeq( (for( i <- 0 until n) yield FPrim( i)).toList))
    }
    "flatten2 shouldn't take too much time" in {
      val n = 3000
      F.flatten2( FSeq( (for( i <- 0 until n) yield FSeq( List( FPrim(i)))).toList)) should be (
         FSeq( (for( i <- 0 until n) yield FPrim( i)).toList))
    }
  }
}

object FBenchmark extends Bench.LocalTime {
  val sizes = Gen.exponential("size")( 128, 8*1024, 2)
  val ranges = for {
    n <- sizes
  } yield FSeq( (for( i <- 0 until n) yield FSeq( List( FPrim(i)))).toList)
  performance of "F" in {
    measure method "flatten" in {
      using(ranges) in { r => { F.flatten( r); ()}}
    }
    measure method "flatten2" in {
      using(ranges) in { r => { F.flatten2( r); ()}}
    }
  }
}
