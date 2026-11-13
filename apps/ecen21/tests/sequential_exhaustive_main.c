#include "sequential_checks.h"
#include <stdio.h>
int main(void)
{
    unsigned checks;int failed=sq_run_exhaustive(&checks);
    printf("%s: M5 exhaustive sequential; %u checks; failure line %d\n",failed?"FAIL":"PASS",checks,failed);
    return failed?1:0;
}
