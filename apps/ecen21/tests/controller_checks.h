#ifndef ECEN21_CONTROLLER_CHECKS_H
#define ECEN21_CONTROLLER_CHECKS_H
/* Returns zero on success, otherwise the source line of the failed check. */
int ec_run_checks(unsigned *checks);
#endif
