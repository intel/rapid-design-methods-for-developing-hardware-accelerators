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
#ifndef __SOBEL_ACC_TB_H__
#define __SOBEL_ACC_TB_H__
//[[[end]]] (checksum: 0f8fda82cf5958eb536388edb667e0a9)

/*[[[cog
     cog.outl("#include \"%s_hls.h\"" % dut.nm)
  ]]]*/
#include "sobel_hls.h"
  //[[[end]]] (checksum: 62f77302f6b837ce4a974bb3e2f0115d)
#include "accl_tb.h"
  
typedef acc_top < dut_t, 100/*Mhz*/, 1000/*mem latency in ns*/, RD_CHANNELS, WR_CHANNELS> AcclApp;

#endif
