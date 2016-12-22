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
//thread_nm=pat_addr_gen
//[[[end]]] (checksum: def7ea517038f1093851d081eac67968)
/*[[[cog
     c = dut.get_cthread(thread_nm)
     cog.outl("void %s() {" % (c.nm,))
     for p in c.ports:
       cog.outl("  %s;" % p.reset)
  ]]]*/
void pat_addr_gen() {
  patReqOut.reset_put();
//[[[end]]] (checksum: 6c1afcc80ef671b43d7f1693d8521590)
  const unsigned int max_incr = 1 << 24;
  Config lconfig;
  unsigned int ip = 0;
  bool already_read = false;
  /*[[[cog
       if c.writes_to_done:
         cog.outl("done = false;")
    ]]]*/
  //[[[end]]] (checksum: d41d8cd98f00b204e9800998ecf8427e)
  wait();
  while (1) {
    if ( start) {
      if ( !already_read) {
        lconfig = config.read();
        already_read = true;
      } else if ( patReqOut.nb_can_put() && ip < lconfig.get_nPat()) {
        unsigned int incr = std::min( max_incr, (unsigned int) lconfig.get_nPat() - ip);

        patReqOut.nb_put(MemTypedReadReqType<BWPattern>( lconfig.getPatAddr( ip), incr)); 
        //std::cout << "pat_addr_gen: " << ip << std::endl;

        ip += incr;
      }
    }
    wait();
  }
}
