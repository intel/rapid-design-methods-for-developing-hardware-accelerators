#!/usr/bin/env python3

import sys
from cogapp import Cog
from dut_params import dut
import os

assert "HLD_ROOT" in os.environ

scripts_dir = os.environ["HLD_ROOT"] + "/scripts/systemc-gen"

ret = False
for m in dut.modules.values():
    out_fn = "%s.h" % m.nm
    if os.path.isfile( out_fn):
      lst = ["cog.py", "-r", "-c", "-I.", "-I" + scripts_dir + "/", out_fn]
    else:
      lst = ["cog.py", "-c", ("-Dmodule_nm=%s" % m.nm), "-I.", "-I" + scripts_dir + "/", "-o", out_fn, scripts_dir + "/module-cog.h"]
    print(lst)
    rc = Cog().main( lst)
    ret = ret or rc

sys.exit( ret)
