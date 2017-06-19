from cog_acctempl import *

dut = DUT("cycle_detection")

dut.add_ut( UserType("Node",[UnsignedLongLongField("next_offset"), UnsignedLongLongField("val")]) )
dut.add_ut( UserType("CycleExistInfo",[UnsignedIntField("info")]) )

#LAB2 specify memory ports to access Node and CycleExistInfo and add them to dut

#END OF LAB2
