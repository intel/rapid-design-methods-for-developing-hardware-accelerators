// See LICENSE for license details.
`include "hld_defines.vh"

`define SHIM_V_NAME(rpc, wpc) `"hld_shim_``rpc``_``wpc``.sv`"
`define MUX_V_NAME(rpc, wpc) `"MultiChannelMux_``rpc``_``wpc``.v`"

`include `SHIM_V_NAME(`HLD_MEM_RD_PORTS,`HLD_MEM_WR_PORTS)
`include `MUX_V_NAME(`HLD_MEM_RD_PORTS,`HLD_MEM_WR_PORTS)

module hld_shim_wrapper
  #(
    parameter RD_PORTS = 1,
    parameter WR_PORTS = 1,
    parameter FIFO_REQ_DEPTH_LOG2 = 7,
    parameter FIFO_RESP_DEPTH_LOG2 = 2
   )
( a_clk, clk, rst, config_, start, spl_rd_resp_valid, 
spl_rd_resp_data, spl_rd_req_ready, spl_wr_resp_valid, spl_wr_resp_data, 
spl_wr_req_ready, done, spl_rd_resp_ready, spl_rd_req_valid, spl_rd_req_data, 
spl_wr_resp_ready, spl_wr_req_valid, spl_wr_req_data);

  input a_clk;
  input clk;
  input rst;
  input [9*64-1:0] config_;
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
  output [79:0] spl_rd_req_data;
  output spl_wr_resp_ready;
  output spl_wr_req_valid;
  output [605:0] spl_wr_req_data;

generate begin
  if (RD_PORTS == 1 && WR_PORTS == 1) begin
    hld_shim_1_1
    #(    
      .FIFO_REQ_DEPTH_LOG2(FIFO_REQ_DEPTH_LOG2),
      .FIFO_RESP_DEPTH_LOG2(FIFO_RESP_DEPTH_LOG2)
    ) shim(.*);
  end else if (RD_PORTS == 4 && WR_PORTS == 1) begin
    hld_shim_4_1
    #(    
      .FIFO_REQ_DEPTH_LOG2(FIFO_REQ_DEPTH_LOG2),
      .FIFO_RESP_DEPTH_LOG2(FIFO_RESP_DEPTH_LOG2)
    ) shim(.*);
  end else if (RD_PORTS == 4 && WR_PORTS == 4) begin
    hld_shim_4_4
    #(    
      .FIFO_REQ_DEPTH_LOG2(FIFO_REQ_DEPTH_LOG2),
      .FIFO_RESP_DEPTH_LOG2(FIFO_RESP_DEPTH_LOG2)
    ) shim(.*);
  end else if (RD_PORTS == 3 && WR_PORTS == 1) begin
    hld_shim_3_1
    #(    
      .FIFO_REQ_DEPTH_LOG2(FIFO_REQ_DEPTH_LOG2),
      .FIFO_RESP_DEPTH_LOG2(FIFO_RESP_DEPTH_LOG2)
    ) shim(.*);
  end else if (RD_PORTS == 2 && WR_PORTS == 1) begin
    hld_shim_2_1
    #(    
      .FIFO_REQ_DEPTH_LOG2(FIFO_REQ_DEPTH_LOG2),
      .FIFO_RESP_DEPTH_LOG2(FIFO_RESP_DEPTH_LOG2)
    ) shim(.*);
  end else if (RD_PORTS == 2 && WR_PORTS == 2) begin
    hld_shim_2_2
    #(    
      .FIFO_REQ_DEPTH_LOG2(FIFO_REQ_DEPTH_LOG2),
      .FIFO_RESP_DEPTH_LOG2(FIFO_RESP_DEPTH_LOG2)
    ) shim(.*);
  end else begin
    hld_shim_this_should_not_exist
    #(    
      .FIFO_REQ_DEPTH_LOG2(FIFO_REQ_DEPTH_LOG2),
      .FIFO_RESP_DEPTH_LOG2(FIFO_RESP_DEPTH_LOG2)
    ) shim(.*);
//    $error( "%m: Unsupported combination of RD_PORTS (%d) and WR_PORTS (%d)", RD_PORTS, WR_PORTS);
  end
end
endgenerate

endmodule
