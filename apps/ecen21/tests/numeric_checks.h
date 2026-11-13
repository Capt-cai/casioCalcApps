#ifndef ECEN21_NUMERIC_CHECKS_H
#define ECEN21_NUMERIC_CHECKS_H
/* Returns zero on success or the failing source line; increments checks. */
int nm_run_checks(unsigned *checks);
#ifdef NM_FULL_HOST_TESTS
int nm_run_exhaustive(unsigned *checks);
#endif
#endif
