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
//thread_nm=out_addr_gen
//[[[end]]] (checksum: 02de886e47c6e6a93354f164b452824f)

// Declare helper methods and class variables

/*[[[cog
     c = dut.get_cthread(thread_nm)
     cog.outl("void %s() {" % (c.nm,))
     for p in c.ports:
       cog.outl("  %s; // type: %s" % (p.reset,p.type(dut)))
  ]]]*/
void out_addr_gen() {
  outReqOut.reset_put(); // type: MemTypedWriteReqType<BlkOut>
//[[[end]]] (checksum: 95ee868b24d44b75fed6e6c0f93c0417)

  bool already_sent = false;

  /*[[[cog
       if c.writes_to_done:
         cog.outl("done = false;")
    ]]]*/
  //[[[end]]] (checksum: d41d8cd98f00b204e9800998ecf8427e)
  wait();
  while (1) {
    if ( start) {

      if ( !already_sent) {
        outReqOut.put( MemTypedWriteReqType<BlkOut>( config.read().getOutAddr(0), config.read().get_nInp()));
        already_sent = true;
      }

    }
    wait();
  }
}
