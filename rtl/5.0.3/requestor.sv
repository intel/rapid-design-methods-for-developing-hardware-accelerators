// See LICENSE for license details.
// ***************************************************************************
// Copyright (c) 2013-2016, Intel Corporation
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation
// and/or other materials provided with the distribution.
// * Neither the name of Intel Corporation nor the names of its contributors
// may be used to endorse or promote products derived from this software
// without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// Module Name:         requestor.v
// Project:             Generic Accelerator
//                      Compliant with CCI v2.1
// Description:         accepts requests from arbiter and formats it per cci
//                      spec. It also implements the flow control.
// ***************************************************************************
//
// The requestor accepts the address index from the arbiter, appends that to the source/destination base address and 
// sends out the request to the CCI module. It arbitrates between the read and the write requests, peforms the flow control,
// implements all the CSRs for source address, destination address, status address, wrthru enable, start and stop the test.
//
//
//

import ccip_if_pkg::*;
module requestor #(parameter PEND_THRESH=1, ADDR_LMT=42, TXHDR_WIDTH=61, RXHDR_WIDTH=18, DATA_WIDTH=512)
(

    //      ---------------------------global signals-------------------------------------------------
    Clk_400,        // in    std_logic;  -- Core clock
    SoftReset,      // in    std_logic;  -- Use SPARINGLY only for control. ACTIVE HIGH
    //      ---------------------------CCI IF signals between CCI and requestor  ---------------------
    af2cp_sTxPort,
    cp2af_sRxPort,

    cr2re_inact_thresh,
    cr2re_interrupt0,
    cr2re_cfg,
    cr2re_ctl,
    cr2re_dsm_base,
    cr2re_dsm_base_valid,

    ab2re_WrAddr,            // [ADDR_LMT-1:0]      arbiter:        Writes are guaranteed to be accepted
    ab2re_WrTID,             // [15:0]              arbiter:        meta data
    ab2re_WrDin,             // [511:0]             arbiter:        Cache line data
    ab2re_WrFence,           //                     arbiter:        write fence.
    ab2re_WrEn,              //                     arbiter:        write enable
    re2ab_WrSent,            //                     arbiter:        can accept writes. Qualify with write enable
    re2ab_WrAlmFull,         //                     arbiter:        write fifo almost full

    ab2re_RdAddr,            // [ADDR_LMT-1:0]      arbiter:        Reads may yield to writes
    ab2re_RdTID,             // [15:0]              arbiter:        meta data
    ab2re_RdEn,              //                     arbiter:        read enable
    re2ab_RdSent,            //                     arbiter:        read issued

    re2ab_RdRspValid,        //                     arbiter:        read response valid
    re2ab_UMsgValid,         //                     arbiter:        UMsg valid
    re2ab_CfgValid,          //                     arbiter:        Cfg Valid
    re2ab_RdRsp,             // [ADDR_LMT-1:0]      arbiter:        read response header
    re2ab_RdData,            // [511:0]             arbiter:        read data
    re2ab_stallRd,           //                     arbiter:        stall read requests FOR LPBK1

    re2ab_WrRspValid,        //                     arbiter:        write response valid
    re2ab_WrRsp,             // [ADDR_LMT-1:0]      arbiter:        write response header
    re2xy_go,                //                     requestor:      start the test

    re2xy_Cont,              //                     requestor:      continuous mode
    re2xy_test_cfg,          // [7:0]               requestor:      8-bit test cfg register.

    re2ab_Mode,              // [2:0]               requestor:      test mode

    ab2re_TestCmp,           //                     arbiter:        Test completion flag
    ab2re_ErrorInfo,         // [255:0]             arbiter:        error information
    ab2re_ErrorValid,        //                     arbiter:        test has detected an error
    test_Reset_n,             //                     requestor:      rest the app
    re2cr_wrlock_n,          //                     requestor:      when low, block csr writes
  
    ab2re_RdLen,
    ab2re_RdSop,
    ab2re_WrLen,
    ab2re_WrSop,
     
    re2ab_RdRspFormat,
    re2ab_RdRspCLnum,
    re2ab_WrRspFormat,
    re2ab_WrRspCLnum,
    re2xy_multiCL_len,

    re2cr_num_reads,
    re2cr_num_writes,
    re2cr_num_Rdpend,
    re2cr_num_Wrpend,
    re2cr_error,
    
    afu_idle_counter,
    ivp_idle_counter

);
    //--------------------------------------------------------------------------------------------------------------
    input                   Clk_400;                //                      ccip_intf:        Clk_400
    input                   SoftReset;              //                      ccip_intf:        system SoftReset
    
    output t_if_ccip_Tx      af2cp_sTxPort;
    input  t_if_ccip_Rx      cp2af_sRxPort;

    input  [31:0]           cr2re_inact_thresh;
    input  [31:0]           cr2re_interrupt0;
    input  t_ccip_mmioData  cr2re_cfg;
    input  [31:0]           cr2re_ctl;
    input  t_ccip_mmioData  cr2re_dsm_base;
    input                   cr2re_dsm_base_valid;
    
    input  [ADDR_LMT-1:0]   ab2re_WrAddr;           // [ADDR_LMT-1:0]        arbiter:       Writes are guaranteed to be accepted
    input  t_ccip_mdata     ab2re_WrTID;            // [15:0]                arbiter:       meta data
    input  t_ccip_clData    ab2re_WrDin;            // [511:0]               arbiter:       Cache line data
    input                   ab2re_WrFence;          //                       arbiter:       write fence 
    input                   ab2re_WrEn;             //                       arbiter:       write enable
    output                  re2ab_WrSent;           //                       arbiter:       write issued
    output                  re2ab_WrAlmFull;        //                       arbiter:       write fifo almost full
    
    input  [ADDR_LMT-1:0]   ab2re_RdAddr;           // [ADDR_LMT-1:0]        arbiter:       Reads may yield to writes
    input  t_ccip_mdata     ab2re_RdTID;            // [15:0]                arbiter:       meta data
    input                   ab2re_RdEn;             //                       arbiter:       read enable
    output                  re2ab_RdSent;           //                       arbiter:       read issued
    
    output                  re2ab_RdRspValid;       //                       arbiter:       read response valid
    output                  re2ab_UMsgValid;        //                       arbiter:       UMsg valid
    output                  re2ab_CfgValid;         //                       arbiter:       Cfg valid
    output t_ccip_mdata     re2ab_RdRsp;            // [15:0]                arbiter:       read response header
    output t_ccip_clData    re2ab_RdData;           // [511:0]               arbiter:       read data
    output                  re2ab_stallRd;          //                       arbiter:       stall read requests FOR LPBK1
    
    output                  re2ab_WrRspValid;       //                       arbiter:       write response valid
    output t_ccip_mdata     re2ab_WrRsp;            // [15:0]                arbiter:       write response header
    
    output                  re2xy_go;               //                       requestor:     start of frame recvd
    output                  re2xy_Cont;             //                       requestor:     continuous mode

    output [7:0]            re2xy_test_cfg;         // [7:0]                 requestor:     8-bit test cfg register.
    output [2:0]            re2ab_Mode;             // [2:0]                 requestor:     test mode
    input                   ab2re_TestCmp;          //                       arbiter:       Test completion flag
    input  [255:0]          ab2re_ErrorInfo;        // [255:0]               arbiter:       error information
    input                   ab2re_ErrorValid;       //                       arbiter:       test has detected an error
    
    output                  test_Reset_n;
    output                  re2cr_wrlock_n;
  
    input [1:0]             ab2re_RdLen;
    input                   ab2re_RdSop;
    input [1:0]             ab2re_WrLen;
    input                   ab2re_WrSop;
    output                  re2ab_RdRspFormat;
    output [1:0]            re2ab_RdRspCLnum;
    output                  re2ab_WrRspFormat;
    output [1:0]            re2ab_WrRspCLnum;
    output [1:0]            re2xy_multiCL_len;

    output  logic [31:0]    re2cr_num_Rdpend;
    output  logic [31:0]    re2cr_num_Wrpend;
    output  logic [31:0]    re2cr_num_reads;
    output  logic [31:0]    re2cr_num_writes;
    output  logic [31:0]    re2cr_error;
    
    
    input [31:0] afu_idle_counter;
    input [31:0] ivp_idle_counter;

    //----------------------------------------------------------------------------------------------------------------------
    //---------------------------------------------------------
    // Default Values ****** May be MODIFIED ******* 
    //---------------------------------------------------------
    localparam      DEF_SRC_ADDR         = 32'h0400_0000;           // Read data starting from here. Cache aligned Address
    localparam      DEF_DST_ADDR         = 32'h0500_0000;           // Copy data to here. Cache aligned Address
    localparam      DEF_DSM_BASE         = 32'h04ff_ffff;           // default status address
    
   
    //----------------------------------------------------------------------------------
    // Device Status Memory (DSM) Address Map ***** DO NOT MODIFY *****
    // This is a shared memory region where AFU writes and SW reads from. It is used for sharing status.
    // Physical address = value at CSR_AFU_DSM_BASE + Byte offset
    //----------------------------------------------------------------------------------
    //                                     Byte Offset                 Attribute    Width   Comments
    localparam      DSM_STATUS           = 32'h40;                  // RO           512b    test status and error info
    
    //----------------------------------------------------------------------------------------------------------------------
    
    reg  [31:0]             ErrorVector;
    reg  [31:0]             Num_Reads;                              // Number of reads performed
    reg  [31:0]             Num_Writes;                             // Number of writes performed
    reg  [19:0]             Num_ticks_low, Num_ticks_high;
    reg                     re2ab_stallRd;
    reg                     RdHdr_valid;

    t_if_ccip_Rx            cp2af_sRxPort_T1;

    reg                     re2ab_CfgValid_d;
    reg                     re2ab_RdSent;
    reg                     status_write;
    
    reg   [31:0]            cr_inact_thresh;
(* dont_merge, maxfan=256 *) reg dsm_status_wren_a0;
(* dont_merge, maxfan=256 *) reg dsm_status_wren_b0;
(* dont_merge, maxfan=256 *) reg dsm_status_wren_a1;
(* dont_merge, maxfan=256 *) reg dsm_status_wren_b1;
(* dont_merge, maxfan=256 *) reg dsm_status_wren_c;
    t_ccip_c0_req           rdreq_type;
    t_ccip_c0_req           rnd_rdreq_type;
    reg                     rnd_rdreq_sel;
    
    integer                 i;
    t_ccip_mmioData         cr_dsm_base;                            // a00h, a04h - DSM base address
    reg   [1:0]             cr_multiCL_len;   
    reg   [31:0]            cr_ctl = 0;                             // a2ch - control register to start and stop the test
    reg                     cr_wrthru_en;                           // a34h - [0]    : test configuration- wrthru_en
    reg                     cr_cont;                                // a34h - [1]    : repeats the test sequence, NO end condition
    reg   [2:0]             cr_mode;                                // a34h - [4:2]  : selects test mode
    reg                     cr_delay_en;                            // a34h - [8]    : use start delay
    reg   [1:0]             cr_rdsel, cr_rdsel_q;                   // a34h - [10:9] : read request type
    reg   [7:0]             cr_test_cfg;                            // a34h - [27:0] : configuration within a selected test mode
    reg   [31:0]            cr_interrupt0;                          // a3ch - SW allocates apic id & interrupt vector
    reg                     cr_interrupt_testmode;
    reg                     cr_interrupt_on_error;
    reg   [1:0]             cr_chsel;
    reg   [41:0]            ds_stat_address;                        // 040h - test status is written to this address
        
    wire                    re2ab_WrSent    = 1'b1;             // stop accepting new requests, after status write=1

    logic                   txFifo_WrValid;

    reg [15:0]             txFifo_WrTID;
    reg [ADDR_LMT-1:0]     txFifo_WrAddr;
    reg                    txFifo_WrFence;
    reg                    txFifo_WrSop;
    reg [1:0]              txFifo_WrLen;
    
    logic [ADDR_LMT-1:0]            ab2re_RdAddr_q;      
    logic                   RdHdr_valid_q;
    logic                   ab2re_RdEn_q;
    logic [15:0]            ab2re_RdTID_q;

        
    t_ccip_c1_req           wrreq_type;  
    t_ccip_clData           txFifo_WrDin;
    t_ccip_clData           WrData_dsm;

    (* dont_merge *) reg      test_Reset_n_internal0;
    (* dont_merge *) reg      test_Reset_n_internal1;
    (* dont_merge *) reg      test_Reset_n_internal2;
    (* dont_merge *) reg      test_Reset_n_internal3;
    assign test_Reset_n = test_Reset_n_internal0;

    reg                     test_go;        
    reg  [2:0]              re2ab_Mode;     
    reg  [7:0]              re2xy_test_cfg;

    reg  [1:0]              re2xy_multiCL_len;
    reg                     re2xy_Cont;
  
    (* dont_merge *)    reg  re2xy_go;
    wire                    re2ab_WrAlmFull  = 1'b0;
    wire                    tx_errorValid    = ErrorVector!=0;
    reg                     re2cr_wrlock_n;
    reg    [14:0]           dsm_number=0;
    
    logic                   re2ab_RdRspValid;
    logic                   re2ab_UMsgValid;
//  logic  [15:0]           re2ab_RdRsp;
//  t_ccip_clData re2ab_RdData;
    logic                   re2ab_WrRspValid;
//  logic  [15:0]           re2ab_WrRsp;
    logic                   re2ab_CfgValid;
  
    logic                   ab2re_RdSop;
    logic [1:0]             ab2re_WrLen;
    logic [1:0]             ab2re_RdLen, ab2re_RdLen_q;
    logic                   ab2re_WrSop;
                        
    logic                   re2ab_RdRspFormat=0;
    logic [1:0]             re2ab_RdRspCLnum;
    logic                   re2ab_WrRspFormat;
    logic [1:0]             re2ab_WrRspCLnum;
  
    logic                   test_stop;
    logic                   WrFence_sent;
    logic                   read_only_test;
    logic                   test_cmplt;
    logic [1:0]             tx_rd_req_len;
    logic                   rx_wr_resp_fmt;
    logic [1:0]             rx_wr_resp_cl_num;
   
   


    logic [9:0] wait_at_end;
    parameter WAIT_THIS_MUCH = 100;
    
    // RdAddr computation takes one cycle :- Delay Rd valid generation from req to upstream by 1 clk
    always @(posedge Clk_400)
    begin
      ab2re_RdAddr_q       <= ab2re_RdAddr;
      ab2re_RdLen_q        <= ab2re_RdLen;
      ab2re_RdTID_q        <= ab2re_RdTID;
      ab2re_RdEn_q         <= ab2re_RdEn;
      RdHdr_valid_q        <= RdHdr_valid;
    end
    
    always @(posedge Clk_400)
    begin
      re2cr_wrlock_n       <= cr_ctl[0] & ~cr_ctl[1];
      // Clears all the states. Either is one then test is out of Reset.
      // explicitly duplicated for fanout convergence
      test_Reset_n_internal0         <= cr_ctl[0];
      test_Reset_n_internal1         <= cr_ctl[0];
      test_Reset_n_internal2         <= cr_ctl[0];
      test_Reset_n_internal3         <= cr_ctl[0];
      test_go              <= cr_ctl[1];                // When 0, it allows reconfiguration of test parameters.
      re2ab_Mode           <= cr_mode;
      re2xy_test_cfg       <= cr_test_cfg;
      re2xy_multiCL_len    <= cr_multiCL_len;
      re2xy_Cont           <= cr_cont;
    end

    always_comb 
    begin
      re2ab_RdRspValid = cp2af_sRxPort_T1.c0.rspValid && (cp2af_sRxPort_T1.c0.hdr.resp_type==eRSP_RDLINE);
      re2ab_UMsgValid  = cp2af_sRxPort_T1.c0.rspValid && cp2af_sRxPort_T1.c0.hdr.resp_type==eRSP_UMSG;
      re2ab_RdRsp      = cp2af_sRxPort_T1.c0.hdr.mdata[15:0];
      re2ab_RdRspCLnum = cp2af_sRxPort_T1.c0.hdr.cl_num[1:0]; 
      re2ab_RdData     = cp2af_sRxPort_T1.c0.data;
      re2ab_WrRspValid = cp2af_sRxPort_T1.c1.rspValid && cp2af_sRxPort_T1.c1.hdr.resp_type==eRSP_WRLINE;;
      re2ab_WrRsp      = cp2af_sRxPort_T1.c1.hdr.mdata[15:0];
      re2ab_WrRspFormat= cp2af_sRxPort_T1.c1.hdr.format;
      re2ab_WrRspCLnum = cp2af_sRxPort_T1.c1.hdr.cl_num[1:0];
      re2ab_CfgValid   = re2ab_CfgValid_d;
      
    end
     
    always @(*)
    begin
        cr_ctl                = cr2re_ctl;
        cr_dsm_base           = cr2re_dsm_base;

        cr_inact_thresh       = cr2re_inact_thresh;
        cr_interrupt0         = cr2re_interrupt0;
        
        cr_wrthru_en          = cr2re_cfg[0];
        cr_cont               = cr2re_cfg[1];
        cr_mode               = cr2re_cfg[4:2];
        cr_multiCL_len        = cr2re_cfg[6:5];          
        cr_delay_en           = 1'b0;
        cr_rdsel              = cr2re_cfg[10:9];
        cr_test_cfg           = cr2re_cfg[27:20];
        cr_interrupt_on_error = cr2re_cfg[28];
        cr_interrupt_testmode = cr2re_cfg[29];
        cr_chsel              = cr2re_cfg[13:12]; 

    end

    always @(posedge Clk_400)
    begin
//       $display("Num_ticks,Num_Reads,Num_Writes: %d %d %d", {Num_ticks_high, Num_ticks_low}, Num_Reads, Num_Writes);
    end

    always @(posedge Clk_400)
    begin    
        if (WrFence_sent && (wait_at_end != WAIT_THIS_MUCH))
        begin
           wait_at_end <= wait_at_end + 1;
        end

        if(!test_Reset_n_internal1)
        begin
           wait_at_end <= 0;
        end
    end

    always @(posedge Clk_400)
    begin    
        re2cr_num_Rdpend       <= afu_idle_counter;
        re2cr_num_Wrpend       <= ivp_idle_counter;
        re2cr_num_reads        <= Num_Reads;
        re2cr_num_writes       <= Num_Writes;
        re2cr_error      <= ErrorVector;
        ds_stat_address  <= dsm_offset2addr(DSM_STATUS,cr_dsm_base);
        cr_rdsel_q       <= cr_rdsel;

        case(cr_rdsel_q)
            2'h0:   rdreq_type <= eREQ_RDLINE_S;
            2'h1:   rdreq_type <= eREQ_RDLINE_I;
            2'h2:   rdreq_type <= eREQ_RDLINE_I;
            2'h3:   rdreq_type <= rnd_rdreq_type;
        endcase
        rnd_rdreq_sel  <= 0;
        if(rnd_rdreq_sel)
            rnd_rdreq_type <= eREQ_RDLINE_I;
        else
            rnd_rdreq_type <= eREQ_RDLINE_S;

        if(test_go )                                             
            re2xy_go    <= 1'b1;
        if(status_write)
            re2xy_go    <= 1'b0;
        
        dsm_status_wren_a0<= ab2re_TestCmp | test_stop;              // Update Status upon test completion
        dsm_status_wren_a1<= ab2re_TestCmp | test_stop;              // Update Status upon test completion
        dsm_status_wren_b0<= ab2re_TestCmp | test_stop;              // Update Status upon test completion
        dsm_status_wren_b1<= ab2re_TestCmp | test_stop;              // Update Status upon test completion
        dsm_status_wren_c<= ab2re_TestCmp | test_stop;              // Update Status upon test completion

        // Wait for multi CL request to complete 
        // If Error detected or SW forced test termination
        // Make sure that multiCL request is completed before sending out DSM Write
        
        if (re2ab_Mode[2:0] == 3'b001)
        read_only_test   <= 1;
        
        if (cr_ctl[2] | tx_errorValid)
        test_cmplt       <= 1;        
        
        if (test_stop == 0)
        test_stop        <= test_cmplt & (read_only_test | (!(|txFifo_WrLen) & txFifo_WrValid));

        WrData_dsm <={ ab2re_ErrorInfo,                             // [511:256] upper half cache line
                       32'h0000_0000,                               // [255:224] zeros
                       32'h0000_0000,                               // [223:192] zeros
                       Num_Writes,                                  // [191:160] Total number of Writes sent / Total Num CX sent
                       Num_Reads,                                   // [159:128] Total number of Reads sent
                       24'h00_0000,Num_ticks_high, Num_ticks_low,   // [127:64]  number of clks
                       ErrorVector,                                 // [63:32]   errors detected            
                       16'h0000,                                    // [31:16]   zeroes
                       dsm_number,                                  // [15:1]    unique id for each dsm status write
                       1'h1                                         // [0]       test completion flag
                 };

        
        //Tx Path
        //--------------------------------------------------------------------------
        af2cp_sTxPort.c1.hdr        <= 0;
        af2cp_sTxPort.c1.valid      <= 0;
        af2cp_sTxPort.c0.hdr        <= 0;
        af2cp_sTxPort.c0.valid      <= 0;

        af2cp_sTxPort.c1.data[3*128 +: 128] <= dsm_status_wren_a0 ? WrData_dsm[3*128 +: 128] : txFifo_WrDin[3*128 +: 128]; 
        af2cp_sTxPort.c1.data[2*128 +: 128] <= dsm_status_wren_a1 ? WrData_dsm[2*128 +: 128] : txFifo_WrDin[2*128 +: 128]; 
        af2cp_sTxPort.c1.data[1*128 +: 128] <= dsm_status_wren_b0 ? WrData_dsm[1*128 +: 128] : txFifo_WrDin[1*128 +: 128]; 
        af2cp_sTxPort.c1.data[0*128 +: 128] <= dsm_status_wren_b1 ? WrData_dsm[0*128 +: 128] : txFifo_WrDin[0*128 +: 128]; 
    
            // Channel 1
            if (re2xy_go)
            begin
                  if( dsm_status_wren_c                                          // Write Fence
                     & !cp2af_sRxPort_T1.c1TxAlmFull
                     & !WrFence_sent
                  )
                  begin                                                         //-----------------------------------
                    if(WrFence_sent==0 ) 
                    begin
                        af2cp_sTxPort.c1.valid         <= 1'b1;
                    end
                    WrFence_sent                       <= 1'b1;
                    af2cp_sTxPort.c1.hdr.vc_sel        <= t_ccip_vc'(cr_chsel);
                    af2cp_sTxPort.c1.hdr.req_type      <= eREQ_WRFENCE;        
                    af2cp_sTxPort.c1.hdr.address[41:0] <= '0;
                    af2cp_sTxPort.c1.hdr.mdata[15:0]   <= '0;
                    af2cp_sTxPort.c1.hdr.sop           <= 1'b0;                 
                    af2cp_sTxPort.c1.hdr.cl_len        <= eCL_LEN_1;
                  end
        
                  if(                                                           // Write DSM Status
                     !cp2af_sRxPort_T1.c1TxAlmFull
                     && WrFence_sent
                     && (wait_at_end == WAIT_THIS_MUCH)
                  )
                  begin                                                         //-----------------------------------
                    if(status_write==0)
                    begin
                        dsm_number                     <= dsm_number + 1'b1;
                        af2cp_sTxPort.c1.valid         <= 1'b1;
                    end
                    status_write                       <= 1'b1;
                    af2cp_sTxPort.c1.hdr.vc_sel        <= t_ccip_vc'(cr_chsel);
                    af2cp_sTxPort.c1.hdr.req_type      <= eREQ_WRLINE_M;
                    af2cp_sTxPort.c1.hdr.address[41:0] <= ds_stat_address;
                    af2cp_sTxPort.c1.hdr.mdata[15:0]   <= 16'hffff;
                    af2cp_sTxPort.c1.hdr.sop           <= 1'b1;                 // DSM Write is single CL write
                    af2cp_sTxPort.c1.hdr.cl_len        <= eCL_LEN_1;
                  end
        
                else if( txFifo_WrValid & !test_stop )                        // Write to Destination Workspace
                begin                                                          //-------------------------------------
                    af2cp_sTxPort.c1.hdr.vc_sel        <= t_ccip_vc'(cr_chsel);
                    af2cp_sTxPort.c1.hdr.req_type      <= wrreq_type;
                    af2cp_sTxPort.c1.hdr.address[41:0] <= txFifo_WrAddr;
                    af2cp_sTxPort.c1.hdr.mdata[15:0]   <= txFifo_WrTID;
                    af2cp_sTxPort.c1.hdr.sop           <= txFifo_WrFence ? 0 : txFifo_WrSop;
                    af2cp_sTxPort.c1.hdr.cl_len        <= t_ccip_clLen'(txFifo_WrLen);
                    af2cp_sTxPort.c1.valid             <= 1'b1;
                    Num_Writes                         <= Num_Writes + 1'b1;
                end
            end // re2xy_go

        // Channel 0
        if(  re2xy_go
          && RdHdr_valid_q)                                                     // Read from Source Workspace
        begin                                                                   //----------------------------------
            af2cp_sTxPort.c0.hdr.vc_sel        <= t_ccip_vc'(cr_chsel);
            af2cp_sTxPort.c0.hdr.req_type      <= rdreq_type;
            af2cp_sTxPort.c0.hdr.address[41:0] <= ab2re_RdAddr_q;
            af2cp_sTxPort.c0.hdr.mdata[15:0]   <= ab2re_RdTID_q;
            af2cp_sTxPort.c0.valid             <= 1'b1;
            af2cp_sTxPort.c0.hdr.cl_len        <= t_ccip_clLen'(ab2re_RdLen_q);
            Num_Reads                          <= Num_Reads + re2xy_multiCL_len + 1'b1;   
        end
        //--------------------------------------------------------------------------
        // Rx Response Path
        //--------------------------------------------------------------------------
        cp2af_sRxPort_T1       <= cp2af_sRxPort;

        // Counters
        //--------------------------------------------------------------------------
        if(re2xy_go)                                                // Count #clks after test start
        begin
            Num_ticks_low   <= Num_ticks_low + 1'b1;
            if(&Num_ticks_low)
                Num_ticks_high  <= Num_ticks_high + 1'b1;
        end

        // Read Request
        tx_rd_req_len          <= af2cp_sTxPort.c0.hdr.cl_len[1:0];
        
        // Write Response
        rx_wr_resp_fmt         <= cp2af_sRxPort_T1.c1.hdr.format;
        rx_wr_resp_cl_num      <= cp2af_sRxPort_T1.c1.hdr.cl_num[1:0];
        
        
        re2ab_stallRd     <= 1'b0;
        
        // Error Detection Logic
        //--------------------------
        // synthesis translate_off
        if(|ErrorVector)
            $finish();
        // synthesis translate_on

        if(ErrorVector[3]==0)
            ErrorVector[3]  <= ab2re_ErrorValid;

        /* synthesis translate_off */
//        if(af2cp_sTxPort.c1.valid )
//            $display("*Req Type: %x \t Addr: %x \n Data: %x", af2cp_sTxPort.c1.hdr.req_type, af2cp_sTxPort.c1.hdr.address, af2cp_sTxPort.c1.data);

//        if(af2cp_sTxPort.c0.valid)
//            $display("*Req Type: %x \t Addr: %x", af2cp_sTxPort.c0.hdr.req_type, af2cp_sTxPort.c0.hdr.address);

        /* synthesis translate_on */

        if(!test_Reset_n_internal2)
        begin
            Num_Reads               <= 0;
            Num_Writes              <= 0;
            Num_ticks_low           <= 0;
            Num_ticks_high          <= 0;
            re2xy_go                <= 0;

            re2ab_CfgValid_d        <= 0;
            ErrorVector             <= 0;
            status_write            <= 0;
            dsm_status_wren_a1      <= 0;     
            dsm_status_wren_b1      <= 0;     
            dsm_status_wren_a0      <= 0;     
            dsm_status_wren_b0      <= 0;     
            dsm_status_wren_c       <= 0;     
            test_stop               <= 0;
            WrFence_sent            <= 0;  
            test_cmplt              <= 0;
            read_only_test          <= 0;            
        end
    end

    always @(*)
    begin
        RdHdr_valid = re2xy_go && !status_write && ab2re_RdEn;
        re2ab_RdSent = RdHdr_valid;
    end

    always @(*)
    begin
        wrreq_type   = txFifo_WrFence ? eREQ_WRFENCE
                      :cr_wrthru_en   ? eREQ_WRLINE_I
                                      : eREQ_WRLINE_M;
    end

    logic [1:0] txFifo_WrLen_P1;
    logic txFifo_WrSop_P1;
    logic txFifo_WrFence_P1;
    logic [DATA_WIDTH-1:0] txFifo_WrDin_P1;
    logic [ADDR_LMT-1:0] txFifo_WrAddr_P1;
    logic [15:0] txFifo_WrTID_P1;

    logic [1:0] txFifo_WrLen_P2;
    logic txFifo_WrSop_P2;
    logic txFifo_WrFence_P2;
    logic [DATA_WIDTH-1:0] txFifo_WrDin_P2;
    logic [ADDR_LMT-1:0] txFifo_WrAddr_P2;
    logic [15:0] txFifo_WrTID_P2;

    reg   txFifo_WrValid_P2;

    (* dont_merge *)    reg delayed_en_0;
    (* dont_merge *)    reg delayed_en_1;
    (* dont_merge *)    reg delayed_en_2;
    (* dont_merge *)    reg delayed_en_3;
    (* dont_merge *)    reg delayed_en_4;
    (* dont_merge *)    reg delayed_en_5;
    (* dont_merge *)    reg delayed_en_6;
    (* dont_merge *)    reg delayed_en_7;
    (* dont_merge *)    reg delayed_en_8;
    (* dont_merge *)    reg delayed_en_9;

    always @(posedge Clk_400)
    begin
   
       delayed_en_0  <= re2xy_go && (ab2re_WrEn | ab2re_WrFence);
       delayed_en_1  <= re2xy_go && (ab2re_WrEn | ab2re_WrFence);
       delayed_en_2  <= re2xy_go && (ab2re_WrEn | ab2re_WrFence);
       delayed_en_3  <= re2xy_go && (ab2re_WrEn | ab2re_WrFence);
       delayed_en_4  <= re2xy_go && (ab2re_WrEn | ab2re_WrFence);
       delayed_en_5  <= re2xy_go && (ab2re_WrEn | ab2re_WrFence);
       delayed_en_6  <= re2xy_go && (ab2re_WrEn | ab2re_WrFence);
       delayed_en_7  <= re2xy_go && (ab2re_WrEn | ab2re_WrFence);
       delayed_en_8  <= re2xy_go && (ab2re_WrEn | ab2re_WrFence);
       delayed_en_9  <= re2xy_go && (ab2re_WrEn | ab2re_WrFence);

       txFifo_WrLen_P1   <= ab2re_WrLen;
       txFifo_WrSop_P1   <= ab2re_WrSop;
       txFifo_WrFence_P1 <= ab2re_WrFence;
       txFifo_WrDin_P1   <= ab2re_WrDin;
       txFifo_WrAddr_P1  <= ab2re_WrAddr;
       txFifo_WrTID_P1   <= ab2re_WrTID;

       if ( delayed_en_0) txFifo_WrDin_P2[0*64 +: 64]  <= txFifo_WrDin_P1[0*64 +: 64];
       if ( delayed_en_1) txFifo_WrDin_P2[1*64 +: 64]  <= txFifo_WrDin_P1[1*64 +: 64];
       if ( delayed_en_2) txFifo_WrDin_P2[2*64 +: 64]  <= txFifo_WrDin_P1[2*64 +: 64];
       if ( delayed_en_3) txFifo_WrDin_P2[3*64 +: 64]  <= txFifo_WrDin_P1[3*64 +: 64];
       if ( delayed_en_4) txFifo_WrDin_P2[4*64 +: 64]  <= txFifo_WrDin_P1[4*64 +: 64];
       if ( delayed_en_5) txFifo_WrDin_P2[5*64 +: 64]  <= txFifo_WrDin_P1[5*64 +: 64];
       if ( delayed_en_6) txFifo_WrDin_P2[6*64 +: 64]  <= txFifo_WrDin_P1[6*64 +: 64];
       if ( delayed_en_7) txFifo_WrDin_P2[7*64 +: 64]  <= txFifo_WrDin_P1[7*64 +: 64];

       if ( delayed_en_8)
       begin
          txFifo_WrLen_P2   <= txFifo_WrLen_P1;
          txFifo_WrSop_P2   <= txFifo_WrSop_P1;
          txFifo_WrFence_P2 <= txFifo_WrFence_P1;
          txFifo_WrTID_P2   <= txFifo_WrTID_P1;
       end
       if ( delayed_en_9)
       begin
          txFifo_WrAddr_P2  <= txFifo_WrAddr_P1;
       end

       txFifo_WrValid_P2 <= delayed_en_8;

/*
       txFifo_WrLen   <= txFifo_WrLen_P2;
       txFifo_WrSop   <= txFifo_WrSop_P2;
       txFifo_WrFence <= txFifo_WrFence_P2;
       txFifo_WrDin   <= txFifo_WrDin_P2;
       txFifo_WrAddr  <= txFifo_WrAddr_P2;
       txFifo_WrTID   <= txFifo_WrTID_P2;

       txFifo_WrValid  <= txFifo_WrValid_P2;
*/

       if (!test_Reset_n_internal3)
       begin
//          txFifo_WrValid <= 0;
          txFifo_WrValid_P2 <= 0;
          delayed_en_0 <= 0;
          delayed_en_1 <= 0;
          delayed_en_2 <= 0;
          delayed_en_3 <= 0;
          delayed_en_4 <= 0;
          delayed_en_5 <= 0;
          delayed_en_6 <= 0;
          delayed_en_7 <= 0;
          delayed_en_8 <= 0;
          delayed_en_9 <= 0;
       end 

    end

    always_comb
      begin
       txFifo_WrLen   = txFifo_WrLen_P2;
       txFifo_WrSop   = txFifo_WrSop_P2;
       txFifo_WrFence = txFifo_WrFence_P2;
       txFifo_WrDin   = txFifo_WrDin_P2;
       txFifo_WrAddr  = txFifo_WrAddr_P2;
       txFifo_WrTID   = txFifo_WrTID_P2;

       txFifo_WrValid  = txFifo_WrValid_P2;
      end

    // Function: Returns physical address for a DSM register
    function automatic [41:0] dsm_offset2addr;
        input    [9:0]  offset_b;
        input    [63:0] base_b;
        begin
            dsm_offset2addr = base_b[47:6] + offset_b[9:6];
        end
    endfunction

endmodule
