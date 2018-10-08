// See LICENSE for license details.

module afifo_split_wrapper2
  #(
    parameter DATA_WIDTH,
    parameter FIFO_DEPTH_LOG2 = 6,
    parameter SPLIT_WAYS = 2,
    parameter WR_TO_RD_CLK_STAGE = 1,
    parameter MLAB_ONLY = 0
   )
( rd_rst, rd_clk, wr_rst, wr_clk, rd_ready, rd_valid, rd_data, wr_ready, wr_valid, wr_data);

   input rd_rst; // positive logic
   input rd_clk;
   input wr_rst; // positive logic
   input wr_clk;

   input rd_ready;
   output reg rd_valid;
   output [DATA_WIDTH-1:0] rd_data ;

   output wr_ready;
   input wr_valid;
   input [DATA_WIDTH-1:0] wr_data;
   
   wire i_ready                [SPLIT_WAYS:0];
   wire i_valid                [SPLIT_WAYS:0];
   wire [DATA_WIDTH-1:0] i_data[SPLIT_WAYS:0];
   
   wire s_clk [SPLIT_WAYS:0];
   wire s_rst [SPLIT_WAYS:0];
   
   initial begin
     if (WR_TO_RD_CLK_STAGE < 1 || WR_TO_RD_CLK_STAGE >  SPLIT_WAYS) begin
       $display (" Value of WR_TO_RD_CLK_STAGE = %d is outside the bounds", WR_TO_RD_CLK_STAGE);
       $stop(1);
     end   
     
   end
   // enumerate all clock starting from first write, then first read, then second stage write, second stage read
   // overall there will be SPLIT_WAYS + 1 clocks (for 2 fifo split, -s_clk-|||-s_clk-|||-s_clk-, for 4 fifo split -s_clk-|||-s_clk-|||-s_clk-|||-s_clk-|||-s_clk-)
   // replicate wr_clk from the first WRITE to WR_TO_RD_CLK_STAGE
   assign  s_clk[WR_TO_RD_CLK_STAGE-1:0] = '{WR_TO_RD_CLK_STAGE{wr_clk}};
   assign  s_rst[WR_TO_RD_CLK_STAGE-1:0] = '{WR_TO_RD_CLK_STAGE{wr_rst}};
   //replicate rd_clk from WR_TO_RD_CLK_STAGE to the last READ stage
   assign  s_clk[SPLIT_WAYS:WR_TO_RD_CLK_STAGE] = '{SPLIT_WAYS-WR_TO_RD_CLK_STAGE+1{rd_clk}};
   assign  s_rst[SPLIT_WAYS:WR_TO_RD_CLK_STAGE] = '{SPLIT_WAYS-WR_TO_RD_CLK_STAGE+1{rd_rst}};

   assign i_valid[0] = wr_valid;
   assign i_data[0] = wr_data;
   assign wr_ready = i_ready[0];

   assign i_ready[SPLIT_WAYS] = rd_ready;
   assign rd_valid = i_valid[SPLIT_WAYS];
   assign rd_data = i_data[SPLIT_WAYS];

   genvar k; 
   generate for (k=0; k<SPLIT_WAYS; k=k+1) begin 
     afifo_wrapper2 
     #(
       .FIFO_DEPTH_LOG2    (FIFO_DEPTH_LOG2-$clog2(SPLIT_WAYS)),
       .DATA_WIDTH         (DATA_WIDTH),
       .MLAB_ONLY          (MLAB_ONLY)
      )
     ififo_gen( 
       .rd_rst(s_rst[k+1]), .rd_clk( s_clk[k+1] ), .wr_clk( s_clk[k]),
       .rd_ready( i_ready[k+1]),
       .rd_valid( i_valid[k+1]),
       .rd_data( i_data[k+1]),
       .wr_ready( i_ready[k]),
       .wr_valid( i_valid[k]),
       .wr_data( i_data[k])
     ); 
   end
   endgenerate
   
endmodule


