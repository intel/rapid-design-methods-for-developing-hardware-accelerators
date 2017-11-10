// See LICENSE for license details.
module vectoradd_acc_rtl(clk, rst, config_, start, spl_rd_resp_valid, 
spl_rd_resp_data, spl_rd_req_ready, spl_wr_resp_valid, spl_wr_resp_data, 
spl_wr_req_ready, done, spl_rd_resp_ready, spl_rd_req_valid, spl_rd_req_data, 
spl_wr_resp_ready, spl_wr_req_valid, spl_wr_req_data);
  input clk;
  input rst;
  input [319:0] config_;
  input start;
  input spl_rd_resp_valid;
  input [527:0] spl_rd_resp_data;
  input spl_rd_req_ready;
  input spl_wr_resp_valid;
  input [16:0] spl_wr_resp_data;
  input spl_wr_req_ready;
  output reg done;
  output spl_rd_resp_ready;
  output spl_rd_req_valid;
  output reg [79:0] spl_rd_req_data;
  output spl_wr_resp_ready;
  output spl_wr_req_valid;
  output reg [605:0] spl_wr_req_data;

   
  HldAcceleratorWrapper chiselAcc(
    .clock(clk), 
    .reset(~rst), 
    .io_config(config_),
    .io_spl_rd_req_ready(spl_rd_req_ready),
    .io_spl_rd_req_valid(spl_rd_req_valid),
    .io_spl_rd_req_bits(spl_rd_req_data),
    .io_spl_rd_resp_ready(spl_rd_resp_ready),
    .io_spl_rd_resp_valid(spl_rd_resp_valid),
    .io_spl_rd_resp_bits(spl_rd_resp_data),
    .io_spl_wr_req_ready(spl_wr_req_ready),
    .io_spl_wr_req_valid(spl_wr_req_valid),
    .io_spl_wr_req_bits(spl_wr_req_data),
    .io_spl_wr_resp_ready(spl_wr_resp_ready),
    .io_spl_wr_resp_valid(spl_wr_resp_valid),
    .io_spl_wr_resp_bits(spl_wr_resp_data),
    .io_done(done),
    .io_start(start)               
                   
  );
   
endmodule
