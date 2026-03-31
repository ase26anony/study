/* caller-save-test.c
 * Compile with: gcc -O2 -fno-omit-frame-pointer -fno-inline -fno-tree-loop-optimize caller-save-test.c -o caller-save-test
 * For RTL dumps: gcc -O3 -fno-schedule-insns -fdump-rtl-all caller-save-test.c
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
    /* Non-constant loop bound to prevent unrolling */
    int N = (argc > 1) ? atoi(argv[1]) : 1000;
    if (N <= 0) N = 1000;
    
    /* Seed RNG for variable initialization */
    srand((unsigned int)time(NULL));
    
    /* Declare many scalar variables of mixed types */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    float f1, f2, f3, f4, f5;
    double d1, d2, d3, d4;
    long l1, l2, l3;
    
    /* Initialize with non-constant values */
    v1 = rand() % 100;
    v2 = rand() % 100;
    v3 = rand() % 100;
    v4 = rand() % 100;
    v5 = rand() % 100;
    v6 = rand() % 100;
    v7 = rand() % 100;
    v8 = rand() % 100;
    v9 = rand() % 100;
    v10 = rand() % 100;
    
    f1 = (float)(rand() % 100) / 10.0f;
    f2 = (float)(rand() % 100) / 10.0f;
    f3 = (float)(rand() % 100) / 10.0f;
    f4 = (float)(rand() % 100) / 10.0f;
    f5 = (float)(rand() % 100) / 10.0f;
    
    d1 = (double)(rand() % 100) / 10.0;
    d2 = (double)(rand() % 100) / 10.0;
    d3 = (double)(rand() % 100) / 10.0;
    d4 = (double)(rand() % 100) / 10.0;
    
    l1 = (long)rand() * rand();
    l2 = (long)rand() * rand();
    l3 = (long)rand() * rand();
    
    /* Volatile variables to prevent optimization */
    volatile int v_vol1 = 0, v_vol2 = 0;
    volatile float f_vol = 0.0f;
    
    /* Main loop with loop-carried dependencies */
    for (int i = 0; i < N; i++) {
        /* Create dense web of data dependencies */
        v1 = v2 + v3;
        v4 = v1 * v5;
        v6 = v4 - v7;
        v8 = v6 ^ v9;
        v10 = v8 | v2;
        
        f1 = f2 * 1.1f + f3;
        f4 = f1 - f5;
        f3 = f4 * 0.9f;
        
        d1 = d2 + d3 * 0.5;
        d4 = d1 - d2;
        d2 = d4 * 1.1;
        
        l1 = l2 << 2;
        l3 = l1 | l2;
        l2 = l3 + i;
        
        /* CRITICAL: Call helper1 with some live variables,
         * leaving others live in call-clobbered registers */
        int ret1 = helper1(v1, v2);
        
        /* Immediately use return value with variables that were
         * live across the call (v3, v4, v5 in call-clobbered regs) */
        v3 = ret1 + v3 + v4;  /* v3, v4 were live across call */
        v5 = v3 * v5;         /* v5 was live across call */
        
        /* Artificial use to force register allocation */
        asm volatile("" : "+r"(v3), "+r"(v4), "+r"(v5));
        
        /* Another call creating a different insertion point */
        float ret2 = helper3(f1, f2);
        
        /* Use return value with variables live across this call */
        f5 = ret2 + f3 + f4;  /* f3, f4 were live across call */
        
        /* Mix integer and float operations to increase pressure */
        v7 = helper2(v6, v8, v9);
        v9 = v7 + v10;
        
        /* Use volatile variables to create artificial dependencies */
        v_vol1 = v1 + v2;
        v_vol2 = v3 + v4;
        f_vol = f1 + f2;
        
        /* More calls with different types */
        double ret3 = helper4(d1, d2);
        d3 = ret3 + d4;
        
        long ret4 = helper5(l1, l2);
        l3 = ret4 ^ l3;
        
        #ifdef __x86_64__
        /* Force specific register usage on x86 */
        int ret5 = helper_regparm(v1, v2, v3);
        v4 = ret5 + v5;
        #endif
        
        /* Create circular dependencies to keep values live */
        v2 = v10 - v1;
        v6 = v9 + v8;
        v8 = v7 * v6;
        
        f2 = f5 * 0.8f;
        f4 = f3 / 1.2f;
        
        d2 = d3 * 1.5;
        d4 = d1 + d2;
        
        l1 = l3 >> 1;
        l2 = l1 + l2;
        
        /* Another call at the end of the iteration */
        v10 = helper1(v9, v8);
    }
    
    /* Compute checksum to prevent dead code elimination */
    long checksum = (long)v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10
                  + (long)(f1 * 100) + (long)(f2 * 100) + (long)(f3 * 100)
                  + (long)(f4 * 100) + (long)(f5 * 100)
                  + (long)d1 + (long)d2 + (long)d3 + (long)d4
                  + l1 + l2 + l3
                  + v_vol1 + v_vol2 + (long)f_vol;
    
    printf("Checksum: %ld\n", checksum);
    return 0;
}
