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
//thread_nm=inp_fetcher
//[[[end]]] (checksum: b3cc55cdf47df8e2833f7683e251c994)

// Declare helper methods and class variables

/*[[[cog
     c = dut.get_cthread(thread_nm)
     cog.outl("void %s() {" % (c.nm,))
     for p in c.ports:
       cog.outl("  %s;" % p.reset)
  ]]]*/
void inp_fetcher() {
  inpRespIn.reset_get();
  outDataOut.reset_put();
//[[[end]]] (checksum: 78e3954429f1b9eaa6d398a4359e0c54)

  unsigned int ip = 0;

  /*[[[cog
       if c.writes_to_done:
         cog.outl("done = false;")
    ]]]*/
  done = false;
  //[[[end]]] (checksum: 872c87e47f55883a9563054415885245)
  wait();
  while (1) {
    if ( start) {

      if ( ip != config.read().get_nCLs()) {
        MemTypedReadRespType<CacheLine> wrapped_cl = inpRespIn.get();
        outDataOut.put(MemTypedWriteDataType<CacheLine>( wrapped_cl.data));
        ++ip;
      } else {
        done = true;
      }

    }
    wait();
  }
}
