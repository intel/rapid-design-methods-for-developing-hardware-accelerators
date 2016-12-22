#!/usr/bin/env python3

import sys
from cogapp import Cog
from dut_params import dut
import os

assert "HLD_ROOT" in os.environ

scripts_dir = os.environ["HLD_ROOT"] + "/scripts/systemc-gen"

ret = False
for c in dut.cthreads_generator:

    out_fn = "%s-%s.h" % (dut.nm,c.nm)

    if os.path.isfile( out_fn):
        lst = ["cog.py", "-r", "-c", "-I.", "-I" + scripts_dir + "/", out_fn]
    else:
        lst = ["cog.py", "-c", ("-Dthread_nm=%s" % c.nm), "-I.", "-I" + scripts_dir + "/", "-o", out_fn, scripts_dir + "/cthread-cog.h"]

    print(lst)
    rc = Cog().main( lst)
    ret = ret or rc

sys.exit( ret)
