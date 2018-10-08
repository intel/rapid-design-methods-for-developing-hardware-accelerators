#set_multicycle_path -setup -end -through [get_nets {*inst_nlb_csr*cr2re_src_address*} ] 2
#set_multicycle_path -hold  -end -through [get_nets {*inst_nlb_csr*cr2re_src_address*} ] 1
#
#set_multicycle_path -setup -end -through [get_nets {*inst_nlb_csr*cr2re_dst_address*} ] 2
#set_multicycle_path -hold  -end -through [get_nets {*inst_nlb_csr*cr2re_dst_address*} ] 1
#
#set_multicycle_path -setup -end -through [get_nets {*inst_nlb_csr*cr2re_num_lines*} ] 2
#set_multicycle_path -hold  -end -through [get_nets {*inst_nlb_csr*cr2re_num_lines*} ] 1
#
#set_multicycle_path -setup -end -through [get_nets {*inst_nlb_csr*cr2re_inact_thresh*} ] 2
#set_multicycle_path -hold  -end -through [get_nets {*inst_nlb_csr*cr2re_inact_thresh*} ] 1
#
#set_multicycle_path -setup -end -through [get_nets {*inst_nlb_csr*cr2re_interrupt0*} ] 2
#set_multicycle_path -hold  -end -through [get_nets {*inst_nlb_csr*cr2re_interrupt0*} ] 1
#
#set_multicycle_path -setup -end -through [get_nets {*inst_nlb_csr*cr2re_cfg*} ] 2
#set_multicycle_path -hold  -end -through [get_nets {*inst_nlb_csr*cr2re_cfg*} ] 1
#
#set_multicycle_path -setup -end -through [get_nets {*inst_nlb_csr*cr2re_dsm_base*} ] 2
#set_multicycle_path -hold  -end -through [get_nets {*inst_nlb_csr*cr2re_dsm_base*} ] 1
