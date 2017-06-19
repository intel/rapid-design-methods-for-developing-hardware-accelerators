// See LICENSE for license details.
`ifndef _hld_app_vh_
`define _hld_app_vh_

`define HLD_MEM_RD_PORTS 4
`define HLD_MEM_WR_PORTS 1

//CLK_400 or CLK_273 or CLK_200 or CLK_136 or CLK_100
`define HLD_ACC_CLK CLK_100

//xxd -l 8 -p /dev/random
`define HLD_AFU_ID_H 64'hce53_7515_9d3a_cabc
`define HLD_AFU_ID_L 64'h94ec_b552_08f1_5b01

//in bytes
`define HLD_APP_CONFIG_WIDTH 320

`define HLD_AFU_MODULE_NAME pr4au_rtl

//dependent on the number of ACC buffers
`define HLD_REQ_ASYNC_FIFO_LOG2DEPTH 4
//
`define HLD_RESP_ASYNC_FIFO_LOG2DEPTH 7


`endif //_hld_app_vh_
