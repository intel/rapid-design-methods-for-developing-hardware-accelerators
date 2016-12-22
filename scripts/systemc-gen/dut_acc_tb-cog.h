// See LICENSE for license details.
/*[[[cog
     import cog
     from cog_acctempl import *
     from dut_params import *
  ]]]*/
//[[[end]]]
/*[[[cog
     cog.outl("#ifndef __%s_ACC_TB_H__" % dut.nm.upper())
     cog.outl("#define __%s_ACC_TB_H__" % dut.nm.upper())
  ]]]*/
//[[[end]]]

/*[[[cog
     cog.outl("#include \"%s_hls.h\"" % dut.nm)
  ]]]*/
  //[[[end]]]
#include "accl_tb.h"
  
typedef acc_top < dut_t, 100/*Mhz*/, 1000/*mem latency in ns*/, RD_CHANNELS, WR_CHANNELS> AcclApp;

#endif
