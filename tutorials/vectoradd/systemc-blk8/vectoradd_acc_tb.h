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
#ifndef __VECTORADD_ACC_TB_H__
#define __VECTORADD_ACC_TB_H__
//[[[end]]] (checksum: a16fa4b923a13d501436d9a1b56b95c6)

/*[[[cog
     cog.outl("#include \"%s_hls.h\"" % dut.nm)
  ]]]*/
#include "vectoradd_hls.h"
  //[[[end]]] (checksum: 734a28abf555b586bcf9359ace5dd4ae)
#include "accl_tb.h"
  
typedef acc_top < dut_t, 100/*Mhz*/, 1000/*mem latency in ns*/, RD_CHANNELS, WR_CHANNELS> AcclApp;

#endif
