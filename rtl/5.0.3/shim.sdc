set_false_path -to *|shim|d_rst
set_false_path -to *|shim|d_config_[*]
set_false_path -to *|shim|d_start
set_false_path -from *|shim|a_config_[*]

set_false_path -from [get_clocks {uClk_usrDiv2}] -to [get_clocks {pClk}]
set_false_path -from [get_clocks {pClk}] -to [get_clocks {uClk_usrDiv2}]

#Want to try this with pClk4 too
#set_false_path -from [get_clocks {pClk4}] -to [get_clocks {pClk}]
#set_false_path -from [get_clocks {pClk}] -to [get_clocks {pClk4}]

#
# Done signal from a_clk domain to clk domain
#
set_false_path -to *|shim|c_done[0]

#
# Want to try without this one
#
set_false_path -from *|accl_top|hld_shim_inst|shim|rd_req_rst0|q -to [get_registers {*dcfifo*rs_dgwp*}] -setup
set_false_path -from *|accl_top|hld_shim_inst|shim|rd_req_rst0|q -to [get_registers {*dcfifo*ws_dgrp*}] -setup

set_false_path -from [get_registers {*dcfifo*delayed_wrptr_g[*]}] -to [get_registers {*dcfifo*rs_dgwp*}]
set_false_path -from [get_registers {*dcfifo*rdptr_g[*]}] -to [get_registers {*dcfifo*ws_dgrp*}]

#
# We don't use the generic csr_reg immediately
#
set_false_path -through inst_ccip_interface_reg|inst_green_top|inst_ccip_std_afu|accl_top|inst_accl_csr|cr2xy_generic_config[*] -setup

#
# We don't need the DSM address immediately either
#
set_false_path -from inst_ccip_interface_reg|inst_green_top|inst_ccip_std_afu|accl_top|inst_accl_csr|csr_reg[*][*] -to inst_ccip_interface_reg|inst_green_top|inst_ccip_std_afu|accl_top|inst_requestor|ds_stat_address[*] -setup
 

