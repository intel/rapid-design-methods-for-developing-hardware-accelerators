
package reporters.decoder

import chisel3._
import chisel3.util._

object Helper {
  def aux( indices: IndexedSeq[Bool], validOpt: Option[Bool]=None) : IndexedSeq[Bool] = {
    val k = indices.size
    val n = 1<<k
    val onehot = for ( i <- 0 until n) yield {
      val s = (0 until k) map ( j => indices(j) === (((1<<j)&i) != 0).B)
//      val s = (0 until k) map ( j => if (((1<<j)&i) != 0) indices(j) else !indices(j))
      VecInit((s ++ validOpt).toIndexedSeq).asUInt.andR
//      VecInit((s ++ validOpt).toIndexedSeq).reduce{_&&_}
    }
    onehot
  }
  def aux2( indices: IndexedSeq[Bool], validOpt: Option[Bool]=None) : IndexedSeq[Bool] = {
    val onehot = UIntToOH( VecInit(indices).asUInt).toBools.toIndexedSeq
    if (validOpt.isDefined)
      onehot map (_ && validOpt.get)
    else
      onehot
  }
  def auxNested( indices: IndexedSeq[Bool], validOpt: Option[Bool]=None) : IndexedSeq[Bool] = {
    val k = indices.size
    require( k>0)

    if ( k == 1) {
      Helper.aux( indices, validOpt)
    } else if ( !validOpt.isDefined && k == 2) {
      Helper.aux( indices, validOpt)
    } else {
      val k0 = (k+1)/2

      val i0 = indices.slice(0,k0)
      val i1 = indices.slice(k0,k)

      val s0 = Helper.auxNested( i0)
      val s1 = Helper.auxNested( i1, validOpt)

      for { u <- s1; v <- s0} yield u && v
    }
  }
  def auxTwoLevel( indices: IndexedSeq[Bool], validOpt: Option[Bool]=None) : IndexedSeq[Bool] = {
    val k = indices.size
    require( k>0)
    val k0 = (k+1)/2
    val i0 = indices.slice(0,k0)
    val i1 = indices.slice(k0,k)
    val s0 = Helper.aux( i0)
    val s1 = Helper.aux( i1, validOpt)
    for { u <- s1; v <- s0} yield u && v
  }
}

class DecoderWithValidIfc(val k : Int) extends Module {
  val n = 1<<k

  val io = IO( new Bundle {
    val idx = Input( Vec( k, Bool()))
    val valid = Input( Bool())
    val onehot = Output( Vec( n, Bool()))
  })
}


class DecoderWithValidFlat(k : Int) extends DecoderWithValidIfc(k) {
  val s = Helper.aux( io.idx, Some(io.valid))
  for ( (u,v) <- s.zip(io.onehot)) v := u
}

class DecoderWithValidNested(k : Int=3) extends DecoderWithValidIfc(k) {
  val s = Helper.auxNested( io.idx, Some(io.valid))
  for ( (u,v) <- s.zip(io.onehot)) v := u
}

class DecoderWithValidTwoLevel(k : Int=3) extends DecoderWithValidIfc(k) {
  val s = Helper.auxTwoLevel( io.idx, Some(io.valid))
  for ( (u,v) <- s.zip(io.onehot)) v := u
}

class DecoderWithValid(k:Int=3) extends DecoderWithValidNested(k)

object Driver extends App {
  chisel3.Driver.execute( args, () => new DecoderWithValid)
}

object DriverNested9 extends App {
  chisel3.Driver.execute( args, () => new DecoderWithValidNested(9))
}

object DriverFlat9 extends App {
  chisel3.Driver.execute( args, () => new DecoderWithValidFlat(9))
}

object DriverTwoLevel9 extends App {
  chisel3.Driver.execute( args, () => new DecoderWithValidTwoLevel(9))
}

object DriverNested7 extends App {
  chisel3.Driver.execute( args, () => new DecoderWithValidNested(7))
}

object DriverFlat7 extends App {
  chisel3.Driver.execute( args, () => new DecoderWithValidFlat(7))
}

object DriverTwoLevel7 extends App {
  chisel3.Driver.execute( args, () => new DecoderWithValidTwoLevel(7))
}

object DriverNested3 extends App {
  chisel3.Driver.execute( args, () => new DecoderWithValidNested(3))
}

object DriverFlat3 extends App {
  chisel3.Driver.execute( args, () => new DecoderWithValidFlat(3))
}

object DriverTwoLevel3 extends App {
  chisel3.Driver.execute( args, () => new DecoderWithValidTwoLevel(3))
}



