/* caller-save-test.c
 * Designed to trigger uncovered lines 905-913 in caller-save.cc.gcov
 * Compile with: gcc -O2 -fno-omit-frame-pointer -fno-inline -fno-tree-loop-optimize caller-save-test.c -o caller-save-test
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
    return (a + b) * c;
}
#endif

/* Force register pressure by using many live variables across calls */
int main(int argc, char *argv[]) {
    /* Non-constant loop bound to prevent optimization */
    int N = (argc > 1) ? atoi(argv[1]) : 1000;
    if (N <= 0) N = 1000;
    
    /* Seed RNG to prevent constant propagation */
    srand(time(NULL));
    
    /* Declare many scalar variables of mixed types */
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
    int v11 = rand() % 100;
    int v12 = rand() % 100;
    int v13 = rand() % 100;
    int v14 = rand() % 100;
    int v15 = rand() % 100;
    float f1 = (float)(rand() % 100) / 10.0f;
    float f2 = (float)(rand() % 100) / 10.0f;
    float f3 = (float)(rand() % 100) / 10.0f;
    double d1 = (double)(rand() % 100) / 5.0;
    double d2 = (double)(rand() % 100) / 5.0;
    long l1 = (long)rand() * rand();
    long l2 = (long)rand() * rand();
    
    /* Mark some variables as volatile to inhibit optimizations */
    volatile int v_volatile = rand() % 50;
    
    /* Main loop with loop-carried dependencies */
    for (int i = 0; i < N; i++) {
        /* Create dense web of data dependencies */
        v1 = v2 + v3 + i;
        v2 = v1 - v4;
        v3 = v5 * v6;
        v4 = v7 ^ v8;
        v5 = v9 + v10;
        v6 = v11 - v12;
        v7 = v13 * v14;
        v8 = v15 ^ v1;
        v9 = v2 + v3;
        v10 = v4 - v5;
        v11 = v6 * v7;
        v12 = v8 ^ v9;
        v13 = v10 + v11;
        v14 = v12 - v13;
        v15 = v14 * v1;
        
        /* Float operations */
        f1 = f2 * 1.1f + (float)v1;
        f2 = f3 - 0.5f * f1;
        f3 = f1 + f2 * 2.0f;
        
        /* Double operations */
        d1 = d2 * 1.5 + (double)v2;
        d2 = d1 - 0.25 * (double)v3;
        
        /* Long operations */
        l1 = l2 << 2;
        l2 = l1 | (v4 & 0xFF);
        
        /* CRITICAL: Function call with live variables as arguments.
         * Variables NOT passed to the function must remain live in
         * call-clobbered registers across the call. */
        int ret1 = helper1(v1, v2);
        
        /* Immediately use return value with variables that were live
         * across the call (v3, v4, etc.). This creates the precise
         * scenario where caller-save code must be inserted between
         * the call and this use. */
        v3 = ret1 + v3 + v_volatile;  /* v3 was live across helper1 call */
        v4 = v4 * ret1;               /* v4 was live across helper1 call */
        
        /* Use inline asm to create artificial register pressure
         * and prevent reordering */
        asm volatile("" : "+r"(v5), "+r"(v6));
        
        /* Another function call, creating another insertion point */
        float ret2 = helper3(f1, f2);
        
        /* Use return value with variables live across this call */
        f3 = ret2 + f3 + (float)v5;
        v6 = v6 + (int)ret2;
        
        /* More operations to keep many values live */
        v7 = helper2(v7, v8, v9);
        v8 = v10 + v11 + v12;
        
        /* Double function call */
        double ret3 = helper4(d1, d2);
        d1 = ret3 * 0.8 + d1;
        
        /* Long function call */
        long ret4 = helper5(l1, l2);
        l2 = ret4 ^ l2;
        
        #ifdef __x86_64__
        /* x86-specific: regparm calling convention uses specific registers */
        int ret5 = helper_regparm(v13, v14, v15);
        v13 = ret5 + v13;
        #endif
        
        /* More arithmetic to maintain dependencies */
        v9 = v9 * 2 - v8;
        v10 = v10 + v11 * 3;
        v11 = v12 ^ v13;
        v12 = v14 + v15;
        v14 = v1 * v2 - v3;
        v15 = v4 + v5 + v6;
        
        /* Use volatile to force memory/register traffic */
        v_volatile = v_volatile + 1;
    }
    
    /* Compute checksum to prevent dead code elimination */
    long checksum = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                    v11 + v12 + v13 + v14 + v15 +
                    (long)f1 + (long)f2 + (long)f3 +
                    (long)d1 + (long)d2 +
                    l1 + l2 + v_volatile;
    
    printf("Checksum: %ld\n", checksum);
    return 0;
}
