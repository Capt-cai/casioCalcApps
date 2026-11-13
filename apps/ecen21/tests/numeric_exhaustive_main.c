#include "numeric_checks.h"
#include <stdio.h>
int main(void)
{
    unsigned checks;
    int failed = nm_run_exhaustive(&checks);
    printf("%s: M3 exhaustive1..8bit and sampled9..32bit; %u checks; failure line %d\n",
        failed ? "FAIL" : "PASS", checks, failed);
    return failed ? 1 : 0;
}
