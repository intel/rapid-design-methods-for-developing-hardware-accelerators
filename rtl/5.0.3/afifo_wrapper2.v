// See LICENSE for license details.

module afifo_wrapper2
  #(
    parameter DATA_WIDTH,
    parameter FIFO_DEPTH_LOG2 = 6,
    parameter MLAB_ONLY = 0,
    parameter FIFO_ALMFULL_GUARD = 7
    )
( rd_rst, rd_clk, wr_clk, rd_ready, rd_valid, rd_data, wr_ready, wr_valid, wr_data);

   input rd_rst; // positive logic
   input rd_clk;
   input wr_clk;

   input rd_ready;
   output reg rd_valid;
   output [DATA_WIDTH-1:0] rd_data;

   output wr_ready;
   input wr_valid;
   input [DATA_WIDTH-1:0] wr_data;
   
   wire   rd_en;
   wire   wr_en;
   wire   rd_empty;

   wire   wralmfull;

   logic [FIFO_DEPTH_LOG2:0] wr_count;
   logic [FIFO_DEPTH_LOG2:0] rd_count;
   logic 		     wrfull;
   
   logic 		     illegal_read;
   logic 		     illegal_write;
      

   always @(posedge rd_clk)
     begin
        rd_valid <= ~rd_empty || ~rd_ready && rd_valid;
        if ( rd_rst)
          begin
             rd_valid <= 0;
          end
     end

   assign wr_ready = ~wralmfull; 
   assign rd_en = ~rd_empty && (rd_ready || ~rd_valid);
   assign wr_en = wr_valid && wr_ready;
   
//   assign wralmfull = (wr_count >= FIFO_ALMFULL_THRESHOLD) ? 1'b1 : 1'b0;
   assign wralmfull = wrfull;

   generate 
    if (MLAB_ONLY == 1) 
 
        dcfifo
        #(
        .add_usedw_msb_bit("ON"),
        .enable_ecc("FALSE"),
        .intended_device_family("Arria 10"),
        .lpm_hint("RAM_BLOCK_TYPE=MLAB,DISABLE_DCFIFO_EMBEDDED_TIMING_CONSTRAINT=TRUE"),
        .lpm_numwords(2**FIFO_DEPTH_LOG2),
        .lpm_showahead("OFF"),
        .lpm_type("dcfifo"),
        .lpm_width(DATA_WIDTH),
        .lpm_widthu(FIFO_DEPTH_LOG2+1),
        .overflow_checking("OFF"),
        .rdsync_delaypipe(4),
        .read_aclr_synch("OFF"),
        .underflow_checking("OFF"),
        .use_eab("ON"),
        .write_aclr_synch("OFF"),
        .wrsync_delaypipe(4)
        )
        dcfifo_component (
                    .aclr (rd_rst),
                    .data (wr_data),
                    .rdclk (rd_clk),
                    .rdreq (rd_en),
                    .wrclk (wr_clk),
                    .wrreq (wr_en),
                    .q (rd_data),
                    .rdempty (rd_empty),
                    .rdfull (),
                    .rdusedw (rd_count),
                    .wrempty (),
                    .wrfull (wrfull),
                    .wrusedw (wr_count),
                    .eccstatus ());
   else 
           dcfifo
        #(
        .add_usedw_msb_bit("ON"),
        .enable_ecc("FALSE"),
        .intended_device_family("Arria 10"),
        .lpm_hint("DISABLE_DCFIFO_EMBEDDED_TIMING_CONSTRAINT=TRUE"),
        .lpm_numwords(2**FIFO_DEPTH_LOG2),
        .lpm_showahead("OFF"),
        .lpm_type("dcfifo"),
        .lpm_width(DATA_WIDTH),
        .lpm_widthu(FIFO_DEPTH_LOG2+1),
        .overflow_checking("OFF"),
        .rdsync_delaypipe(4),
        .read_aclr_synch("OFF"),
        .underflow_checking("OFF"),
        .use_eab("ON"),
        .write_aclr_synch("OFF"),
        .wrsync_delaypipe(4)
        )
        dcfifo_component (
                    .aclr (rd_rst),
                    .data (wr_data),
                    .rdclk (rd_clk),
                    .rdreq (rd_en),
                    .wrclk (wr_clk),
                    .wrreq (wr_en),
                    .q (rd_data),
                    .rdempty (rd_empty),
                    .rdfull (),
                    .rdusedw (rd_count),
                    .wrempty (),
                    .wrfull (wrfull),
                    .wrusedw (wr_count),
                    .eccstatus ());

   endgenerate 
   assign illegal_read  = rd_empty && rd_en;   
   assign illegal_write = wrfull  && wr_en;
      
   // synthesis translate_off
   always @(posedge rd_clk) begin     
      if (illegal_read) begin
	 $display ("%m : Illegal read @ ", $time);
	 $finish;
      end      
   end
   always @(posedge wr_clk) begin
      if (illegal_write) begin
	 $display ("%m : Illegal write @ ", $time);
	 $finish;
      end
   end
   // synthesis translate_on

endmodule


