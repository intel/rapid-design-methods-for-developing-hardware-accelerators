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
#ifndef __LINKEDLIST_ACC_TB_H__
#define __LINKEDLIST_ACC_TB_H__
//[[[end]]] (checksum: 43470ac4a406eb191b5571effdb68c8b)

/*[[[cog
     cog.outl("#include \"%s_hls.h\"" % dut.nm)
  ]]]*/
#include "linkedlist_hls.h"
  //[[[end]]] (checksum: af1ec8d4f8f1eb38968b7addc793626b)
#include "accl_tb.h"
  
typedef acc_top < dut_t, 100/*Mhz*/, 1000/*mem latency in ns*/, RD_CHANNELS, WR_CHANNELS> AcclApp;

#endif
