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
//thread_nm=mo_addr_gen
//[[[end]]] (checksum: 67d6bbc227e8e873d746652d24a2867c)

// Declare helper methods and class variables

/*[[[cog
     c = dut.get_cthread(thread_nm)
     cog.outl("void %s() {" % (c.nm,))
     for p in c.ports:
       cog.outl("  %s; // type: %s" % (p.reset,p.type(dut)))
  ]]]*/
void mo_addr_gen() {
  moReqOut.reset_put(); // type: MemTypedWriteReqType<UCacheLine>
//[[[end]]] (checksum: 5e07590574702ebfadbca573f8af5c59)

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
      unsigned int nSlices = config.read().get_nSlices();

      if ( !already_sent) {
        moReqOut.put( MemTypedWriteReqType<UCacheLine>( config.read().getMoAddr( 0), nSlices*nCLperRow));
        already_sent = true;
      }
    }
    wait();
  }
}
