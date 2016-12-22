// See LICENSE for license details.
 
#ifdef USE_HLS

#ifndef STRATUS

#if defined(__SYNTHESIS__) || defined(NCSC)
#include "ctos_fx.h"
#define REAL_VAL_TYPE(___I, ___F) ctos_sc_dt::sc_fixed<___I, ___F, sc_dt::SC_RND, sc_dt::SC_SAT>
#define UREAL_VAL_TYPE(___I, ___F) ctos_sc_dt::sc_ufixed<___I, ___F, sc_dt::SC_RND, sc_dt::SC_SAT>
#else
#include "sysc/datatypes/fx/sc_fixed.h"
#include "sysc/datatypes/fx/sc_ufixed.h"
#define REAL_VAL_TYPE(___I, ___F) sc_dt::sc_fixed<___I, ___F, sc_dt::SC_RND, sc_dt::SC_SAT>
#define UREAL_VAL_TYPE(___I, ___F) sc_dt::sc_ufixed<___I, ___F, sc_dt::SC_RND, sc_dt::SC_SAT>
#endif // #ifdef __SYNTHESIS__

#endif

#else
#if defined(MRH_SPL) || defined(USE_FIXED)
#include "sysc/datatypes/fx/sc_fixed.h"
#include "sysc/datatypes/fx/sc_ufixed.h"
#define REAL_VAL_TYPE(___I, ___F) sc_dt::sc_fixed<___I, ___F, sc_dt::SC_RND, sc_dt::SC_SAT>
#define UREAL_VAL_TYPE(___I, ___F) sc_dt::sc_ufixed<___I, ___F, sc_dt::SC_RND, sc_dt::SC_SAT>
#endif

#endif // #ifdef USE_HLS
 
