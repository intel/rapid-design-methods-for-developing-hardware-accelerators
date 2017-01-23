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
//thread_nm=precomp_addr_gen
//[[[end]]] (checksum: 3dad680ec0d262f0cbec2563f9b39cb0)

// Declare helper methods and class variables

/*[[[cog
     c = dut.get_cthread(thread_nm)
     cog.outl("void %s() {" % (c.nm,))
     for p in c.ports:
       cog.outl("  %s; // type: %s" % (p.reset,p.type(dut)))
  ]]]*/
void precomp_addr_gen() {
  preReqOut.reset_put(); // type: MemTypedReadReqType<BWResult>
//[[[end]]] (checksum: 5cffd07d6c6fee828201df8a46586d29)

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
	UInt16 precomp_size = 1U<<(2*config.read().get_precomp_len());
	preReqOut.put(MemTypedReadReqType<BWResult>( config.read().getPreAddr( 0), precomp_size)); 
        already_sent = true;
      }
    }
    wait();
  }
}
