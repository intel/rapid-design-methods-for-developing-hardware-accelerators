// See LICENSE for license details.
/*
 * ga_tlm_fifo_hls_head.h
 *
 *  Created on: Mar 13, 2015
 *      Author: aayupov
 */


#include "ctos_tlm.h"
#include "ctos_flex_channels_utils.h"

#define FLEX_CHANNELS_LABEL(s) \

#define FLEX_CHANNELS_PROTOCOL(s) \


#if defined(SYU_HLS) || defined(ALM_HLS)
#define FLEX_CHANNELS_LABEL(s) \

#define FLEX_CHANNELS_PROTOCOL(s) \

#endif

#include "ctos_flex_channels.h"
#ifndef __SYNTHESIS__
#include "tlm.h"
#define TLM_NMSPC tlm
#else
#define TLM_NMSPC ctos_tlm
#endif

#ifdef __SYNTHESIS__
//#define SC_SIGNAL_MULTIW(PAYLOAD, VAR)   sc_signal<PAYLOAD, SC_MANY_WRITERS> VAR
#define SC_SIGNAL_MULTIW(PAYLOAD, VAR)   sc_signal<PAYLOAD> VAR
#else
#define SC_SIGNAL_MULTIW(PAYLOAD, VAR)   sc_signal<PAYLOAD, SC_MANY_WRITERS> VAR
#endif
