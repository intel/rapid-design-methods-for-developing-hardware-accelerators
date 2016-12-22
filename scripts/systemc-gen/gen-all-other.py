#!/usr/bin/env python3

import sys
from cogapp import Cog
from dut_params import dut
import os

assert "HLD_ROOT" in os.environ

scripts_dir = os.environ["HLD_ROOT"] + "/scripts/systemc-gen"

ret = False

pairs = [("Config.h", "Config-addresses-cog.h"),
         ( dut.nm + "_hls.h", "dut_hls-hierarchy-cog.h"),
         ( dut.nm + "_hls_tb.h", "dut_hls_tb-cog.h"),
         ( dut.nm + "_acc.h", "dut_acc-cog.h"),
         ( dut.nm + "_acc_tb.h", "dut_acc_tb-cog.h"),
         ( dut.nm + "_sched.h", "dut_sched-cog.h")]

for (tgt,src) in pairs:
    if os.path.isfile( tgt):
      lst = ["cog.py", "-r", "-c", "-I.", "-I" + scripts_dir + "/", tgt]
    else:
      lst = ["cog.py", "-c", "-I.", "-I" + scripts_dir + "/", "-o", tgt, scripts_dir + "/" + src]
    print(lst)
    rc = Cog().main( lst)
    ret = ret or rc

sys.exit( ret)
