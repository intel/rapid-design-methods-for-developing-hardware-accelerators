// See LICENSE for license details.
/*
 * acc_rename.h
 *
 *  Created on: Nov 8, 2016
 *      Author: aayupov
 */

#ifndef ACC_RENAME_H_
#define ACC_RENAME_H_



template <size_t SIZE, typename DATA_STATE>
struct AccRenameBuffer {
  typedef DATA_STATE DataState;
  typedef typename SizeT<SIZE>::Type Tag;

  DataState state[SIZE];

  SC_SIGNAL_MULTIW(bool, valid[SIZE]);
  ga::ga_storage_fifo<Tag, SIZE> credit_fifo;

  Tag reset_counter;

  AccRenameBuffer() {}

  bool can_alloc() {
    return credit_fifo.nb_can_get();
  }

  Tag alloc(const DataState &data) {
    assert(can_alloc());
    Tag tag = 0;
    bool found = credit_fifo.nb_get(tag);
    assert(tag < SIZE);
    state[tag] = data;
    valid[tag] = true;

    return tag;
  }

  // should be called in reset section of the process where free() will be called
  void reset_free () {
    reset_counter = SIZE;
    credit_fifo.reset_put();
  }

  // needs to run in a process
  bool multi_cycle_reset () {

    if (reset_counter != 0) {
      --reset_counter;
      free(reset_counter);
    }
    return  (reset_counter == 0);
  }

  void reset_alloc () {
    credit_fifo.reset_get();
  }
  DataState free(const Tag &tag) {
    assert(credit_fifo.nb_can_put());
    valid[tag] = false;
    credit_fifo.nb_put(tag);
    return state[tag];
  }
};



#endif /* ACC_RENAME_H_ */
