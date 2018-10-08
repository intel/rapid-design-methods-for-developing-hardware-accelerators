// See LICENSE for license details.
`ifdef RANDOMIZE_GARBAGE_ASSIGN
`define RANDOMIZE
`endif
`ifdef RANDOMIZE_INVALID_ASSIGN
`define RANDOMIZE
`endif
`ifdef RANDOMIZE_REG_INIT
`define RANDOMIZE
`endif
`ifdef RANDOMIZE_MEM_INIT
`define RANDOMIZE
`endif

module DecoupledStge(
  input   clock,
  input   reset,
  output  io_inp_ready,
  input   io_inp_valid,
  input  [79:0] io_inp_bits,
  input   io_out_ready,
  output  io_out_valid,
  output [79:0] io_out_bits
);
  reg  out_valid;
  reg [31:0] GEN_1;
  reg [79:0] out_bits;
  reg [95:0] GEN_2;
  wire  T_24;
  wire  T_25;
  wire  T_26;
  wire  T_27;
  wire [79:0] GEN_0;
  assign io_inp_ready = T_25;
  assign io_out_valid = out_valid;
  assign io_out_bits = out_bits;
  assign T_24 = ~ io_out_valid;
  assign T_25 = io_out_ready | T_24;
  assign T_26 = ~ io_inp_ready;
  assign T_27 = io_inp_valid | T_26;
  assign GEN_0 = io_inp_ready ? io_inp_bits : out_bits;
`ifdef RANDOMIZE
  integer initvar;
  initial begin
    `ifndef verilator
      #0.002 begin end
    `endif
  `ifdef RANDOMIZE_REG_INIT
  GEN_1 = {1{$random}};
  out_valid = GEN_1[0:0];
  `endif
  `ifdef RANDOMIZE_REG_INIT
  GEN_2 = {3{$random}};
  out_bits = GEN_2[79:0];
  `endif
  end
`endif
  always @(posedge clock) begin
    if(reset) begin
      out_valid <= 1'h0;
    end else begin
      out_valid <= T_27;
    end
    if(1'h0) begin
    end else begin
      if(io_inp_ready) begin
        out_bits <= io_inp_bits;
      end
    end
  end
endmodule
module RRArbiter(
  input   clock,
  input   reset,
  output  io_in_0_ready,
  input   io_in_0_valid,
  input  [79:0] io_in_0_bits,
  output  io_in_1_ready,
  input   io_in_1_valid,
  input  [79:0] io_in_1_bits,
  input   io_out_ready,
  output  io_out_valid,
  output [79:0] io_out_bits,
  output  io_chosen
);
  wire  choice;
  wire  GEN_0_ready;
  wire  GEN_0_valid;
  wire [79:0] GEN_0_bits;
  wire  GEN_2;
  wire  GEN_3;
  wire [79:0] GEN_4;
  wire  GEN_1_ready;
  wire  GEN_1_valid;
  wire [79:0] GEN_1_bits;
  wire  T_73;
  reg  lastGrant;
  reg [31:0] GEN_0;
  wire  GEN_5;
  wire  grantMask_1;
  wire  validMask_1;
  wire  T_77;
  wire  T_81;
  wire  T_83;
  wire  T_87;
  wire  T_88;
  wire  T_89;
  wire  GEN_6;
  wire  GEN_7;
  assign io_in_0_ready = T_88;
  assign io_in_1_ready = T_89;
  assign io_out_valid = GEN_0_valid;
  assign io_out_bits = GEN_1_bits;
  assign io_chosen = choice;
  assign choice = GEN_7;
  assign GEN_0_ready = GEN_2;
  assign GEN_0_valid = GEN_3;
  assign GEN_0_bits = GEN_4;
  assign GEN_2 = io_chosen ? io_in_1_ready : io_in_0_ready;
  assign GEN_3 = io_chosen ? io_in_1_valid : io_in_0_valid;
  assign GEN_4 = io_chosen ? io_in_1_bits : io_in_0_bits;
  assign GEN_1_ready = GEN_2;
  assign GEN_1_valid = GEN_3;
  assign GEN_1_bits = GEN_4;
  assign T_73 = io_out_ready & io_out_valid;
  assign GEN_5 = T_73 ? io_chosen : lastGrant;
  assign grantMask_1 = 1'h1 > lastGrant;
  assign validMask_1 = io_in_1_valid & grantMask_1;
  assign T_77 = validMask_1 | io_in_0_valid;
  assign T_81 = validMask_1 == 1'h0;
  assign T_83 = T_77 == 1'h0;
  assign T_87 = grantMask_1 | T_83;
  assign T_88 = T_81 & io_out_ready;
  assign T_89 = T_87 & io_out_ready;
  assign GEN_6 = io_in_0_valid ? 1'h0 : 1'h1;
  assign GEN_7 = validMask_1 ? 1'h1 : GEN_6;
`ifdef RANDOMIZE
  integer initvar;
  initial begin
    `ifndef verilator
      #0.002 begin end
    `endif
  `ifdef RANDOMIZE_REG_INIT
  GEN_0 = {1{$random}};
  lastGrant = GEN_0[0:0];
  `endif
  end
`endif
  always @(posedge clock) begin
    if(reset) begin
      lastGrant <= 1'h0;
    end else begin
      if(T_73) begin
        lastGrant <= io_chosen;
      end
    end
  end
endmodule
module ArbiterReged(
  input   clock,
  input   reset,
  output  io_qin_0_ready,
  input   io_qin_0_valid,
  input  [79:0] io_qin_0_bits,
  output  io_qin_1_ready,
  input   io_qin_1_valid,
  input  [79:0] io_qin_1_bits,
  input   io_qout_ready,
  output  io_qout_valid,
  output [79:0] io_qout_bits
);
  wire  DecoupledStge_3_clock;
  wire  DecoupledStge_3_reset;
  wire  DecoupledStge_3_io_inp_ready;
  wire  DecoupledStge_3_io_inp_valid;
  wire [79:0] DecoupledStge_3_io_inp_bits;
  wire  DecoupledStge_3_io_out_ready;
  wire  DecoupledStge_3_io_out_valid;
  wire [79:0] DecoupledStge_3_io_out_bits;
  wire  DecoupledStge_1_1_clock;
  wire  DecoupledStge_1_1_reset;
  wire  DecoupledStge_1_1_io_inp_ready;
  wire  DecoupledStge_1_1_io_inp_valid;
  wire [79:0] DecoupledStge_1_1_io_inp_bits;
  wire  DecoupledStge_1_1_io_out_ready;
  wire  DecoupledStge_1_1_io_out_valid;
  wire [79:0] DecoupledStge_1_1_io_out_bits;
  wire  in_stage_inps_0_ready;
  wire  in_stage_inps_0_valid;
  wire [79:0] in_stage_inps_0_bits;
  wire  in_stage_inps_1_ready;
  wire  in_stage_inps_1_valid;
  wire [79:0] in_stage_inps_1_bits;
  wire  arb_clock;
  wire  arb_reset;
  wire  arb_io_in_0_ready;
  wire  arb_io_in_0_valid;
  wire [79:0] arb_io_in_0_bits;
  wire  arb_io_in_1_ready;
  wire  arb_io_in_1_valid;
  wire [79:0] arb_io_in_1_bits;
  wire  arb_io_out_ready;
  wire  arb_io_out_valid;
  wire [79:0] arb_io_out_bits;
  wire  arb_io_chosen;
  wire  stage_out_clock;
  wire  stage_out_reset;
  wire  stage_out_io_inp_ready;
  wire  stage_out_io_inp_valid;
  wire [79:0] stage_out_io_inp_bits;
  wire  stage_out_io_out_ready;
  wire  stage_out_io_out_valid;
  wire [79:0] stage_out_io_out_bits;
  DecoupledStge DecoupledStge_3 (
    .clock(DecoupledStge_3_clock),
    .reset(DecoupledStge_3_reset),
    .io_inp_ready(DecoupledStge_3_io_inp_ready),
    .io_inp_valid(DecoupledStge_3_io_inp_valid),
    .io_inp_bits(DecoupledStge_3_io_inp_bits),
    .io_out_ready(DecoupledStge_3_io_out_ready),
    .io_out_valid(DecoupledStge_3_io_out_valid),
    .io_out_bits(DecoupledStge_3_io_out_bits)
  );
  DecoupledStge DecoupledStge_1_1 (
    .clock(DecoupledStge_1_1_clock),
    .reset(DecoupledStge_1_1_reset),
    .io_inp_ready(DecoupledStge_1_1_io_inp_ready),
    .io_inp_valid(DecoupledStge_1_1_io_inp_valid),
    .io_inp_bits(DecoupledStge_1_1_io_inp_bits),
    .io_out_ready(DecoupledStge_1_1_io_out_ready),
    .io_out_valid(DecoupledStge_1_1_io_out_valid),
    .io_out_bits(DecoupledStge_1_1_io_out_bits)
  );
  RRArbiter arb (
    .clock(arb_clock),
    .reset(arb_reset),
    .io_in_0_ready(arb_io_in_0_ready),
    .io_in_0_valid(arb_io_in_0_valid),
    .io_in_0_bits(arb_io_in_0_bits),
    .io_in_1_ready(arb_io_in_1_ready),
    .io_in_1_valid(arb_io_in_1_valid),
    .io_in_1_bits(arb_io_in_1_bits),
    .io_out_ready(arb_io_out_ready),
    .io_out_valid(arb_io_out_valid),
    .io_out_bits(arb_io_out_bits),
    .io_chosen(arb_io_chosen)
  );
  DecoupledStge stage_out (
    .clock(stage_out_clock),
    .reset(stage_out_reset),
    .io_inp_ready(stage_out_io_inp_ready),
    .io_inp_valid(stage_out_io_inp_valid),
    .io_inp_bits(stage_out_io_inp_bits),
    .io_out_ready(stage_out_io_out_ready),
    .io_out_valid(stage_out_io_out_valid),
    .io_out_bits(stage_out_io_out_bits)
  );
  assign io_qin_0_ready = in_stage_inps_0_ready;
  assign io_qin_1_ready = in_stage_inps_1_ready;
  assign io_qout_valid = stage_out_io_out_valid;
  assign io_qout_bits = stage_out_io_out_bits;
  assign DecoupledStge_3_clock = clock;
  assign DecoupledStge_3_reset = reset;
  assign DecoupledStge_3_io_inp_valid = in_stage_inps_0_valid;
  assign DecoupledStge_3_io_inp_bits = in_stage_inps_0_bits;
  assign DecoupledStge_3_io_out_ready = arb_io_in_0_ready;
  assign DecoupledStge_1_1_clock = clock;
  assign DecoupledStge_1_1_reset = reset;
  assign DecoupledStge_1_1_io_inp_valid = in_stage_inps_1_valid;
  assign DecoupledStge_1_1_io_inp_bits = in_stage_inps_1_bits;
  assign DecoupledStge_1_1_io_out_ready = arb_io_in_1_ready;
  assign in_stage_inps_0_ready = DecoupledStge_3_io_inp_ready;
  assign in_stage_inps_0_valid = io_qin_0_valid;
  assign in_stage_inps_0_bits = io_qin_0_bits;
  assign in_stage_inps_1_ready = DecoupledStge_1_1_io_inp_ready;
  assign in_stage_inps_1_valid = io_qin_1_valid;
  assign in_stage_inps_1_bits = io_qin_1_bits;
  assign arb_clock = clock;
  assign arb_reset = reset;
  assign arb_io_in_0_valid = DecoupledStge_3_io_out_valid;
  assign arb_io_in_0_bits = DecoupledStge_3_io_out_bits;
  assign arb_io_in_1_valid = DecoupledStge_1_1_io_out_valid;
  assign arb_io_in_1_bits = DecoupledStge_1_1_io_out_bits;
  assign arb_io_out_ready = stage_out_io_inp_ready;
  assign stage_out_clock = clock;
  assign stage_out_reset = reset;
  assign stage_out_io_inp_valid = arb_io_out_valid;
  assign stage_out_io_inp_bits = arb_io_out_bits;
  assign stage_out_io_out_ready = io_qout_ready;
endmodule
module DecoupledStge_3(
  input   clock,
  input   reset,
  output  io_inp_ready,
  input   io_inp_valid,
  input  [140:0] io_inp_bits,
  input   io_out_ready,
  output  io_out_valid,
  output [140:0] io_out_bits
);
  reg  out_valid;
  reg [31:0] GEN_1;
  reg [140:0] out_bits;
  reg [159:0] GEN_2;
  wire  T_24;
  wire  T_25;
  wire  T_26;
  wire  T_27;
  wire [140:0] GEN_0;
  assign io_inp_ready = T_25;
  assign io_out_valid = out_valid;
  assign io_out_bits = out_bits;
  assign T_24 = ~ io_out_valid;
  assign T_25 = io_out_ready | T_24;
  assign T_26 = ~ io_inp_ready;
  assign T_27 = io_inp_valid | T_26;
  assign GEN_0 = io_inp_ready ? io_inp_bits : out_bits;
`ifdef RANDOMIZE
  integer initvar;
  initial begin
    `ifndef verilator
      #0.002 begin end
    `endif
  `ifdef RANDOMIZE_REG_INIT
  GEN_1 = {1{$random}};
  out_valid = GEN_1[0:0];
  `endif
  `ifdef RANDOMIZE_REG_INIT
  GEN_2 = {5{$random}};
  out_bits = GEN_2[140:0];
  `endif
  end
`endif
  always @(posedge clock) begin
    if(reset) begin
      out_valid <= 1'h0;
    end else begin
      out_valid <= T_27;
    end
    if(1'h0) begin
    end else begin
      if(io_inp_ready) begin
        out_bits <= io_inp_bits;
      end
    end
  end
endmodule
module SteerReged(
  input   clock,
  input   reset,
  output  io_qin_ready,
  input   io_qin_valid,
  input  [140:0] io_qin_bits,
  input   io_qout_0_ready,
  output  io_qout_0_valid,
  output [140:0] io_qout_0_bits,
  input   io_qout_1_ready,
  output  io_qout_1_valid,
  output [140:0] io_qout_1_bits
);
  wire  DecoupledStge_3_1_clock;
  wire  DecoupledStge_3_1_reset;
  wire  DecoupledStge_3_1_io_inp_ready;
  wire  DecoupledStge_3_1_io_inp_valid;
  wire [140:0] DecoupledStge_3_1_io_inp_bits;
  wire  DecoupledStge_3_1_io_out_ready;
  wire  DecoupledStge_3_1_io_out_valid;
  wire [140:0] DecoupledStge_3_1_io_out_bits;
  wire  DecoupledStge_4_1_clock;
  wire  DecoupledStge_4_1_reset;
  wire  DecoupledStge_4_1_io_inp_ready;
  wire  DecoupledStge_4_1_io_inp_valid;
  wire [140:0] DecoupledStge_4_1_io_inp_bits;
  wire  DecoupledStge_4_1_io_out_ready;
  wire  DecoupledStge_4_1_io_out_valid;
  wire [140:0] DecoupledStge_4_1_io_out_bits;
  wire  out_stage_inps_0_ready;
  wire  out_stage_inps_0_valid;
  wire [140:0] out_stage_inps_0_bits;
  wire  out_stage_inps_1_ready;
  wire  out_stage_inps_1_valid;
  wire [140:0] out_stage_inps_1_bits;
  wire [140:0] T_93;
  wire [140:0] T_97;
  reg [140:0] pendData;
  reg [159:0] GEN_0;
  reg  pendValid;
  reg [31:0] GEN_3;
  wire  pendAuId;
  wire  GEN_0_ready;
  wire  GEN_0_valid;
  wire [140:0] GEN_0_bits;
  wire  GEN_4;
  wire  GEN_5;
  wire [140:0] GEN_6;
  wire  T_105;
  wire  GEN_1;
  wire  GEN_7;
  wire  GEN_8;
  wire [140:0] GEN_2;
  wire [140:0] GEN_9;
  wire [140:0] GEN_10;
  wire  GEN_13;
  wire [140:0] GEN_15;
  wire  GEN_12;
  wire [140:0] GEN_16;
  wire  GEN_3_ready;
  wire  GEN_3_valid;
  wire [140:0] GEN_3_bits;
  wire  T_119;
  wire  T_121;
  wire  T_122;
  wire [140:0] T_124;
  wire [140:0] GEN_17;
  wire  GEN_18;
  wire [140:0] GEN_21;
  DecoupledStge_3 DecoupledStge_3_1 (
    .clock(DecoupledStge_3_1_clock),
    .reset(DecoupledStge_3_1_reset),
    .io_inp_ready(DecoupledStge_3_1_io_inp_ready),
    .io_inp_valid(DecoupledStge_3_1_io_inp_valid),
    .io_inp_bits(DecoupledStge_3_1_io_inp_bits),
    .io_out_ready(DecoupledStge_3_1_io_out_ready),
    .io_out_valid(DecoupledStge_3_1_io_out_valid),
    .io_out_bits(DecoupledStge_3_1_io_out_bits)
  );
  DecoupledStge_3 DecoupledStge_4_1 (
    .clock(DecoupledStge_4_1_clock),
    .reset(DecoupledStge_4_1_reset),
    .io_inp_ready(DecoupledStge_4_1_io_inp_ready),
    .io_inp_valid(DecoupledStge_4_1_io_inp_valid),
    .io_inp_bits(DecoupledStge_4_1_io_inp_bits),
    .io_out_ready(DecoupledStge_4_1_io_out_ready),
    .io_out_valid(DecoupledStge_4_1_io_out_valid),
    .io_out_bits(DecoupledStge_4_1_io_out_bits)
  );
  assign io_qin_ready = T_122;
  assign io_qout_0_valid = DecoupledStge_3_1_io_out_valid;
  assign io_qout_0_bits = DecoupledStge_3_1_io_out_bits;
  assign io_qout_1_valid = DecoupledStge_4_1_io_out_valid;
  assign io_qout_1_bits = DecoupledStge_4_1_io_out_bits;
  assign DecoupledStge_3_1_clock = clock;
  assign DecoupledStge_3_1_reset = reset;
  assign DecoupledStge_3_1_io_inp_valid = out_stage_inps_0_valid;
  assign DecoupledStge_3_1_io_inp_bits = out_stage_inps_0_bits;
  assign DecoupledStge_3_1_io_out_ready = io_qout_0_ready;
  assign DecoupledStge_4_1_clock = clock;
  assign DecoupledStge_4_1_reset = reset;
  assign DecoupledStge_4_1_io_inp_valid = out_stage_inps_1_valid;
  assign DecoupledStge_4_1_io_inp_bits = out_stage_inps_1_bits;
  assign DecoupledStge_4_1_io_out_ready = io_qout_1_ready;
  assign out_stage_inps_0_ready = DecoupledStge_3_1_io_inp_ready;
  assign out_stage_inps_0_valid = GEN_12;
  assign out_stage_inps_0_bits = GEN_15;
  assign out_stage_inps_1_ready = DecoupledStge_4_1_io_inp_ready;
  assign out_stage_inps_1_valid = GEN_13;
  assign out_stage_inps_1_bits = GEN_16;
  assign T_93 = 141'h0;
  assign T_97 = 141'h0;
  assign pendAuId = pendData[131];
  assign GEN_0_ready = GEN_4;
  assign GEN_0_valid = GEN_5;
  assign GEN_0_bits = GEN_6;
  assign GEN_4 = pendAuId ? out_stage_inps_1_ready : out_stage_inps_0_ready;
  assign GEN_5 = pendAuId ? out_stage_inps_1_valid : out_stage_inps_0_valid;
  assign GEN_6 = pendAuId ? out_stage_inps_1_bits : out_stage_inps_0_bits;
  assign T_105 = pendValid & GEN_0_ready;
  assign GEN_1 = 1'h1;
  assign GEN_7 = 1'h0 == pendAuId ? GEN_1 : 1'h0;
  assign GEN_8 = pendAuId ? GEN_1 : 1'h0;
  assign GEN_2 = pendData;
  assign GEN_9 = 1'h0 == pendAuId ? GEN_2 : T_93;
  assign GEN_10 = pendAuId ? GEN_2 : T_97;
  assign GEN_13 = T_105 ? GEN_8 : 1'h0;
  assign GEN_15 = T_105 ? GEN_9 : T_93;
  assign GEN_12 = T_105 ? GEN_7 : 1'h0;
  assign GEN_16 = T_105 ? GEN_10 : T_97;
  assign GEN_3_ready = GEN_4;
  assign GEN_3_valid = GEN_5;
  assign GEN_3_bits = GEN_6;
  assign T_119 = pendValid & GEN_3_ready;
  assign T_121 = pendValid == 1'h0;
  assign T_122 = T_119 | T_121;
  assign T_124 = io_qin_bits;
  assign GEN_17 = io_qin_valid ? T_124 : pendData;
  assign GEN_18 = T_122 ? io_qin_valid : pendValid;
  assign GEN_21 = T_122 ? GEN_17 : pendData;
`ifdef RANDOMIZE
  integer initvar;
  initial begin
    `ifndef verilator
      #0.002 begin end
    `endif
  `ifdef RANDOMIZE_REG_INIT
  GEN_0 = {5{$random}};
  pendData = GEN_0[140:0];
  `endif
  `ifdef RANDOMIZE_REG_INIT
  GEN_3 = {1{$random}};
  pendValid = GEN_3[0:0];
  `endif
  end
`endif
  always @(posedge clock) begin
    if(1'h0) begin
    end else begin
      if(T_122) begin
        if(io_qin_valid) begin
          pendData <= T_124;
        end
      end
    end
    if(reset) begin
      pendValid <= 1'h0;
    end else begin
      if(T_122) begin
        pendValid <= io_qin_valid;
      end
    end
  end
endmodule
module SteerReged_1(
  input   clock,
  input   reset,
  output  io_qin_ready,
  input   io_qin_valid,
  input  [140:0] io_qin_bits,
  input   io_qout_0_ready,
  output  io_qout_0_valid,
  output [140:0] io_qout_0_bits,
  input   io_qout_1_ready,
  output  io_qout_1_valid,
  output [140:0] io_qout_1_bits
);
  wire  DecoupledStge_5_1_clock;
  wire  DecoupledStge_5_1_reset;
  wire  DecoupledStge_5_1_io_inp_ready;
  wire  DecoupledStge_5_1_io_inp_valid;
  wire [140:0] DecoupledStge_5_1_io_inp_bits;
  wire  DecoupledStge_5_1_io_out_ready;
  wire  DecoupledStge_5_1_io_out_valid;
  wire [140:0] DecoupledStge_5_1_io_out_bits;
  wire  DecoupledStge_6_1_clock;
  wire  DecoupledStge_6_1_reset;
  wire  DecoupledStge_6_1_io_inp_ready;
  wire  DecoupledStge_6_1_io_inp_valid;
  wire [140:0] DecoupledStge_6_1_io_inp_bits;
  wire  DecoupledStge_6_1_io_out_ready;
  wire  DecoupledStge_6_1_io_out_valid;
  wire [140:0] DecoupledStge_6_1_io_out_bits;
  wire  out_stage_inps_0_ready;
  wire  out_stage_inps_0_valid;
  wire [140:0] out_stage_inps_0_bits;
  wire  out_stage_inps_1_ready;
  wire  out_stage_inps_1_valid;
  wire [140:0] out_stage_inps_1_bits;
  wire [140:0] T_93;
  wire [140:0] T_97;
  reg [140:0] pendData;
  reg [159:0] GEN_0;
  reg  pendValid;
  reg [31:0] GEN_3;
  wire  pendAuId;
  wire  GEN_0_ready;
  wire  GEN_0_valid;
  wire [140:0] GEN_0_bits;
  wire  GEN_4;
  wire  GEN_5;
  wire [140:0] GEN_6;
  wire  T_105;
  wire  GEN_1;
  wire  GEN_7;
  wire  GEN_8;
  wire [140:0] GEN_2;
  wire [140:0] GEN_9;
  wire [140:0] GEN_10;
  wire  GEN_12;
  wire  GEN_13;
  wire [140:0] GEN_16;
  wire [140:0] GEN_15;
  wire  GEN_3_ready;
  wire  GEN_3_valid;
  wire [140:0] GEN_3_bits;
  wire  T_119;
  wire  T_121;
  wire  T_122;
  wire [140:0] T_124;
  wire [140:0] GEN_17;
  wire  GEN_18;
  wire [140:0] GEN_21;
  DecoupledStge_3 DecoupledStge_5_1 (
    .clock(DecoupledStge_5_1_clock),
    .reset(DecoupledStge_5_1_reset),
    .io_inp_ready(DecoupledStge_5_1_io_inp_ready),
    .io_inp_valid(DecoupledStge_5_1_io_inp_valid),
    .io_inp_bits(DecoupledStge_5_1_io_inp_bits),
    .io_out_ready(DecoupledStge_5_1_io_out_ready),
    .io_out_valid(DecoupledStge_5_1_io_out_valid),
    .io_out_bits(DecoupledStge_5_1_io_out_bits)
  );
  DecoupledStge_3 DecoupledStge_6_1 (
    .clock(DecoupledStge_6_1_clock),
    .reset(DecoupledStge_6_1_reset),
    .io_inp_ready(DecoupledStge_6_1_io_inp_ready),
    .io_inp_valid(DecoupledStge_6_1_io_inp_valid),
    .io_inp_bits(DecoupledStge_6_1_io_inp_bits),
    .io_out_ready(DecoupledStge_6_1_io_out_ready),
    .io_out_valid(DecoupledStge_6_1_io_out_valid),
    .io_out_bits(DecoupledStge_6_1_io_out_bits)
  );
  assign io_qin_ready = T_122;
  assign io_qout_0_valid = DecoupledStge_5_1_io_out_valid;
  assign io_qout_0_bits = DecoupledStge_5_1_io_out_bits;
  assign io_qout_1_valid = DecoupledStge_6_1_io_out_valid;
  assign io_qout_1_bits = DecoupledStge_6_1_io_out_bits;
  assign DecoupledStge_5_1_clock = clock;
  assign DecoupledStge_5_1_reset = reset;
  assign DecoupledStge_5_1_io_inp_valid = out_stage_inps_0_valid;
  assign DecoupledStge_5_1_io_inp_bits = out_stage_inps_0_bits;
  assign DecoupledStge_5_1_io_out_ready = io_qout_0_ready;
  assign DecoupledStge_6_1_clock = clock;
  assign DecoupledStge_6_1_reset = reset;
  assign DecoupledStge_6_1_io_inp_valid = out_stage_inps_1_valid;
  assign DecoupledStge_6_1_io_inp_bits = out_stage_inps_1_bits;
  assign DecoupledStge_6_1_io_out_ready = io_qout_1_ready;
  assign out_stage_inps_0_ready = DecoupledStge_5_1_io_inp_ready;
  assign out_stage_inps_0_valid = GEN_12;
  assign out_stage_inps_0_bits = GEN_15;
  assign out_stage_inps_1_ready = DecoupledStge_6_1_io_inp_ready;
  assign out_stage_inps_1_valid = GEN_13;
  assign out_stage_inps_1_bits = GEN_16;
  assign T_93 = 141'h0;
  assign T_97 = 141'h0;
  assign pendAuId = pendData[131];
  assign GEN_0_ready = GEN_4;
  assign GEN_0_valid = GEN_5;
  assign GEN_0_bits = GEN_6;
  assign GEN_4 = pendAuId ? out_stage_inps_1_ready : out_stage_inps_0_ready;
  assign GEN_5 = pendAuId ? out_stage_inps_1_valid : out_stage_inps_0_valid;
  assign GEN_6 = pendAuId ? out_stage_inps_1_bits : out_stage_inps_0_bits;
  assign T_105 = pendValid & GEN_0_ready;
  assign GEN_1 = 1'h1;
  assign GEN_7 = 1'h0 == pendAuId ? GEN_1 : 1'h0;
  assign GEN_8 = pendAuId ? GEN_1 : 1'h0;
  assign GEN_2 = pendData;
  assign GEN_9 = 1'h0 == pendAuId ? GEN_2 : T_93;
  assign GEN_10 = pendAuId ? GEN_2 : T_97;
  assign GEN_12 = T_105 ? GEN_7 : 1'h0;
  assign GEN_13 = T_105 ? GEN_8 : 1'h0;
  assign GEN_16 = T_105 ? GEN_10 : T_97;
  assign GEN_15 = T_105 ? GEN_9 : T_93;
  assign GEN_3_ready = GEN_4;
  assign GEN_3_valid = GEN_5;
  assign GEN_3_bits = GEN_6;
  assign T_119 = pendValid & GEN_3_ready;
  assign T_121 = pendValid == 1'h0;
  assign T_122 = T_119 | T_121;
  assign T_124 = io_qin_bits;
  assign GEN_17 = io_qin_valid ? T_124 : pendData;
  assign GEN_18 = T_122 ? io_qin_valid : pendValid;
  assign GEN_21 = T_122 ? GEN_17 : pendData;
`ifdef RANDOMIZE
  integer initvar;
  initial begin
    `ifndef verilator
      #0.002 begin end
    `endif
  `ifdef RANDOMIZE_REG_INIT
  GEN_0 = {5{$random}};
  pendData = GEN_0[140:0];
  `endif
  `ifdef RANDOMIZE_REG_INIT
  GEN_3 = {1{$random}};
  pendValid = GEN_3[0:0];
  `endif
  end
`endif
  always @(posedge clock) begin
    if(1'h0) begin
    end else begin
      if(T_122) begin
        if(io_qin_valid) begin
          pendData <= T_124;
        end
      end
    end
    if(reset) begin
      pendValid <= 1'h0;
    end else begin
      if(T_122) begin
        pendValid <= io_qin_valid;
      end
    end
  end
endmodule
module SteerReged_2(
  input   clock,
  input   reset,
  output  io_qin_ready,
  input   io_qin_valid,
  input  [140:0] io_qin_bits,
  input   io_qout_0_ready,
  output  io_qout_0_valid,
  output [140:0] io_qout_0_bits,
  input   io_qout_1_ready,
  output  io_qout_1_valid,
  output [140:0] io_qout_1_bits
);
  wire  DecoupledStge_7_1_clock;
  wire  DecoupledStge_7_1_reset;
  wire  DecoupledStge_7_1_io_inp_ready;
  wire  DecoupledStge_7_1_io_inp_valid;
  wire [140:0] DecoupledStge_7_1_io_inp_bits;
  wire  DecoupledStge_7_1_io_out_ready;
  wire  DecoupledStge_7_1_io_out_valid;
  wire [140:0] DecoupledStge_7_1_io_out_bits;
  wire  DecoupledStge_8_1_clock;
  wire  DecoupledStge_8_1_reset;
  wire  DecoupledStge_8_1_io_inp_ready;
  wire  DecoupledStge_8_1_io_inp_valid;
  wire [140:0] DecoupledStge_8_1_io_inp_bits;
  wire  DecoupledStge_8_1_io_out_ready;
  wire  DecoupledStge_8_1_io_out_valid;
  wire [140:0] DecoupledStge_8_1_io_out_bits;
  wire  out_stage_inps_0_ready;
  wire  out_stage_inps_0_valid;
  wire [140:0] out_stage_inps_0_bits;
  wire  out_stage_inps_1_ready;
  wire  out_stage_inps_1_valid;
  wire [140:0] out_stage_inps_1_bits;
  wire [140:0] T_93;
  wire [140:0] T_97;
  reg [140:0] pendData;
  reg [159:0] GEN_0;
  reg  pendValid;
  reg [31:0] GEN_3;
  wire  pendAuId;
  wire  GEN_0_ready;
  wire  GEN_0_valid;
  wire [140:0] GEN_0_bits;
  wire  GEN_4;
  wire  GEN_5;
  wire [140:0] GEN_6;
  wire  T_105;
  wire  GEN_1;
  wire  GEN_7;
  wire  GEN_8;
  wire [140:0] GEN_2;
  wire [140:0] GEN_9;
  wire [140:0] GEN_10;
  wire [140:0] GEN_16;
  wire [140:0] GEN_15;
  wire  GEN_13;
  wire  GEN_12;
  wire  GEN_3_ready;
  wire  GEN_3_valid;
  wire [140:0] GEN_3_bits;
  wire  T_119;
  wire  T_121;
  wire  T_122;
  wire [140:0] T_124;
  wire [140:0] GEN_17;
  wire  GEN_18;
  wire [140:0] GEN_21;
  DecoupledStge_3 DecoupledStge_7_1 (
    .clock(DecoupledStge_7_1_clock),
    .reset(DecoupledStge_7_1_reset),
    .io_inp_ready(DecoupledStge_7_1_io_inp_ready),
    .io_inp_valid(DecoupledStge_7_1_io_inp_valid),
    .io_inp_bits(DecoupledStge_7_1_io_inp_bits),
    .io_out_ready(DecoupledStge_7_1_io_out_ready),
    .io_out_valid(DecoupledStge_7_1_io_out_valid),
    .io_out_bits(DecoupledStge_7_1_io_out_bits)
  );
  DecoupledStge_3 DecoupledStge_8_1 (
    .clock(DecoupledStge_8_1_clock),
    .reset(DecoupledStge_8_1_reset),
    .io_inp_ready(DecoupledStge_8_1_io_inp_ready),
    .io_inp_valid(DecoupledStge_8_1_io_inp_valid),
    .io_inp_bits(DecoupledStge_8_1_io_inp_bits),
    .io_out_ready(DecoupledStge_8_1_io_out_ready),
    .io_out_valid(DecoupledStge_8_1_io_out_valid),
    .io_out_bits(DecoupledStge_8_1_io_out_bits)
  );
  assign io_qin_ready = T_122;
  assign io_qout_0_valid = DecoupledStge_7_1_io_out_valid;
  assign io_qout_0_bits = DecoupledStge_7_1_io_out_bits;
  assign io_qout_1_valid = DecoupledStge_8_1_io_out_valid;
  assign io_qout_1_bits = DecoupledStge_8_1_io_out_bits;
  assign DecoupledStge_7_1_clock = clock;
  assign DecoupledStge_7_1_reset = reset;
  assign DecoupledStge_7_1_io_inp_valid = out_stage_inps_0_valid;
  assign DecoupledStge_7_1_io_inp_bits = out_stage_inps_0_bits;
  assign DecoupledStge_7_1_io_out_ready = io_qout_0_ready;
  assign DecoupledStge_8_1_clock = clock;
  assign DecoupledStge_8_1_reset = reset;
  assign DecoupledStge_8_1_io_inp_valid = out_stage_inps_1_valid;
  assign DecoupledStge_8_1_io_inp_bits = out_stage_inps_1_bits;
  assign DecoupledStge_8_1_io_out_ready = io_qout_1_ready;
  assign out_stage_inps_0_ready = DecoupledStge_7_1_io_inp_ready;
  assign out_stage_inps_0_valid = GEN_12;
  assign out_stage_inps_0_bits = GEN_15;
  assign out_stage_inps_1_ready = DecoupledStge_8_1_io_inp_ready;
  assign out_stage_inps_1_valid = GEN_13;
  assign out_stage_inps_1_bits = GEN_16;
  assign T_93 = 141'h0;
  assign T_97 = 141'h0;
  assign pendAuId = pendData[131];
  assign GEN_0_ready = GEN_4;
  assign GEN_0_valid = GEN_5;
  assign GEN_0_bits = GEN_6;
  assign GEN_4 = pendAuId ? out_stage_inps_1_ready : out_stage_inps_0_ready;
  assign GEN_5 = pendAuId ? out_stage_inps_1_valid : out_stage_inps_0_valid;
  assign GEN_6 = pendAuId ? out_stage_inps_1_bits : out_stage_inps_0_bits;
  assign T_105 = pendValid & GEN_0_ready;
  assign GEN_1 = 1'h1;
  assign GEN_7 = 1'h0 == pendAuId ? GEN_1 : 1'h0;
  assign GEN_8 = pendAuId ? GEN_1 : 1'h0;
  assign GEN_2 = pendData;
  assign GEN_9 = 1'h0 == pendAuId ? GEN_2 : T_93;
  assign GEN_10 = pendAuId ? GEN_2 : T_97;
  assign GEN_16 = T_105 ? GEN_10 : T_97;
  assign GEN_15 = T_105 ? GEN_9 : T_93;
  assign GEN_13 = T_105 ? GEN_8 : 1'h0;
  assign GEN_12 = T_105 ? GEN_7 : 1'h0;
  assign GEN_3_ready = GEN_4;
  assign GEN_3_valid = GEN_5;
  assign GEN_3_bits = GEN_6;
  assign T_119 = pendValid & GEN_3_ready;
  assign T_121 = pendValid == 1'h0;
  assign T_122 = T_119 | T_121;
  assign T_124 = io_qin_bits;
  assign GEN_17 = io_qin_valid ? T_124 : pendData;
  assign GEN_18 = T_122 ? io_qin_valid : pendValid;
  assign GEN_21 = T_122 ? GEN_17 : pendData;
`ifdef RANDOMIZE
  integer initvar;
  initial begin
    `ifndef verilator
      #0.002 begin end
    `endif
  `ifdef RANDOMIZE_REG_INIT
  GEN_0 = {5{$random}};
  pendData = GEN_0[140:0];
  `endif
  `ifdef RANDOMIZE_REG_INIT
  GEN_3 = {1{$random}};
  pendValid = GEN_3[0:0];
  `endif
  end
`endif
  always @(posedge clock) begin
    if(1'h0) begin
    end else begin
      if(T_122) begin
        if(io_qin_valid) begin
          pendData <= T_124;
        end
      end
    end
    if(reset) begin
      pendValid <= 1'h0;
    end else begin
      if(T_122) begin
        pendValid <= io_qin_valid;
      end
    end
  end
endmodule
module SteerReged_3(
  input   clock,
  input   reset,
  output  io_qin_ready,
  input   io_qin_valid,
  input  [140:0] io_qin_bits,
  input   io_qout_0_ready,
  output  io_qout_0_valid,
  output [140:0] io_qout_0_bits,
  input   io_qout_1_ready,
  output  io_qout_1_valid,
  output [140:0] io_qout_1_bits
);
  wire  DecoupledStge_9_1_clock;
  wire  DecoupledStge_9_1_reset;
  wire  DecoupledStge_9_1_io_inp_ready;
  wire  DecoupledStge_9_1_io_inp_valid;
  wire [140:0] DecoupledStge_9_1_io_inp_bits;
  wire  DecoupledStge_9_1_io_out_ready;
  wire  DecoupledStge_9_1_io_out_valid;
  wire [140:0] DecoupledStge_9_1_io_out_bits;
  wire  DecoupledStge_10_1_clock;
  wire  DecoupledStge_10_1_reset;
  wire  DecoupledStge_10_1_io_inp_ready;
  wire  DecoupledStge_10_1_io_inp_valid;
  wire [140:0] DecoupledStge_10_1_io_inp_bits;
  wire  DecoupledStge_10_1_io_out_ready;
  wire  DecoupledStge_10_1_io_out_valid;
  wire [140:0] DecoupledStge_10_1_io_out_bits;
  wire  out_stage_inps_0_ready;
  wire  out_stage_inps_0_valid;
  wire [140:0] out_stage_inps_0_bits;
  wire  out_stage_inps_1_ready;
  wire  out_stage_inps_1_valid;
  wire [140:0] out_stage_inps_1_bits;
  wire [140:0] T_93;
  wire [140:0] T_97;
  reg [140:0] pendData;
  reg [159:0] GEN_0;
  reg  pendValid;
  reg [31:0] GEN_3;
  wire  pendAuId;
  wire  GEN_0_ready;
  wire  GEN_0_valid;
  wire [140:0] GEN_0_bits;
  wire  GEN_4;
  wire  GEN_5;
  wire [140:0] GEN_6;
  wire  T_105;
  wire  GEN_1;
  wire  GEN_7;
  wire  GEN_8;
  wire [140:0] GEN_2;
  wire [140:0] GEN_9;
  wire [140:0] GEN_10;
  wire  GEN_13;
  wire [140:0] GEN_16;
  wire  GEN_12;
  wire [140:0] GEN_15;
  wire  GEN_3_ready;
  wire  GEN_3_valid;
  wire [140:0] GEN_3_bits;
  wire  T_119;
  wire  T_121;
  wire  T_122;
  wire [140:0] T_124;
  wire [140:0] GEN_17;
  wire  GEN_18;
  wire [140:0] GEN_21;
  DecoupledStge_3 DecoupledStge_9_1 (
    .clock(DecoupledStge_9_1_clock),
    .reset(DecoupledStge_9_1_reset),
    .io_inp_ready(DecoupledStge_9_1_io_inp_ready),
    .io_inp_valid(DecoupledStge_9_1_io_inp_valid),
    .io_inp_bits(DecoupledStge_9_1_io_inp_bits),
    .io_out_ready(DecoupledStge_9_1_io_out_ready),
    .io_out_valid(DecoupledStge_9_1_io_out_valid),
    .io_out_bits(DecoupledStge_9_1_io_out_bits)
  );
  DecoupledStge_3 DecoupledStge_10_1 (
    .clock(DecoupledStge_10_1_clock),
    .reset(DecoupledStge_10_1_reset),
    .io_inp_ready(DecoupledStge_10_1_io_inp_ready),
    .io_inp_valid(DecoupledStge_10_1_io_inp_valid),
    .io_inp_bits(DecoupledStge_10_1_io_inp_bits),
    .io_out_ready(DecoupledStge_10_1_io_out_ready),
    .io_out_valid(DecoupledStge_10_1_io_out_valid),
    .io_out_bits(DecoupledStge_10_1_io_out_bits)
  );
  assign io_qin_ready = T_122;
  assign io_qout_0_valid = DecoupledStge_9_1_io_out_valid;
  assign io_qout_0_bits = DecoupledStge_9_1_io_out_bits;
  assign io_qout_1_valid = DecoupledStge_10_1_io_out_valid;
  assign io_qout_1_bits = DecoupledStge_10_1_io_out_bits;
  assign DecoupledStge_9_1_clock = clock;
  assign DecoupledStge_9_1_reset = reset;
  assign DecoupledStge_9_1_io_inp_valid = out_stage_inps_0_valid;
  assign DecoupledStge_9_1_io_inp_bits = out_stage_inps_0_bits;
  assign DecoupledStge_9_1_io_out_ready = io_qout_0_ready;
  assign DecoupledStge_10_1_clock = clock;
  assign DecoupledStge_10_1_reset = reset;
  assign DecoupledStge_10_1_io_inp_valid = out_stage_inps_1_valid;
  assign DecoupledStge_10_1_io_inp_bits = out_stage_inps_1_bits;
  assign DecoupledStge_10_1_io_out_ready = io_qout_1_ready;
  assign out_stage_inps_0_ready = DecoupledStge_9_1_io_inp_ready;
  assign out_stage_inps_0_valid = GEN_12;
  assign out_stage_inps_0_bits = GEN_15;
  assign out_stage_inps_1_ready = DecoupledStge_10_1_io_inp_ready;
  assign out_stage_inps_1_valid = GEN_13;
  assign out_stage_inps_1_bits = GEN_16;
  assign T_93 = 141'h0;
  assign T_97 = 141'h0;
  assign pendAuId = pendData[131];
  assign GEN_0_ready = GEN_4;
  assign GEN_0_valid = GEN_5;
  assign GEN_0_bits = GEN_6;
  assign GEN_4 = pendAuId ? out_stage_inps_1_ready : out_stage_inps_0_ready;
  assign GEN_5 = pendAuId ? out_stage_inps_1_valid : out_stage_inps_0_valid;
  assign GEN_6 = pendAuId ? out_stage_inps_1_bits : out_stage_inps_0_bits;
  assign T_105 = pendValid & GEN_0_ready;
  assign GEN_1 = 1'h1;
  assign GEN_7 = 1'h0 == pendAuId ? GEN_1 : 1'h0;
  assign GEN_8 = pendAuId ? GEN_1 : 1'h0;
  assign GEN_2 = pendData;
  assign GEN_9 = 1'h0 == pendAuId ? GEN_2 : T_93;
  assign GEN_10 = pendAuId ? GEN_2 : T_97;
  assign GEN_13 = T_105 ? GEN_8 : 1'h0;
  assign GEN_16 = T_105 ? GEN_10 : T_97;
  assign GEN_12 = T_105 ? GEN_7 : 1'h0;
  assign GEN_15 = T_105 ? GEN_9 : T_93;
  assign GEN_3_ready = GEN_4;
  assign GEN_3_valid = GEN_5;
  assign GEN_3_bits = GEN_6;
  assign T_119 = pendValid & GEN_3_ready;
  assign T_121 = pendValid == 1'h0;
  assign T_122 = T_119 | T_121;
  assign T_124 = io_qin_bits;
  assign GEN_17 = io_qin_valid ? T_124 : pendData;
  assign GEN_18 = T_122 ? io_qin_valid : pendValid;
  assign GEN_21 = T_122 ? GEN_17 : pendData;
`ifdef RANDOMIZE
  integer initvar;
  initial begin
    `ifndef verilator
      #0.002 begin end
    `endif
  `ifdef RANDOMIZE_REG_INIT
  GEN_0 = {5{$random}};
  pendData = GEN_0[140:0];
  `endif
  `ifdef RANDOMIZE_REG_INIT
  GEN_3 = {1{$random}};
  pendValid = GEN_3[0:0];
  `endif
  end
`endif
  always @(posedge clock) begin
    if(1'h0) begin
    end else begin
      if(T_122) begin
        if(io_qin_valid) begin
          pendData <= T_124;
        end
      end
    end
    if(reset) begin
      pendValid <= 1'h0;
    end else begin
      if(T_122) begin
        pendValid <= io_qin_valid;
      end
    end
  end
endmodule
module SteerRegedSplit(
  input   clock,
  input   reset,
  output  io_qin_ready,
  input   io_qin_valid,
  input  [527:0] io_qin_bits,
  input   io_qout_0_ready,
  output  io_qout_0_valid,
  output [527:0] io_qout_0_bits,
  input   io_qout_1_ready,
  output  io_qout_1_valid,
  output [527:0] io_qout_1_bits
);
  wire  SteerReged_4_clock;
  wire  SteerReged_4_reset;
  wire  SteerReged_4_io_qin_ready;
  wire  SteerReged_4_io_qin_valid;
  wire [140:0] SteerReged_4_io_qin_bits;
  wire  SteerReged_4_io_qout_0_ready;
  wire  SteerReged_4_io_qout_0_valid;
  wire [140:0] SteerReged_4_io_qout_0_bits;
  wire  SteerReged_4_io_qout_1_ready;
  wire  SteerReged_4_io_qout_1_valid;
  wire [140:0] SteerReged_4_io_qout_1_bits;
  wire  SteerReged_1_1_clock;
  wire  SteerReged_1_1_reset;
  wire  SteerReged_1_1_io_qin_ready;
  wire  SteerReged_1_1_io_qin_valid;
  wire [140:0] SteerReged_1_1_io_qin_bits;
  wire  SteerReged_1_1_io_qout_0_ready;
  wire  SteerReged_1_1_io_qout_0_valid;
  wire [140:0] SteerReged_1_1_io_qout_0_bits;
  wire  SteerReged_1_1_io_qout_1_ready;
  wire  SteerReged_1_1_io_qout_1_valid;
  wire [140:0] SteerReged_1_1_io_qout_1_bits;
  wire  SteerReged_2_1_clock;
  wire  SteerReged_2_1_reset;
  wire  SteerReged_2_1_io_qin_ready;
  wire  SteerReged_2_1_io_qin_valid;
  wire [140:0] SteerReged_2_1_io_qin_bits;
  wire  SteerReged_2_1_io_qout_0_ready;
  wire  SteerReged_2_1_io_qout_0_valid;
  wire [140:0] SteerReged_2_1_io_qout_0_bits;
  wire  SteerReged_2_1_io_qout_1_ready;
  wire  SteerReged_2_1_io_qout_1_valid;
  wire [140:0] SteerReged_2_1_io_qout_1_bits;
  wire  SteerReged_3_1_clock;
  wire  SteerReged_3_1_reset;
  wire  SteerReged_3_1_io_qin_ready;
  wire  SteerReged_3_1_io_qin_valid;
  wire [140:0] SteerReged_3_1_io_qin_bits;
  wire  SteerReged_3_1_io_qout_0_ready;
  wire  SteerReged_3_1_io_qout_0_valid;
  wire [140:0] SteerReged_3_1_io_qout_0_bits;
  wire  SteerReged_3_1_io_qout_1_ready;
  wire  SteerReged_3_1_io_qout_1_valid;
  wire [140:0] SteerReged_3_1_io_qout_1_bits;
  wire  steer_submod_ins_0_ready;
  wire  steer_submod_ins_0_valid;
  wire [140:0] steer_submod_ins_0_bits;
  wire  steer_submod_ins_1_ready;
  wire  steer_submod_ins_1_valid;
  wire [140:0] steer_submod_ins_1_bits;
  wire  steer_submod_ins_2_ready;
  wire  steer_submod_ins_2_valid;
  wire [140:0] steer_submod_ins_2_bits;
  wire  steer_submod_ins_3_ready;
  wire  steer_submod_ins_3_valid;
  wire [140:0] steer_submod_ins_3_bits;
  wire  steer_submod_outs_0_0_ready;
  wire  steer_submod_outs_0_0_valid;
  wire [140:0] steer_submod_outs_0_0_bits;
  wire  steer_submod_outs_0_1_ready;
  wire  steer_submod_outs_0_1_valid;
  wire [140:0] steer_submod_outs_0_1_bits;
  wire  steer_submod_outs_1_0_ready;
  wire  steer_submod_outs_1_0_valid;
  wire [140:0] steer_submod_outs_1_0_bits;
  wire  steer_submod_outs_1_1_ready;
  wire  steer_submod_outs_1_1_valid;
  wire [140:0] steer_submod_outs_1_1_bits;
  wire  steer_submod_outs_2_0_ready;
  wire  steer_submod_outs_2_0_valid;
  wire [140:0] steer_submod_outs_2_0_bits;
  wire  steer_submod_outs_2_1_ready;
  wire  steer_submod_outs_2_1_valid;
  wire [140:0] steer_submod_outs_2_1_bits;
  wire  steer_submod_outs_3_0_ready;
  wire  steer_submod_outs_3_0_valid;
  wire [140:0] steer_submod_outs_3_0_bits;
  wire  steer_submod_outs_3_1_ready;
  wire  steer_submod_outs_3_1_valid;
  wire [140:0] steer_submod_outs_3_1_bits;
  wire [130:0] T_330;
  wire [130:0] T_331;
  wire [9:0] T_332;
  wire [140:0] T_333;
  wire [130:0] T_334;
  wire [130:0] T_335;
  wire [140:0] T_337;
  wire [130:0] T_338;
  wire [130:0] T_339;
  wire [140:0] T_341;
  wire [130:0] T_342;
  wire [130:0] T_343;
  wire [140:0] T_345;
  wire [128:0] T_352;
  wire [128:0] T_353;
  wire [128:0] T_354;
  wire [257:0] T_356;
  wire [386:0] T_357;
  wire [527:0] T_358;
  wire [128:0] T_359;
  wire [128:0] T_360;
  wire [128:0] T_361;
  wire [257:0] T_363;
  wire [386:0] T_364;
  wire [527:0] T_365;
  SteerReged SteerReged_4 (
    .clock(SteerReged_4_clock),
    .reset(SteerReged_4_reset),
    .io_qin_ready(SteerReged_4_io_qin_ready),
    .io_qin_valid(SteerReged_4_io_qin_valid),
    .io_qin_bits(SteerReged_4_io_qin_bits),
    .io_qout_0_ready(SteerReged_4_io_qout_0_ready),
    .io_qout_0_valid(SteerReged_4_io_qout_0_valid),
    .io_qout_0_bits(SteerReged_4_io_qout_0_bits),
    .io_qout_1_ready(SteerReged_4_io_qout_1_ready),
    .io_qout_1_valid(SteerReged_4_io_qout_1_valid),
    .io_qout_1_bits(SteerReged_4_io_qout_1_bits)
  );
  SteerReged_1 SteerReged_1_1 (
    .clock(SteerReged_1_1_clock),
    .reset(SteerReged_1_1_reset),
    .io_qin_ready(SteerReged_1_1_io_qin_ready),
    .io_qin_valid(SteerReged_1_1_io_qin_valid),
    .io_qin_bits(SteerReged_1_1_io_qin_bits),
    .io_qout_0_ready(SteerReged_1_1_io_qout_0_ready),
    .io_qout_0_valid(SteerReged_1_1_io_qout_0_valid),
    .io_qout_0_bits(SteerReged_1_1_io_qout_0_bits),
    .io_qout_1_ready(SteerReged_1_1_io_qout_1_ready),
    .io_qout_1_valid(SteerReged_1_1_io_qout_1_valid),
    .io_qout_1_bits(SteerReged_1_1_io_qout_1_bits)
  );
  SteerReged_2 SteerReged_2_1 (
    .clock(SteerReged_2_1_clock),
    .reset(SteerReged_2_1_reset),
    .io_qin_ready(SteerReged_2_1_io_qin_ready),
    .io_qin_valid(SteerReged_2_1_io_qin_valid),
    .io_qin_bits(SteerReged_2_1_io_qin_bits),
    .io_qout_0_ready(SteerReged_2_1_io_qout_0_ready),
    .io_qout_0_valid(SteerReged_2_1_io_qout_0_valid),
    .io_qout_0_bits(SteerReged_2_1_io_qout_0_bits),
    .io_qout_1_ready(SteerReged_2_1_io_qout_1_ready),
    .io_qout_1_valid(SteerReged_2_1_io_qout_1_valid),
    .io_qout_1_bits(SteerReged_2_1_io_qout_1_bits)
  );
  SteerReged_3 SteerReged_3_1 (
    .clock(SteerReged_3_1_clock),
    .reset(SteerReged_3_1_reset),
    .io_qin_ready(SteerReged_3_1_io_qin_ready),
    .io_qin_valid(SteerReged_3_1_io_qin_valid),
    .io_qin_bits(SteerReged_3_1_io_qin_bits),
    .io_qout_0_ready(SteerReged_3_1_io_qout_0_ready),
    .io_qout_0_valid(SteerReged_3_1_io_qout_0_valid),
    .io_qout_0_bits(SteerReged_3_1_io_qout_0_bits),
    .io_qout_1_ready(SteerReged_3_1_io_qout_1_ready),
    .io_qout_1_valid(SteerReged_3_1_io_qout_1_valid),
    .io_qout_1_bits(SteerReged_3_1_io_qout_1_bits)
  );
  assign io_qin_ready = steer_submod_ins_0_ready;
  assign io_qout_0_valid = steer_submod_outs_0_0_valid;
  assign io_qout_0_bits = T_358;
  assign io_qout_1_valid = steer_submod_outs_0_1_valid;
  assign io_qout_1_bits = T_365;
  assign SteerReged_4_clock = clock;
  assign SteerReged_4_reset = reset;
  assign SteerReged_4_io_qin_valid = steer_submod_ins_0_valid;
  assign SteerReged_4_io_qin_bits = steer_submod_ins_0_bits;
  assign SteerReged_4_io_qout_0_ready = steer_submod_outs_0_0_ready;
  assign SteerReged_4_io_qout_1_ready = steer_submod_outs_0_1_ready;
  assign SteerReged_1_1_clock = clock;
  assign SteerReged_1_1_reset = reset;
  assign SteerReged_1_1_io_qin_valid = steer_submod_ins_1_valid;
  assign SteerReged_1_1_io_qin_bits = steer_submod_ins_1_bits;
  assign SteerReged_1_1_io_qout_0_ready = steer_submod_outs_1_0_ready;
  assign SteerReged_1_1_io_qout_1_ready = steer_submod_outs_1_1_ready;
  assign SteerReged_2_1_clock = clock;
  assign SteerReged_2_1_reset = reset;
  assign SteerReged_2_1_io_qin_valid = steer_submod_ins_2_valid;
  assign SteerReged_2_1_io_qin_bits = steer_submod_ins_2_bits;
  assign SteerReged_2_1_io_qout_0_ready = steer_submod_outs_2_0_ready;
  assign SteerReged_2_1_io_qout_1_ready = steer_submod_outs_2_1_ready;
  assign SteerReged_3_1_clock = clock;
  assign SteerReged_3_1_reset = reset;
  assign SteerReged_3_1_io_qin_valid = steer_submod_ins_3_valid;
  assign SteerReged_3_1_io_qin_bits = steer_submod_ins_3_bits;
  assign SteerReged_3_1_io_qout_0_ready = steer_submod_outs_3_0_ready;
  assign SteerReged_3_1_io_qout_1_ready = steer_submod_outs_3_1_ready;
  assign steer_submod_ins_0_ready = SteerReged_4_io_qin_ready;
  assign steer_submod_ins_0_valid = io_qin_valid;
  assign steer_submod_ins_0_bits = T_333;
  assign steer_submod_ins_1_ready = SteerReged_1_1_io_qin_ready;
  assign steer_submod_ins_1_valid = io_qin_valid;
  assign steer_submod_ins_1_bits = T_337;
  assign steer_submod_ins_2_ready = SteerReged_2_1_io_qin_ready;
  assign steer_submod_ins_2_valid = io_qin_valid;
  assign steer_submod_ins_2_bits = T_341;
  assign steer_submod_ins_3_ready = SteerReged_3_1_io_qin_ready;
  assign steer_submod_ins_3_valid = io_qin_valid;
  assign steer_submod_ins_3_bits = T_345;
  assign steer_submod_outs_0_0_ready = io_qout_0_ready;
  assign steer_submod_outs_0_0_valid = SteerReged_4_io_qout_0_valid;
  assign steer_submod_outs_0_0_bits = SteerReged_4_io_qout_0_bits;
  assign steer_submod_outs_0_1_ready = io_qout_1_ready;
  assign steer_submod_outs_0_1_valid = SteerReged_4_io_qout_1_valid;
  assign steer_submod_outs_0_1_bits = SteerReged_4_io_qout_1_bits;
  assign steer_submod_outs_1_0_ready = io_qout_0_ready;
  assign steer_submod_outs_1_0_valid = SteerReged_1_1_io_qout_0_valid;
  assign steer_submod_outs_1_0_bits = SteerReged_1_1_io_qout_0_bits;
  assign steer_submod_outs_1_1_ready = io_qout_1_ready;
  assign steer_submod_outs_1_1_valid = SteerReged_1_1_io_qout_1_valid;
  assign steer_submod_outs_1_1_bits = SteerReged_1_1_io_qout_1_bits;
  assign steer_submod_outs_2_0_ready = io_qout_0_ready;
  assign steer_submod_outs_2_0_valid = SteerReged_2_1_io_qout_0_valid;
  assign steer_submod_outs_2_0_bits = SteerReged_2_1_io_qout_0_bits;
  assign steer_submod_outs_2_1_ready = io_qout_1_ready;
  assign steer_submod_outs_2_1_valid = SteerReged_2_1_io_qout_1_valid;
  assign steer_submod_outs_2_1_bits = SteerReged_2_1_io_qout_1_bits;
  assign steer_submod_outs_3_0_ready = io_qout_0_ready;
  assign steer_submod_outs_3_0_valid = SteerReged_3_1_io_qout_0_valid;
  assign steer_submod_outs_3_0_bits = SteerReged_3_1_io_qout_0_bits;
  assign steer_submod_outs_3_1_ready = io_qout_1_ready;
  assign steer_submod_outs_3_1_valid = SteerReged_3_1_io_qout_1_valid;
  assign steer_submod_outs_3_1_bits = SteerReged_3_1_io_qout_1_bits;
  assign T_330 = io_qin_bits[130:0];
  assign T_331 = T_330;
  assign T_332 = io_qin_bits[527:518];
  assign T_333 = {T_332,T_331};
  assign T_334 = io_qin_bits[259:129];
  assign T_335 = T_334;
  assign T_337 = {T_332,T_335};
  assign T_338 = io_qin_bits[388:258];
  assign T_339 = T_338;
  assign T_341 = {T_332,T_339};
  assign T_342 = io_qin_bits[517:387];
  assign T_343 = T_342;
  assign T_345 = {T_332,T_343};
  assign T_352 = steer_submod_outs_0_0_bits[128:0];
  assign T_353 = steer_submod_outs_1_0_bits[128:0];
  assign T_354 = steer_submod_outs_2_0_bits[128:0];
  assign T_356 = {T_353,T_352};
  assign T_357 = {T_354,T_356};
  assign T_358 = {steer_submod_outs_3_0_bits,T_357};
  assign T_359 = steer_submod_outs_0_1_bits[128:0];
  assign T_360 = steer_submod_outs_1_1_bits[128:0];
  assign T_361 = steer_submod_outs_2_1_bits[128:0];
  assign T_363 = {T_360,T_359};
  assign T_364 = {T_361,T_363};
  assign T_365 = {steer_submod_outs_3_1_bits,T_364};
endmodule
module DecoupledStge_11(
  input   clock,
  input   reset,
  output  io_inp_ready,
  input   io_inp_valid,
  input  [605:0] io_inp_bits,
  input   io_out_ready,
  output  io_out_valid,
  output [605:0] io_out_bits
);
  reg  out_valid;
  reg [31:0] GEN_1;
  reg [605:0] out_bits;
  reg [607:0] GEN_2;
  wire  T_24;
  wire  T_25;
  wire  T_26;
  wire  T_27;
  wire [605:0] GEN_0;
  assign io_inp_ready = T_25;
  assign io_out_valid = out_valid;
  assign io_out_bits = out_bits;
  assign T_24 = ~ io_out_valid;
  assign T_25 = io_out_ready | T_24;
  assign T_26 = ~ io_inp_ready;
  assign T_27 = io_inp_valid | T_26;
  assign GEN_0 = io_inp_ready ? io_inp_bits : out_bits;
`ifdef RANDOMIZE
  integer initvar;
  initial begin
    `ifndef verilator
      #0.002 begin end
    `endif
  `ifdef RANDOMIZE_REG_INIT
  GEN_1 = {1{$random}};
  out_valid = GEN_1[0:0];
  `endif
  `ifdef RANDOMIZE_REG_INIT
  GEN_2 = {19{$random}};
  out_bits = GEN_2[605:0];
  `endif
  end
`endif
  always @(posedge clock) begin
    if(reset) begin
      out_valid <= 1'h0;
    end else begin
      out_valid <= T_27;
    end
    if(1'h0) begin
    end else begin
      if(io_inp_ready) begin
        out_bits <= io_inp_bits;
      end
    end
  end
endmodule
module RRArbiter_1(
  input   clock,
  input   reset,
  output  io_in_0_ready,
  input   io_in_0_valid,
  input  [605:0] io_in_0_bits,
  output  io_in_1_ready,
  input   io_in_1_valid,
  input  [605:0] io_in_1_bits,
  input   io_out_ready,
  output  io_out_valid,
  output [605:0] io_out_bits,
  output  io_chosen
);
  wire  choice;
  wire  GEN_0_ready;
  wire  GEN_0_valid;
  wire [605:0] GEN_0_bits;
  wire  GEN_2;
  wire  GEN_3;
  wire [605:0] GEN_4;
  wire  GEN_1_ready;
  wire  GEN_1_valid;
  wire [605:0] GEN_1_bits;
  wire  T_73;
  reg  lastGrant;
  reg [31:0] GEN_0;
  wire  GEN_5;
  wire  grantMask_1;
  wire  validMask_1;
  wire  T_77;
  wire  T_81;
  wire  T_83;
  wire  T_87;
  wire  T_88;
  wire  T_89;
  wire  GEN_6;
  wire  GEN_7;
  assign io_in_0_ready = T_88;
  assign io_in_1_ready = T_89;
  assign io_out_valid = GEN_0_valid;
  assign io_out_bits = GEN_1_bits;
  assign io_chosen = choice;
  assign choice = GEN_7;
  assign GEN_0_ready = GEN_2;
  assign GEN_0_valid = GEN_3;
  assign GEN_0_bits = GEN_4;
  assign GEN_2 = io_chosen ? io_in_1_ready : io_in_0_ready;
  assign GEN_3 = io_chosen ? io_in_1_valid : io_in_0_valid;
  assign GEN_4 = io_chosen ? io_in_1_bits : io_in_0_bits;
  assign GEN_1_ready = GEN_2;
  assign GEN_1_valid = GEN_3;
  assign GEN_1_bits = GEN_4;
  assign T_73 = io_out_ready & io_out_valid;
  assign GEN_5 = T_73 ? io_chosen : lastGrant;
  assign grantMask_1 = 1'h1 > lastGrant;
  assign validMask_1 = io_in_1_valid & grantMask_1;
  assign T_77 = validMask_1 | io_in_0_valid;
  assign T_81 = validMask_1 == 1'h0;
  assign T_83 = T_77 == 1'h0;
  assign T_87 = grantMask_1 | T_83;
  assign T_88 = T_81 & io_out_ready;
  assign T_89 = T_87 & io_out_ready;
  assign GEN_6 = io_in_0_valid ? 1'h0 : 1'h1;
  assign GEN_7 = validMask_1 ? 1'h1 : GEN_6;
`ifdef RANDOMIZE
  integer initvar;
  initial begin
    `ifndef verilator
      #0.002 begin end
    `endif
  `ifdef RANDOMIZE_REG_INIT
  GEN_0 = {1{$random}};
  lastGrant = GEN_0[0:0];
  `endif
  end
`endif
  always @(posedge clock) begin
    if(reset) begin
      lastGrant <= 1'h0;
    end else begin
      if(T_73) begin
        lastGrant <= io_chosen;
      end
    end
  end
endmodule
module ArbiterReged_1(
  input   clock,
  input   reset,
  output  io_qin_0_ready,
  input   io_qin_0_valid,
  input  [605:0] io_qin_0_bits,
  output  io_qin_1_ready,
  input   io_qin_1_valid,
  input  [605:0] io_qin_1_bits,
  input   io_qout_ready,
  output  io_qout_valid,
  output [605:0] io_qout_bits
);
  wire  DecoupledStge_11_1_clock;
  wire  DecoupledStge_11_1_reset;
  wire  DecoupledStge_11_1_io_inp_ready;
  wire  DecoupledStge_11_1_io_inp_valid;
  wire [605:0] DecoupledStge_11_1_io_inp_bits;
  wire  DecoupledStge_11_1_io_out_ready;
  wire  DecoupledStge_11_1_io_out_valid;
  wire [605:0] DecoupledStge_11_1_io_out_bits;
  wire  DecoupledStge_12_1_clock;
  wire  DecoupledStge_12_1_reset;
  wire  DecoupledStge_12_1_io_inp_ready;
  wire  DecoupledStge_12_1_io_inp_valid;
  wire [605:0] DecoupledStge_12_1_io_inp_bits;
  wire  DecoupledStge_12_1_io_out_ready;
  wire  DecoupledStge_12_1_io_out_valid;
  wire [605:0] DecoupledStge_12_1_io_out_bits;
  wire  in_stage_inps_0_ready;
  wire  in_stage_inps_0_valid;
  wire [605:0] in_stage_inps_0_bits;
  wire  in_stage_inps_1_ready;
  wire  in_stage_inps_1_valid;
  wire [605:0] in_stage_inps_1_bits;
  wire  arb_clock;
  wire  arb_reset;
  wire  arb_io_in_0_ready;
  wire  arb_io_in_0_valid;
  wire [605:0] arb_io_in_0_bits;
  wire  arb_io_in_1_ready;
  wire  arb_io_in_1_valid;
  wire [605:0] arb_io_in_1_bits;
  wire  arb_io_out_ready;
  wire  arb_io_out_valid;
  wire [605:0] arb_io_out_bits;
  wire  arb_io_chosen;
  wire  stage_out_clock;
  wire  stage_out_reset;
  wire  stage_out_io_inp_ready;
  wire  stage_out_io_inp_valid;
  wire [605:0] stage_out_io_inp_bits;
  wire  stage_out_io_out_ready;
  wire  stage_out_io_out_valid;
  wire [605:0] stage_out_io_out_bits;
  DecoupledStge_11 DecoupledStge_11_1 (
    .clock(DecoupledStge_11_1_clock),
    .reset(DecoupledStge_11_1_reset),
    .io_inp_ready(DecoupledStge_11_1_io_inp_ready),
    .io_inp_valid(DecoupledStge_11_1_io_inp_valid),
    .io_inp_bits(DecoupledStge_11_1_io_inp_bits),
    .io_out_ready(DecoupledStge_11_1_io_out_ready),
    .io_out_valid(DecoupledStge_11_1_io_out_valid),
    .io_out_bits(DecoupledStge_11_1_io_out_bits)
  );
  DecoupledStge_11 DecoupledStge_12_1 (
    .clock(DecoupledStge_12_1_clock),
    .reset(DecoupledStge_12_1_reset),
    .io_inp_ready(DecoupledStge_12_1_io_inp_ready),
    .io_inp_valid(DecoupledStge_12_1_io_inp_valid),
    .io_inp_bits(DecoupledStge_12_1_io_inp_bits),
    .io_out_ready(DecoupledStge_12_1_io_out_ready),
    .io_out_valid(DecoupledStge_12_1_io_out_valid),
    .io_out_bits(DecoupledStge_12_1_io_out_bits)
  );
  RRArbiter_1 arb (
    .clock(arb_clock),
    .reset(arb_reset),
    .io_in_0_ready(arb_io_in_0_ready),
    .io_in_0_valid(arb_io_in_0_valid),
    .io_in_0_bits(arb_io_in_0_bits),
    .io_in_1_ready(arb_io_in_1_ready),
    .io_in_1_valid(arb_io_in_1_valid),
    .io_in_1_bits(arb_io_in_1_bits),
    .io_out_ready(arb_io_out_ready),
    .io_out_valid(arb_io_out_valid),
    .io_out_bits(arb_io_out_bits),
    .io_chosen(arb_io_chosen)
  );
  DecoupledStge_11 stage_out (
    .clock(stage_out_clock),
    .reset(stage_out_reset),
    .io_inp_ready(stage_out_io_inp_ready),
    .io_inp_valid(stage_out_io_inp_valid),
    .io_inp_bits(stage_out_io_inp_bits),
    .io_out_ready(stage_out_io_out_ready),
    .io_out_valid(stage_out_io_out_valid),
    .io_out_bits(stage_out_io_out_bits)
  );
  assign io_qin_0_ready = in_stage_inps_0_ready;
  assign io_qin_1_ready = in_stage_inps_1_ready;
  assign io_qout_valid = stage_out_io_out_valid;
  assign io_qout_bits = stage_out_io_out_bits;
  assign DecoupledStge_11_1_clock = clock;
  assign DecoupledStge_11_1_reset = reset;
  assign DecoupledStge_11_1_io_inp_valid = in_stage_inps_0_valid;
  assign DecoupledStge_11_1_io_inp_bits = in_stage_inps_0_bits;
  assign DecoupledStge_11_1_io_out_ready = arb_io_in_0_ready;
  assign DecoupledStge_12_1_clock = clock;
  assign DecoupledStge_12_1_reset = reset;
  assign DecoupledStge_12_1_io_inp_valid = in_stage_inps_1_valid;
  assign DecoupledStge_12_1_io_inp_bits = in_stage_inps_1_bits;
  assign DecoupledStge_12_1_io_out_ready = arb_io_in_1_ready;
  assign in_stage_inps_0_ready = DecoupledStge_11_1_io_inp_ready;
  assign in_stage_inps_0_valid = io_qin_0_valid;
  assign in_stage_inps_0_bits = io_qin_0_bits;
  assign in_stage_inps_1_ready = DecoupledStge_12_1_io_inp_ready;
  assign in_stage_inps_1_valid = io_qin_1_valid;
  assign in_stage_inps_1_bits = io_qin_1_bits;
  assign arb_clock = clock;
  assign arb_reset = reset;
  assign arb_io_in_0_valid = DecoupledStge_11_1_io_out_valid;
  assign arb_io_in_0_bits = DecoupledStge_11_1_io_out_bits;
  assign arb_io_in_1_valid = DecoupledStge_12_1_io_out_valid;
  assign arb_io_in_1_bits = DecoupledStge_12_1_io_out_bits;
  assign arb_io_out_ready = stage_out_io_inp_ready;
  assign stage_out_clock = clock;
  assign stage_out_reset = reset;
  assign stage_out_io_inp_valid = arb_io_out_valid;
  assign stage_out_io_inp_bits = arb_io_out_bits;
  assign stage_out_io_out_ready = io_qout_ready;
endmodule
module DecoupledStge_14(
  input   clock,
  input   reset,
  output  io_inp_ready,
  input   io_inp_valid,
  input  [16:0] io_inp_bits,
  input   io_out_ready,
  output  io_out_valid,
  output [16:0] io_out_bits
);
  reg  out_valid;
  reg [31:0] GEN_1;
  reg [16:0] out_bits;
  reg [31:0] GEN_2;
  wire  T_24;
  wire  T_25;
  wire  T_26;
  wire  T_27;
  wire [16:0] GEN_0;
  assign io_inp_ready = T_25;
  assign io_out_valid = out_valid;
  assign io_out_bits = out_bits;
  assign T_24 = ~ io_out_valid;
  assign T_25 = io_out_ready | T_24;
  assign T_26 = ~ io_inp_ready;
  assign T_27 = io_inp_valid | T_26;
  assign GEN_0 = io_inp_ready ? io_inp_bits : out_bits;
`ifdef RANDOMIZE
  integer initvar;
  initial begin
    `ifndef verilator
      #0.002 begin end
    `endif
  `ifdef RANDOMIZE_REG_INIT
  GEN_1 = {1{$random}};
  out_valid = GEN_1[0:0];
  `endif
  `ifdef RANDOMIZE_REG_INIT
  GEN_2 = {1{$random}};
  out_bits = GEN_2[16:0];
  `endif
  end
`endif
  always @(posedge clock) begin
    if(reset) begin
      out_valid <= 1'h0;
    end else begin
      out_valid <= T_27;
    end
    if(1'h0) begin
    end else begin
      if(io_inp_ready) begin
        out_bits <= io_inp_bits;
      end
    end
  end
endmodule
module SteerReged_4(
  input   clock,
  input   reset,
  output  io_qin_ready,
  input   io_qin_valid,
  input  [16:0] io_qin_bits,
  input   io_qout_0_ready,
  output  io_qout_0_valid,
  output [16:0] io_qout_0_bits,
  input   io_qout_1_ready,
  output  io_qout_1_valid,
  output [16:0] io_qout_1_bits
);
  wire  DecoupledStge_14_1_clock;
  wire  DecoupledStge_14_1_reset;
  wire  DecoupledStge_14_1_io_inp_ready;
  wire  DecoupledStge_14_1_io_inp_valid;
  wire [16:0] DecoupledStge_14_1_io_inp_bits;
  wire  DecoupledStge_14_1_io_out_ready;
  wire  DecoupledStge_14_1_io_out_valid;
  wire [16:0] DecoupledStge_14_1_io_out_bits;
  wire  DecoupledStge_15_1_clock;
  wire  DecoupledStge_15_1_reset;
  wire  DecoupledStge_15_1_io_inp_ready;
  wire  DecoupledStge_15_1_io_inp_valid;
  wire [16:0] DecoupledStge_15_1_io_inp_bits;
  wire  DecoupledStge_15_1_io_out_ready;
  wire  DecoupledStge_15_1_io_out_valid;
  wire [16:0] DecoupledStge_15_1_io_out_bits;
  wire  out_stage_inps_0_ready;
  wire  out_stage_inps_0_valid;
  wire [16:0] out_stage_inps_0_bits;
  wire  out_stage_inps_1_ready;
  wire  out_stage_inps_1_valid;
  wire [16:0] out_stage_inps_1_bits;
  wire [16:0] T_93;
  wire [16:0] T_97;
  reg [16:0] pendData;
  reg [31:0] GEN_0;
  reg  pendValid;
  reg [31:0] GEN_3;
  wire  pendAuId;
  wire  GEN_0_ready;
  wire  GEN_0_valid;
  wire [16:0] GEN_0_bits;
  wire  GEN_4;
  wire  GEN_5;
  wire [16:0] GEN_6;
  wire  T_105;
  wire  GEN_1;
  wire  GEN_7;
  wire  GEN_8;
  wire [16:0] GEN_2;
  wire [16:0] GEN_9;
  wire [16:0] GEN_10;
  wire  GEN_13;
  wire [16:0] GEN_16;
  wire [16:0] GEN_15;
  wire  GEN_12;
  wire  GEN_3_ready;
  wire  GEN_3_valid;
  wire [16:0] GEN_3_bits;
  wire  T_119;
  wire  T_121;
  wire  T_122;
  wire [16:0] T_124;
  wire [16:0] GEN_17;
  wire  GEN_18;
  wire [16:0] GEN_21;
  DecoupledStge_14 DecoupledStge_14_1 (
    .clock(DecoupledStge_14_1_clock),
    .reset(DecoupledStge_14_1_reset),
    .io_inp_ready(DecoupledStge_14_1_io_inp_ready),
    .io_inp_valid(DecoupledStge_14_1_io_inp_valid),
    .io_inp_bits(DecoupledStge_14_1_io_inp_bits),
    .io_out_ready(DecoupledStge_14_1_io_out_ready),
    .io_out_valid(DecoupledStge_14_1_io_out_valid),
    .io_out_bits(DecoupledStge_14_1_io_out_bits)
  );
  DecoupledStge_14 DecoupledStge_15_1 (
    .clock(DecoupledStge_15_1_clock),
    .reset(DecoupledStge_15_1_reset),
    .io_inp_ready(DecoupledStge_15_1_io_inp_ready),
    .io_inp_valid(DecoupledStge_15_1_io_inp_valid),
    .io_inp_bits(DecoupledStge_15_1_io_inp_bits),
    .io_out_ready(DecoupledStge_15_1_io_out_ready),
    .io_out_valid(DecoupledStge_15_1_io_out_valid),
    .io_out_bits(DecoupledStge_15_1_io_out_bits)
  );
  assign io_qin_ready = T_122;
  assign io_qout_0_valid = DecoupledStge_14_1_io_out_valid;
  assign io_qout_0_bits = DecoupledStge_14_1_io_out_bits;
  assign io_qout_1_valid = DecoupledStge_15_1_io_out_valid;
  assign io_qout_1_bits = DecoupledStge_15_1_io_out_bits;
  assign DecoupledStge_14_1_clock = clock;
  assign DecoupledStge_14_1_reset = reset;
  assign DecoupledStge_14_1_io_inp_valid = out_stage_inps_0_valid;
  assign DecoupledStge_14_1_io_inp_bits = out_stage_inps_0_bits;
  assign DecoupledStge_14_1_io_out_ready = io_qout_0_ready;
  assign DecoupledStge_15_1_clock = clock;
  assign DecoupledStge_15_1_reset = reset;
  assign DecoupledStge_15_1_io_inp_valid = out_stage_inps_1_valid;
  assign DecoupledStge_15_1_io_inp_bits = out_stage_inps_1_bits;
  assign DecoupledStge_15_1_io_out_ready = io_qout_1_ready;
  assign out_stage_inps_0_ready = DecoupledStge_14_1_io_inp_ready;
  assign out_stage_inps_0_valid = GEN_12;
  assign out_stage_inps_0_bits = GEN_15;
  assign out_stage_inps_1_ready = DecoupledStge_15_1_io_inp_ready;
  assign out_stage_inps_1_valid = GEN_13;
  assign out_stage_inps_1_bits = GEN_16;
  assign T_93 = 17'h0;
  assign T_97 = 17'h0;
  assign pendAuId = pendData[7];
  assign GEN_0_ready = GEN_4;
  assign GEN_0_valid = GEN_5;
  assign GEN_0_bits = GEN_6;
  assign GEN_4 = pendAuId ? out_stage_inps_1_ready : out_stage_inps_0_ready;
  assign GEN_5 = pendAuId ? out_stage_inps_1_valid : out_stage_inps_0_valid;
  assign GEN_6 = pendAuId ? out_stage_inps_1_bits : out_stage_inps_0_bits;
  assign T_105 = pendValid & GEN_0_ready;
  assign GEN_1 = 1'h1;
  assign GEN_7 = 1'h0 == pendAuId ? GEN_1 : 1'h0;
  assign GEN_8 = pendAuId ? GEN_1 : 1'h0;
  assign GEN_2 = pendData;
  assign GEN_9 = 1'h0 == pendAuId ? GEN_2 : T_93;
  assign GEN_10 = pendAuId ? GEN_2 : T_97;
  assign GEN_13 = T_105 ? GEN_8 : 1'h0;
  assign GEN_16 = T_105 ? GEN_10 : T_97;
  assign GEN_15 = T_105 ? GEN_9 : T_93;
  assign GEN_12 = T_105 ? GEN_7 : 1'h0;
  assign GEN_3_ready = GEN_4;
  assign GEN_3_valid = GEN_5;
  assign GEN_3_bits = GEN_6;
  assign T_119 = pendValid & GEN_3_ready;
  assign T_121 = pendValid == 1'h0;
  assign T_122 = T_119 | T_121;
  assign T_124 = io_qin_bits;
  assign GEN_17 = io_qin_valid ? T_124 : pendData;
  assign GEN_18 = T_122 ? io_qin_valid : pendValid;
  assign GEN_21 = T_122 ? GEN_17 : pendData;
`ifdef RANDOMIZE
  integer initvar;
  initial begin
    `ifndef verilator
      #0.002 begin end
    `endif
  `ifdef RANDOMIZE_REG_INIT
  GEN_0 = {1{$random}};
  pendData = GEN_0[16:0];
  `endif
  `ifdef RANDOMIZE_REG_INIT
  GEN_3 = {1{$random}};
  pendValid = GEN_3[0:0];
  `endif
  end
`endif
  always @(posedge clock) begin
    if(1'h0) begin
    end else begin
      if(T_122) begin
        if(io_qin_valid) begin
          pendData <= T_124;
        end
      end
    end
    if(reset) begin
      pendValid <= 1'h0;
    end else begin
      if(T_122) begin
        pendValid <= io_qin_valid;
      end
    end
  end
endmodule
module MultiChannelMux(
  input   clock,
  input   reset,
  output  io_acc_rd_req_in_0_ready,
  input   io_acc_rd_req_in_0_valid,
  input  [79:0] io_acc_rd_req_in_0_bits,
  output  io_acc_rd_req_in_1_ready,
  input   io_acc_rd_req_in_1_valid,
  input  [79:0] io_acc_rd_req_in_1_bits,
  input   io_mem_rd_req_out_ready,
  output  io_mem_rd_req_out_valid,
  output [79:0] io_mem_rd_req_out_bits,
  output  io_mem_rd_resp_in_ready,
  input   io_mem_rd_resp_in_valid,
  input  [527:0] io_mem_rd_resp_in_bits,
  input   io_acc_rd_resp_out_0_ready,
  output  io_acc_rd_resp_out_0_valid,
  output [527:0] io_acc_rd_resp_out_0_bits,
  input   io_acc_rd_resp_out_1_ready,
  output  io_acc_rd_resp_out_1_valid,
  output [527:0] io_acc_rd_resp_out_1_bits,
  output  io_acc_wr_req_in_0_ready,
  input   io_acc_wr_req_in_0_valid,
  input  [605:0] io_acc_wr_req_in_0_bits,
  output  io_acc_wr_req_in_1_ready,
  input   io_acc_wr_req_in_1_valid,
  input  [605:0] io_acc_wr_req_in_1_bits,
  input   io_mem_wr_req_out_ready,
  output  io_mem_wr_req_out_valid,
  output [605:0] io_mem_wr_req_out_bits,
  output  io_mem_wr_resp_in_ready,
  input   io_mem_wr_resp_in_valid,
  input  [16:0] io_mem_wr_resp_in_bits,
  input   io_acc_wr_resp_out_0_ready,
  output  io_acc_wr_resp_out_0_valid,
  output [16:0] io_acc_wr_resp_out_0_bits,
  input   io_acc_wr_resp_out_1_ready,
  output  io_acc_wr_resp_out_1_valid,
  output [16:0] io_acc_wr_resp_out_1_bits
);
  wire  arbiter_read_clock;
  wire  arbiter_read_reset;
  wire  arbiter_read_io_qin_0_ready;
  wire  arbiter_read_io_qin_0_valid;
  wire [79:0] arbiter_read_io_qin_0_bits;
  wire  arbiter_read_io_qin_1_ready;
  wire  arbiter_read_io_qin_1_valid;
  wire [79:0] arbiter_read_io_qin_1_bits;
  wire  arbiter_read_io_qout_ready;
  wire  arbiter_read_io_qout_valid;
  wire [79:0] arbiter_read_io_qout_bits;
  wire  steer_read_clock;
  wire  steer_read_reset;
  wire  steer_read_io_qin_ready;
  wire  steer_read_io_qin_valid;
  wire [527:0] steer_read_io_qin_bits;
  wire  steer_read_io_qout_0_ready;
  wire  steer_read_io_qout_0_valid;
  wire [527:0] steer_read_io_qout_0_bits;
  wire  steer_read_io_qout_1_ready;
  wire  steer_read_io_qout_1_valid;
  wire [527:0] steer_read_io_qout_1_bits;
  wire  arbiter_write_clock;
  wire  arbiter_write_reset;
  wire  arbiter_write_io_qin_0_ready;
  wire  arbiter_write_io_qin_0_valid;
  wire [605:0] arbiter_write_io_qin_0_bits;
  wire  arbiter_write_io_qin_1_ready;
  wire  arbiter_write_io_qin_1_valid;
  wire [605:0] arbiter_write_io_qin_1_bits;
  wire  arbiter_write_io_qout_ready;
  wire  arbiter_write_io_qout_valid;
  wire [605:0] arbiter_write_io_qout_bits;
  wire  steer_write_clock;
  wire  steer_write_reset;
  wire  steer_write_io_qin_ready;
  wire  steer_write_io_qin_valid;
  wire [16:0] steer_write_io_qin_bits;
  wire  steer_write_io_qout_0_ready;
  wire  steer_write_io_qout_0_valid;
  wire [16:0] steer_write_io_qout_0_bits;
  wire  steer_write_io_qout_1_ready;
  wire  steer_write_io_qout_1_valid;
  wire [16:0] steer_write_io_qout_1_bits;
  ArbiterReged arbiter_read (
    .clock(arbiter_read_clock),
    .reset(arbiter_read_reset),
    .io_qin_0_ready(arbiter_read_io_qin_0_ready),
    .io_qin_0_valid(arbiter_read_io_qin_0_valid),
    .io_qin_0_bits(arbiter_read_io_qin_0_bits),
    .io_qin_1_ready(arbiter_read_io_qin_1_ready),
    .io_qin_1_valid(arbiter_read_io_qin_1_valid),
    .io_qin_1_bits(arbiter_read_io_qin_1_bits),
    .io_qout_ready(arbiter_read_io_qout_ready),
    .io_qout_valid(arbiter_read_io_qout_valid),
    .io_qout_bits(arbiter_read_io_qout_bits)
  );
  SteerRegedSplit steer_read (
    .clock(steer_read_clock),
    .reset(steer_read_reset),
    .io_qin_ready(steer_read_io_qin_ready),
    .io_qin_valid(steer_read_io_qin_valid),
    .io_qin_bits(steer_read_io_qin_bits),
    .io_qout_0_ready(steer_read_io_qout_0_ready),
    .io_qout_0_valid(steer_read_io_qout_0_valid),
    .io_qout_0_bits(steer_read_io_qout_0_bits),
    .io_qout_1_ready(steer_read_io_qout_1_ready),
    .io_qout_1_valid(steer_read_io_qout_1_valid),
    .io_qout_1_bits(steer_read_io_qout_1_bits)
  );
  ArbiterReged_1 arbiter_write (
    .clock(arbiter_write_clock),
    .reset(arbiter_write_reset),
    .io_qin_0_ready(arbiter_write_io_qin_0_ready),
    .io_qin_0_valid(arbiter_write_io_qin_0_valid),
    .io_qin_0_bits(arbiter_write_io_qin_0_bits),
    .io_qin_1_ready(arbiter_write_io_qin_1_ready),
    .io_qin_1_valid(arbiter_write_io_qin_1_valid),
    .io_qin_1_bits(arbiter_write_io_qin_1_bits),
    .io_qout_ready(arbiter_write_io_qout_ready),
    .io_qout_valid(arbiter_write_io_qout_valid),
    .io_qout_bits(arbiter_write_io_qout_bits)
  );
  SteerReged_4 steer_write (
    .clock(steer_write_clock),
    .reset(steer_write_reset),
    .io_qin_ready(steer_write_io_qin_ready),
    .io_qin_valid(steer_write_io_qin_valid),
    .io_qin_bits(steer_write_io_qin_bits),
    .io_qout_0_ready(steer_write_io_qout_0_ready),
    .io_qout_0_valid(steer_write_io_qout_0_valid),
    .io_qout_0_bits(steer_write_io_qout_0_bits),
    .io_qout_1_ready(steer_write_io_qout_1_ready),
    .io_qout_1_valid(steer_write_io_qout_1_valid),
    .io_qout_1_bits(steer_write_io_qout_1_bits)
  );
  assign io_acc_rd_req_in_0_ready = arbiter_read_io_qin_0_ready;
  assign io_acc_rd_req_in_1_ready = arbiter_read_io_qin_1_ready;
  assign io_mem_rd_req_out_valid = arbiter_read_io_qout_valid;
  assign io_mem_rd_req_out_bits = arbiter_read_io_qout_bits;
  assign io_mem_rd_resp_in_ready = steer_read_io_qin_ready;
  assign io_acc_rd_resp_out_0_valid = steer_read_io_qout_0_valid;
  assign io_acc_rd_resp_out_0_bits = steer_read_io_qout_0_bits;
  assign io_acc_rd_resp_out_1_valid = steer_read_io_qout_1_valid;
  assign io_acc_rd_resp_out_1_bits = steer_read_io_qout_1_bits;
  assign io_acc_wr_req_in_0_ready = arbiter_write_io_qin_0_ready;
  assign io_acc_wr_req_in_1_ready = arbiter_write_io_qin_1_ready;
  assign io_mem_wr_req_out_valid = arbiter_write_io_qout_valid;
  assign io_mem_wr_req_out_bits = arbiter_write_io_qout_bits;
  assign io_mem_wr_resp_in_ready = steer_write_io_qin_ready;
  assign io_acc_wr_resp_out_0_valid = steer_write_io_qout_0_valid;
  assign io_acc_wr_resp_out_0_bits = steer_write_io_qout_0_bits;
  assign io_acc_wr_resp_out_1_valid = steer_write_io_qout_1_valid;
  assign io_acc_wr_resp_out_1_bits = steer_write_io_qout_1_bits;
  assign arbiter_read_clock = clock;
  assign arbiter_read_reset = reset;
  assign arbiter_read_io_qin_0_valid = io_acc_rd_req_in_0_valid;
  assign arbiter_read_io_qin_0_bits = io_acc_rd_req_in_0_bits;
  assign arbiter_read_io_qin_1_valid = io_acc_rd_req_in_1_valid;
  assign arbiter_read_io_qin_1_bits = io_acc_rd_req_in_1_bits;
  assign arbiter_read_io_qout_ready = io_mem_rd_req_out_ready;
  assign steer_read_clock = clock;
  assign steer_read_reset = reset;
  assign steer_read_io_qin_valid = io_mem_rd_resp_in_valid;
  assign steer_read_io_qin_bits = io_mem_rd_resp_in_bits;
  assign steer_read_io_qout_0_ready = io_acc_rd_resp_out_0_ready;
  assign steer_read_io_qout_1_ready = io_acc_rd_resp_out_1_ready;
  assign arbiter_write_clock = clock;
  assign arbiter_write_reset = reset;
  assign arbiter_write_io_qin_0_valid = io_acc_wr_req_in_0_valid;
  assign arbiter_write_io_qin_0_bits = io_acc_wr_req_in_0_bits;
  assign arbiter_write_io_qin_1_valid = io_acc_wr_req_in_1_valid;
  assign arbiter_write_io_qin_1_bits = io_acc_wr_req_in_1_bits;
  assign arbiter_write_io_qout_ready = io_mem_wr_req_out_ready;
  assign steer_write_clock = clock;
  assign steer_write_reset = reset;
  assign steer_write_io_qin_valid = io_mem_wr_resp_in_valid;
  assign steer_write_io_qin_bits = io_mem_wr_resp_in_bits;
  assign steer_write_io_qout_0_ready = io_acc_wr_resp_out_0_ready;
  assign steer_write_io_qout_1_ready = io_acc_wr_resp_out_1_ready;
endmodule
