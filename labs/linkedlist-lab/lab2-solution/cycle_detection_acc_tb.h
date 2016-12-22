// See LICENSE for license details.
/*[[[cog
     import cog
     from cog_acctempl import *
     from dut_params import *
  ]]]*/
//[[[end]]] (checksum: d41d8cd98f00b204e9800998ecf8427e)
/*[[[cog
     cog.outl("#ifndef __%s_ACC_TB_H__" % dut.nm.upper())
     cog.outl("#define __%s_ACC_TB_H__" % dut.nm.upper())
  ]]]*/
#ifndef __CYCLE_DETECTION_ACC_TB_H__
#define __CYCLE_DETECTION_ACC_TB_H__
//[[[end]]] (checksum: 7e6ccedaf8cd168a4b0fee6a05623f8a)

/*[[[cog
     cog.outl("#include \"%s_hls.h\"" % dut.nm)
  ]]]*/
#include "cycle_detection_hls.h"
  //[[[end]]] (checksum: 4490a8f47b99043136f554cba88c4a45)
#include "accl_tb.h"
  
typedef acc_top < dut_t, 100/*Mhz*/, 1000/*mem latency in ns*/, RD_CHANNELS, WR_CHANNELS> AcclApp;

#endif
