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
//thread_nm=pat_fetcher
//[[[end]]] (checksum: eff0db2d5c270237b3ddce7786574318)
/*[[[cog
     c = dut.get_cthread(thread_nm)
     cog.outl("void %s() {" % (c.nm,))
     for p in c.ports:
       cog.outl("  %s;" % p.reset)
  ]]]*/
void pat_fetcher() {
  patQ.reset_get();
  irowQ.reset_put();
  partialResultQ.reset_get();
  finalResultQ.reset_put();
//[[[end]]] (checksum: d71bffc98901b32a53e6aa141d0f3016)
    //    unsigned int jp = 0;
    unsigned int jp_p1 = 1;

    UInt16 slack = 0;

    bool slack_initialized = false;

  /*[[[cog
       if c.writes_to_done:
         cog.outl("done = false;")
    ]]]*/
  done = false;
  //[[[end]]] (checksum: 872c87e47f55883a9563054415885245)
  wait();
  while (1) {
    if ( start) {
      if ( !slack_initialized) {
        slack = config.read().get_max_recirculating();
        slack_initialized = true;
      } else {
        bool cond1 = patQ.nb_can_get();
	bool cond2 = partialResultQ.nb_can_get();
	bool cond3 = 0 != slack;
        bool done_p1 = config.read().get_nPat() == jp_p1;
        unsigned int jp_p2 = jp_p1+1;

        if ( !done && ((cond1 && cond3) || cond2)) {
	  BWState tup;
	  if ( cond1 && cond3) {
	    patQ.nb_get(tup);
	  } else if ( cond2) {
	    partialResultQ.nb_get(tup);
	  }

          if (tup.pat.length > 0) {
            //            std::cout << "Working on " << tup.idx << " with pattern length " << tup.pat.length << " " << tup.res.l << " " << tup.res.u << std::endl;

            irowQ.put(tup);
            if ( cond1 && cond3) {
              --slack;
            }
          } else {
            //            std::cout << "Retiring " << tup.idx << " (" << jp_p1-1 << ")" << std::endl;
	    finalResultQ.put(tup);

            if ( cond1 && cond3) {
              /* one retired (++jp) and one introduced (++ip) */
            } else {
              ++slack;
            }

	    if (done_p1) {
	      done = true;
	    }
            //            jp = jp_p1;
	    jp_p1 = jp_p2;

	  }
        }
      }
    }
    wait();
  }
}
