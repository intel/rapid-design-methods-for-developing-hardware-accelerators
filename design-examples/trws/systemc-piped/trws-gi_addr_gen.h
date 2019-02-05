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
//thread_nm=gi_addr_gen
//[[[end]]] (checksum: 5ebd34652692d06fa390461f9a7f84cf)

// Declare helper methods and class variables

/*[[[cog
     c = dut.get_cthread(thread_nm)
     cog.outl("void %s() {" % (c.nm,))
     for p in c.ports:
       cog.outl("  %s; // type: %s" % (p.reset,p.type(dut)))
  ]]]*/
void gi_addr_gen() {
  giReqOut.reset_put(); // type: MemTypedReadReqType<UCacheLine>
//[[[end]]] (checksum: c93c1ffe1ae55fe3ae514bbf8a023a72)

  bool already_sent = false;

  /*[[[cog
       if c.writes_to_done:
         cog.outl("done = false;")
    ]]]*/
  //[[[end]]] (checksum: d41d8cd98f00b204e9800998ecf8427e)
  wait();
  while (1) {
    if ( start) {
      // only need 3 bits
      unsigned int nCLperRow = config.read().get_nCLperRow() & (8-1);

      if ( !already_sent) {
          giReqOut.put( MemTypedReadReqType<UCacheLine>( config.read().getGiAddr( 0), nCLperRow));
        already_sent = true;
      }
    }
    wait();
  }
}
