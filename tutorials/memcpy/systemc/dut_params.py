#!/usr/bin/env python3

from cog_acctempl import *

dut = DUT("memcpy")

dut.add_rds( [TypedRead("CacheLine","inp","__inp_Slots__","1 << 24","1")])
dut.add_wrs( [TypedWrite("CacheLine","out")])

dut.add_ut( UserType("CacheLine",[ArrayField(UnsignedLongLongField("words"),8)]))

dut.add_extra_config_fields( [BitReducedField(UnsignedIntField("nCLs"),32)])

dut.module.add_cthreads( [CThread("inp_fetcher",writes_to_done=True),
                          CThread("inp_addr_gen"),
                          CThread("out_addr_gen")])

dut.get_cthread( "inp_fetcher").add_ports( [RdRespPort("inp"),
                                            WrDataPort("out")])

dut.get_cthread( "inp_addr_gen").add_ports( [RdReqPort("inp")])
dut.get_cthread( "out_addr_gen").add_ports( [WrReqPort("out")])

dut.semantic()

if __name__ == "__main__":
    dut.dump_dot( dut.nm + ".dot")
