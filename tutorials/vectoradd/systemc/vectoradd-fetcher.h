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
//thread_nm=fetcher
//[[[end]]] (checksum: 703d107d9ea4388deb0005904fbed4ec)

// Declare helper methods and class variables

/*[[[cog
     c = dut.get_cthread(thread_nm)
     cog.outl("void %s() {" % (c.nm,))
     for p in c.ports:
       cog.outl("  %s;" % p.reset)
  ]]]*/
void fetcher() {
  inaRespIn.reset_get();
  inbRespIn.reset_get();
  outDataOut.reset_put();
//[[[end]]] (checksum: 8beb7a998b73434e4e4406ac6913b565)

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
      // check if it was the the last one (we process 16 elements at a time)
      if ( ip != (config.read().get_n() >> 4)) {
        // reading input from main memory using memory read ports
        Blk cla = inaRespIn.get().data;
        Blk clb = inbRespIn.get().data;
        Blk clo; 
        
        // vector add is implemented in the overloaded operator+ for Blk
        clo = cla + clb;

        // writing back to memory via a memory write port
        outDataOut.put( clo);

        ++ip;
      } else {
        done = true;
      }
    }
    wait();
  }
}
