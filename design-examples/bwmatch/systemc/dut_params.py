#!/usr/bin/env python3

from cog_acctempl import *

dut = DUT("bwmatch")

dut.add_rds( [TypedRead("BWPattern",  "pat","__BWPatternLoadParamsSlots__",  "1<<24","1"),
              SingleRead("BWCacheLine", "BWState", "cl", "__BWCacheLineLoadParamsSlots__"),
              TypedRead("BWResult",   "pre","__BWResultLoadParamsSlots__",   "1<<24","1")])

dut.add_wr( TypedWrite("BWResultLine","res"))

dut.add_ut( UserType("BWCacheLine",[ArrayField(UnsignedLongLongField("bwa"),4),
                                    ArrayField(UnsignedIntField("ranks"),4),
                                    ArrayField(UnsignedLongLongField("pad"),2)]))

dut.add_ut( UserType("BWResult",[BitReducedField(UnsignedIntField("l"),32),
                                 BitReducedField(UnsignedIntField("u"),32)]))

dut.add_ut( UserType("BWResultLine",[ArrayField(UserTypeField("line",dut.get_ut("BWResult")),8)]))

dut.add_ut( UserType("BWPattern",[BitReducedField(UnsignedLongLongField("pat"),58),
                                  BitReducedField(UnsignedLongLongField("length"),6)]))

dut.add_ut( UserType("BWState",[UnsignedIntField("idx"),
                                UserTypeField("res",dut.get_ut("BWResult")),
                                UserTypeField("pat",dut.get_ut("BWPattern")),
                                BitReducedField(UnsignedIntField("state"),2)]))

dut.add_extra_config_fields( [BitReducedField(UnsignedLongLongField("nPat"),48),
                              BitReducedField(UnsignedLongLongField("max_recirculating"),12),
                              BitReducedField(UnsignedLongLongField("precomp_len"),4),
                              BitReducedField(UnsignedIntField("end_pos"),32),
                              BitReducedField(UnsignedIntField("u0"),32),
                              BitReducedField(UnsignedIntField("u1"),32),
                              BitReducedField(UnsignedIntField("u2"),32),
                              BitReducedField(UnsignedIntField("u3"),32)])

dut.add_ports_without_an_address( [])

dut.add_storage_fifo( StorageFIFO( "BWState", 4, "patQ"))
dut.add_storage_fifo( StorageFIFO( "BWState", 4, "irowQ"))
dut.add_storage_fifo( StorageFIFO( "BWState", 4, "partialResultQ"))
dut.add_storage_fifo( StorageFIFO( "BWState", 4, "finalResultQ"))
dut.add_storage_fifo( StorageFIFO( "BWIdx", 4, "reserveQ"))
dut.add_storage_fifo( StorageFIFO( "BWEmpty", 4, "reserveAckQ"))

dut.add_module( Module("bwmatch_retire"))
dut.add_module( Module("bwmatch_compute"))
dut.get_module("bwmatch_retire").add_cthread( CThread("res_reorder"))
dut.get_module("bwmatch_retire").add_cthread( CThread("pat_gadget"))
dut.get_module("bwmatch_retire").add_cthread( CThread("precomp_addr_gen"))
dut.get_module("bwmatch_retire").add_cthread( CThread("precomp_fill"))
dut.get_module("bwmatch_retire").add_cthread( CThread("pat_gadget2"))
dut.get_module("bwmatch_retire").add_cthread( CThread("pat_addr_gen"))
dut.get_module("bwmatch_compute").add_cthread( CThread("pat_fetcher",writes_to_done=True))
dut.get_module("bwmatch_compute").add_cthread( CThread("occurrence_wr_loop"))
dut.get_module("bwmatch_compute").add_cthread( CThread("occurrence_rd_loop"))

dut.get_cthread( "pat_fetcher").add_port( DequeuePort("patQ"))
dut.get_cthread( "pat_fetcher").add_port( EnqueuePort("irowQ"))
dut.get_cthread( "pat_fetcher").add_port( DequeuePort("partialResultQ"))
dut.get_cthread( "pat_fetcher").add_port( EnqueuePort("finalResultQ"))

dut.get_cthread( "pat_gadget").add_port( EnqueuePort("reserveQ"))

dut.get_cthread( "precomp_addr_gen").add_port( RdReqPort("pre"))
dut.get_cthread( "precomp_fill").add_port( RdRespPort("pre"))
dut.get_cthread( "pat_gadget2").add_port( RdRespPort("pat"))
dut.get_cthread( "pat_gadget2").add_port( DequeuePort("reserveAckQ"))
dut.get_cthread( "pat_gadget2").add_port( EnqueuePort("patQ"))

dut.get_cthread( "pat_addr_gen").add_port( RdReqPort("pat"))

dut.get_cthread( "res_reorder").add_port( DequeuePort("reserveQ"))
dut.get_cthread( "res_reorder").add_port( EnqueuePort("reserveAckQ"))
dut.get_cthread( "res_reorder").add_port( DequeuePort("finalResultQ"))
dut.get_cthread( "res_reorder").add_port( WrReqPort("res"))
dut.get_cthread( "res_reorder").add_port( WrDataPort("res"))

dut.get_cthread( "occurrence_wr_loop").add_port( RdReqPort("cl"))
dut.get_cthread( "occurrence_wr_loop").add_port( DequeuePort("irowQ"))

dut.get_cthread( "occurrence_rd_loop").add_port( RdRespPort("cl"))
dut.get_cthread( "occurrence_rd_loop").add_port( EnqueuePort("partialResultQ"))

dut.semantic()

if __name__ == "__main__":
    dut.dump_dot("bwmatch.dot")
