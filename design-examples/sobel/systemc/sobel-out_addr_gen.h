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
       cog.outl("  %s;" % p.reset)
  ]]]*/
void out_addr_gen() {
  outReqOut.reset_put();
//[[[end]]] (checksum: bc50dbc73db202f1ef97c1ba2f64e7fe)

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
