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
//thread_nm=wi_addr_gen
//[[[end]]] (checksum: 14a6e559f77c72cf24a74744c3ec24ab)

// Declare helper methods and class variables

/*[[[cog
     c = dut.get_cthread(thread_nm)
     cog.outl("void %s() {" % (c.nm,))
     for p in c.ports:
       cog.outl("  %s; // type: %s" % (p.reset,p.type(dut)))
  ]]]*/
void wi_addr_gen() {
  wiReqOut.reset_put(); // type: MemTypedReadReqType<CacheLine>
//[[[end]]] (checksum: db00325b98d3dff17fdc4466ff064e17)

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
        wiReqOut.put( MemTypedReadReqType<CacheLine>( config.read().getWiAddr( 0), (nCLperRow*nCLperRow)<<log2ElementsPerCL));
        already_sent = true;
      }
    }
    wait();
  }
}
