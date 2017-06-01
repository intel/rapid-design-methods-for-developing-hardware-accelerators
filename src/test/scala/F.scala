package imperative

import org.scalatest.{ Matchers, FlatSpec, FreeSpec}

import org.scalatest.tagobjects.Slow

import org.scalameter.api._

import scala.annotation.tailrec

class F
case class FSeq( lst : Seq[F]) extends F
case class FPrim( x : Int) extends F

object F {
  def e( s : FSeq) = FSeq.unapply( s).get
  def flatten( f : F) : FSeq = f match {
    case FSeq( lst) => lst.foldLeft( FSeq( Seq[F]())){ case ( l,x) =>
      FSeq( e( l) ++ e( flatten( x)))
    }
    case FPrim( _) => FSeq( List(f))
  }

  def flatten2( f : F) : FSeq = {
    def aux( accum : List[F], f : F) : List[F] = f match {
      case FPrim( _) => f :: accum
      case FSeq( seq) if !seq.isEmpty => aux( aux( accum, seq.head), FSeq( seq.tail))
      case FSeq( seq) if seq.isEmpty => accum
    }
    FSeq( aux( List(), f).reverse.toSeq)
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
      F.flatten( FSeq( (for( i <- 0 until n) yield FSeq( Seq( FPrim(i)))))) should be (
         FSeq( (for( i <- 0 until n) yield FPrim( i))))
    }
    "flatten2 shouldn't take too much time" in {
      val n = 3000
      F.flatten2( FSeq( (for( i <- 0 until n) yield FSeq( Seq( FPrim(i)))))) should be (
         FSeq( (for( i <- 0 until n) yield FPrim( i))))
    }
  }
}

/*
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
 */
