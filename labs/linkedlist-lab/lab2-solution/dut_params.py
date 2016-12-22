from cog_acctempl import *

dut = DUT("cycle_detection")

#LAB2 specify memory ports to access Node and CycleExistInfo and add them to dut

inps = [TypedRead("Node","inp",buf_size_in_cl = 2, max_burst_count = 1)]
outs = [TypedWrite("CycleExistInfo","out")]

dut.add_rds(inps)
dut.add_wrs(outs)

#END OF LAB2

dut.add_ut( UserType("Node",[UnsignedLongLongField("next_offset"), UnsignedLongLongField("val")]) )
dut.add_ut( UserType("CycleExistInfo",[UnsignedIntField("info")]) )
