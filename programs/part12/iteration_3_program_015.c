/* caller-save-trigger.c
 * Compile with: gcc -O2 -fno-omit-frame-pointer -fno-inline -fno-tree-loop-optimize caller-save-trigger.c -o caller-save-trigger
 * For RTL dumps: gcc -O3 -fno-schedule-insns -fdump-rtl-all caller-save-trigger.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Non-inlineable helper functions with different calling conventions */
int __attribute__((noinline)) helper1(int a, int b) {
    return (a ^ b) + 1;
}

int __attribute__((noinline)) helper2(int a, int b, int c) {
    return (a * b) - c;
}

float __attribute__((noinline)) helper3(float a, float b) {
    return a * 0.5f + b * 0.5f;
}

double __attribute__((noinline)) helper4(double a, double b) {
    return a * 2.0 - b;
}

long __attribute__((noinline)) helper5(long a, long b) {
    return (a << 3) | (b & 0xFF);
}

/* x86-specific: force use of specific registers */
#ifdef __x86_64__
int __attribute__((regparm(3), noinline)) helper_regparm(int a, int b, int c) {
    return a + b * c;
}
#endif

int main(int argc, char *argv[]) {
    /* Non-constant loop bound to prevent optimization */
    int N = (argc > 1) ? atoi(argv[1]) : 1000;
    if (N <= 0) N = 1000;
    
    /* Seed RNG for variable initialization */
    srand(time(NULL));
    
    /* Declare many scalar variables of mixed types - all potentially live */
    int v1 = rand() % 100;
    int v2 = rand() % 100;
    int v3 = rand() % 100;
    int v4 = rand() % 100;
    int v5 = rand() % 100;
    int v6 = rand() % 100;
    int v7 = rand() % 100;
    int v8 = rand() % 100;
    int v9 = rand() % 100;
    int v10 = rand() % 100;
    float f1 = (float)(rand() % 100) * 0.1f;
    float f2 = (float)(rand() % 100) * 0.1f;
    float f3 = (float)(rand() % 100) * 0.1f;
    double d1 = (double)(rand() % 100) * 0.01;
    double d2 = (double)(rand() % 100) * 0.01;
    long l1 = (long)rand() * rand();
    long l2 = (long)rand() * rand();
    long l3 = (long)rand() * rand();
    long l4 = (long)rand() * rand();
    
    /* Additional volatile variables to inhibit optimizations */
    volatile int v_vol1 = rand() % 50;
    volatile float f_vol1 = (float)(rand() % 50) * 0.1f;
    
    printf("Starting computation with N=%d\n", N);
    
    for (int i = 0; i < N; i++) {
        /* Create dense web of data dependencies between variables */
        v1 = v2 + v3;
        v2 = v1 - v4;
        v3 = v2 * v5;
        v4 = v3 ^ v6;
        v5 = v4 + v7;
        v6 = v5 - v8;
        v7 = v6 * v9;
        v8 = v7 ^ v10;
        
        /* Mix in float operations */
        f1 = f2 * 1.1f + f3;
        f2 = f1 * 0.9f - f3;
        f3 = f2 + f1 * 0.5f;
        
        /* Double precision operations */
        d1 = d2 * 1.01 + (double)v1 * 0.001;
        d2 = d1 * 0.99 - (double)v2 * 0.001;
        
        /* Long integer operations */
        l1 = l2 << 2;
        l2 = l3 >> 1;
        l3 = l4 | 0xFFFF;
        l4 = l1 & l2;
        
        /* CRITICAL: Call helper function with some live variables,
         * then immediately use other variables that were live across the call */
        int ret1 = helper1(v1, v2);
        
        /* v3, v4, f1, d1, l1 are live across the call above */
        v9 = v3 + v4 + ret1;  /* Uses v3, v4 which were live across call */
        
        /* Artificial use to force register allocation */
        asm volatile("" : "+r"(v3), "+r"(v4));
        
        /* Another call with different arguments */
        int ret2 = helper2(v5, v6, v7);
        
        /* v8, v9, f2, d2, l2 are live across this call */
        v10 = v8 * v9 - ret2;  /* Uses v8, v9 which were live across call */
        
        /* Float/double calls */
        float fret = helper3(f1, f2);
        f1 = fret + f3;  /* f3 was live across helper3 call */
        
        double dret = helper4(d1, d2);
        d2 = dret * 0.5;  /* d1 was live across helper4 call */
        
        long lret = helper5(l1, l2);
        l3 = lret ^ l4;  /* l4 was live across helper5 call */
        
        #ifdef __x86_64__
        /* x86-specific: force register parameter passing */
        int regret = helper_regparm(v1, v2, v3);
        v4 = v5 + regret;  /* v5 was live across helper_regparm call */
        #endif
        
        /* Use volatile variables to create artificial dependencies */
        v1 += v_vol1;
        f1 += f_vol1;
        
        /* Loop-carried dependencies to prevent register elimination */
        v1 += i;
        v2 -= i % 7;
        v3 ^= i;
        v4 += (i << 1);
        v5 -= i % 11;
        v6 ^= (i * 3);
        v7 += i % 13;
        v8 -= (i << 2);
        v9 ^= i % 17;
        v10 += i % 19;
        
        f1 += (float)i * 0.01f;
        f2 -= (float)(i % 5) * 0.1f;
        f3 += (float)(i % 3) * 0.05f;
        
        d1 += (double)i * 0.001;
        d2 -= (double)(i % 7) * 0.002;
        
        l1 += i;
        l2 ^= i;
        l3 += (i << 3);
        l4 ^= (i << 1);
    }
    
    /* Compute checksum to prevent dead code elimination */
    long checksum = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    checksum += (long)f1 + (long)f2 + (long)f3;
    checksum += (long)d1 + (long)d2;
    checksum += l1 + l2 + l3 + l4;
    
    printf("Checksum: %ld\n", checksum);
    
    return 0;
}
