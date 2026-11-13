#ifndef ECEN21_TIMING_SAMPLES_H
#define ECEN21_TIMING_SAMPLES_H
#include "timing.h"
/*0=max64,1..5=Review Timing1..5,6=reset,7=SR/X,8=SRlatch,9=nonconvergence. */
bool tm_sample(tm_model *model,unsigned sample);
const char *tm_sample_name(unsigned sample);
#endif
