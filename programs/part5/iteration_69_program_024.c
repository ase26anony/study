/* test_sched_coverage.c
 *
 * Compilation instructions for coverage:
 * 1. Basic scheduling debug:
 *    gcc -O2 -fsched-verbose=3 test_sched_coverage.c -o test_sched_coverage
 * 2. Register pressure modeling:
 *    gcc -O3 -fsched-verbose=4 -fsel-sched-pipelining -fsel-sched-pipelining-outer-loops test_sched_coverage.c -o test_sched_coverage_pressure
 * 3. Aggressive scheduling with delays:
 *    gcc -O2 -fsched-verbose=5 -fno-schedule-insns -fno-schedule-insns2 -march=native test_sched_coverage.c -o test_sched_coverage_delay
 *
 * The scheduling debug output will be printed to stderr during compilation.
 * The runtime execution validates correctness but the key coverage occurs during compilation.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define SIZE 1024
#define ITERS 100000

/* Volatile variables to prevent optimization and create artificial dependencies */
volatile int vol_a = 1, vol_b = 2, vol_c = 3;
volatile float vol_f1 = 1.5f, vol_f2 = 2.5f, vol_f3 = 3.5f;

/* Inline assembly to clobber registers and increase pressure */
#define CLOBBER_MANY \
    __asm__ volatile ("" : : : \
        "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", \
        "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15", \
        "memory")

/* Function with high register pressure and independent instruction groups */
void high_pressure_loop(int *restrict a, int *restrict b, int *restrict c, int n) {
    int i;
    /* Many live variables across loop iterations */
    int t0, t1, t2, t3, t4, t5, t6, t7, t8, t9;
    int u0, u1, u2, u3, u4, u5, u6, u7, u8, u9;
    float f0, f1, f2, f3, f4, f5, f6, f7, f8, f9;

    for (i = 0; i < n; i++) {
        /* Independent integer computation groups */
        t0 = a[i] + b[i];
        t1 = a[i] - b[i];
        t2 = a[i] * b[i];
        t3 = a[i] ^ b[i];
        t4 = a[i] | b[i];
        t5 = b[i] + c[i];
        t6 = b[i] - c[i];
        t7 = b[i] * c[i];
        t8 = b[i] ^ c[i];
        t9 = b[i] | c[i];

        /* Independent float computation groups (mixing types) */
        f0 = vol_f1 * vol_f2;
        f1 = vol_f2 / vol_f3;  /* Long latency divide */
        f2 = vol_f1 + vol_f3;
        f3 = vol_f2 - vol_f3;
        f4 = sqrtf(f0 + f1);   /* Long latency sqrt */
        f5 = f0 * f1;
        f6 = f1 / f2;
        f7 = f2 + f3;
        f8 = f3 - f0;
        f9 = f4 * f5;

        /* More independent groups with volatile dependencies */
        u0 = t0 + vol_a;
        u1 = t1 - vol_b;
        u2 = t2 * vol_c;
        u3 = t3 ^ vol_a;
        u4 = t4 | vol_b;
        u5 = t5 + vol_c;
        u6 = t6 - vol_a;
        u7 = t7 * vol_b;
        u8 = t8 ^ vol_c;
        u9 = t9 | vol_a;

        /* Cross dependencies to create priority differences */
        t0 = u0 + u1;
        t1 = u2 - u3;
        t2 = u4 * u5;
        t3 = u6 ^ u7;
        t4 = u8 | u9;
        t5 = u0 + u9;
        t6 = u1 - u8;
        t7 = u2 * u7;
        t8 = u3 ^ u6;
        t9 = u4 | u5;

        /* Store results to prevent dead code elimination */
        a[i] = t0 + t1 + t2 + t3 + t4;
        b[i] = t5 + t6 + t7 + t8 + t9;
        c[i] = (int)(f0 + f1 + f2 + f3 + f4 + f5 + f6 + f7 + f8 + f9);

        /* Clobber many registers to increase pressure */
        CLOBBER_MANY;
    }
}

/* Function with mixed dependencies and resource conflicts */
void mixed_dependency(int *restrict arr, int n) {
    int i;
    int dep0 = vol_a;
    int dep1 = vol_b;
    float fdep0 = vol_f1;
    float fdep1 = vol_f2;

    for (i = 0; i < n; i++) {
        /* Long latency memory access with aliasing possibility */
        int *ptr = &arr[i & (SIZE - 1)];
        int val = *ptr;

        /* Chain of dependencies causing delays */
        dep0 = (dep0 * val) / (vol_c + 1);  /* Integer divide */
        dep1 = (dep1 + dep0) ^ val;
        fdep0 = fdep0 / (vol_f3 + 0.1f);    /* Float divide */
        fdep1 = fdep1 * fdep0 + sqrtf(fdep0); /* sqrt */

        /* Independent operations that can be reordered */
        int ind0 = arr[(i + 1) & (SIZE - 1)] + 1;
        int ind1 = arr[(i + 2) & (SIZE - 1)] - 2;
        int ind2 = arr[(i + 3) & (SIZE - 1)] * 3;
        int ind3 = arr[(i + 4) & (SIZE - 1)] ^ 4;
        int ind4 = arr[(i + 5) & (SIZE - 1)] | 5;

        /* Mix with volatile to prevent reordering across barrier */
        ind0 += vol_a;
        ind1 -= vol_b;
        ind2 *= vol_c;
        ind3 ^= vol_a;
        ind4 |= vol_b;

        /* Store results with dependencies */
        arr[i] = dep0 + dep1 + (int)fdep0 + (int)fdep1 +
                 ind0 + ind1 + ind2 + ind3 + ind4;
    }
}

/* Main driver that creates hot loops for scheduler analysis */
int main() {
    int i;
    int *data1 = malloc(SIZE * sizeof(int));
    int *data2 = malloc(SIZE * sizeof(int));
    int *data3 = malloc(SIZE * sizeof(int));
    int *data4 = malloc(SIZE * sizeof(int));

    if (!data1 || !data2 || !data3 || !data4) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }

    /* Initialize with random data */
    srand(time(NULL));
    for (i = 0; i < SIZE; i++) {
        data1[i] = rand() % 1000;
        data2[i] = rand() % 1000;
        data3[i] = rand() % 1000;
        data4[i] = rand() % 1000;
    }

    /* Performance-critical loops that will be scheduled */
    for (i = 0; i < ITERS; i++) {
        high_pressure_loop(data1, data2, data3, SIZE);
        mixed_dependency(data4, SIZE);
    }

    /* Final checksum to prevent dead code elimination */
    unsigned long long sum = 0;
    for (i = 0; i < SIZE; i++) {
        sum += data1[i] + data2[i] + data3[i] + data4[i];
    }
    printf("Checksum: %llu\n", sum);

    free(data1);
    free(data2);
    free(data3);
    free(data4);

    return 0;
}
