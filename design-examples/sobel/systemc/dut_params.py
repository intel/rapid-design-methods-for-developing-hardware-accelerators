#!/usr/bin/env python3

from cog_acctempl import *

dut = DUT("sobel")


dut.add_ut( UserType("BlkInp",[ArrayField(UnsignedCharField("data"),64)]))
dut.add_ut( UserType("BlkOut",[ArrayField(SignedCharField("data"),64)]))
dut.add_ut( UserType("BlkMid",[ArrayField(SignedShortField("data"),64)]))

dut.add_rd( TypedRead( "BlkInp", "inp", "__inp_Slots__",  "1<<24", "1"))
dut.add_wr( TypedWrite("BlkOut","out"))

dut.add_extra_config_fields( [UnsignedIntField("nInp"),
                              UnsignedIntField("num_of_rows"),
                              UnsignedIntField("row_size_in_blks"),
                              UnsignedIntField("image_size_in_bytes"),
                              UnsignedIntField("num_of_images")])

dut.add_storage_fifo( StorageFIFO( "BlkMid", 2, "mid0"))
dut.add_storage_fifo( StorageFIFO( "BlkMid", 2, "mid1"))

dut.add_module( Module("sobel_frontend"))
dut.add_module( Module("sobel_backend"))
dut.add_module( Module("sobel_restrict"))
dut.add_cthread( CThread("out_addr_gen"))
dut.add_cthread( CThread("inp_addr_gen"))
dut.get_module("sobel_frontend").add_cthread( CThread("deltax"))
dut.get_module("sobel_backend").add_cthread( CThread("deltay"))
dut.get_module("sobel_restrict").add_cthread( CThread("clip",writes_to_done=1))

dut.get_cthread( "inp_addr_gen").add_port( RdReqPort("inp"))
dut.get_cthread( "deltax").add_port( RdRespPort("inp"))
dut.get_cthread( "deltax").add_port( EnqueuePort( "mid0"))

dut.get_cthread( "deltay").add_port( DequeuePort( "mid0"))
dut.get_cthread( "deltay").add_port( EnqueuePort( "mid1"))

dut.get_cthread( "out_addr_gen").add_port( WrReqPort("out"))
dut.get_cthread( "clip").add_port( WrDataPort("out"))
dut.get_cthread( "clip").add_port( DequeuePort( "mid1"))

#dut.semantic()

if __name__ == "__main__":
    dut.dump_dot( dut.nm + ".dot")
