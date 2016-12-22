#!/bin/csh

cog.py -d -D RD_PORTS=1 -D WR_PORTS=1 shim_cog_template.sv > hld_shim_1_1.sv
cog.py -d -D RD_PORTS=2 -D WR_PORTS=1 shim_cog_template.sv > hld_shim_2_1.sv
cog.py -d -D RD_PORTS=2 -D WR_PORTS=2 shim_cog_template.sv > hld_shim_2_2.sv
cog.py -d -D RD_PORTS=4 -D WR_PORTS=1 shim_cog_template.sv > hld_shim_4_1.sv
cog.py -d -D RD_PORTS=4 -D WR_PORTS=4 shim_cog_template.sv > hld_shim_4_4.sv
