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
       cog.outl("  %s; // type: %s" % (p.reset,p.type(dut)))
  ]]]*/
void deltax() {
  inpRespIn.reset_get(); // type: MemTypedReadRespType<BlkInp>
  mid0.reset_put(); // type: BlkMid
//[[[end]]] (checksum: 5213e88798d40c4770430812caf0123f)

  const unsigned int bj = 2;
  const unsigned int words_per_blk = BlkInp::ArrayLength;
  const short kr[1][2*bj+1] = { {1,2,0,-2,-1} };

  BlkInp lastcl;
  BlkInp::ElementType left[bj];

  UInt16 ip = 0;
  UInt16 jc = 0;
  bool do_final = false;

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

      BlkInp cl;
      if (!do_final) {
        cl = inpRespIn.get().data;
      }

      // combinational
      BlkInp::ElementType right[bj];

    UNROLL_INITIAL_RIGHT:
      for( unsigned int k=0; k<bj; ++k) {
        right[k] = 0;
      }
      if ( do_final) {
      } else if ( ip == 0 && jc == 0) {
      } else if ( ip > 0 && jc == 0) {
      } else {
      UNROLL_RIGHT_C:
        for( unsigned int k=0; k<bj; ++k) {
          right[k] = cl.data[k];
        }
      }

      if ( ip == 0 && jc == 0) {
      } else {
        BlkInp::ElementType buf[bj+words_per_blk+bj];

      UNROLL_BUF_A:
        for ( unsigned int k=0; k<bj; ++k) {
          buf[k] = left[k];
        }
      UNROLL_BUF_B:
        for ( unsigned int k=0; k<words_per_blk; ++k) {
          buf[bj+k] = lastcl.data[k];
        }
      UNROLL_BUF_C:
        for ( unsigned int k=0; k<bj; ++k) {
          buf[bj+words_per_blk+k] = right[k];
        }

        BlkMid newlastcl(0);

      UNROLL_SUM:
        for( unsigned int jj=1; jj<bj+1; ++jj) {
        UNROLL_NEWLASTCL:
          for( unsigned int k=0; k<words_per_blk; ++k) {
            newlastcl.data[k] += (BlkMid::ElementType) buf[bj-jj+k] * kr[0][bj-jj];
            newlastcl.data[k] += (BlkMid::ElementType) buf[bj+jj+k] * kr[0][bj+jj];
          }
        }
        mid0.put( newlastcl);      
      }

      if ( ip == 0 && jc == 0) {
      UNROLL_LEFT_A:
        for( unsigned int k=0; k<bj; ++k) {
          left[k] = 0;
        }
      } else if ( ip > 0 && jc == 0) {
      UNROLL_LEFT_B:
        for( unsigned int k=0; k<bj; ++k) {
          left[k] = 0;
        }
      } else {
      UNROLL_LEFT_C:
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
    wait();
  }
}
