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

int __attribute__((noinline)) helper2(int a, int b) {
    return (a & b) | 0x5555;
}

float __attribute__((noinline)) helper3(float a, float b) {
    return a * 0.5f + b * 0.5f;
}

double __attribute__((noinline)) helper4(double a, double b) {
    return a * 2.0 - b;
}

long __attribute__((noinline)) helper5(long a, long b) {
    return (a << 3) | (b & 0x7);
}

/* x86-specific: force use of specific registers */
#ifdef __x86_64__
int __attribute__((regparm(3), noinline)) helper_regparm(int a, int b, int c) {
    return a + b - c;
}
#endif

int main(int argc, char **argv) {
    int N = (argc > 1) ? atoi(argv[1]) : 1000;
    if (N <= 0) N = 1000;
    
    /* Seed RNG to prevent constant propagation */
    srand((unsigned int)time(NULL));
    
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
    float f4 = (float)(rand() % 100) / 10.0f;
    
    double d1 = (double)(rand() % 100) / 5.0;
    double d2 = (double)(rand() % 100) / 5.0;
    double d3 = (double)(rand() % 100) / 5.0;
    
    long l1 = (long)rand() * rand();
    long l2 = (long)rand() * rand();
    
    /* Mark some as volatile to inhibit optimizations */
    volatile int v_vol1 = v1;
    volatile float f_vol1 = f1;
    
    /* Main loop with loop-carried dependencies */
    for (int i = 0; i < N; i++) {
        /* Create dense web of data dependencies */
        v1 = v2 + v3;
        v2 = v1 ^ v4;
        v3 = v2 * v5;
        v4 = v3 - v6;
        v5 = v4 | v7;
        v6 = v5 & v8;
        v7 = v6 + v9;
        v8 = v7 ^ v10;
        v9 = v8 * v11;
        v10 = v9 - v12;
        v11 = v10 | v13;
        v12 = v11 & v14;
        v13 = v12 + v15;
        v14 = v13 ^ v1;  /* Circular dependency */
        v15 = v14 * v2;
        
        /* Float computations */
        f1 = f2 * 1.1f + f3;
        f2 = f1 * 0.9f - f4;
        f3 = f2 + f1 * 0.5f;
        f4 = f3 * 2.0f - f2;
        
        /* Double computations */
        d1 = d2 * 1.5 + d3;
        d2 = d1 * 0.8 - d3;
        d3 = d2 + d1 * 0.25;
        
        /* Long computations */
        l1 = (l1 << 2) | (l2 & 0xF);
        l2 = (l2 >> 1) ^ l1;
        
        /* CRITICAL: Function call with immediate use of live values after call */
        /* v1, v2, v3 are live across this call in call-clobbered registers */
        int ret1 = helper1(v1, v2);
        
        /* Immediate use of v3 (live across call) with return value */
        /* This forces save/restore insertion BETWEEN the call and this instruction */
        v3 = v3 + ret1;  /* v3 was live in call-clobbered register across helper1 call */
        
        /* Artificial use to prevent reordering */
        asm volatile("" : "+r"(v3));
        
        /* Another call with different register pressure */
        float ret2 = helper3(f1, f2);
        
        /* Immediate use of f3 (live across call) */
        f3 = f3 * ret2;  /* f3 was live across helper3 call */
        
        /* Use volatile variables to create artificial pressure */
        v_vol1 = v3;
        f_vol1 = f3;
        
        /* More calls with mixed types */
        double ret3 = helper4(d1, d2);
        d3 = d3 + ret3;  /* d3 live across call */
        
        long ret4 = helper5(l1, l2);
        l2 = l2 ^ ret4;  /* l2 live across call */
        
        /* Second integer helper */
        int ret5 = helper2(v4, v5);
        v6 = v6 * ret5;  /* v6 live across call */
        
        #ifdef __x86_64__
        /* x86-specific: regparm calling convention uses specific registers */
        int ret6 = helper_regparm(v7, v8, v9);
        v10 = v10 + ret6;  /* v10 live across call */
        #endif
        
        /* Create more dependencies for next iteration */
        v15 = v15 + i;
        f4 = f4 + (float)i * 0.1f;
        d1 = d1 + (double)i * 0.01;
        l1 = l1 + i;
    }
    
    /* Compute checksum to prevent dead code elimination */
    long checksum = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                   v11 + v12 + v13 + v14 + v15 +
                   (long)f1 + (long)f2 + (long)f3 + (long)f4 +
                   (long)d1 + (long)d2 + (long)d3 +
                   l1 + l2;
    
    printf("Checksum: %ld\n", checksum);
    return 0;
}
