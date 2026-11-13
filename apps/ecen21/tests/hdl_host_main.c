#include "hdl_checks.h"
#include <stdio.h>
int main(void){unsigned checks;int rc=hg_run_checks(&checks);printf("%s: M8 HDL generator/export; %u checks; failure line %d\n",rc?"FAIL":"PASS",checks,rc);return rc?1:0;}
