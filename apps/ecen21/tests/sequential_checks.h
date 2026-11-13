#ifndef ECEN21_SEQUENTIAL_CHECKS_H
#define ECEN21_SEQUENTIAL_CHECKS_H
int sq_run_checks(unsigned *checks);
#ifdef SQ_FULL_HOST_TESTS
int sq_run_exhaustive(unsigned *checks);
#endif
#endif
