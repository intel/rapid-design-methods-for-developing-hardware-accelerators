#!/usr/bin/env python3

from cog_acctempl import *

dut = DUT("trws")

dut.add_ut( UserType("CacheLine",[ArrayField(SignedShortField("words"),32)]))
dut.add_ut( UserType("UCacheLine",[ArrayField(UnsignedShortField("words"),32)]))
dut.add_ut( UserType("WrappedShort",[SignedShortField("data")]))
dut.add_ut( UserType("Pair",[SignedShortField("a"),UnsignedShortField("e")]))

dut.add_extra_config_fields( [UnsignedIntField("nCLperRow"),
                              UnsignedIntField("nSlices")])



dut.add_module( Module("trws_wu"))
dut.add_module( Module("trws_loaf"))
dut.add_module( Module("trws_clamper"))

dut.add_rds( [TypedRead("UCacheLine","gi","__gi_Slots__","1 << 28","1"),
              TypedRead("CacheLine","wi","__wi_Slots__","1 << 28","1"),
              TypedRead("UCacheLine","mi","__mi_Slots__","1 << 28","1")])
dut.add_wrs( [TypedWrite("UCacheLine","mo")])

dut.add_rds( [TypedRead("Pair","inp","__inp_Slots__","1 << 28","1")])

dut.add_storage_fifo( StorageFIFO( "CacheLine", 2, "off"))
dut.add_storage_fifo( StorageFIFO( "CacheLine", 2, "out"))

dut.add_storage_fifo( StorageFIFO( "CacheLine", 2, "out0"))
dut.add_storage_fifo( StorageFIFO( "CacheLine", 6, "out1"))
dut.add_storage_fifo( StorageFIFO( "WrappedShort", 2, "obest"))

dut.add_storage_fifo( StorageFIFO( "UCacheLine", 2, "res"))

dut.get_module("trws_wu").add_cthreads( [CThread("updater",writes_to_done=True),
                                         CThread("gi_addr_gen"),
                                         CThread("wi_addr_gen"),
                                         CThread("mi_addr_gen"),
                                         CThread("mo_addr_gen")])

dut.get_cthread( "updater").add_ports( [RdRespPort("gi"),
                                        RdRespPort("wi"),
                                        RdRespPort("mi"),
                                        DequeuePort("res"),
                                        EnqueuePort("off"),
                                        WrDataPort("mo")])

dut.get_cthread( "gi_addr_gen").add_ports( [RdReqPort("gi")])
dut.get_cthread( "wi_addr_gen").add_ports( [RdReqPort("wi")])
dut.get_cthread( "mi_addr_gen").add_ports( [RdReqPort("mi")])
dut.get_cthread( "mo_addr_gen").add_ports( [WrReqPort("mo")])

dut.get_module("trws_clamper").add_cthreads( [CThread("split_out"),
                                              CThread("compute_best"),
                                              CThread("subtract_out")])

dut.get_cthread( "split_out").add_ports( [DequeuePort("out"),
                                          EnqueuePort("out0"),
                                          EnqueuePort("out1")])

dut.get_cthread( "compute_best").add_ports( [DequeuePort("out0"),
                                             EnqueuePort("obest")])

dut.get_cthread( "subtract_out").add_ports( [DequeuePort("out1"),
                                             DequeuePort("obest"),
                                             EnqueuePort("res")])

dut.get_module("trws_loaf").add_cthreads( [CThread("inp_fetcher"),
                                           CThread("inp_addr_gen")])

dut.get_cthread( "inp_fetcher").add_ports( [RdRespPort("inp"),
                                            DequeuePort("off"),
                                            EnqueuePort("out")])

dut.get_cthread( "inp_addr_gen").add_ports( [RdReqPort("inp")])


dut.semantic()

if __name__ == "__main__":
    dut.semantic()
    dut.dump_dot( dut.nm + ".dot")
