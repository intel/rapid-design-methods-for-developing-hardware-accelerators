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
//thread_nm=split_out
//[[[end]]] (checksum: 4a7bce7968bdf51cdbebe706f1210782)

// Declare helper methods and class variables

/*[[[cog
     c = dut.get_cthread(thread_nm)
     cog.outl("void %s() {" % (c.nm,))
     for p in c.ports:
       cog.outl("  %s; // type: %s" % (p.reset,p.type(dut)))
  ]]]*/
void split_out() {
  out.reset_get(); // type: CacheLine
  out0.reset_put(); // type: CacheLine
  out1.reset_put(); // type: CacheLine
//[[[end]]] (checksum: 2d5de5847f19546da6627a11bf330fb3)

  /*[[[cog
       if c.writes_to_done:
         cog.outl("done = false;")
    ]]]*/
  //[[[end]]] (checksum: d41d8cd98f00b204e9800998ecf8427e)
  wait();
  while (1) {
    if ( start) {
      if ( out.nb_can_get() && out0.nb_can_put() && out1.nb_can_put()) {
        CacheLine cl;
        out.nb_get( cl);
        out0.nb_put( cl);
        out1.nb_put( cl);
      }
    }
    wait();
  }
}
