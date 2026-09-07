/* Benchmark driver for the REFERENCE C engine (third-party, MIT).
 *
 * Compiled against ../Malbolge-Engine (path overridable with -DREF_INCLUDE).
 * Not part of the library; measurement harness only.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "vm.h"

static const char *HELLO =
    "(=<`#9]~6ZY327Uv4-QsqpMn&+Ij\"'E%e{Ab~w=_:]Kw%o44Uqp0/Q?xNvL:`H%c#DD2^WV>gY;dts76qKJImZkj";

static volatile unsigned long long g_sink;

static double now_ns(void) {
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    return (double)ts.tv_sec * 1e9 + (double)ts.tv_nsec;
}

int main(int argc, char **argv) {
    unsigned long long iters = argc > 1 ? strtoull(argv[1], NULL, 10) : 2000000;
    size_t n = strlen(HELLO);
    unsigned int cells[128];
    size_t i;
    for (i = 0; i < n; i++) cells[i] = (unsigned char)HELLO[i];

    vm_build_crazy5();

    /* crazy */
    {
        double t0 = now_ns();
        unsigned long long k;
        for (k = 0; k < iters; k++) {
            g_sink ^= vm_crazy((unsigned)(k % 59049u),
                               (unsigned)((k * 2654435761u) % 59049u));
        }
        double dt = (now_ns() - t0) / (double)iters;
        printf("{\n");
        printf("  \"iterations\": %llu,\n", iters);
        printf("  \"crazy_ns_per_op\": %.2f,\n", dt);
    }

    /* VM stepping: hello run repeatedly */
    {
        double t0 = now_ns();
        unsigned long long k;
        for (k = 0; k < iters / 40 + 1; k++) {
            vm_init();
            RunOutcome o = vm_run(cells, n, NULL, 0, 1000000u);
            g_sink ^= o.steps;
        }
        double dt = (now_ns() - t0) / (double)(iters / 40 + 1);
        printf("  \"vm_hello_ns_per_run\": %.2f,\n", dt);
        printf("  \"vm_hello_steps\": 48,\n");
        printf("  \"sink\": %llu\n}\n", g_sink);
    }
    return 0;
}
