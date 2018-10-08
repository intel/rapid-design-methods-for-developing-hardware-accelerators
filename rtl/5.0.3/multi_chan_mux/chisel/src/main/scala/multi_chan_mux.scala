package MemArbiter

import chisel3._
import chisel3.iotesters._
import chisel3.util._
import chisel3.Driver
import java.io.File
import sys.process._


class DecoupledStage[T <: Data](gen: T) extends Module {
  val io = IO(new Bundle { 
    val inp = Flipped(Decoupled( gen ))
    val out = Decoupled( gen )
  })

  val out_valid = Reg( init=Bool(false) ) 
  val out_bits = Reg( gen )

  io.out.valid := out_valid
  io.out.bits  := out_bits

  io.inp.ready := io.out.ready | ~io.out.valid

  out_valid := io.inp.valid | ~io.inp.ready
  
  //printf("io.inp.valid=%d io.inp.ready=%d\n", io.inp.valid, io.inp.ready)
  
  when ( io.inp.ready) {
     out_bits := io.inp.bits
  }
}

class DecoupledStageMoore[T <: Data](gen: T) extends Module {
  val io = IO(new Bundle { 
    val inp = Flipped(Decoupled( gen ))
    val out = Decoupled( gen )
  })

  val data_aux = Reg(gen)
  val out_bits = Reg(gen)

  val out_valid = Reg( init=Bool(false)) 
  val inp_ready = Reg( init=Bool(true)) 

  io.inp.ready := inp_ready
  io.out.valid := out_valid
  io.out.bits  := out_bits

  when        (  inp_ready && !out_valid) {
      when ( io.inp.valid) {
         inp_ready := Bool(true); out_valid := Bool(true);
         out_bits := io.inp.bits;
      }
  } .elsewhen (  inp_ready &&  out_valid) {
      when        ( !io.inp.valid && io.out.ready) {
         inp_ready := Bool(true); out_valid := Bool(false);
      } .elsewhen (  io.inp.valid && io.out.ready) {
         inp_ready := Bool(true); out_valid := Bool(true);
	 out_bits := io.inp.bits;
      } .elsewhen (  io.inp.valid && io.out.ready) {
         inp_ready := Bool(false); out_valid := Bool(true);
         data_aux := io.inp.bits;
      }
  } .elsewhen ( !inp_ready && out_valid) {
      when ( io.out.ready) {
         inp_ready := Bool(true); out_valid := Bool(true);
	 out_bits := data_aux;
      }
  }
}

class SteerReged[T<:UInt]( gen : T, ways: Int, steerBit : Int) extends Module {
  val io = IO(new Bundle {
    val qin  = Flipped(Decoupled( gen ))
    val qout = Vec(ways, Decoupled( gen ))
  })
  
  val stage_out_N = Array.fill(ways)(Module(new DecoupledStage( gen )))
  //val out_stage_inps = stage_out_N.map(_.io.inp)
  val out_stage_inps = Vec(stage_out_N.map(_.io.inp))
  //val out_stage_inps = Vec(ways, stage_out_N(0).io.inp)
  for (i <- 0 until ways) {
      stage_out_N(i).io.inp  <> out_stage_inps(i)
  }
  
  
  printf("out_stage_inps = %d, stage_out_N(0).io.inp.valid = %d\n", out_stage_inps(0).valid, stage_out_N(0).io.inp.valid)
  val out_stage_outs = stage_out_N.map(_.io.out)
  io.qin.nodeq()

  for (out_stage <- out_stage_inps) {
    out_stage.noenq()
  }
  
  //assert(isPow2(ways))
  val pendData = Reg( gen )
  val pendValid = Reg(init=Bool(false))

  val pendAuId = pendData(steerBit+log2Up(ways)-1, steerBit)

  //printf("In data %d\n", pendData)
  //printf("Pending valid %d, auid=%d\n", pendValid, pendAuId)

  when (pendValid && out_stage_inps(pendAuId).ready) {
    out_stage_inps(pendAuId).enq(pendData)
  }
    
  when (pendValid && out_stage_inps(pendAuId).ready || !pendValid) {
    pendValid := io.qin.valid
    val inpData = Wire(init = io.qin.deq())
    when (io.qin.valid) {
      pendData := inpData
    }
  }
  
  for ((out_stage, qout) <- out_stage_outs.zip(io.qout)) {
    out_stage <> qout
  }
}

class SteerRegedSplit [T<:UInt]( gen : UInt, steerWays : Int, steerBit : Int, splitWays : Int) extends Module { 
    val io = IO(new Bundle {
    val qin  = Flipped(Decoupled( gen ))
    val qout = Vec(steerWays, Decoupled( gen ))
  })
  assert(splitWays > 1)
  val tot_width = gen.getWidth 
  val steer_header_width = tot_width - steerBit

  println ("tot_width - steer_header_width = " + (tot_width - steer_header_width))
  //assert((tot_width - steer_header_width)%splitWays == 0)
  
  val split_width = (tot_width - steer_header_width)/splitWays + steer_header_width
  val split_width_pad = (tot_width - steer_header_width)%splitWays
  val split_width_last = split_width + split_width_pad
  
  assert(split_width - steer_header_width > 0)
  // make all split component of biggest size split_width_last
  val steer_submods = Array.fill(splitWays)(Module(new SteerReged( UInt(width = split_width_last), ways = steerWays, steerBit = split_width_last - steer_header_width )))
  val steer_submod_ins = steer_submods.map(_.io.qin).toList
  val steer_submod_outs = steer_submods.map(_.io.qout).toList
  
  // connect split SteerReged inputs with primary inputs
  println("split_width_last = "+split_width_last + " split_width = " +split_width)
  for (partition <- (0 until splitWays)) {
    val part_of_data = Wire(init=io.qin.bits((partition + 1) * (split_width - steer_header_width ) - 1 + split_width_pad, 
                                                      (partition * (split_width - steer_header_width))))
                                                      
    //println ("msb = " + ((partition + 1) * (split_width - steer_header_width ) - 1 + split_width_pad) + " " + 
    //                                                  (partition * (split_width - steer_header_width)))
                                                      
    //printf ("dv = %d, partition = %d, part of data = %d from in = %d\n ", io.qin.valid, UInt(partition), part_of_data, io.qin.bits) 
    steer_submod_ins(partition).valid := io.qin.valid
    steer_submod_ins(partition).bits := ((io.qin.bits(tot_width - 1, tot_width - steer_header_width)) ##   //header
                                          part_of_data)  // part of data
  }
  io.qin.ready := steer_submod_ins(0).ready
  //printf ("in valid, ready, data = %d, %d, %d\n", io.qin.valid, io.qin.ready, io.qin.bits)  //io.qout.zipWithIndex.map {case(vec, i) => printf ("%d valid, ready, data %d, %d, %d\n", UInt(i), io.qout(i).valid, io.qout(i).ready, io.qout(i).bits)}
  
  // connect split steerreged outputs with primary outputs
  val out_accum_array = Array.fill(steerWays, splitWays-1) { UInt(width = (split_width - steer_header_width)) }
  for (way <- (0 until steerWays)) {
    // exclude last one as it may be larger by split_width_pad and we'll add it later along with the header
    for (partition <- 0 until (splitWays-1)) {
      out_accum_array(way)(partition) = steer_submod_outs(partition)(way).bits(split_width - steer_header_width - 1, 0)
      steer_submod_outs(partition)(way).ready  := io.qout(way).ready
    }
    // concat header with split data 
    //printf ("dv=%d, last partition data %d\n", steer_submod_outs(0)(way).valid, steer_submod_outs(splitWays-1)(way).bits(split_width_last - 1, 0))
    io.qout(way).bits := steer_submod_outs(splitWays-1)(way).bits(split_width_last - 1, 0) ## out_accum_array(way).reduceLeft((a,b) => b##a)
    steer_submod_outs(splitWays-1)(way).ready  := io.qout(way).ready

    io.qout(way).valid := steer_submod_outs(0)(way).valid // we can get valid from any partition
  }
}

class ArbiterReged[T<:Data]( gen : T, ways: Int) extends Module {
  val io = IO(new Bundle {
    val qin  = Flipped(Vec(ways, Decoupled( gen )))
    val qout = Decoupled( gen )
  })
 

  val stage_in_N = Array.fill(ways)(Module(new DecoupledStage( gen )))
  val in_stage_outs = stage_in_N.map(_.io.out)
  
  
  //printf("in_stage_inps.valid=%d %d\n", in_stage_inps(0).valid, stage_in_N(0).io.inp.valid )
  //printf("in_stage_outs.valid=%d\n", in_stage_outs(0).valid)

  for ((stage_in, in) <- stage_in_N.toList.zip(io.qin)) {
    stage_in.io.inp <> in
  }

  //printf("io.qin.enq.valid=%d\n", io.qin(0).valid)
  val arb = Module(new RRArbiter( gen, ways))
  for ((stage_in_out, arb_in) <- in_stage_outs.zip(arb.io.in)) {
    arb_in <> stage_in_out
  }
  //printf("arb_in.valid=%d\n", arb.io.in(0).valid)
  
  val stage_out = Module(new DecoupledStage( gen ))
  stage_out.io.inp <> arb.io.out
  
  io.qout <> stage_out.io.out

}

class MultiChannelMux (rd_ports: Int, wr_ports: Int) extends Module {
  val RD_REQ_W = 80;
  val RD_RESP_W = 528;
  val RD_RESP_QUARTER_W = 144;
  val WR_REQ_W = 606;
  val WR_RESP_W = 17;
  val TAG_W = 9;
  val io = IO(new Bundle {
    val acc_rd_req_in  = Flipped(Vec(rd_ports, Decoupled( UInt(width = RD_REQ_W) )))
    val mem_rd_req_out = Decoupled( UInt(width = RD_REQ_W) ) 
    val mem_rd_resp_in = Flipped(Decoupled( UInt(width = RD_RESP_W) ) )
    val acc_rd_resp_out  = Vec(rd_ports, Decoupled( UInt(width = RD_RESP_W) ))

    val acc_wr_req_in  = Flipped(Vec(wr_ports, Decoupled( UInt(width = WR_REQ_W) )))
    val mem_wr_req_out = Decoupled( UInt(width = WR_REQ_W) ) 
    val mem_wr_resp_in = Flipped(Decoupled( UInt(width = WR_RESP_W) ) )
    val acc_wr_resp_out  = Vec(wr_ports, Decoupled( UInt(width = WR_RESP_W) ))
  })
  val arbiter_read = Module( new ArbiterReged(UInt(width = RD_REQ_W), rd_ports) )
  //val steer_read = Module( new SteerReged(UInt(width = RD_RESP_W), ways = rd_ports, steerBit=RD_RESP_W-TAG_W-log2Up(rd_ports)) )
  val steer_read = Module( new SteerRegedSplit(UInt(width = RD_RESP_W), steerWays = rd_ports, steerBit=RD_RESP_W-TAG_W-log2Up(rd_ports), splitWays = 4))
  val arbiter_write = Module( new ArbiterReged(UInt(width = WR_REQ_W), wr_ports) )
  val steer_write = Module( new SteerReged(UInt(width = WR_RESP_W), ways = wr_ports, steerBit=WR_RESP_W - TAG_W - log2Up(wr_ports)) )
  
  io.acc_rd_req_in <> arbiter_read.io.qin
  io.mem_rd_req_out <> arbiter_read.io.qout
  io.mem_rd_resp_in <> steer_read.io.qin
  io.acc_rd_resp_out <> steer_read.io.qout

  io.acc_wr_req_in <> arbiter_write.io.qin
  io.mem_wr_req_out <> arbiter_write.io.qout
  io.mem_wr_resp_in <> steer_write.io.qin
  io.acc_wr_resp_out <> steer_write.io.qout
  
}
object Arbiter {
  def genRTL(factory: () => Module) {
    val circuit = Driver.elaborate(() => (factory()))
    //Driver.parseArgs(args)
    val fnprefix = Driver.targetDir + "/" + circuit.name 
    val output_file = new File(s"$fnprefix.fir")
    Driver.dumpFirrtl(circuit, Option(output_file))
    s"firrtl -i $fnprefix.fir -o $fnprefix.v -X verilog".!
  }

  def main(args: Array[String]): Unit = {
    Driver.parseArgs(args)
    val arg_map = Map((for(pair <- args.map(_.split("="))) yield (pair(0), pair(1))):_*)
    //genRTL(()=>new ArbiterReged(UInt(width = 80), ways = 4))
    //genRTL(()=>new SteerReged(UInt(width=144), ways = 4, steerBit=142))
    val RD_PORTS = arg_map("RD_PORTS").toInt
    val WR_PORTS = arg_map("WR_PORTS").toInt
    genRTL(()=>new MultiChannelMux(arg_map("RD_PORTS").toInt, arg_map("WR_PORTS").toInt))
    s"mv MultiChannelMux.v MultiChannelMux_${RD_PORTS}_$WR_PORTS.v".!
  }
}
