// See LICENSE for license details.

`ifndef _hld_app_vh_
`define _hld_app_vh_


// Supported combinations (rd-wr): 1-1, 2-1, 2-2, 4-1, 4-4
`define HLD_MEM_RD_PORTS 1
`define HLD_MEM_WR_PORTS 1

//CLK_400 or CLK_200 or CLK_100
`define HLD_ACC_CLK CLK_200

// ---- REGENERATE THE IDS BELOW FOR YOUR AFU ----- 
//xxd -l 8 -p /dev/random 
`define HLD_AFU_ID_H 64'h5cdf79ed09527e2c
`define HLD_AFU_ID_L 64'hbfa1f873fa0e52cf

//in bits
`define HLD_APP_CONFIG_WIDTH 320

// the name of the top module in HLS
`define HLD_AFU_MODULE_NAME vectoradd_acc_rtl

//
`define HLD_RESP_ASYNC_FIFO_LOG2DEPTH 7

//dependent on the number of ACC buffers
`define HLD_REQ_ASYNC_FIFO_LOG2DEPTH 4

`endif //_hld_app_vh_
