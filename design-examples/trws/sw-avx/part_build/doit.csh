#!/bin/csh -f


foreach i ( 1 2 4 6 8 10 12 14 16)

make clean && make USE_TBB=1 NUM_OF_THREADS=${i} && ./trws_part > LOG_${i}

end
