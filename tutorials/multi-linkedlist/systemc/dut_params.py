#!/usr/bin/env python3
from cog_acctempl import *

dut = DUT("linkedlist")

dut.add_ut( UserType("HeadPtr",[UnsignedLongLongField("head")]))
dut.add_ut( UserType("Node",[UnsignedLongLongField("next"),
                             UnsignedLongLongField("val")]))

dut.add_rds( [SingleRead("Node","State","lst","__lst_Slots__")])
dut.add_rds( [TypedRead("HeadPtr","inp","__inp_Slots__","1 << 30","1")])
dut.add_wrs( [TypedWrite("HeadPtr","out")])

dut.add_extra_config_fields( [UnsignedLongLongField("m")])

dut.module.add_cthreads( [CThread("fetcher",ports=[RdRespPort("inp"),
                                                   WrDataPort("out"),
                                                   RdReqPort("lst"),
                                                   RdRespPort("lst")],
                                  writes_to_done=True),
                          CThread("inp_addr_gen",ports=[RdReqPort("inp")]),
                          CThread("out_addr_gen",ports=[WrReqPort("out")])])

dut.semantic()

if __name__ == "__main__":
    dut.dump_dot( dut.nm + ".dot")
