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
#ifndef __MEMCPY_ACC_TB_H__
#define __MEMCPY_ACC_TB_H__
//[[[end]]] (checksum: ef64e1df5128368340e8a2ddae818a4a)

/*[[[cog
     cog.outl("#include \"%s_hls.h\"" % dut.nm)
  ]]]*/
#include "memcpy_hls.h"
  //[[[end]]] (checksum: 2780e279a206974b1f8efe5b2859e55c)
#include "accl_tb.h"
  
typedef acc_top < dut_t, 100/*Mhz*/, 1000/*mem latency in ns*/, RD_CHANNELS, WR_CHANNELS> AcclApp;

#endif
