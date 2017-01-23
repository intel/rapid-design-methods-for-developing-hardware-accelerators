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
#ifndef __BWMATCH_ACC_TB_H__
#define __BWMATCH_ACC_TB_H__
//[[[end]]] (checksum: 99e0a6b080679477c1442120c1b5933a)

/*[[[cog
     cog.outl("#include \"%s_hls.h\"" % dut.nm)
  ]]]*/
#include "bwmatch_hls.h"
  //[[[end]]] (checksum: b996b0b7fea12f0b1ad2e293d2df7677)
#include "accl_tb.h"
  
typedef acc_top < dut_t, 136/*Mhz*/, 1000/*mem latency in ns*/, RD_CHANNELS, WR_CHANNELS, 100/*mem latency spread in ns*/> AcclApp;

#endif
