#!/usr/bin/env python3

import sys
from cogapp import Cog
from dut_params import dut
import os

assert "HLD_ROOT" in os.environ

scripts_dir = os.environ["HLD_ROOT"] + "/scripts/systemc-gen"

ret = False
for ty in dut.usertypes.keys():
    out_fn = "%s.h" % ty
    if os.path.isfile( out_fn):
      lst = ["cog.py", "-r", "-c", "-I.", "-I" + scripts_dir + "/", out_fn]
    else:
      lst = ["cog.py", "-c", "-I.", ("-Dty=%s" % ty), "-I" + scripts_dir + "/", "-o", out_fn, scripts_dir + "/Usertype-cog.h"]
    print(lst)
    rc = Cog().main( lst)
    ret = ret or rc

sys.exit( ret)
