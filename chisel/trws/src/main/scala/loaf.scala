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

  val validFlagTSR = new TappedShiftRegister2( 14, Bool(), false.B)
  val sendFlagTSR = new TappedShiftRegister2( 14, Bool(), false.B)
  val clearFlagTSR = new TappedShiftRegister2( 12, Bool(), false.B)

  when ( io.start && !done) {

    when ( doneLoading && io.modeCompute) {

//
// Needs to be able to empty the pipe even if the inputs are not valid---otherwise we get deadlock
//   should stall everything if (sendFlage && !io.out.ready)
//   should not if io.off.valid or io.lof.valid is not true; just introduce false validFlags
//

      val inputsNotValid = phase === 0.U && c === 0.U && (!io.off.valid || (r === 0.U && !io.lof.valid))

      when ( !sendFlagTSR.last || io.out.ready) {

	validFlagTSR.shift( phase === 0.U && !inputsNotValid)
	sendFlagTSR.shift( phase === 0.U && r === (ngr-1).U && !inputsNotValid)
	clearFlagTSR.shift( phase === 0.U && r === 0.U && !inputsNotValid)

        when ( sendFlagTSR.last) {
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


	  def stage( ctree_dim : Int,
	             tree_dim : Int,
		     lhs : (Int) => SInt,
		     rhs : (Int) => SInt) {

            def vMin( x : SInt, y : SInt) : SInt = {
              val w = WireInit( y)
              when ( x < y) { w := x}
              w
            }

            for ( ii<-0 until elements_per_cl/ctree_dim) {
              val d = tree_dim
              if ( d == 4) {
                lhs(ii) := vMin( vMin( rhs(d*ii+0), rhs(d*ii+1)),
                                 vMin( rhs(d*ii+2), rhs(d*ii+3)))
              } else if ( d == 2) {
                lhs(ii) := vMin( rhs(d*ii+0), rhs(d*ii+1))
              } else if ( d == 1) {
                lhs(ii) := rhs(d*ii+0)
              } else {
                assert( false)
              }
            }
          }

	  stage( ctree_dims(0), tree_dims(0), (i:Int) => tm8(i)(j), (i:Int) => elements(i))
	  stage( ctree_dims(1), tree_dims(1), (i:Int) => tm9(i)(j), (i:Int) => tm8(i)(j))
	  stage( ctree_dims(2), tree_dims(2), (i:Int) => tma(i)(j), (i:Int) => tm9(i)(j))
	  stage( ctree_dims(3), tree_dims(3), (i:Int) => tmb(i)(j), (i:Int) => tma(i)(j))
	  stage( ctree_dims(4), tree_dims(4), (i:Int) => tmc(i)(j), (i:Int) => tmb(i)(j))
          val cand = Wire( SInt( bitwidth.W))
	  stage( ctree_dims(5), tree_dims(5), (i:Int) => cand, (i:Int) => tmc(i)(j))

          when( clearFlagTSR.last || cand < best(j)) {
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
                  islice := 0.U
                  phase := 1.U
                }
              }
            }
          }
        }

	when ( phase === 1.U) {
	  // Don't include the first one (non-delayed input)
	  when ( validFlagTSR().tail.foldLeft(true.B){ case (x,y) => x && !y}) {
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

