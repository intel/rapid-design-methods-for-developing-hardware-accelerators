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
//thread_nm=pat_gadget
//[[[end]]] (checksum: 88c6fbb9474b86fc5a25ac617666d9e5)
/*[[[cog
     c = dut.get_cthread(thread_nm)
     cog.outl("void %s() {" % (c.nm,))
     for p in c.ports:
       cog.outl("  %s;" % p.reset)
  ]]]*/
void pat_gadget() {
  reserveQ.reset_put();
//[[[end]]] (checksum: e65df3c5c563a8295d19dc90ec784142)
    unsigned int ip = 0;
  /*[[[cog
       if c.writes_to_done:
         cog.outl("done = false;")
    ]]]*/
  //[[[end]]] (checksum: d41d8cd98f00b204e9800998ecf8427e)
  wait();
  while (1) {
    if ( start) {
      if ( ip < config.read().get_nPat()) {
	reserveQ.put(ip);
	++ip;
      }
    }
    wait();
  }
}
