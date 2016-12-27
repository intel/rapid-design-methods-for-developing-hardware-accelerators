#!/usr/bin/env python3

from cog_acctempl import *

dut = DUT("vectoradd")

dut.add_ut( UserType("Blk",[ArrayField(UnsignedIntField("words"),16)]))

dut.add_rds( [TypedRead("Blk","ina","__inp_Slots__","1 << 30","1")])
dut.add_rds( [TypedRead("Blk","inb","__inp_Slots__","1 << 30","1")])
dut.add_wrs( [TypedWrite("Blk","out")])

dut.add_extra_config_fields( [BitReducedField(UnsignedIntField("n"),32)])

dut.module.add_cthreads( [CThread("fetcher",writes_to_done=True),
                          CThread("ina_addr_gen",ports=[RdReqPort("ina")]),
                          CThread("inb_addr_gen",ports=[RdReqPort("inb")]),
                          CThread("out_addr_gen",ports=[WrReqPort("out")])])

dut.get_cthread( "fetcher").add_ports( [RdRespPort("ina"),
                                        RdRespPort("inb"),
                                        WrDataPort("out")])

dut.semantic()

if __name__ == "__main__":
    dut.dump_dot( dut.nm + ".dot")
