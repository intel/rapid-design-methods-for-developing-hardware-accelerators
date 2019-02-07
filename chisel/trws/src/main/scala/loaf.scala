package trws

import chisel3._
import chisel3.util._
import chisel3.experimental._

class Pair extends Bundle {
  val e = UInt(bitwidth.W) // reverse to correspond with C
  val a = UInt(bitwidth.W)
}

class LoafIfc extends Module {
  val io = IO(new Bundle {
    val start = Input( Bool())
    val done = Output( Bool())
    val doneLoading = Output( Bool())

    val modeLoad = Input( Bool())
    val modeCompute = Input( Bool())

    val loadIdx = Input( UInt(log2_ncontexts.W))
    val computeIdx = Input( UInt(log2_ncontexts.W))

    val off = Flipped( DecoupledIO( SSIMD()))
    val out = DecoupledIO( SSIMD())
    val slc = Flipped( DecoupledIO( new Pair))
    val lof = Flipped( DecoupledIO( new Pair))
  })
}

trait MACAbstractIfc extends BaseModule {
  val io = IO(new Bundle{
    val clock = Input( Clock())
    val en = Input( Bool())
    val a = Input( UInt(16.W))
    val b = Input( UInt(16.W))
    val o = Input( SInt(32.W))
    val r = Output( SInt(32.W))
  })
}

class MACBlackBox extends BlackBox with HasBlackBoxInline with MACAbstractIfc {
  setInline("MACBlackBox.v",
  s"""|module MACBlackBox( input clock, input en, input [15:0] a, input [15:0] b, input [31:0] o, output [31:0] r);
      |  reg [15:0] a2;
      |  reg [15:0] b2;
      |  reg [31:0] o2;
      |  wire [36:0] r5;
      |  reg [31:0] r6;
      |
      |  assign r = r6;
      |  always @(posedge clock) begin
      |    if ( en) begin
      |      a2 <= a;
      |      b2 <= b;
      |      o2 <= o;
      |      r6 <= r5[31:0];
      |    end
      |  end
      |
      |  mac u0 ( .ax({2'b0,a2}), .ay({2'b0,b2}),
      |           .bx({4'b0,o2[31:18]}), .by(o2[17:0]),
      |           .aclr( 2'b0),
      |           .clk( {clock,clock,clock}), .ena( {en,en,en}),
      |           .resulta( r5));
      |
      |endmodule""".stripMargin)
}

class MAC extends Module with MACAbstractIfc {

  val a2 = Reg( UInt(16.W))
  val a3 = Reg( UInt(16.W))
  val a4 = Reg( UInt(16.W))
  val b2 = Reg( UInt(16.W))
  val b3 = Reg( UInt(16.W))
  val b4 = Reg( UInt(16.W))
  val o2 = Reg( SInt(32.W))
  val o3 = Reg( SInt(32.W))
  val o4 = Reg( SInt(32.W))

  val r5 = Reg( SInt(32.W))
  val r6 = Reg( SInt(32.W))

  when ( io.en) {
    a2 := io.a
    a3 := a2
    a4 := a3
    b2 := io.b
    b3 := b2
    b4 := b3
    o2 := io.o
    o3 := o2
    o4 := o3
    r5 := (a4 * b4).asSInt + o4
    r6 := r5
  }

  io.r := r6
}

class Loaf extends LoafIfc {
  val n = cl_per_row*elements_per_cl
  val n_slices = n*(n-1)/2
  val ngc = cl_per_row
  val ngr = cl_per_row

  val islice = RegInit( 0.U((2*(log2_max_cl_per_row+log2_elements_per_cl)-1).W))
  val done = RegInit( false.B)

  val phase = RegInit( 0.U(1.W)) // currently only 0 and 1

  val offCLs = Reg( Vec( max_cl_per_row, SSIMD()))

  val o1 = Reg( SSIMD())
  val o2 = Reg( SSIMD())
  val mx2 = IndexedSeq.fill( elements_per_cl, elements_per_cl){ Reg( UInt( bitwidth.W))}
  val ab2 = IndexedSeq.fill( elements_per_cl, elements_per_cl){ Reg( UInt( bitwidth.W))}

  val macs = IndexedSeq.fill( elements_per_cl, elements_per_cl){ Module( new MAC/*BlackBox*/)}

  val tm8 = IndexedSeq.fill( elements_per_cl/ctree_dims(0), elements_per_cl){ Reg( SInt( bitwidth.W))}
  val tm9 = IndexedSeq.fill( elements_per_cl/ctree_dims(1), elements_per_cl){ Reg( SInt( bitwidth.W))}
  val tma = IndexedSeq.fill( elements_per_cl/ctree_dims(2), elements_per_cl){ Reg( SInt( bitwidth.W))}
  val tmb = IndexedSeq.fill( elements_per_cl/ctree_dims(3), elements_per_cl){ Reg( SInt( bitwidth.W))}
  val tmc = IndexedSeq.fill( elements_per_cl/ctree_dims(4), elements_per_cl){ Reg( SInt( bitwidth.W))}

  val best = Reg( SSIMD())
  val bestBufe = Reg( SSIMD())

  val r = RegInit( 0.U(log2_max_cl_per_row.W))
  val c = RegInit( 0.U(log2_max_cl_per_row.W))

  val validFlag1 = RegInit( false.B)
  val validFlag2 = RegInit( false.B)
  val validFlag3 = RegInit( false.B)
  val validFlag4 = RegInit( false.B)
  val validFlag5 = RegInit( false.B)
  val validFlag6 = RegInit( false.B)
  val validFlag7 = RegInit( false.B)
  val validFlag8 = RegInit( false.B)
  val validFlag9 = RegInit( false.B)
  val validFlaga = RegInit( false.B)
  val validFlagb = RegInit( false.B)
  val validFlagc = RegInit( false.B)
  val validFlagd = RegInit( false.B)
  val validFlage = RegInit( false.B)

  val sendFlag1 = RegInit( false.B)
  val sendFlag2 = RegInit( false.B)
  val sendFlag3 = RegInit( false.B)
  val sendFlag4 = RegInit( false.B)
  val sendFlag5 = RegInit( false.B)
  val sendFlag6 = RegInit( false.B)
  val sendFlag7 = RegInit( false.B)
  val sendFlag8 = RegInit( false.B)
  val sendFlag9 = RegInit( false.B)
  val sendFlaga = RegInit( false.B)
  val sendFlagb = RegInit( false.B)
  val sendFlagc = RegInit( false.B)
  val sendFlagd = RegInit( false.B)
  val sendFlage = RegInit( false.B)

  val clearFlag1 = RegInit( false.B)
  val clearFlag2 = RegInit( false.B)
  val clearFlag3 = RegInit( false.B)
  val clearFlag4 = RegInit( false.B)
  val clearFlag5 = RegInit( false.B)
  val clearFlag6 = RegInit( false.B)
  val clearFlag7 = RegInit( false.B)
  val clearFlag8 = RegInit( false.B)
  val clearFlag9 = RegInit( false.B)
  val clearFlaga = RegInit( false.B)
  val clearFlagb = RegInit( false.B)
  val clearFlagc = RegInit( false.B)
  val clearFlagd = RegInit( false.B)
  val clearFlage = RegInit( false.B)

  val BFbuf = Reg( new Pair)

  val load_i = RegInit( 0.U((log2_elements_per_cl+log2_max_cl_per_row).W))
  val load_j = RegInit( 0.U((log2_elements_per_cl+log2_max_cl_per_row).W))
  val doneLoading = RegInit( false.B)
  val lastLoad = RegInit( false.B)

  val prev_lo_i = Reg( UInt( scala.math.max(1,log2_elements_per_cl).W))
  val prev_lo_j = Reg( UInt( scala.math.max(1,log2_elements_per_cl).W))
  val prev_idx =  Reg( UInt( (2*log2_max_cl_per_row+log2_ncontexts).W))
  val slcBuf = Reg( new Pair)
  val slcValid = RegInit( false.B)

  val AE = IndexedSeq.fill( elements_per_cl, elements_per_cl){ Mem( max_cl_per_row*max_cl_per_row*ncontexts, UInt((2*bitwidth).W))}
  val AEbuf = IndexedSeq.fill( elements_per_cl, elements_per_cl){ Reg( new Pair)}

  io.doneLoading := doneLoading
  io.done := done && doneLoading

  io.slc.nodeq
  io.off.nodeq
  io.lof.nodeq

  io.out.noenq

  for{ i<-0 until elements_per_cl
       j<-0 until elements_per_cl} {
    macs(i)(j).io.en := false.B
    macs(i)(j).io.clock := clock
    macs(i)(j).io.a := 0.U
    macs(i)(j).io.b := 0.U
    macs(i)(j).io.o := 0.S
  }

//  printf( "io.start,prev_lo_i,prev_lo_j,io.modeLoad,doneLoading,done: %d,%d,%d,%d,%d,%d\n", io.start, prev_lo_i, prev_lo_j, io.modeLoad, doneLoading, done)

  when ( io.start && !doneLoading) {

    when ( io.modeLoad) {

      when ( io.slc.valid || lastLoad) {

        when ( slcValid) {
          for { i <- 0 until elements_per_cl
                j <- 0 until elements_per_cl} {
            when ( i.U === prev_lo_i && j.U === prev_lo_j) {
              AE(i)(j).write( prev_idx, slcBuf.asTypeOf(UInt((2*bitwidth).W)))
            }
          }
        }

        when ( lastLoad) {
          doneLoading := true.B
        } .otherwise {
          slcValid := true.B
          io.slc.ready := true.B
          val pair = io.slc.bits

          val lo_i = if ( log2_elements_per_cl == 0) { 0.U} else {
            load_i(log2_elements_per_cl-1,0)
          }
          val hi_i = load_i(log2_elements_per_cl+log2_max_cl_per_row-1,log2_elements_per_cl)
          val lo_j = if ( log2_elements_per_cl == 0) { 0.U} else {
            load_j(log2_elements_per_cl-1,0)
          }
          val hi_j = load_j(log2_elements_per_cl+log2_max_cl_per_row-1,log2_elements_per_cl)

          val idx = (io.loadIdx<<(2*log2_max_cl_per_row)) + ( hi_i << log2_max_cl_per_row) + hi_j


          prev_lo_i := lo_i
          prev_lo_j := lo_j
          prev_idx := idx
          slcBuf := pair

          when ( load_j =/= (n-1).U) {
            load_j := load_j + 1.U
          } .otherwise {
            load_j := 0.U
            when ( load_i =/= (n-1).U) {
              load_i := load_i + 1.U
            } .otherwise {
              load_i := 0.U
              lastLoad := true.B
            }
          }
        }
      }
    } .otherwise {
      doneLoading := true.B
    }
  }

//  printf( "io.start,done,doneLoading,io.modeCompute: %d,%d,%d,%d\n", io.start, done, doneLoading, io.modeCompute)
//  printf( "phase,c,io.off.valid,r,io.lof.valid,sendFlage,io.out.ready: %d,%d,%d,%d,%d,%d,%d\n", phase, c, io.off.valid, r, io.lof.valid, sendFlage, io.out.ready)

  val flagEn = WireInit( false.B)

  val inputsNotValid = phase === 0.U && c === 0.U && (!io.off.valid || (r === 0.U && !io.lof.valid))

  val validFlagVec = TappedShiftRegister( 14, phase === 0.U && !inputsNotValid, false.B, flagEn)
  val validFlagMap = (for { (r,idx) <- validFlagVec zipWithIndex } yield (s"validFlag${(idx+1).toHexString}",r)).toMap

  val sendFlagVec = TappedShiftRegister( 14, phase === 0.U && r === (ngr-1).U && !inputsNotValid, false.B, flagEn)
  val sendFlagMap = (for { (r,idx) <- sendFlagVec zipWithIndex } yield (s"sendFlag${(idx+1).toHexString}",r)).toMap

  val clearFlagVec = TappedShiftRegister( 12, phase === 0.U && r === 0.U && !inputsNotValid, false.B, flagEn)
  val clearFlagMap = (for { (r,idx) <- clearFlagVec zipWithIndex } yield (s"clearFlag${(idx+1).toHexString}",r)).toMap

  when ( io.start && !done) {

    when ( doneLoading && io.modeCompute) {

//
// Needs to be able to empty the pipe even if the inputs are not valid---otherwise we get deadlock
//   should stall everything if (sendFlage && !io.out.ready)
//   should not if io.off.valid or io.lof.valid is not true; just introduce false validFlags
//

//      val inputsNotValid = phase === 0.U && c === 0.U && (!io.off.valid || (r === 0.U && !io.lof.valid))
      val outputsNotReady = sendFlage && !io.out.ready


      when ( !outputsNotReady) {

        flagEn := true.B

        validFlage := validFlagd
        validFlagd := validFlagc
        validFlagc := validFlagb
        validFlagb := validFlaga
        validFlaga := validFlag9
        validFlag9 := validFlag8
        validFlag8 := validFlag7
        validFlag7 := validFlag6
        validFlag6 := validFlag5
        validFlag5 := validFlag4
        validFlag4 := validFlag3
        validFlag3 := validFlag2
        validFlag2 := validFlag1
        validFlag1 := phase === 0.U && !inputsNotValid


        sendFlage := sendFlagd
        sendFlagd := sendFlagc
        sendFlagc := sendFlagb
        sendFlagb := sendFlaga
        sendFlaga := sendFlag9
        sendFlag9 := sendFlag8
        sendFlag8 := sendFlag7
        sendFlag7 := sendFlag6
        sendFlag6 := sendFlag5
        sendFlag5 := sendFlag4
        sendFlag4 := sendFlag3
        sendFlag3 := sendFlag2
        sendFlag2 := sendFlag1
        sendFlag1 := phase === 0.U && r === (ngr-1).U && !inputsNotValid

        clearFlage := clearFlagd
        clearFlagd := clearFlagc
        clearFlagc := clearFlagb
        clearFlagb := clearFlaga
        clearFlaga := clearFlag9
        clearFlag9 := clearFlag8
        clearFlag8 := clearFlag7
        clearFlag7 := clearFlag6
        clearFlag6 := clearFlag5
        clearFlag5 := clearFlag4
        clearFlag4 := clearFlag3
        clearFlag3 := clearFlag2
        clearFlag2 := clearFlag1
        clearFlag1 := phase === 0.U && r === 0.U && !inputsNotValid

        when ( sendFlagMap("sendFlage")) {
          io.out.bits := bestBufe
          io.out.valid := true.B
        }

        when ( phase === 0.U && !inputsNotValid) {
          when( c === 0.U) {
            io.off.ready := true.B
            offCLs(r) := io.off.bits
            o1 := io.off.bits // bypass
          } .otherwise {
            o1 := offCLs(r)
          }
        }

        when ( phase === 0.U && c === 0.U && r === 0.U && !inputsNotValid) {
          io.lof.ready := true.B
          BFbuf := io.lof.bits
        }

        when ( phase === 0.U && !inputsNotValid) {
          for { j <- 0 until elements_per_cl} {
            for { i <- 0 until elements_per_cl} yield { 
              val idx = (io.computeIdx<<(2*log2_max_cl_per_row)) + ( r << log2_max_cl_per_row) + c
              AEbuf(i)(j) := AE(i)(j).read( idx).asTypeOf(new Pair)
            }
          }
        }

        o2 := o1

        for { j <- 0 until elements_per_cl} {

          val elements = for { i <- 0 until elements_per_cl} yield {
            val pair = AEbuf(i)(j)
            val a = pair.a
            val b = BFbuf.a
            val e = pair.e
            val f = BFbuf.e
            val mx = WireInit( e)
            when ( f > e) { mx := f }
            mx2(i)(j) := mx
            val ab = WireInit( a-b)
            when ( a<b) { ab := b-a }
            ab2(i)(j) := ab
            val mn = WireInit( ab2(i)(j))
            when ( ab2(i)(j)>204.U) { mn := 204.U}

            macs(i)(j).io.en := true.B

            macs(i)(j).io.o := o2(i) << radixPoint
            macs(i)(j).io.a := mx2(i)(j)
            macs(i)(j).io.b := mn

/*
            printf( "o,o',a,b,r,r': %d,%d,%d,%d,%d,%d\n",
              o2(i),
              o2(i) << radixPoint,
              macs(i)(j).io.a,
              macs(i)(j).io.b,
              macs(i)(j).io.r,
              macs(i)(j).io.r >> radixPoint)
 */

            macs(i)(j).io.r >> radixPoint
          }

          def vMin( x : SInt, y : SInt) : SInt = {
            val w = WireInit( y)
            when ( x < y) {
              w := x
            }
            w
          }

          for ( ii<-0 until elements_per_cl/ctree_dims(0)) {
            val d = tree_dims(0)
            if ( d == 4) {
              tm8(ii)(j) := vMin( vMin( elements(d*ii+0), elements(d*ii+1)),
                                  vMin( elements(d*ii+2), elements(d*ii+3)))
            } else if ( d == 2) {
              tm8(ii)(j) := vMin( elements(d*ii+0), elements(d*ii+1))
            } else if ( d == 1) {
              tm8(ii)(j) := elements(d*ii+0)
            } else {
              assert( false)
            }
          }

          for ( ii<-0 until elements_per_cl/ctree_dims(1)) {
            val d = tree_dims(1)
            if ( d == 4) {
              tm9(ii)(j) := vMin( vMin( tm8(d*ii+0)(j), tm8(d*ii+1)(j)),
                                  vMin( tm8(d*ii+2)(j), tm8(d*ii+3)(j)))
            } else if ( d == 2) {
              tm9(ii)(j) := vMin( tm8(d*ii+0)(j), tm8(d*ii+1)(j))
            } else if ( d == 1) {
              tm9(ii)(j) := tm8(d*ii+0)(j)
            } else {
              assert( false)
            }
          }

          for ( ii<-0 until elements_per_cl/ctree_dims(2)) {
            val d = tree_dims(2)
            if ( d == 4) {
              tma(ii)(j) := vMin( vMin( tm9(d*ii+0)(j), tm9(d*ii+1)(j)),
                                  vMin( tm9(d*ii+2)(j), tm9(d*ii+3)(j)))
            } else if ( d == 2) {
              tma(ii)(j) := vMin( tm9(d*ii+0)(j), tm9(d*ii+1)(j))
            } else if ( d == 1) {
              tma(ii)(j) := tm9(d*ii+0)(j)
            } else {
              assert( false)
            }
          }

          for ( ii<-0 until elements_per_cl/ctree_dims(3)) {
            val d = tree_dims(3)
            if ( d == 4) {
              tmb(ii)(j) := vMin( vMin( tma(d*ii+0)(j), tma(d*ii+1)(j)),
                                  vMin( tma(d*ii+2)(j), tma(d*ii+3)(j)))
            } else if ( d == 2) {
              tmb(ii)(j) := vMin( tma(d*ii+0)(j), tma(d*ii+1)(j))
            } else if ( d == 1) {
              tmb(ii)(j) := tma(d*ii+0)(j)
            } else {
              assert( false)
            }
          }

          for ( ii<-0 until elements_per_cl/ctree_dims(4)) {
            val d = tree_dims(4)
            if ( d == 4) {
              tmc(ii)(j) := vMin( vMin( tmb(d*ii+0)(j), tmb(d*ii+1)(j)),
                                  vMin( tmb(d*ii+2)(j), tmb(d*ii+3)(j)))
            } else if ( d == 2) {
              tmc(ii)(j) := vMin( tmb(d*ii+0)(j), tmb(d*ii+1)(j))
            } else if ( d == 1) {
              tmc(ii)(j) := tmb(d*ii+0)(j)
            } else {
              assert( false)
            }
          }

          val d = tree_dims(5)
          val cand = Wire( SInt( bitwidth.W))

          if ( d == 4) {
            cand := vMin( vMin( tmc(0)(j), tmc(1)(j)),
                          vMin( tmc(2)(j), tmc(3)(j)))
          } else if ( d == 2) {
            cand := vMin( tmc(0)(j), tmc(1)(j))
          } else if ( d == 1) {
            cand := tmc(0)(j)
          } else {
            assert( false)
          }

          when( clearFlagMap("clearFlagc") || cand < best(j)) {
            best(j) := cand
          }

        }

        bestBufe := best

        when ( phase === 0.U && !inputsNotValid) {
          when ( r =/= (ngr-1).U) {
            r := r + 1.U
          } .otherwise {
            r := 0.U
            when ( c =/= (ngc-1).U) {
              c := c + 1.U
            } .otherwise {
              c := 0.U
              if ( n_slices == 0) {
                islice := 0.U
                phase := 1.U
              } else {
                when ( islice =/= (n_slices-1).U) {
                  islice := islice + 1.U
                } .otherwise {
//		  printf( "Setting phase to 1\n")
                  islice := 0.U
                  phase := 1.U
                }
              }
            }
          }
        }

	when ( phase === 1.U) {
//	  printf( "Evaluating validFlags...\n")

	  val f = validFlagVec.foldLeft(true.B){ case (x,y) => x && !y}

/*
          when ( !validFlag1 &&
            !validFlag2 &&
            !validFlag3 &&
            !validFlag4 &&
            !validFlag5 &&
            !validFlag6 &&
            !validFlag7 &&
            !validFlag8 &&
            !validFlag9 &&
            !validFlaga &&
            !validFlagb &&
            !validFlagc &&
            !validFlagd &&
            !validFlage) {
            done := true.B
          }
*/
	  when (f) {
            done := true.B
	  }
        }

      }
    } .elsewhen ( doneLoading) {
      done := true.B
    }
  }

}


object LoafDriver extends App {
  Driver.execute( args, () => new Loaf)
}

