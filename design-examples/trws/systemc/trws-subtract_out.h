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
//thread_nm=subtract_out
//[[[end]]] (checksum: e24fed67e7aa26f4ba009c8811d48600)

// Declare helper methods and class variables

/*[[[cog
     c = dut.get_cthread(thread_nm)
     cog.outl("void %s() {" % (c.nm,))
     for p in c.ports:
       cog.outl("  %s; // type: %s" % (p.reset,p.type(dut)))
  ]]]*/
void subtract_out() {
  out1.reset_get(); // type: CacheLine
  obest.reset_get(); // type: WrappedShort
  res.reset_put(); // type: UCacheLine
//[[[end]]] (checksum: c05446ca8ed721a7d038d5bfc800c86c)

  short best;
  unsigned int jj = 0;

  /*[[[cog
       if c.writes_to_done:
         cog.outl("done = false;")
    ]]]*/
  //[[[end]]] (checksum: d41d8cd98f00b204e9800998ecf8427e)
  wait();
  while (1) {
    if ( start) {
      const unsigned int nCLperRow = config.read().get_nCLperRow();
      if ( (jj != 0 || obest.nb_can_get()) && out1.nb_can_get() && res.nb_can_put()) {
        if ( jj == 0) {
          WrappedShort ws;
          obest.nb_get( ws);
          best = ws.data;
        }
        CacheLine cl;
        UCacheLine ucl;
        out1.nb_get( cl);
        for ( unsigned int j=0; j<32; ++j) {
            ucl.words[j] = cl.words[j] - best;
        }
        res.nb_put( ucl);

        if ( jj != nCLperRow-1) {
          ++jj;
        } else {
          jj = 0;
        }
      }

    }
    wait();
  }
}
