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
//thread_nm=inp_addr_gen
//[[[end]]] (checksum: a73551a91ae75c367d3cee0188bead57)

// Declare helper methods and class variables

/*[[[cog
     c = dut.get_cthread(thread_nm)
     cog.outl("void %s() {" % (c.nm,))
     for p in c.ports:
       cog.outl("  %s; // type: %s" % (p.reset,p.type(dut)))
  ]]]*/
void inp_addr_gen() {
  inpReqOut.reset_put(); // type: MemTypedReadReqType<Pair>
//[[[end]]] (checksum: ebd7ed96f1cf2a800bdafa7d1da980a1)

  bool already_sent = false;

  /*[[[cog
       if c.writes_to_done:
         cog.outl("done = false;")
    ]]]*/
  //[[[end]]] (checksum: d41d8cd98f00b204e9800998ecf8427e)
  wait();
  while (1) {
    if ( start) {

      unsigned int nSlices = config.read().get_nSlices();
      unsigned int nCLperRow = config.read().get_nCLperRow();

      assert( nCLperRow <= 4);
      nCLperRow = nCLperRow & 7U;

      unsigned int totalPairs = ((nCLperRow*nCLperRow)<<(2*log2ElementsPerCL)) + nSlices;

      if ( !already_sent) {
        inpReqOut.put( MemTypedReadReqType<Pair>( config.read().getInpAddr( 0), totalPairs));
        already_sent = true;
      }

    }
    wait();
  }
}
