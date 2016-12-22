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
//thread_nm=res_reorder
//[[[end]]] (checksum: 775fc2fdd6fb632e19675681cda80bff)

  class CAMAddrHi {
  public:
    unsigned int rest  : 26;
    unsigned int valid : 1;
  };
  static const unsigned int log2_cam_size = 5;
  static const unsigned int cam_size = (1<<log2_cam_size);
  static const unsigned int mask_cam_size = cam_size - 1;

  static const unsigned int log2_items_per_cl = 3;
  static const unsigned int items_per_cl = (1 << log2_items_per_cl);
  static const unsigned int mask_items_per_cl = items_per_cl - 1;

  static const unsigned int mask_item_bv = (1 << items_per_cl) - 1;

  class ItemBV {
  public:
#ifndef __SYNTHESIS__
    static_assert( items_per_cl <= 8, "Items bitvector too small.");
#endif
    unsigned int bv : 8;
  };


/*[[[cog
     c = dut.get_cthread(thread_nm)
     cog.outl("void %s() {" % (c.nm,))
     for p in c.ports:
       cog.outl("  %s;" % p.reset)
  ]]]*/
void res_reorder() {
  reserveQ.reset_get();
  reserveAckQ.reset_put();
  finalResultQ.reset_get();
  resReqOut.reset_put();
  resDataOut.reset_put();
//[[[end]]] (checksum: d05d302ffb36bf3b42b800aa0b645f31)

    /*
     *    One BWIdx input port for reserving
     *    One BWEmpty output port for acknowledging reservation
     *    One BWState input port for the write destination and data
     */

    // Declare variables

    CAMAddrHi cam[cam_size];

    BWResultLine buf[cam_size];

    ItemBV reserved[cam_size];
    ItemBV fulfilled[cam_size];

    RESET_CAM_CONTROL: for( unsigned int i=0; i<cam_size; ++i) {
      cam[i].valid = false;
      reserved[i].bv = 0;
      fulfilled[i].bv = 0;
    }

    bool pending_reservation = false;
    unsigned int r_addr = 0;

  /*[[[cog
       if c.writes_to_done:
         cog.outl("done = false;")
    ]]]*/
  //[[[end]]] (checksum: d41d8cd98f00b204e9800998ecf8427e)
  wait();
  while (1) {
    if ( start) {

      if ( pending_reservation) {
	assert( reserveAckQ.nb_can_put());

        //	unsigned int r_tag = r_addr >> log2_items_per_cl;
	unsigned int r_pos = r_addr & mask_items_per_cl;

        unsigned int r_key  = (r_addr >> log2_items_per_cl) & mask_cam_size;
        unsigned int r_rest = (r_addr >> log2_items_per_cl) >> log2_cam_size;

	if ( !cam[r_key].valid) {
	  cam[r_key].valid = true;
	  cam[r_key].rest  = r_rest;
	  reserved[r_key].bv |= 1<<r_pos;

	  reserveAckQ.nb_put( BWEmpty());
          pending_reservation = false;
        }
      } else if ( reserveQ.nb_can_get() && reserveAckQ.nb_can_put()) {
	// Read in reservation request into r_addr
	BWIdx tmp;
	reserveQ.nb_get(tmp);
	r_addr = tmp;

        //	unsigned int r_tag = r_addr >> log2_items_per_cl;
	unsigned int r_pos = r_addr & mask_items_per_cl;

        unsigned int r_key  = (r_addr >> log2_items_per_cl) & mask_cam_size;
        unsigned int r_rest = (r_addr >> log2_items_per_cl) >> log2_cam_size;

	bool empty_found = !cam[r_key].valid;
        bool found = cam[r_key].valid && cam[r_key].rest == r_rest;

	if ( found) {
	  assert( !(reserved[r_key].bv & (1<<r_pos)));
	} else if ( !found && empty_found) {
	  cam[r_key].valid = true;
	  cam[r_key].rest = r_rest;
	}
	if ( found || empty_found) {
	  reserved[r_key].bv |= 1<<r_pos;

	  BWEmpty ack;
	  reserveAckQ.nb_put( ack);
	} else {
	  pending_reservation = true;
	}
      }

      // write match
      if ( finalResultQ.nb_can_get()) {
	BWState tup;
	finalResultQ.nb_get(tup);

	// Read in write request
	unsigned int addr = tup.idx;

	unsigned int tag = addr >> log2_items_per_cl;
	unsigned int pos = addr & mask_items_per_cl;

        unsigned int key  = (addr >> log2_items_per_cl) & mask_cam_size;
        unsigned int rest = (addr >> log2_items_per_cl) >> log2_cam_size;

	assert( cam[key].valid && cam[key].rest == rest);
	assert( reserved[key].bv & (1U<<pos));
	
	if ( (fulfilled[key].bv | (1U<<pos)) == mask_item_bv) {

	  // write out cache line id including res is position pos

	  BWResultLine d = buf[key];
          d.line[pos] = tup.res;

          std::cout << "REORDER: writing out line " << key << " " << tag << " from addr,pos " << addr << "," << pos << std::endl;

	  resReqOut.put(MemTypedWriteReqType<BWResultLine>( config.read().getResAddr( tag), 1)); 
	  resDataOut.put(MemTypedWriteDataType<BWResultLine>(d));

	  cam[key].valid = false;
	  reserved[key].bv = 0;
	  fulfilled[key].bv = 0;

#if 0
	  if ( pending_reservation) {
            //	    unsigned int r_tag = r_addr >> log2_items_per_cl;
	    unsigned int r_pos = r_addr & mask_items_per_cl;

            unsigned int r_key  = (r_addr >> log2_items_per_cl) & mask_cam_size;
            unsigned int r_rest = (r_addr >> log2_items_per_cl) >> log2_cam_size;

            bool empty_found = !cam[r_key].valid;
            bool found = cam[r_key].valid && cam[r_key].rest == r_rest;


	    reserved[r_key].bv = 1<<r_pos;
	    fulfilled[r_key].bv = 0;
	    cam[r_key].valid = true;
	    cam[r_key].rest = r_rest;

	    BWEmpty ack;
	    reserveAckQ.nb_put( ack);

	    pending_reservation = false;
	  }
#endif

	} else {
	  buf[key].line[pos] = tup.res;
	  fulfilled[key].bv |= (1<<pos);
	}
      }


    }
    wait();
  }
}
