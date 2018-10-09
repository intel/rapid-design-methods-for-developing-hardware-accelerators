package trws

import chisel3._
import chisel3.util._

import designutils.{ DecoupledStage, MooreStage}

class SplitOut extends Module {
  val io = IO(new Bundle {
    val start = Input( Bool())

    val out = Flipped( DecoupledIO( SSIMD()))
    val out0 = DecoupledIO( SSIMD())
    val out1 = DecoupledIO( SSIMD())
  })

  io.out.nodeq
  io.out0.noenq
  io.out1.noenq

  when ( io.start) {
//    printf( "io.out.valid,io.out0.ready,io.out1.ready: %d,%d,%d\n", io.out.valid, io.out0.ready, io.out1.ready)
    when ( io.out.valid && io.out0.ready && io.out1.ready) {
      io.out.ready := true.B
      io.out0.valid := true.B
      io.out1.valid := true.B
      io.out0.bits := io.out.bits
      io.out1.bits := io.out.bits
    }
  }

}

class ComputeBest extends Module {
  val io = IO(new Bundle {
    val start = Input( Bool())

    val out0 = Flipped( DecoupledIO( SSIMD()))
    val obest = DecoupledIO( SInt(bitwidth.W))
  })

  val jj = RegInit( 0.U(log2_max_cl_per_row.W))
  val best = Reg( SInt(bitwidth.W))

  val ((c0,s0),(c1,s1)) =
    if ( elements_per_cl == 32) {
      ((2,2),(4,2))
    } else if ( elements_per_cl == 16) {
      ((1,1),(3,2))
    } else if ( elements_per_cl == 8) {
      ((1,1),(2,1))
    } else if ( elements_per_cl == 4) {
      ((1,1),(2,1))
    } else if ( elements_per_cl == 2) {
      ((1,1),(1,0))
    } else if ( elements_per_cl == 1) {
      ((0,0),(0,0))
    } else {
      throw new Exception( s"Bad elements_per_cl (${elements_per_cl}) for Clamper implementation")
    }


  val validPipe0 = RegInit( false.B)
  val prev_jj0 = RegInit( 0.U(log2_max_cl_per_row.W))
  val m0 = Reg( Vec( elements_per_cl>>s0, SInt(bitwidth.W)))

  val validPipe1 = RegInit( false.B)
  val prev_jj1 = RegInit( 0.U(log2_max_cl_per_row.W))
  val m1 = Reg( Vec( elements_per_cl>>s1, SInt(bitwidth.W)))

  def sMin( a : SInt, b : SInt, force : Bool = false.B) : SInt = {
    val r = Wire( SInt())
    when( force || a < b) {
      r := a
    } .otherwise {
      r := b
    }
    r
  }

  io.out0.nodeq
  io.obest.noenq

  when ( io.start) {

//    printf("validPipe0,validPipe1,prev_jj1,prev_jj0,io.obest.ready,io.out0.valid: %d,%d,%d,%d,%d,%d\n",validPipe0,validPipe1,prev_jj1,prev_jj0,io.obest.ready,io.out0.valid)

    val combValidPipe1 = WireInit( validPipe1)

    when ( validPipe1 && ( prev_jj1 =/= (cl_per_row-1).U || io.obest.ready)) {
      val wbest =
        if ( elements_per_cl>>c1 == 1) {
          sMin( m1(0), best, prev_jj1 === 0.U)
        } else if ( elements_per_cl>>c1 == 2) {
          sMin( sMin( m1(0), m1(1)), best, prev_jj1 === 0.U)
        } else {
          throw new Exception( s"Bad elements_per_cl (${elements_per_cl} for Clamper implementation")
        }
      best := wbest
      when ( prev_jj1 === (cl_per_row-1).U) {
        io.obest.valid := true.B
        io.obest.bits := wbest
      }
      combValidPipe1 := false.B
      validPipe1 := false.B
    }

    val combValidPipe0 = WireInit( validPipe0)

    when ( validPipe0 && !combValidPipe1) {
      for ( k<-0 until (elements_per_cl>>c1)) {
        if ( s1 == 2) {
          m1(k) := sMin( sMin( m0(4*k+0), m0(4*k+1)),
                         sMin( m0(4*k+2), m0(4*k+3)))
        } else if ( s1 == 1) {
          m1(k) := sMin( m0(2*k+0), m0(2*k+1))
        } else if ( s1 == 0) {
          m1(k) := m0(1*k+0)
        } else {
          throw new Exception( s"Bad s1 (${s1} for Clamper implementation")
        }
      }
      prev_jj1 := prev_jj0
      combValidPipe0 := false.B
      validPipe0 := false.B
      validPipe1 := true.B
    }

    when ( io.out0.valid && !combValidPipe0) {
      io.out0.ready := true.B
      val cl = io.out0.bits
      for ( k<-0 until (elements_per_cl>>c0)) {
        if ( s0 == 2) {
          m0(k) := sMin( sMin( cl(4*k+0), cl(4*k+1)),
                         sMin( cl(4*k+2), cl(4*k+3)))
        } else if ( s0 == 1) {
          m0(k) := sMin( cl(2*k+0), cl(2*k+1))
        } else if ( s0 == 0) {
          m0(k) := cl(1*k+0)
        } else {
          throw new Exception( s"Bad s0 (${s0} for Clamper implementation")
        }
      }
      prev_jj0 := jj
      when ( jj =/= (cl_per_row-1).U) {
        jj := jj + 1.U
      } .otherwise {
        jj := 0.U
      }
      validPipe0 := true.B
    }

  }

}

class SubtractOut extends Module {
  val io = IO(new Bundle {
    val start = Input( Bool())

    val out1 = Flipped( DecoupledIO( SSIMD()))
    val obest = Flipped( DecoupledIO( SInt(bitwidth.W)))
    val res = DecoupledIO( USIMD())
  })

  val jj = RegInit( 0.U(log2_max_cl_per_row.W))

  val best = Reg( SInt(bitwidth.W))

  io.out1.nodeq
  io.obest.nodeq

  io.res.noenq

  when( io.start) {
//    printf( "jj,io.obest.valid,io.out1.valid,io.res.ready: %d,%d,%d,%d\n",jj,io.obest.valid,io.out1.valid,io.res.ready)

    when ( ( jj =/= 0.U || io.obest.valid) && io.out1.valid && io.res.ready) {
      val wbest = WireInit( best)
      when( jj === 0.U) {
        io.obest.ready := true.B
        wbest := io.obest.bits
      }
      best := wbest
      io.out1.ready := true.B
      io.res.valid := true.B
      for( j<-0 until elements_per_cl) {
        io.res.bits(j) := (io.out1.bits(j) - wbest).asUInt
      }
      when ( jj =/= (cl_per_row-1).U) {
        jj := jj + 1.U
      } .otherwise {
        jj := 0.U
      }
    }
  }

}

class Clamper extends Module {

  val io = IO(new Bundle {
    val start = Input( Bool())

    val out = Flipped( DecoupledIO( SSIMD()))
    val res = DecoupledIO( USIMD())
  })

  val splitOut = Module( new SplitOut)
  val computeBest = Module( new ComputeBest)
  val subtractOut = Module( new SubtractOut)

  splitOut.io.out <> io.out
  splitOut.io.start := io.start

  computeBest.io.out0 <> DecoupledStage( splitOut.io.out0)
  computeBest.io.start := io.start

  subtractOut.io.obest <> DecoupledStage( computeBest.io.obest)
  subtractOut.io.out1 <> Queue( splitOut.io.out1, max_cl_per_row, pipe=true)
  subtractOut.io.start := io.start

  io.res <> DecoupledStage( subtractOut.io.res)

}

object ClamperDriver extends App {
  Driver.execute( args, () => new Clamper)
}
