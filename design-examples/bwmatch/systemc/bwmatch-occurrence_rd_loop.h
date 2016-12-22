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
//thread_nm=occurrence_rd_loop
//[[[end]]] (checksum: 13228e5e0a802ced3dd57e3a4e20dc32)

  // 71845 -1 71808 37
  // 71877 -1 71808 69

  // 71845 71877
  // count_if_eq 0 71808 37 561 561 101 0
  // count_if_eq 0 71808 69 561 561 101 0


  unsigned int count_if_eq( const BWCacheLine& lcl, unsigned int l, unsigned int cnt, int r, const Config& config)
  {

    //
    // And 4 64 bit ulong per 256 bw stored in one 512 bit cache line
    //
    const unsigned int log2_qq = 2;
    const unsigned int qq = 1 << log2_qq;
    const unsigned int mask_qq = qq-1;

    // There are 4 pairs of bits in a byte
    // And 32 pairs of bits in a ulonglong
    const unsigned int log2_q = 5;
    const unsigned int q = 1 << log2_q;
    const unsigned int mask_q = q-1;

    assert( l % 128 == 0);
    // Can equal 128 now because of last row special case
    assert( cnt <= 128);

    bool cond1 = (l >> 7) == (config.get_end_pos() >> 7);
    bool cond2 = (config.get_end_pos() & 127) < cnt;

    int delta = ( cond1 && cond2 && r == 0) ? -1 : 0;

    unsigned int sum = 0;


    UNROLL_POPCOUNT: for( unsigned int idx=0; idx<128; ++idx) {
      const unsigned int jj = idx >> log2_q;
      const unsigned int k = idx & mask_q;
      const unsigned int j = jj & mask_qq;
      if ( idx < cnt) {
	int cand = 0x3 & (lcl.bwa[j] >> (2*k));
	if ( cand == r) {
	  ++sum;
	}
      }
    }
    /*
    std::cout << std::dec << l+cnt
              << " " << delta << " " << l
              << " " << cnt
              << " " << cond1
              << " " << cond2
              << " " << (l & 127)
              << " " << (config.get_end_pos() & 127)
              << std::endl;
    */
    return sum + delta;
  }

/*[[[cog
     c = dut.get_cthread(thread_nm)
     cog.outl("void %s() {" % (c.nm,))
     for p in c.ports:
       cog.outl("  %s;" % p.reset)
  ]]]*/
void occurrence_rd_loop() {
  clRespIn.reset_get();
  partialResultQ.reset_put();
//[[[end]]] (checksum: 9d402b4c158b91c35f4fce68d96a677e)

  /*[[[cog
       if c.writes_to_done:
         cog.outl("done = false;")
    ]]]*/
  //[[[end]]] (checksum: d41d8cd98f00b204e9800998ecf8427e)
  wait();
  while (1) {
    if ( start && clRespIn.nb_can_get() && partialResultQ.nb_can_put()) {

#if defined __SYNTHESIS__ && !defined __CTOS_BUILD_ONLY__
        wait();
#endif

        MemSingleReadRespType<BWCacheLine,BWState> d;
        clRespIn.nb_get( d);

        BWState inp = d.utag;

        UInt2 r = inp.pat.pat & 0x3;
        UInt32 irow;

        if ( inp.state == 0)
          irow = inp.res.l;
        else
          irow = inp.res.u;

        unsigned int crow = irow >> 7;
        unsigned int off  = crow << 7;
        unsigned int cnt  = irow - off;

        if (crow == config.read().m()) {
          off -= 128;
          assert( cnt == 0);
          cnt = 128;
        }

        UInt32 orow = config.read().first( r, 0) + d.data.ranks[r] + count_if_eq( d.data, off, cnt, r, config.read());

#if defined __SYNTHESIS__ && !defined __CTOS_BUILD_ONLY__
        wait();
        wait();
#endif

        BWState out = inp;
        if ( inp.state == 0) {
          out.res.l = orow;
          out.state = 1;
        } else {
          out.res.u = orow;
          out.state = 0;
          --out.pat.length;
          out.pat.pat >>= 2;
        }
        partialResultQ.nb_put( out);
    }
    wait();
  }
}
