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
//thread_nm=pat_gadget2
//[[[end]]] (checksum: 8bfe0ead155bee9fc69d14a2d98c323b)
  // max len is 7 by default

#ifndef MAX_PRECOMP_LEN 
#define MAX_PRECOMP_LEN 7
#endif  

  BWResult precomp_res[1<<(2*MAX_PRECOMP_LEN)];

  BWState ff( BWIdx idx_in, BWPattern pat_in, const Config& config) {
    BWState tup;

    tup.idx = idx_in;
    tup.pat = pat_in;
    tup.state = 0;

#ifndef __SYNTHESIS__
    if ( tup.pat.length <= 0) {
      std::cout << "Tuple length <= 0: idx:" << tup.idx << " pat:" << std::hex << std::setfill('0') << std::setw(8) << tup.pat.pat << " length:" << std::dec << tup.pat.length << std::endl;
    }                             
#endif

    if ( config.get_precomp_len() == 0) {
      assert( tup.pat.length > 0);

      int r = tup.pat.pat & 0x3;
      tup.res.l = config.first( r, 0);
      tup.res.u = config.first( r, 1);

      --tup.pat.length;
      tup.pat.pat >>= 2;
    } else {
      UInt6 l = config.get_precomp_len();
      assert( l <= MAX_PRECOMP_LEN);
      unsigned long long key = tup.pat.pat & ((1ULL << (2*l))-1);
      tup.res = precomp_res[key];
      //      std::cout << key << " ==> " << tup.res;
      assert( tup.pat.length >= l);
      tup.pat.length -= l;
      tup.pat.pat >>= 2*l;
    }

    return tup;
  }

/*[[[cog
     c = dut.get_cthread(thread_nm)
     cog.outl("void %s() {" % (c.nm,))
     for p in c.ports:
       cog.outl("  %s;" % p.reset)
  ]]]*/
void pat_gadget2() {
  patRespIn.reset_get();
  preReqOut.reset_put();
  preRespIn.reset_get();
  reserveAckQ.reset_get();
  patQ.reset_put();
//[[[end]]] (checksum: 069f5ca9be07fdf7564f7b9fe6d071a4)

    UInt16 precomp_i = 0;

    unsigned int ip = 0;
    UInt2 phase = 0;

  /*[[[cog
       if c.writes_to_done:
         cog.outl("done = false;")
    ]]]*/
  //[[[end]]] (checksum: d41d8cd98f00b204e9800998ecf8427e)
  wait();
  while (1) {
    if ( start) {

      if ( start && phase == 0) {
	unsigned int precomp_size = 1U<<(2*config.read().get_precomp_len());
#if defined __SYNTHESIS__ && !defined __CTOS_BUILD_ONLY__
          wait();
#endif
	preReqOut.put(MemTypedReadReqType<BWResult>( config.read().getPreAddr( 0), precomp_size)); 
	phase = 1;
      } else if ( start && phase == 1) {
	unsigned int precomp_size_m1 = (1U<<(2*config.read().get_precomp_len()))-1U;
#if defined __SYNTHESIS__ && !defined __CTOS_BUILD_ONLY__
          wait();
#endif
        MemTypedReadRespType<BWResult> wrapped_resp;
        preRespIn.get(wrapped_resp);
        precomp_res[precomp_i] = wrapped_resp.data;
        if ( precomp_i == precomp_size_m1) {
          phase = 2;
        } else {
          ++precomp_i;
        }

      // Make sure the precomp data is available for ff(...)
      } else if ( phase == 2 && ip < config.read().get_nPat()) {
	if ( patRespIn.nb_can_get() && reserveAckQ.nb_can_get()) {
	  MemTypedReadRespType<BWPattern> wrapped_pat;
	  patRespIn.nb_get(wrapped_pat);
	  BWEmpty ack;
	  reserveAckQ.nb_get(ack);

	  BWState tup = ff( ip, wrapped_pat.data, config.read());

#if defined __SYNTHESIS__ && !defined __CTOS_BUILD_ONLY__
          wait();
#endif
	  patQ.put( tup);

	  ++ip;
	}
      }
    }
    wait();
  }
}
