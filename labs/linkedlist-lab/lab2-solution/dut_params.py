from cog_acctempl import *

dut = DUT("cycle_detection")

dut.add_ut( UserType("Node",[UnsignedLongLongField("next_offset"), UnsignedLongLongField("val")]) )
dut.add_ut( UserType("CycleExistInfo",[UnsignedIntField("info")]) )

#LAB2 specify memory ports to access Node and CycleExistInfo and add them to dut
inps = [TypedRead("Node","inp",buf_size_in_cl = 2, max_burst_count = 1)]
outs = [TypedWrite("CycleExistInfo","out")]
#buf_size_in_cl = 2 is because we will do at most 2 parallel memory requests for data at slow and fast pointers and we would like those requests to fly in parallel
#the max_burst_count is 1 also because we request single element from memory each time (no streaming in this application)

dut.add_rds(inps)
dut.add_wrs(outs)

#END OF LAB2
