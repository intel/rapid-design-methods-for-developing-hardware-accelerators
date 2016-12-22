from cog_acctempl import *

dut_name = "cycle_detection"
inps = [TypedRead("Node","inp",buf_size_in_cl = 2, max_burst_count = 1)]
outs = [TypedWrite("CycleExistInfo","out")]

usertypes = {}

usertypes["Node"] = UserType("Node",[UnsignedLongLongField("next_offset"), UnsignedLongLongField("val")])
usertypes["CycleExistInfo"] = UserType("CycleExistInfo",[BitReducedField(UnsignedIntField("info"),32)])

extra_config_fields = []

__all__ = ["dut_name", "inps", "outs", "usertypes", "extra_config_fields"]

