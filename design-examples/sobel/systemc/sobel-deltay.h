// See LICENSE for license details.
/*[[[cog
     import cog
     from cog_acctempl import *
     from dut_params import dut
     if "thread_nm" not in globals():
       lst = cog.previous.lstrip('/').rstrip('\n').split('=')
       assert( lst[0]=="thread_nm")
       assert( len(lst)==2)
       global thread_nm
       thread_nm = lst[1]
     cog.outl( "//thread_nm=" + thread_nm)
  ]]]*/
//thread_nm=deltay
//[[[end]]] (checksum: 04a09a1951c5149f3f28f4d522d164af)

// Declare helper methods and class variables

/*[[[cog
     c = dut.get_cthread(thread_nm)
     cog.outl("void %s() {" % (c.nm,))
     for p in c.ports:
       cog.outl("  %s; // type: %s" % (p.reset,p.type(dut)))
  ]]]*/
void deltay() {
  mid0.reset_get(); // type: BlkMid
  mid1.reset_put(); // type: BlkMid
//[[[end]]] (checksum: 05b98a018b857f70e8c910da07d806a4)

  UInt16 ip = 0;
  UInt16 jc = 0;
  UInt16 jc_last;

  const unsigned int MAXBPR = 40 * 64 / BlkMid::words_per_blk;

  BlkMid d0[MAXBPR];
  BlkMid d1[MAXBPR];
  BlkMid d2[MAXBPR];
  BlkMid d3[MAXBPR];

  BlkMid cl;

  BlkMid d0_buf;
  BlkMid d1_buf;
  BlkMid d2_buf;
  BlkMid d3_buf;

  /*[[[cog
       if c.writes_to_done:
         cog.outl("done = false;")
    ]]]*/
  //[[[end]]] (checksum: d41d8cd98f00b204e9800998ecf8427e)
  wait();
  while (1) {
    if ( start) {

      UInt16 ni = config.read().get_num_of_rows();
      UInt16 bpr = config.read().get_row_size_in_blks();

      const unsigned int bi = 2;
      const short kl[2*bi+1][1] = { {1}, {4}, {6}, {4}, {1}};

      if ( ip == 0 && jc == 0) {
        jc_last = bpr-1;
      }

      
      if ( ip == 0 || ip < ni) {
        cl = mid0.get();
      }
      if (ip >= ni) {
        mid1.put( d0[jc]);
      } else if (ip >= bi) {
        mid1.put( d0[jc] + cl*kl[4][0]);
      }
      
      BlkMid z = BlkMid( 0);

      d0[jc_last] = d0_buf;
      d0_buf = ((ip<ni) ? cl*kl[3][0] : z) + ((ip> 0) ? d1[jc] : z);

      d1[jc_last] = d1_buf;
      d1_buf = ((ip<ni) ? cl*kl[2][0] : z) + ((ip> 0) ? d2[jc] : z);

      d2[jc_last] = d2_buf;
      d2_buf = ((ip<ni) ? cl*kl[1][0] : z) + ((ip> 0) ? d3[jc] : z);

      d3[jc_last] = d3_buf;
      d3_buf = ((ip<ni) ? cl*kl[0][0] : z);

      jc_last = jc;

      ++jc;
      if ( jc == bpr) {
        ++ip;
        jc = 0;
        if ( ip == ni+bi) {
          ip = 0;
        }
      }

    }
    wait();
  }
}
