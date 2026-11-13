#include "combinational_checks.h"
#include "hdl_checks.h"
#include "controller_checks.h"
#include "solver_checks.h"
#include "numeric_checks.h"
#include "flipflop_checks.h"
#include "sequential_checks.h"
#include "connection_checks.h"
#include "timing_checks.h"
#include "timing_sample_checks.h"
#include <stdio.h>

int main(void)
{
    unsigned checks;
    int failed = ec_run_checks(&checks);
    printf("%s: ECEN21 controller; %u checks; failure line %d\n",
        failed ? "FAIL" : "PASS", checks, failed);
    int solver_failed = bs_run_checks(&checks);
    printf("%s: ECEN21 M2B solver; %u checks; failure line %d\n",
        solver_failed ? "FAIL" : "PASS", checks, solver_failed);
    int numeric_failed = nm_run_checks(&checks);
    printf("%s: ECEN21 M3 numeric; %u checks; failure line %d\n",
        numeric_failed ? "FAIL" : "PASS", checks, numeric_failed);
    int flipflop_failed = ff_run_checks(&checks);
    printf("%s: ECEN21 M4 flip-flops; %u checks; failure line %d\n",
        flipflop_failed ? "FAIL" : "PASS", checks, flipflop_failed);
    int sequential_failed = sq_run_checks(&checks);
    printf("%s: ECEN21 M5 sequential; %u checks; failure line %d\n",
        sequential_failed ? "FAIL" : "PASS", checks, sequential_failed);
    int connections_failed = cn_run_checks(&checks);
    printf("%s: ECEN21 M5 connections; %u checks; failure line %d\n",
        connections_failed ? "FAIL" : "PASS", checks, connections_failed);
    int timing_failed = tm_run_checks(&checks);
    printf("%s: ECEN21 M6 timing; %u checks; failure line %d\n",
        timing_failed ? "FAIL" : "PASS", checks, timing_failed);
    int timing_samples_failed = ts_run_checks(&checks);
    printf("%s: ECEN21 M6 presets; %u checks; failure line %d\n",
        timing_samples_failed ? "FAIL" : "PASS", checks, timing_samples_failed);
    int combinational_failed = cb_run_checks(&checks);
    printf("%s: ECEN21 M7 combinational; %u checks; failure line %d\n",
        combinational_failed ? "FAIL" : "PASS", checks, combinational_failed);
    int hdl_failed = hg_run_checks(&checks);
    printf("%s: ECEN21 M8 HDL; %u checks; failure line %d\n",
        hdl_failed ? "FAIL" : "PASS", checks, hdl_failed);
    return failed || solver_failed || numeric_failed || flipflop_failed || sequential_failed || connections_failed || timing_failed || timing_samples_failed || combinational_failed || hdl_failed ? 1 : 0;
}
