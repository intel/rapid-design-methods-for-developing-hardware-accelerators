// See LICENSE for license details.
`ifndef _hld_app_vh_
`define _hld_app_vh_

`define HLD_MEM_RD_PORTS 1
`define HLD_MEM_WR_PORTS 1


// in Mhz //CLK_400=3'b000, CLK_200=3'b001, CLK_100=3'b010
`define HLD_ACC_CLK CLK_200

// ---- REGENERATE THE IDS BELOW FOR YOUR AFU ----- 
//xxd -l 8 -p /dev/random 
`define HLD_AFU_ID_H 64'h30b1cf9b9bee84e7
`define HLD_AFU_ID_L 64'hfd2e242e7efcb9d8

//in bits
`define HLD_APP_CONFIG_WIDTH 128

`define HLD_AFU_MODULE_NAME cycledetection_acc_rtl

//dependent on the number of ACC buffers
`define HLD_REQ_ASYNC_FIFO_LOG2DEPTH 4
//
`define HLD_RESP_ASYNC_FIFO_LOG2DEPTH 4


`endif //_hld_app_vh_
