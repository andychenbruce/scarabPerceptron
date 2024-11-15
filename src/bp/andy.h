#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "bp/bp.h"

/*************Interface to Scarab***************/
void bp_andy_init(void);
void bp_andy_timestamp(Op*);
uns8 bp_andy_pred(Op*);
void bp_andy_spec_update(Op*);
void bp_andy_update(Op*);
void bp_andy_retire(Op*);
void bp_andy_recover(Recovery_Info*);
uns8 bp_andy_full(uns);

#ifdef __cplusplus
}
#endif
