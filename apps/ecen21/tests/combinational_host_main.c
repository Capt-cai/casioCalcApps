#include "combinational_checks.h"
#include <stdio.h>
int main(void) { unsigned checks;int rc=cb_run_checks(&checks);printf("%s: M7 combinational; %u checks; failure line %d\n",rc?"FAIL":"PASS",checks,rc);return rc?1:0; }
