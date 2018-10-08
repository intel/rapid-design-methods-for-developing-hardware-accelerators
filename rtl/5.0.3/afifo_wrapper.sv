// See LICENSE for license details.
module afifo_wrapper
  #(
    parameter FIFO_WIDTH,
    parameter FIFO_DEPTH_LOG2 = 6,
    parameter FIFO_ALMFULL_GUARD = 7
    )
   (
    input logic 		  rst,
    input logic 		  rdclk,
    input logic 		  wrclk,
    input logic [FIFO_WIDTH-1:0]  data_in,
    input logic 		  read_en,
    output logic [FIFO_WIDTH-1:0] data_out,
    input logic 		  write_en,
    output logic 		  rdempty,
    output logic 		  rdfull,
    output logic 		  wrempty,
    output logic 		  wralmfull
    );

   logic [FIFO_DEPTH_LOG2:0] wr_count;
   logic [FIFO_DEPTH_LOG2:0] rd_count;
   logic 		     wrfull;
   
   logic 		     illegal_read;
   logic 		     illegal_write;
      
   localparam FIFO_ALMFULL_THRESHOLD = 2**FIFO_DEPTH_LOG2 - FIFO_ALMFULL_GUARD;
   
//   assign wralmfull = (wr_count >= FIFO_ALMFULL_THRESHOLD) ? 1'b1 : 1'b0;
   assign wralmfull = wrfull;
   
   // Wrapped Megafunction instance  
   generate
      if (FIFO_WIDTH == 528 && FIFO_DEPTH_LOG2 == 6)
      begin
   altr_afifo_alt_528_64
   altr_afifo
     (
      .aclr    (rst),
      .data    (data_in),
      .rdclk   (rdclk),
      .rdreq   (read_en),
      .wrclk   (wrclk),
      .wrreq   (write_en),
      .q       (data_out),
      .rdempty (rdempty),
      .rdfull  (rdfull),
      .rdusedw (rd_count),
      .wrempty (wrempty),
      .wrfull  (wrfull),
      .wrusedw (wr_count)
      );
     end

      if (FIFO_WIDTH == 528 && FIFO_DEPTH_LOG2 == 5)
      begin
   altr_afifo_alt_528_32
   altr_afifo
     (
      .aclr    (rst),
      .data    (data_in),
      .rdclk   (rdclk),
      .rdreq   (read_en),
      .wrclk   (wrclk),
      .wrreq   (write_en),
      .q       (data_out),
      .rdempty (rdempty),
      .rdfull  (rdfull),
      .rdusedw (rd_count),
      .wrempty (wrempty),
      .wrfull  (wrfull),
      .wrusedw (wr_count)
      );
     end
      if (FIFO_WIDTH == 17 && FIFO_DEPTH_LOG2 == 6)
      begin
   altr_afifo_17_64
   altr_afifo
     (
      .aclr    (rst),
      .data    (data_in),
      .rdclk   (rdclk),
      .rdreq   (read_en),
      .wrclk   (wrclk),
      .wrreq   (write_en),
      .q       (data_out),
      .rdempty (rdempty),
      .rdfull  (rdfull),
      .rdusedw (rd_count),
      .wrempty (wrempty),
      .wrfull  (wrfull),
      .wrusedw (wr_count)
      );
     end
      if (FIFO_WIDTH == 80 && FIFO_DEPTH_LOG2 == 6)
      begin
   altr_afifo_80_64
   altr_afifo
     (
      .aclr    (rst),
      .data    (data_in),
      .rdclk   (rdclk),
      .rdreq   (read_en),
      .wrclk   (wrclk),
      .wrreq   (write_en),
      .q       (data_out),
      .rdempty (rdempty),
      .rdfull  (rdfull),
      .rdusedw (rd_count),
      .wrempty (wrempty),
      .wrfull  (wrfull),
      .wrusedw (wr_count)
      );
     end
      if (FIFO_WIDTH == 606 && FIFO_DEPTH_LOG2 == 6)
      begin
   altr_afifo_alt_606_64
   altr_afifo
     (
      .aclr    (rst),
      .data    (data_in),
      .rdclk   (rdclk),
      .rdreq   (read_en),
      .wrclk   (wrclk),
      .wrreq   (write_en),
      .q       (data_out),
      .rdempty (rdempty),
      .rdfull  (rdfull),
      .rdusedw (rd_count),
      .wrempty (wrempty),
      .wrfull  (wrfull),
      .wrusedw (wr_count)
      );

      
     end
   endgenerate

   assign illegal_read  = rdempty && read_en;   
   assign illegal_write = wrfull  && write_en;
      
   // synthesis translate_off
   always @(posedge rdclk) begin     
      if (illegal_read) begin
	 $display ("%m : Illegal read @ ", $time);
	 $finish;
      end      
   end
   always @(posedge wrclk) begin
      if (illegal_write) begin
	 $display ("%m : Illegal write @ ", $time);
	 $finish;
      end
   end
   // synthesis translate_on

endmodule
