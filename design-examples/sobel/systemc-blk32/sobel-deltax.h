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
//thread_nm=deltax
//[[[end]]] (checksum: 65df815f54143e13bbc1713118f08ca6)

// Declare helper methods and class variables

/*[[[cog
     c = dut.get_cthread(thread_nm)
     cog.outl("void %s() {" % (c.nm,))
     for p in c.ports:
       cog.outl("  %s;" % p.reset)
  ]]]*/
void deltax() {
  inpRespIn.reset_get();
  mid0.reset_put();
//[[[end]]] (checksum: ba03ad9d2785f745991721442effe01a)

  const unsigned int bj = 2;
  const unsigned int words_per_blk = 32;
  const short kr[1][2*bj+1] = { {1,2,0,-2,-1} };

  BlkInp lastcl;
  unsigned char left[bj];

  UInt16 ip = 0;
  UInt16 jc = 0;
  bool do_final = false;

  // Declare and initialize local variables

  /*[[[cog
       if c.writes_to_done:
         cog.outl("done = false;")
    ]]]*/
  //[[[end]]] (checksum: d41d8cd98f00b204e9800998ecf8427e)
  wait();
  while (1) {
    if ( start) {
      UInt16 ni = config.read().get_num_of_rows();
      UInt16 bpr = config.read().get_row_size_in_blks();

 
    /* this is an annoying thing to add for flow control */
      if( (do_final || inpRespIn.nb_can_get()) &&
          ((ip ==0 && jc == 0) || mid0.nb_can_put())) {

        BlkInp cl;
        if (!do_final) {
          MemTypedReadRespType<BlkInp> mem_resp;
          inpRespIn.nb_get(mem_resp);
          cl = mem_resp.data;
        }

        // combinational
        unsigned char right[bj];
        for( unsigned int k=0; k<bj; ++k) {
          right[k] = 0;
        }
        if ( do_final) {
        } else if ( ip == 0 && jc == 0) {
        } else if ( ip > 0 && jc == 0) {
        } else {
          for( unsigned int k=0; k<bj; ++k) {
            right[k] = cl.data[k];
          }
        }

        if ( ip == 0 && jc == 0) {
        } else {
          unsigned char buf[bj+words_per_blk+bj];

          for ( unsigned int k=0; k<bj; ++k) {
            buf[k] = left[k];
          }
          for ( unsigned int k=0; k<words_per_blk; ++k) {
            buf[bj+k] = lastcl.data[k];
          }

          for ( unsigned int k=0; k<bj; ++k) {
            buf[bj+words_per_blk+k] = right[k];
          }

          BlkMid newlastcl;

          for( unsigned int k=0; k<words_per_blk; ++k) {
            newlastcl.data[k] = 0;
          }
          for( unsigned int jj=1; jj<bj+1; ++jj) {
            for( unsigned int k=0; k<words_per_blk; ++k) {
              newlastcl.data[k] += (short) buf[bj-jj+k] * kr[0][bj-jj];
              newlastcl.data[k] += (short) buf[bj+jj+k] * kr[0][bj+jj];
            }
          }
          mid0.nb_put( newlastcl);      
        }

        if ( ip == 0 && jc == 0) {
          for( unsigned int k=0; k<bj; ++k) {
            left[k] = 0;
          }
        } else if ( ip > 0 && jc == 0) {
          for( unsigned int k=0; k<bj; ++k) {
            left[k] = 0;
          }
        } else {
          for( unsigned int k=0; k<bj; ++k) {
            left[k] = lastcl.data[words_per_blk-bj+k];
          }
        }

        lastcl = cl;

        if ( do_final) {
          ip = 0;
          jc = 0;
          do_final = 0;
        } else {
          ++jc;
          if ( jc >= bpr) {
            jc = 0;
            ++ip;
            if ( ip == ni) {
              do_final = 1;
            }
          }
        }
      }

    }
    wait();
  }
}
