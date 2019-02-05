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
//thread_nm=compute_best
//[[[end]]] (checksum: dc31314a1922e344d96ce190292a111d)

short sMin( short a, short b, bool force = false) {
  return ( force || a < b)? a : b;
}

/*[[[cog
     c = dut.get_cthread(thread_nm)
     cog.outl("void %s() {" % (c.nm,))
     for p in c.ports:
       cog.outl("  %s; // type: %s" % (p.reset,p.type(dut)))
  ]]]*/
void compute_best() {
  out0.reset_get(); // type: CacheLine
  obest.reset_put(); // type: WrappedShort
//[[[end]]] (checksum: 4967c173fb93941107fa5dd6df65908d)

  unsigned int jj=0;
  short best;

  bool validPipe0 = false;
  unsigned int prev_jj0 = 0;
  short m0[32>>2];
  bool validPipe1 = false;
  unsigned int prev_jj1 = 0;
  short m1[32>>4];

  /*[[[cog
       if c.writes_to_done:
         cog.outl("done = false;")
    ]]]*/
  //[[[end]]] (checksum: d41d8cd98f00b204e9800998ecf8427e)
  wait();
  while (1) {
    if ( start) {
      const unsigned int nCLperRow = config.read().get_nCLperRow();

      if ( validPipe1 && ( prev_jj1 != nCLperRow-1 || obest.nb_can_put())) {
        best = sMin( sMin( m1[0], m1[1]), best, prev_jj1 == 0);
        if ( prev_jj1 == nCLperRow-1) {
          WrappedShort ws;
          ws.data = best;
          obest.nb_put( ws);
        }
        validPipe1 = false;
      }

      if ( validPipe0 && !validPipe1) {
        for( unsigned int k=0; k<32>>4; ++k) {
          m1[k] = sMin( sMin( m0[4*k+0], m0[4*k+1]),
                        sMin( m0[4*k+2], m0[4*k+3]));
        }
        validPipe0 = false;
        validPipe1 = true;
        prev_jj1 = prev_jj0;
      }

      if ( out0.nb_can_get() && !validPipe0) {
        CacheLine cl;  
        out0.nb_get( cl);
        for( unsigned int k=0; k<32>>2; ++k) {
          m0[k] = sMin( sMin( cl.words[4*k+0], cl.words[4*k+1]),
                        sMin( cl.words[4*k+2], cl.words[4*k+3]));
        }
        validPipe0 = true;
        prev_jj0 = jj;
        if ( jj != nCLperRow-1) {
          ++jj;
        } else {
          jj=0;
        }
        
      }
    }
    wait();
  }
}
