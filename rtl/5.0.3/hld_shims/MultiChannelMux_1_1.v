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
  input   io_out_ready,
  output  io_out_valid,
  output [79:0] io_out_bits,
  output  io_chosen
);
  wire  choice;
  wire  T_63;
  reg  lastGrant;
  reg [31:0] GEN_1;
  wire  GEN_0;
  assign io_in_0_ready = io_out_ready;
  assign io_out_valid = io_in_0_valid;
  assign io_out_bits = io_in_0_bits;
  assign io_chosen = choice;
  assign choice = 1'h0;
  assign T_63 = io_out_ready & io_out_valid;
  assign GEN_0 = T_63 ? io_chosen : lastGrant;
`ifdef RANDOMIZE
  integer initvar;
  initial begin
    `ifndef verilator
      #0.002 begin end
    `endif
  `ifdef RANDOMIZE_REG_INIT
  GEN_1 = {1{$random}};
  lastGrant = GEN_1[0:0];
  `endif
  end
`endif
  always @(posedge clock) begin
    if(reset) begin
      lastGrant <= 1'h0;
    end else begin
      if(T_63) begin
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
  input   io_qout_ready,
  output  io_qout_valid,
  output [79:0] io_qout_bits
);
  wire  DecoupledStge_2_clock;
  wire  DecoupledStge_2_reset;
  wire  DecoupledStge_2_io_inp_ready;
  wire  DecoupledStge_2_io_inp_valid;
  wire [79:0] DecoupledStge_2_io_inp_bits;
  wire  DecoupledStge_2_io_out_ready;
  wire  DecoupledStge_2_io_out_valid;
  wire [79:0] DecoupledStge_2_io_out_bits;
  wire  in_stage_inps_0_ready;
  wire  in_stage_inps_0_valid;
  wire [79:0] in_stage_inps_0_bits;
  wire  arb_clock;
  wire  arb_reset;
  wire  arb_io_in_0_ready;
  wire  arb_io_in_0_valid;
  wire [79:0] arb_io_in_0_bits;
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
  DecoupledStge DecoupledStge_2 (
    .clock(DecoupledStge_2_clock),
    .reset(DecoupledStge_2_reset),
    .io_inp_ready(DecoupledStge_2_io_inp_ready),
    .io_inp_valid(DecoupledStge_2_io_inp_valid),
    .io_inp_bits(DecoupledStge_2_io_inp_bits),
    .io_out_ready(DecoupledStge_2_io_out_ready),
    .io_out_valid(DecoupledStge_2_io_out_valid),
    .io_out_bits(DecoupledStge_2_io_out_bits)
  );
  RRArbiter arb (
    .clock(arb_clock),
    .reset(arb_reset),
    .io_in_0_ready(arb_io_in_0_ready),
    .io_in_0_valid(arb_io_in_0_valid),
    .io_in_0_bits(arb_io_in_0_bits),
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
  assign io_qout_valid = stage_out_io_out_valid;
  assign io_qout_bits = stage_out_io_out_bits;
  assign DecoupledStge_2_clock = clock;
  assign DecoupledStge_2_reset = reset;
  assign DecoupledStge_2_io_inp_valid = in_stage_inps_0_valid;
  assign DecoupledStge_2_io_inp_bits = in_stage_inps_0_bits;
  assign DecoupledStge_2_io_out_ready = arb_io_in_0_ready;
  assign in_stage_inps_0_ready = DecoupledStge_2_io_inp_ready;
  assign in_stage_inps_0_valid = io_qin_0_valid;
  assign in_stage_inps_0_bits = io_qin_0_bits;
  assign arb_clock = clock;
  assign arb_reset = reset;
  assign arb_io_in_0_valid = DecoupledStge_2_io_out_valid;
  assign arb_io_in_0_bits = DecoupledStge_2_io_out_bits;
  assign arb_io_out_ready = stage_out_io_inp_ready;
  assign stage_out_clock = clock;
  assign stage_out_reset = reset;
  assign stage_out_io_inp_valid = arb_io_out_valid;
  assign stage_out_io_inp_bits = arb_io_out_bits;
  assign stage_out_io_out_ready = io_qout_ready;
endmodule
module DecoupledStge_2(
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
  output [140:0] io_qout_0_bits
);
  wire  DecoupledStge_2_1_clock;
  wire  DecoupledStge_2_1_reset;
  wire  DecoupledStge_2_1_io_inp_ready;
  wire  DecoupledStge_2_1_io_inp_valid;
  wire [140:0] DecoupledStge_2_1_io_inp_bits;
  wire  DecoupledStge_2_1_io_out_ready;
  wire  DecoupledStge_2_1_io_out_valid;
  wire [140:0] DecoupledStge_2_1_io_out_bits;
  wire  out_stage_inps_0_ready;
  wire  out_stage_inps_0_valid;
  wire [140:0] out_stage_inps_0_bits;
  wire [140:0] T_75;
  reg [140:0] pendData;
  reg [159:0] GEN_0;
  reg  pendValid;
  reg [31:0] GEN_4;
  wire  T_84;
  wire [140:0] GEN_1;
  wire  T_102;
  wire  T_103;
  wire [140:0] T_105;
  wire [140:0] GEN_2;
  wire  GEN_3;
  wire [140:0] GEN_6;
  DecoupledStge_2 DecoupledStge_2_1 (
    .clock(DecoupledStge_2_1_clock),
    .reset(DecoupledStge_2_1_reset),
    .io_inp_ready(DecoupledStge_2_1_io_inp_ready),
    .io_inp_valid(DecoupledStge_2_1_io_inp_valid),
    .io_inp_bits(DecoupledStge_2_1_io_inp_bits),
    .io_out_ready(DecoupledStge_2_1_io_out_ready),
    .io_out_valid(DecoupledStge_2_1_io_out_valid),
    .io_out_bits(DecoupledStge_2_1_io_out_bits)
  );
  assign io_qin_ready = T_103;
  assign io_qout_0_valid = DecoupledStge_2_1_io_out_valid;
  assign io_qout_0_bits = DecoupledStge_2_1_io_out_bits;
  assign DecoupledStge_2_1_clock = clock;
  assign DecoupledStge_2_1_reset = reset;
  assign DecoupledStge_2_1_io_inp_valid = out_stage_inps_0_valid;
  assign DecoupledStge_2_1_io_inp_bits = out_stage_inps_0_bits;
  assign DecoupledStge_2_1_io_out_ready = io_qout_0_ready;
  assign out_stage_inps_0_ready = DecoupledStge_2_1_io_inp_ready;
  assign out_stage_inps_0_valid = T_84;
  assign out_stage_inps_0_bits = GEN_1;
  assign T_75 = 141'h0;
  assign T_84 = pendValid & out_stage_inps_0_ready;
  assign GEN_1 = T_84 ? pendData : T_75;
  assign T_102 = pendValid == 1'h0;
  assign T_103 = T_84 | T_102;
  assign T_105 = io_qin_bits;
  assign GEN_2 = io_qin_valid ? T_105 : pendData;
  assign GEN_3 = T_103 ? io_qin_valid : pendValid;
  assign GEN_6 = T_103 ? GEN_2 : pendData;
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
  GEN_4 = {1{$random}};
  pendValid = GEN_4[0:0];
  `endif
  end
`endif
  always @(posedge clock) begin
    if(1'h0) begin
    end else begin
      if(T_103) begin
        if(io_qin_valid) begin
          pendData <= T_105;
        end
      end
    end
    if(reset) begin
      pendValid <= 1'h0;
    end else begin
      if(T_103) begin
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
  output [140:0] io_qout_0_bits
);
  wire  DecoupledStge_3_1_clock;
  wire  DecoupledStge_3_1_reset;
  wire  DecoupledStge_3_1_io_inp_ready;
  wire  DecoupledStge_3_1_io_inp_valid;
  wire [140:0] DecoupledStge_3_1_io_inp_bits;
  wire  DecoupledStge_3_1_io_out_ready;
  wire  DecoupledStge_3_1_io_out_valid;
  wire [140:0] DecoupledStge_3_1_io_out_bits;
  wire  out_stage_inps_0_ready;
  wire  out_stage_inps_0_valid;
  wire [140:0] out_stage_inps_0_bits;
  wire [140:0] T_75;
  reg [140:0] pendData;
  reg [159:0] GEN_0;
  reg  pendValid;
  reg [31:0] GEN_4;
  wire  T_84;
  wire [140:0] GEN_1;
  wire  T_102;
  wire  T_103;
  wire [140:0] T_105;
  wire [140:0] GEN_2;
  wire  GEN_3;
  wire [140:0] GEN_6;
  DecoupledStge_2 DecoupledStge_3_1 (
    .clock(DecoupledStge_3_1_clock),
    .reset(DecoupledStge_3_1_reset),
    .io_inp_ready(DecoupledStge_3_1_io_inp_ready),
    .io_inp_valid(DecoupledStge_3_1_io_inp_valid),
    .io_inp_bits(DecoupledStge_3_1_io_inp_bits),
    .io_out_ready(DecoupledStge_3_1_io_out_ready),
    .io_out_valid(DecoupledStge_3_1_io_out_valid),
    .io_out_bits(DecoupledStge_3_1_io_out_bits)
  );
  assign io_qin_ready = T_103;
  assign io_qout_0_valid = DecoupledStge_3_1_io_out_valid;
  assign io_qout_0_bits = DecoupledStge_3_1_io_out_bits;
  assign DecoupledStge_3_1_clock = clock;
  assign DecoupledStge_3_1_reset = reset;
  assign DecoupledStge_3_1_io_inp_valid = out_stage_inps_0_valid;
  assign DecoupledStge_3_1_io_inp_bits = out_stage_inps_0_bits;
  assign DecoupledStge_3_1_io_out_ready = io_qout_0_ready;
  assign out_stage_inps_0_ready = DecoupledStge_3_1_io_inp_ready;
  assign out_stage_inps_0_valid = T_84;
  assign out_stage_inps_0_bits = GEN_1;
  assign T_75 = 141'h0;
  assign T_84 = pendValid & out_stage_inps_0_ready;
  assign GEN_1 = T_84 ? pendData : T_75;
  assign T_102 = pendValid == 1'h0;
  assign T_103 = T_84 | T_102;
  assign T_105 = io_qin_bits;
  assign GEN_2 = io_qin_valid ? T_105 : pendData;
  assign GEN_3 = T_103 ? io_qin_valid : pendValid;
  assign GEN_6 = T_103 ? GEN_2 : pendData;
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
  GEN_4 = {1{$random}};
  pendValid = GEN_4[0:0];
  `endif
  end
`endif
  always @(posedge clock) begin
    if(1'h0) begin
    end else begin
      if(T_103) begin
        if(io_qin_valid) begin
          pendData <= T_105;
        end
      end
    end
    if(reset) begin
      pendValid <= 1'h0;
    end else begin
      if(T_103) begin
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
  output [140:0] io_qout_0_bits
);
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
  wire [140:0] T_75;
  reg [140:0] pendData;
  reg [159:0] GEN_0;
  reg  pendValid;
  reg [31:0] GEN_4;
  wire  T_84;
  wire [140:0] GEN_1;
  wire  T_102;
  wire  T_103;
  wire [140:0] T_105;
  wire [140:0] GEN_2;
  wire  GEN_3;
  wire [140:0] GEN_6;
  DecoupledStge_2 DecoupledStge_4_1 (
    .clock(DecoupledStge_4_1_clock),
    .reset(DecoupledStge_4_1_reset),
    .io_inp_ready(DecoupledStge_4_1_io_inp_ready),
    .io_inp_valid(DecoupledStge_4_1_io_inp_valid),
    .io_inp_bits(DecoupledStge_4_1_io_inp_bits),
    .io_out_ready(DecoupledStge_4_1_io_out_ready),
    .io_out_valid(DecoupledStge_4_1_io_out_valid),
    .io_out_bits(DecoupledStge_4_1_io_out_bits)
  );
  assign io_qin_ready = T_103;
  assign io_qout_0_valid = DecoupledStge_4_1_io_out_valid;
  assign io_qout_0_bits = DecoupledStge_4_1_io_out_bits;
  assign DecoupledStge_4_1_clock = clock;
  assign DecoupledStge_4_1_reset = reset;
  assign DecoupledStge_4_1_io_inp_valid = out_stage_inps_0_valid;
  assign DecoupledStge_4_1_io_inp_bits = out_stage_inps_0_bits;
  assign DecoupledStge_4_1_io_out_ready = io_qout_0_ready;
  assign out_stage_inps_0_ready = DecoupledStge_4_1_io_inp_ready;
  assign out_stage_inps_0_valid = T_84;
  assign out_stage_inps_0_bits = GEN_1;
  assign T_75 = 141'h0;
  assign T_84 = pendValid & out_stage_inps_0_ready;
  assign GEN_1 = T_84 ? pendData : T_75;
  assign T_102 = pendValid == 1'h0;
  assign T_103 = T_84 | T_102;
  assign T_105 = io_qin_bits;
  assign GEN_2 = io_qin_valid ? T_105 : pendData;
  assign GEN_3 = T_103 ? io_qin_valid : pendValid;
  assign GEN_6 = T_103 ? GEN_2 : pendData;
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
  GEN_4 = {1{$random}};
  pendValid = GEN_4[0:0];
  `endif
  end
`endif
  always @(posedge clock) begin
    if(1'h0) begin
    end else begin
      if(T_103) begin
        if(io_qin_valid) begin
          pendData <= T_105;
        end
      end
    end
    if(reset) begin
      pendValid <= 1'h0;
    end else begin
      if(T_103) begin
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
  output [140:0] io_qout_0_bits
);
  wire  DecoupledStge_5_1_clock;
  wire  DecoupledStge_5_1_reset;
  wire  DecoupledStge_5_1_io_inp_ready;
  wire  DecoupledStge_5_1_io_inp_valid;
  wire [140:0] DecoupledStge_5_1_io_inp_bits;
  wire  DecoupledStge_5_1_io_out_ready;
  wire  DecoupledStge_5_1_io_out_valid;
  wire [140:0] DecoupledStge_5_1_io_out_bits;
  wire  out_stage_inps_0_ready;
  wire  out_stage_inps_0_valid;
  wire [140:0] out_stage_inps_0_bits;
  wire [140:0] T_75;
  reg [140:0] pendData;
  reg [159:0] GEN_0;
  reg  pendValid;
  reg [31:0] GEN_4;
  wire  T_84;
  wire [140:0] GEN_1;
  wire  T_102;
  wire  T_103;
  wire [140:0] T_105;
  wire [140:0] GEN_2;
  wire  GEN_3;
  wire [140:0] GEN_6;
  DecoupledStge_2 DecoupledStge_5_1 (
    .clock(DecoupledStge_5_1_clock),
    .reset(DecoupledStge_5_1_reset),
    .io_inp_ready(DecoupledStge_5_1_io_inp_ready),
    .io_inp_valid(DecoupledStge_5_1_io_inp_valid),
    .io_inp_bits(DecoupledStge_5_1_io_inp_bits),
    .io_out_ready(DecoupledStge_5_1_io_out_ready),
    .io_out_valid(DecoupledStge_5_1_io_out_valid),
    .io_out_bits(DecoupledStge_5_1_io_out_bits)
  );
  assign io_qin_ready = T_103;
  assign io_qout_0_valid = DecoupledStge_5_1_io_out_valid;
  assign io_qout_0_bits = DecoupledStge_5_1_io_out_bits;
  assign DecoupledStge_5_1_clock = clock;
  assign DecoupledStge_5_1_reset = reset;
  assign DecoupledStge_5_1_io_inp_valid = out_stage_inps_0_valid;
  assign DecoupledStge_5_1_io_inp_bits = out_stage_inps_0_bits;
  assign DecoupledStge_5_1_io_out_ready = io_qout_0_ready;
  assign out_stage_inps_0_ready = DecoupledStge_5_1_io_inp_ready;
  assign out_stage_inps_0_valid = T_84;
  assign out_stage_inps_0_bits = GEN_1;
  assign T_75 = 141'h0;
  assign T_84 = pendValid & out_stage_inps_0_ready;
  assign GEN_1 = T_84 ? pendData : T_75;
  assign T_102 = pendValid == 1'h0;
  assign T_103 = T_84 | T_102;
  assign T_105 = io_qin_bits;
  assign GEN_2 = io_qin_valid ? T_105 : pendData;
  assign GEN_3 = T_103 ? io_qin_valid : pendValid;
  assign GEN_6 = T_103 ? GEN_2 : pendData;
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
  GEN_4 = {1{$random}};
  pendValid = GEN_4[0:0];
  `endif
  end
`endif
  always @(posedge clock) begin
    if(1'h0) begin
    end else begin
      if(T_103) begin
        if(io_qin_valid) begin
          pendData <= T_105;
        end
      end
    end
    if(reset) begin
      pendValid <= 1'h0;
    end else begin
      if(T_103) begin
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
  output [527:0] io_qout_0_bits
);
  wire  SteerReged_4_clock;
  wire  SteerReged_4_reset;
  wire  SteerReged_4_io_qin_ready;
  wire  SteerReged_4_io_qin_valid;
  wire [140:0] SteerReged_4_io_qin_bits;
  wire  SteerReged_4_io_qout_0_ready;
  wire  SteerReged_4_io_qout_0_valid;
  wire [140:0] SteerReged_4_io_qout_0_bits;
  wire  SteerReged_1_1_clock;
  wire  SteerReged_1_1_reset;
  wire  SteerReged_1_1_io_qin_ready;
  wire  SteerReged_1_1_io_qin_valid;
  wire [140:0] SteerReged_1_1_io_qin_bits;
  wire  SteerReged_1_1_io_qout_0_ready;
  wire  SteerReged_1_1_io_qout_0_valid;
  wire [140:0] SteerReged_1_1_io_qout_0_bits;
  wire  SteerReged_2_1_clock;
  wire  SteerReged_2_1_reset;
  wire  SteerReged_2_1_io_qin_ready;
  wire  SteerReged_2_1_io_qin_valid;
  wire [140:0] SteerReged_2_1_io_qin_bits;
  wire  SteerReged_2_1_io_qout_0_ready;
  wire  SteerReged_2_1_io_qout_0_valid;
  wire [140:0] SteerReged_2_1_io_qout_0_bits;
  wire  SteerReged_3_1_clock;
  wire  SteerReged_3_1_reset;
  wire  SteerReged_3_1_io_qin_ready;
  wire  SteerReged_3_1_io_qin_valid;
  wire [140:0] SteerReged_3_1_io_qin_bits;
  wire  SteerReged_3_1_io_qout_0_ready;
  wire  SteerReged_3_1_io_qout_0_valid;
  wire [140:0] SteerReged_3_1_io_qout_0_bits;
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
  wire  steer_submod_outs_1_0_ready;
  wire  steer_submod_outs_1_0_valid;
  wire [140:0] steer_submod_outs_1_0_bits;
  wire  steer_submod_outs_2_0_ready;
  wire  steer_submod_outs_2_0_valid;
  wire [140:0] steer_submod_outs_2_0_bits;
  wire  steer_submod_outs_3_0_ready;
  wire  steer_submod_outs_3_0_valid;
  wire [140:0] steer_submod_outs_3_0_bits;
  wire [130:0] T_258;
  wire [130:0] T_259;
  wire [9:0] T_260;
  wire [140:0] T_261;
  wire [130:0] T_262;
  wire [130:0] T_263;
  wire [140:0] T_265;
  wire [130:0] T_266;
  wire [130:0] T_267;
  wire [140:0] T_269;
  wire [130:0] T_270;
  wire [130:0] T_271;
  wire [140:0] T_273;
  wire [128:0] T_277;
  wire [128:0] T_278;
  wire [128:0] T_279;
  wire [257:0] T_281;
  wire [386:0] T_282;
  wire [527:0] T_283;
  SteerReged SteerReged_4 (
    .clock(SteerReged_4_clock),
    .reset(SteerReged_4_reset),
    .io_qin_ready(SteerReged_4_io_qin_ready),
    .io_qin_valid(SteerReged_4_io_qin_valid),
    .io_qin_bits(SteerReged_4_io_qin_bits),
    .io_qout_0_ready(SteerReged_4_io_qout_0_ready),
    .io_qout_0_valid(SteerReged_4_io_qout_0_valid),
    .io_qout_0_bits(SteerReged_4_io_qout_0_bits)
  );
  SteerReged_1 SteerReged_1_1 (
    .clock(SteerReged_1_1_clock),
    .reset(SteerReged_1_1_reset),
    .io_qin_ready(SteerReged_1_1_io_qin_ready),
    .io_qin_valid(SteerReged_1_1_io_qin_valid),
    .io_qin_bits(SteerReged_1_1_io_qin_bits),
    .io_qout_0_ready(SteerReged_1_1_io_qout_0_ready),
    .io_qout_0_valid(SteerReged_1_1_io_qout_0_valid),
    .io_qout_0_bits(SteerReged_1_1_io_qout_0_bits)
  );
  SteerReged_2 SteerReged_2_1 (
    .clock(SteerReged_2_1_clock),
    .reset(SteerReged_2_1_reset),
    .io_qin_ready(SteerReged_2_1_io_qin_ready),
    .io_qin_valid(SteerReged_2_1_io_qin_valid),
    .io_qin_bits(SteerReged_2_1_io_qin_bits),
    .io_qout_0_ready(SteerReged_2_1_io_qout_0_ready),
    .io_qout_0_valid(SteerReged_2_1_io_qout_0_valid),
    .io_qout_0_bits(SteerReged_2_1_io_qout_0_bits)
  );
  SteerReged_3 SteerReged_3_1 (
    .clock(SteerReged_3_1_clock),
    .reset(SteerReged_3_1_reset),
    .io_qin_ready(SteerReged_3_1_io_qin_ready),
    .io_qin_valid(SteerReged_3_1_io_qin_valid),
    .io_qin_bits(SteerReged_3_1_io_qin_bits),
    .io_qout_0_ready(SteerReged_3_1_io_qout_0_ready),
    .io_qout_0_valid(SteerReged_3_1_io_qout_0_valid),
    .io_qout_0_bits(SteerReged_3_1_io_qout_0_bits)
  );
  assign io_qin_ready = steer_submod_ins_0_ready;
  assign io_qout_0_valid = steer_submod_outs_0_0_valid;
  assign io_qout_0_bits = T_283;
  assign SteerReged_4_clock = clock;
  assign SteerReged_4_reset = reset;
  assign SteerReged_4_io_qin_valid = steer_submod_ins_0_valid;
  assign SteerReged_4_io_qin_bits = steer_submod_ins_0_bits;
  assign SteerReged_4_io_qout_0_ready = steer_submod_outs_0_0_ready;
  assign SteerReged_1_1_clock = clock;
  assign SteerReged_1_1_reset = reset;
  assign SteerReged_1_1_io_qin_valid = steer_submod_ins_1_valid;
  assign SteerReged_1_1_io_qin_bits = steer_submod_ins_1_bits;
  assign SteerReged_1_1_io_qout_0_ready = steer_submod_outs_1_0_ready;
  assign SteerReged_2_1_clock = clock;
  assign SteerReged_2_1_reset = reset;
  assign SteerReged_2_1_io_qin_valid = steer_submod_ins_2_valid;
  assign SteerReged_2_1_io_qin_bits = steer_submod_ins_2_bits;
  assign SteerReged_2_1_io_qout_0_ready = steer_submod_outs_2_0_ready;
  assign SteerReged_3_1_clock = clock;
  assign SteerReged_3_1_reset = reset;
  assign SteerReged_3_1_io_qin_valid = steer_submod_ins_3_valid;
  assign SteerReged_3_1_io_qin_bits = steer_submod_ins_3_bits;
  assign SteerReged_3_1_io_qout_0_ready = steer_submod_outs_3_0_ready;
  assign steer_submod_ins_0_ready = SteerReged_4_io_qin_ready;
  assign steer_submod_ins_0_valid = io_qin_valid;
  assign steer_submod_ins_0_bits = T_261;
  assign steer_submod_ins_1_ready = SteerReged_1_1_io_qin_ready;
  assign steer_submod_ins_1_valid = io_qin_valid;
  assign steer_submod_ins_1_bits = T_265;
  assign steer_submod_ins_2_ready = SteerReged_2_1_io_qin_ready;
  assign steer_submod_ins_2_valid = io_qin_valid;
  assign steer_submod_ins_2_bits = T_269;
  assign steer_submod_ins_3_ready = SteerReged_3_1_io_qin_ready;
  assign steer_submod_ins_3_valid = io_qin_valid;
  assign steer_submod_ins_3_bits = T_273;
  assign steer_submod_outs_0_0_ready = io_qout_0_ready;
  assign steer_submod_outs_0_0_valid = SteerReged_4_io_qout_0_valid;
  assign steer_submod_outs_0_0_bits = SteerReged_4_io_qout_0_bits;
  assign steer_submod_outs_1_0_ready = io_qout_0_ready;
  assign steer_submod_outs_1_0_valid = SteerReged_1_1_io_qout_0_valid;
  assign steer_submod_outs_1_0_bits = SteerReged_1_1_io_qout_0_bits;
  assign steer_submod_outs_2_0_ready = io_qout_0_ready;
  assign steer_submod_outs_2_0_valid = SteerReged_2_1_io_qout_0_valid;
  assign steer_submod_outs_2_0_bits = SteerReged_2_1_io_qout_0_bits;
  assign steer_submod_outs_3_0_ready = io_qout_0_ready;
  assign steer_submod_outs_3_0_valid = SteerReged_3_1_io_qout_0_valid;
  assign steer_submod_outs_3_0_bits = SteerReged_3_1_io_qout_0_bits;
  assign T_258 = io_qin_bits[130:0];
  assign T_259 = T_258;
  assign T_260 = io_qin_bits[527:518];
  assign T_261 = {T_260,T_259};
  assign T_262 = io_qin_bits[259:129];
  assign T_263 = T_262;
  assign T_265 = {T_260,T_263};
  assign T_266 = io_qin_bits[388:258];
  assign T_267 = T_266;
  assign T_269 = {T_260,T_267};
  assign T_270 = io_qin_bits[517:387];
  assign T_271 = T_270;
  assign T_273 = {T_260,T_271};
  assign T_277 = steer_submod_outs_0_0_bits[128:0];
  assign T_278 = steer_submod_outs_1_0_bits[128:0];
  assign T_279 = steer_submod_outs_2_0_bits[128:0];
  assign T_281 = {T_278,T_277};
  assign T_282 = {T_279,T_281};
  assign T_283 = {steer_submod_outs_3_0_bits,T_282};
endmodule
module DecoupledStge_6(
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
  input   io_out_ready,
  output  io_out_valid,
  output [605:0] io_out_bits,
  output  io_chosen
);
  wire  choice;
  wire  T_63;
  reg  lastGrant;
  reg [31:0] GEN_1;
  wire  GEN_0;
  assign io_in_0_ready = io_out_ready;
  assign io_out_valid = io_in_0_valid;
  assign io_out_bits = io_in_0_bits;
  assign io_chosen = choice;
  assign choice = 1'h0;
  assign T_63 = io_out_ready & io_out_valid;
  assign GEN_0 = T_63 ? io_chosen : lastGrant;
`ifdef RANDOMIZE
  integer initvar;
  initial begin
    `ifndef verilator
      #0.002 begin end
    `endif
  `ifdef RANDOMIZE_REG_INIT
  GEN_1 = {1{$random}};
  lastGrant = GEN_1[0:0];
  `endif
  end
`endif
  always @(posedge clock) begin
    if(reset) begin
      lastGrant <= 1'h0;
    end else begin
      if(T_63) begin
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
  input   io_qout_ready,
  output  io_qout_valid,
  output [605:0] io_qout_bits
);
  wire  DecoupledStge_6_1_clock;
  wire  DecoupledStge_6_1_reset;
  wire  DecoupledStge_6_1_io_inp_ready;
  wire  DecoupledStge_6_1_io_inp_valid;
  wire [605:0] DecoupledStge_6_1_io_inp_bits;
  wire  DecoupledStge_6_1_io_out_ready;
  wire  DecoupledStge_6_1_io_out_valid;
  wire [605:0] DecoupledStge_6_1_io_out_bits;
  wire  in_stage_inps_0_ready;
  wire  in_stage_inps_0_valid;
  wire [605:0] in_stage_inps_0_bits;
  wire  arb_clock;
  wire  arb_reset;
  wire  arb_io_in_0_ready;
  wire  arb_io_in_0_valid;
  wire [605:0] arb_io_in_0_bits;
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
  DecoupledStge_6 DecoupledStge_6_1 (
    .clock(DecoupledStge_6_1_clock),
    .reset(DecoupledStge_6_1_reset),
    .io_inp_ready(DecoupledStge_6_1_io_inp_ready),
    .io_inp_valid(DecoupledStge_6_1_io_inp_valid),
    .io_inp_bits(DecoupledStge_6_1_io_inp_bits),
    .io_out_ready(DecoupledStge_6_1_io_out_ready),
    .io_out_valid(DecoupledStge_6_1_io_out_valid),
    .io_out_bits(DecoupledStge_6_1_io_out_bits)
  );
  RRArbiter_1 arb (
    .clock(arb_clock),
    .reset(arb_reset),
    .io_in_0_ready(arb_io_in_0_ready),
    .io_in_0_valid(arb_io_in_0_valid),
    .io_in_0_bits(arb_io_in_0_bits),
    .io_out_ready(arb_io_out_ready),
    .io_out_valid(arb_io_out_valid),
    .io_out_bits(arb_io_out_bits),
    .io_chosen(arb_io_chosen)
  );
  DecoupledStge_6 stage_out (
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
  assign io_qout_valid = stage_out_io_out_valid;
  assign io_qout_bits = stage_out_io_out_bits;
  assign DecoupledStge_6_1_clock = clock;
  assign DecoupledStge_6_1_reset = reset;
  assign DecoupledStge_6_1_io_inp_valid = in_stage_inps_0_valid;
  assign DecoupledStge_6_1_io_inp_bits = in_stage_inps_0_bits;
  assign DecoupledStge_6_1_io_out_ready = arb_io_in_0_ready;
  assign in_stage_inps_0_ready = DecoupledStge_6_1_io_inp_ready;
  assign in_stage_inps_0_valid = io_qin_0_valid;
  assign in_stage_inps_0_bits = io_qin_0_bits;
  assign arb_clock = clock;
  assign arb_reset = reset;
  assign arb_io_in_0_valid = DecoupledStge_6_1_io_out_valid;
  assign arb_io_in_0_bits = DecoupledStge_6_1_io_out_bits;
  assign arb_io_out_ready = stage_out_io_inp_ready;
  assign stage_out_clock = clock;
  assign stage_out_reset = reset;
  assign stage_out_io_inp_valid = arb_io_out_valid;
  assign stage_out_io_inp_bits = arb_io_out_bits;
  assign stage_out_io_out_ready = io_qout_ready;
endmodule
module DecoupledStge_8(
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
  output [16:0] io_qout_0_bits
);
  wire  DecoupledStge_8_1_clock;
  wire  DecoupledStge_8_1_reset;
  wire  DecoupledStge_8_1_io_inp_ready;
  wire  DecoupledStge_8_1_io_inp_valid;
  wire [16:0] DecoupledStge_8_1_io_inp_bits;
  wire  DecoupledStge_8_1_io_out_ready;
  wire  DecoupledStge_8_1_io_out_valid;
  wire [16:0] DecoupledStge_8_1_io_out_bits;
  wire  out_stage_inps_0_ready;
  wire  out_stage_inps_0_valid;
  wire [16:0] out_stage_inps_0_bits;
  wire [16:0] T_75;
  reg [16:0] pendData;
  reg [31:0] GEN_0;
  reg  pendValid;
  reg [31:0] GEN_4;
  wire  T_84;
  wire [16:0] GEN_1;
  wire  T_102;
  wire  T_103;
  wire [16:0] T_105;
  wire [16:0] GEN_2;
  wire  GEN_3;
  wire [16:0] GEN_6;
  DecoupledStge_8 DecoupledStge_8_1 (
    .clock(DecoupledStge_8_1_clock),
    .reset(DecoupledStge_8_1_reset),
    .io_inp_ready(DecoupledStge_8_1_io_inp_ready),
    .io_inp_valid(DecoupledStge_8_1_io_inp_valid),
    .io_inp_bits(DecoupledStge_8_1_io_inp_bits),
    .io_out_ready(DecoupledStge_8_1_io_out_ready),
    .io_out_valid(DecoupledStge_8_1_io_out_valid),
    .io_out_bits(DecoupledStge_8_1_io_out_bits)
  );
  assign io_qin_ready = T_103;
  assign io_qout_0_valid = DecoupledStge_8_1_io_out_valid;
  assign io_qout_0_bits = DecoupledStge_8_1_io_out_bits;
  assign DecoupledStge_8_1_clock = clock;
  assign DecoupledStge_8_1_reset = reset;
  assign DecoupledStge_8_1_io_inp_valid = out_stage_inps_0_valid;
  assign DecoupledStge_8_1_io_inp_bits = out_stage_inps_0_bits;
  assign DecoupledStge_8_1_io_out_ready = io_qout_0_ready;
  assign out_stage_inps_0_ready = DecoupledStge_8_1_io_inp_ready;
  assign out_stage_inps_0_valid = T_84;
  assign out_stage_inps_0_bits = GEN_1;
  assign T_75 = 17'h0;
  assign T_84 = pendValid & out_stage_inps_0_ready;
  assign GEN_1 = T_84 ? pendData : T_75;
  assign T_102 = pendValid == 1'h0;
  assign T_103 = T_84 | T_102;
  assign T_105 = io_qin_bits;
  assign GEN_2 = io_qin_valid ? T_105 : pendData;
  assign GEN_3 = T_103 ? io_qin_valid : pendValid;
  assign GEN_6 = T_103 ? GEN_2 : pendData;
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
  GEN_4 = {1{$random}};
  pendValid = GEN_4[0:0];
  `endif
  end
`endif
  always @(posedge clock) begin
    if(1'h0) begin
    end else begin
      if(T_103) begin
        if(io_qin_valid) begin
          pendData <= T_105;
        end
      end
    end
    if(reset) begin
      pendValid <= 1'h0;
    end else begin
      if(T_103) begin
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
  input   io_mem_rd_req_out_ready,
  output  io_mem_rd_req_out_valid,
  output [79:0] io_mem_rd_req_out_bits,
  output  io_mem_rd_resp_in_ready,
  input   io_mem_rd_resp_in_valid,
  input  [527:0] io_mem_rd_resp_in_bits,
  input   io_acc_rd_resp_out_0_ready,
  output  io_acc_rd_resp_out_0_valid,
  output [527:0] io_acc_rd_resp_out_0_bits,
  output  io_acc_wr_req_in_0_ready,
  input   io_acc_wr_req_in_0_valid,
  input  [605:0] io_acc_wr_req_in_0_bits,
  input   io_mem_wr_req_out_ready,
  output  io_mem_wr_req_out_valid,
  output [605:0] io_mem_wr_req_out_bits,
  output  io_mem_wr_resp_in_ready,
  input   io_mem_wr_resp_in_valid,
  input  [16:0] io_mem_wr_resp_in_bits,
  input   io_acc_wr_resp_out_0_ready,
  output  io_acc_wr_resp_out_0_valid,
  output [16:0] io_acc_wr_resp_out_0_bits
);
  wire  arbiter_read_clock;
  wire  arbiter_read_reset;
  wire  arbiter_read_io_qin_0_ready;
  wire  arbiter_read_io_qin_0_valid;
  wire [79:0] arbiter_read_io_qin_0_bits;
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
  wire  arbiter_write_clock;
  wire  arbiter_write_reset;
  wire  arbiter_write_io_qin_0_ready;
  wire  arbiter_write_io_qin_0_valid;
  wire [605:0] arbiter_write_io_qin_0_bits;
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
  ArbiterReged arbiter_read (
    .clock(arbiter_read_clock),
    .reset(arbiter_read_reset),
    .io_qin_0_ready(arbiter_read_io_qin_0_ready),
    .io_qin_0_valid(arbiter_read_io_qin_0_valid),
    .io_qin_0_bits(arbiter_read_io_qin_0_bits),
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
    .io_qout_0_bits(steer_read_io_qout_0_bits)
  );
  ArbiterReged_1 arbiter_write (
    .clock(arbiter_write_clock),
    .reset(arbiter_write_reset),
    .io_qin_0_ready(arbiter_write_io_qin_0_ready),
    .io_qin_0_valid(arbiter_write_io_qin_0_valid),
    .io_qin_0_bits(arbiter_write_io_qin_0_bits),
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
    .io_qout_0_bits(steer_write_io_qout_0_bits)
  );
  assign io_acc_rd_req_in_0_ready = arbiter_read_io_qin_0_ready;
  assign io_mem_rd_req_out_valid = arbiter_read_io_qout_valid;
  assign io_mem_rd_req_out_bits = arbiter_read_io_qout_bits;
  assign io_mem_rd_resp_in_ready = steer_read_io_qin_ready;
  assign io_acc_rd_resp_out_0_valid = steer_read_io_qout_0_valid;
  assign io_acc_rd_resp_out_0_bits = steer_read_io_qout_0_bits;
  assign io_acc_wr_req_in_0_ready = arbiter_write_io_qin_0_ready;
  assign io_mem_wr_req_out_valid = arbiter_write_io_qout_valid;
  assign io_mem_wr_req_out_bits = arbiter_write_io_qout_bits;
  assign io_mem_wr_resp_in_ready = steer_write_io_qin_ready;
  assign io_acc_wr_resp_out_0_valid = steer_write_io_qout_0_valid;
  assign io_acc_wr_resp_out_0_bits = steer_write_io_qout_0_bits;
  assign arbiter_read_clock = clock;
  assign arbiter_read_reset = reset;
  assign arbiter_read_io_qin_0_valid = io_acc_rd_req_in_0_valid;
  assign arbiter_read_io_qin_0_bits = io_acc_rd_req_in_0_bits;
  assign arbiter_read_io_qout_ready = io_mem_rd_req_out_ready;
  assign steer_read_clock = clock;
  assign steer_read_reset = reset;
  assign steer_read_io_qin_valid = io_mem_rd_resp_in_valid;
  assign steer_read_io_qin_bits = io_mem_rd_resp_in_bits;
  assign steer_read_io_qout_0_ready = io_acc_rd_resp_out_0_ready;
  assign arbiter_write_clock = clock;
  assign arbiter_write_reset = reset;
  assign arbiter_write_io_qin_0_valid = io_acc_wr_req_in_0_valid;
  assign arbiter_write_io_qin_0_bits = io_acc_wr_req_in_0_bits;
  assign arbiter_write_io_qout_ready = io_mem_wr_req_out_ready;
  assign steer_write_clock = clock;
  assign steer_write_reset = reset;
  assign steer_write_io_qin_valid = io_mem_wr_resp_in_valid;
  assign steer_write_io_qin_bits = io_mem_wr_resp_in_bits;
  assign steer_write_io_qout_0_ready = io_acc_wr_resp_out_0_ready;
endmodule
