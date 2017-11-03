// See LICENSE for license details.

package priorityencoder

import chisel3._

class CarryBlockIfc extends Module {
  val io = IO( new Bundle {
    val a =  Input(Bool())
    val ci = Input(Bool())
    val co = Output(Bool())
  })
}

class ORBlock extends CarryBlockIfc {
  io.co := io.a | io.ci
}

class CarryChainIfc(n:Int) extends Module {
  val io = IO(new Bundle {
    val a  = Input( Vec( n, Bool()))
    val co = Output(Vec( n, Bool()))
  })
}

// Decided to drop the generic spec "[ +T <: CarryBlockIfc ]"

class SimpleChainImperative(N:Int, factory:() => CarryBlockIfc) extends CarryChainIfc(N) {
   require( io.a.length == io.co.length)
   require( io.a.length == N)
// Scala vars aren't always evil. This one is a loop carry over.
   var carry = Bool()
   for( ((a,co),idx)<-io.a.zip( io.co).zipWithIndex) {
      if ( idx == 0) {
         co := a
      } else {
         val b = Module( factory())
         b.io.a := a
         b.io.ci := carry
         co := b.io.co
      }
      carry = co
   }
}

class SimpleChainImperative2(N:Int,factory:() => CarryBlockIfc) extends CarryChainIfc(N) {
  require( io.a.length == io.co.length)
  require( io.a.length == N)
// Scala vars aren't always evil. This one is a loop carry over.
  val q = io.a.zip( io.co)

  q.head._2 := q.head._1
  var carry = q.head._2
  for( (a,co)<-q.tail) {
    val b = Module( factory())
    b.io.a := a
    b.io.ci := carry
    co := b.io.co
    carry = co
  }
}

class SimpleChainFoldLeft(N:Int, factory:() => CarryBlockIfc) extends CarryChainIfc(N) {
  println( s"SimpleChainFoldLeft")

  require( io.a.length == io.co.length)
  require( io.a.length == N)
  val q = io.a.zip( io.co)

  q.head._2 := q.head._1

  q.tail.foldLeft( q.head._2) { case (carryIn,(a,co)) =>
    val b = Module( factory())
    b.io.a := a
    b.io.ci := carryIn
    co := b.io.co
    co
  }
}

class SimpleChainFunctional(N:Int, factory:() => CarryBlockIfc) extends CarryChainIfc(N) {
   require( io.a.length == io.co.length)
   require( io.a.length == N)
   println(s"SimpleChainFunctional: ${io.a.length} $N")
   io.co(0) := io.a(0)
   for( ((a,ci),co)<-io.a.toList.tail.zip( io.co).zip( io.co.tail)) {
      val b = Module( factory())
      b.io.a := a
      b.io.ci := ci
      co := b.io.co
   }
}


class KoggeStone(N:Int, factory:() => CarryBlockIfc) extends CarryChainIfc(N) {
   require( io.a.length == io.co.length)

   val levels = util.log2Ceil( N)

   val internal = Wire( Vec( levels+1, Vec( N, Bool())))

   internal(0) := io.a
   io.co := internal(levels) 

   for( j<-0 until levels) {
      val skip = 1<<j
      for( i<-0 until N) {
         if ( i >= skip) {
	    val other = i - skip
            val b = Module( factory())
            b.io.a := internal(j)(i)
            b.io.ci := internal(j)(other)
            internal(j+1)(i) := b.io.co
	    //println( s"$i <- $other")
         } else {
            internal(j+1)(i) := internal(j)(i)
         }
      }
/*

      for( ((a,ci),co)<-internal(j).zip( List.tabulate(1<<j)(_ => UInt(1)) ::: internal(j).toList).zip( internal(j+1))) {
         val b = Module( factory())
         b.io.a := a
         b.io.ci := ci
         co := b.io.co
      }
*/
   }
}

class LadnerFischer(N:Int, factory:() => CarryBlockIfc) extends CarryChainIfc(N) {
   require( io.a.length == io.co.length)

   val levels = util.log2Ceil( N)

   val internal = Wire( Vec( levels+1, Vec( N, Bool())))

   internal(0) := io.a
   io.co := internal(levels) 

   for( j<-0 until levels) {
/*
 N=10
   9 <- 8, 7 <- 6, 5 <- 4, 3 <- 2, 1 <- 0
           7 <- 5, 6 <- 5, 3 <- 1, 2 <- 1
           7 <- 3, 6 <- 3, 5 <- 3, 4 <- 3
   8 <- 7
 N=9
   7 <- 6, 5 <- 4, 3 <- 2, 1 <- 0
   7 <- 5, 6 <- 5, 3 <- 1, 2 <- 1
   7 <- 3, 6 <- 3, 5 <- 3, 4 <- 3
   8 <- 7
 N=8
   7 <- 6, 5 <- 4, 3 <- 2, 1 <- 0
   7 <- 5, 6 <- 5, 3 <- 1, 2 <- 1
   7 <- 3, 6 <- 3, 5 <- 3, 4 <- 3
=====
skip=4, 5,4=>3, 7,6=>5

*/
      val skip = 1<<j
      val hasBlock = (for( i <- 0 until N if ((i % (2*skip)) >= skip)) yield i).toSet
      for( i<-0 until N) {
         if ( hasBlock contains i) {
	    val other = (i / skip)*skip-1
            val b = Module( factory())
            b.io.a := internal(j)(i)
            b.io.ci := internal(j)(other)
            internal(j+1)(i) := b.io.co
	    //println( s"$i <- $other")
         } else {
            internal(j+1)(i) := internal(j)(i)
         }
      }
   }
}

class BrentKung(N:Int, factory:() => CarryBlockIfc) extends CarryChainIfc(N) {
   require( io.a.length == io.co.length)

   val flevels = util.log2Floor( N)
   val blevels = util.log2Ceil( N)-1

   val internal = Wire( Vec( flevels+blevels+1, Vec( N, Bool())))

   internal(0) := io.a
   io.co := internal(flevels+blevels)

   for( j<-0 until flevels) { // compress
/*
 N=16
    15 <- 14, 13 <- 12, 11 <- 10, 9 <- 8, 7 <- 6, 5 <- 4, 3 <- 2, 1 <- 0
    15 <- 13, 11 <- 9, 7 <- 5, 3 <- 1
    15 <- 11, 7 <- 3
    15 <- 7
 N=9
    7 <- 6, 5 <- 4, 3 <- 2, 1 <- 0
    7 <- 5, 3 <- 1
    7 <- 3
 N=8
    7 <- 6, 5 <- 4, 3 <- 2, 1 <- 0
    7 <- 5, 3 <- 1
    7 <- 3
 N=8
    3 <- 2, 1 <- 0
    3 <- 1
 */
      val skip = 1<<(j+1)
      val hasBlock = ((skip-1) until N by skip).toSet
      for( i<-0 until N) {
         if ( hasBlock contains i) {
            val b = Module( factory())
            b.io.a := internal(j)(i)
            b.io.ci := internal(j)(i-skip/2)
            internal(j+1)(i) := b.io.co
	    //println( s"frontend: $i <- ${i-skip/2}")
         } else {
            internal(j+1)(i) := internal(j)(i)
         }
      }
   }
   for( j<-flevels until flevels+blevels) { // expand
/*
 N=16
    11 <- 7                                                     : (8 until 16 by 8) [+ 4] - 1
    13 <- 11, 9 <- 7, 5 <- 3                                    : (4 until 16 by 4) [+ 2] - 1
    14 <- 13, 12 <- 11, 10 <- 9, 8 <- 8, 6 <- 5, 4 <- 3, 2 <- 1 : (2 until 16 by 2) [+ 1] - 1
 N=9
    (11 <- 7)                      : (8 until 9 by 8) [+ 4] - 1
    (9 <- 7), 5 <- 3               : (4 until 9 by 4) [+ 2] - 1
    8 <- 7, 6 <- 5, 4 <- 3, 2 <- 1 : (2 until 9 by 2) [+ 1] - 1
 N=8
    5 <- 3                 : (4 until 8 by 4) [+ 2] - 1
    6 <- 5, 4 <- 3, 2 <- 1 : (2 until 8 by 2) [+ 1] - 1
 N=4
    2 <- 1                 : (2 until 4 by 2) [+ 1] - 1
*/
      val skip = 1<<(flevels+blevels-j)
      val hasBlock = ((skip+skip/2-1) until N by skip).toSet
      for( i<-0 until N) {
         if ( hasBlock contains i) {
            val b = Module( factory())
            b.io.a := internal(j)(i)
            b.io.ci := internal(j)(i-skip/2)
	    println( s"backend: $skip $i <- ${i-skip/2}")
            internal(j+1)(i) := b.io.co
         } else {
            internal(j+1)(i) := internal(j)(i)
         }
      }
   }
}


object orTree {
  def apply[T <: Data]( a : IndexedSeq[T]) : T = {
    if ( a.length == 0) {
      throw new Exception("Don't use orTree with an empty sequence")
    } else if ( a.length == 1) {
      a(0)
    } else {
      (orTree( a.slice( 0, a.length/2)).asUInt | orTree( a.slice( a.length/2, a.length)).asUInt).asTypeOf( a(0))
    }
  }
}

object oneHotMux {
  def apply[T<:Data] (a: IndexedSeq[T], ohSel: IndexedSeq[Bool]) : T = {
    require(a.length == ohSel.length)
    val N = a.length
    val gen: T = a(0).cloneType
    val vBits = Wire( Vec(N, gen) )
  
    for { idx <- 0 until N} {
      vBits(idx) := 0.U.asTypeOf(gen)
      when ( ohSel(idx)) {
        vBits(idx) := a(idx)
      }
    }
    orTree(vBits)
  }
}

class FastPriorityEncoder( n : Int) extends Module {

  val io = IO(new Bundle {
    val a = Input( Vec( n, Bool()))
    val v = Output( Vec( n, Bool()))
    val hasValid = Output( Bool())
    val z = Output( (n-1).U.cloneType)
  })

// Try others
  val m = Module( new KoggeStone(n,{()=>new ORBlock}))
  m.io.a := io.a

  io.hasValid := m.io.co(n-1)

  val s = Wire( Vec( n, (n-1).U.cloneType))

  val bbb = ((((Seq(false.B) ++ m.io.co.toSeq) zip m.io.co.toSeq) zip s.toSeq) zip io.v).zipWithIndex

  bbb.foreach {
    case ((((q,p),o),v),idx) =>
      v := ~q & p
      o := 0.U
      when( v) { o := idx.U}
  }

  io.z := orTree( s.toIndexedSeq)

}

class Encoder( n: Int) extends Module {
  val io = IO(new Bundle {
    val a = Input( Vec( n, Bool()))
    val z = Output( (n-1).U.cloneType)
  })

  val s = Wire( Vec( n, (n-1).U.cloneType))

  val bbb = (io.a.toSeq zip s.toSeq).zipWithIndex

  bbb.foreach {
    case ((a,o),idx) =>
      o := 0.U
      when( a) { o := idx.U}
  }

  io.z := orTree( s.toIndexedSeq)

}


object FastPriorityEncoder {
  def apply( a : IndexedSeq[Bool]) : (UInt,Vec[Bool],Bool) = {
    val m = Module( new FastPriorityEncoder( a.length))
    m.io.a := VecInit( a)
    ( m.io.z, m.io.v, m.io.hasValid)
  }
}

object Encoder {
  def apply( a : IndexedSeq[Bool]) : (UInt) = {
    val m = Module( new Encoder( a.length))
    m.io.a := VecInit( a)
    m.io.z
  }
}


object FastPriorityEncoder_gen extends App {
  chisel3.Driver.execute(args, () => new FastPriorityEncoder( 256))
}

