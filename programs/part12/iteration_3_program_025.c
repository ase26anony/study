/* caller-save-trigger.c */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Non-inlineable helper functions with different calling conventions */
int __attribute__((noinline)) helper1(int a, int b) {
    return (a ^ b) * 31;
}

int __attribute__((noinline)) helper2(int a, int b, int c) {
    return (a + b) * c;
}

float __attribute__((noinline)) helper3(float a, float b) {
    return a * 0.5f + b * 0.5f;
}

double __attribute__((noinline)) helper4(double a, double b) {
    return a * 2.0 + b * 0.25;
}

long __attribute__((noinline)) helper5(long a, long b) {
    return (a << 3) | (b & 0x7F);
}

/* x86-specific: force use of specific registers */
#ifdef __x86_64__
int __attribute__((regparm(3), noinline)) helper_regparm(int a, int b, int c) {
    return a * b + c;
}
#endif

int main(int argc, char *argv[]) {
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
    
    float f1 = (float)(rand() % 100) * 0.1f;
    float f2 = (float)(rand() % 100) * 0.1f;
    float f3 = (float)(rand() % 100) * 0.1f;
    float f4 = (float)(rand() % 100) * 0.1f;
    
    double d1 = (double)(rand() % 100) * 0.01;
    double d2 = (double)(rand() % 100) * 0.01;
    double d3 = (double)(rand() % 100) * 0.01;
    
    long l1 = (long)rand() * rand();
    long l2 = (long)rand() * rand();
    long l3 = (long)rand() * rand();
    
    /* Volatile variables to inhibit optimizations */
    volatile int v_vol1 = v1;
    volatile float f_vol1 = f1;
    volatile double d_vol1 = d1;
    
    /* Main loop with loop-carried dependencies */
    for (int i = 0; i < N; i++) {
        /* Create dense web of data dependencies */
        v1 = v2 + v3;
        v2 = v1 * v4 - v5;
        v3 = v6 ^ v7;
        v4 = v8 + v9 * v10;
        v5 = v1 | v2;
        v6 = v3 & v4;
        v7 = v5 - v6;
        v8 = v7 * 31;
        v9 = v8 / (v10 + 1);
        v10 = v9 + i;
        
        /* Mix in float operations */
        f1 = f2 * 1.1f + f3;
        f2 = f1 * 0.9f - f4;
        f3 = f2 + (float)v1 * 0.01f;
        f4 = f3 * f1;
        
        /* Double operations */
        d1 = d2 * 1.01 + d3;
        d2 = d1 * 0.99 - (double)v2 * 0.001;
        d3 = d2 + d1 * 0.5;
        
        /* Long operations */
        l1 = l2 << 2;
        l2 = l3 >> 1;
        l3 = l1 | l2;
        
        /* CRITICAL: Call function with some live variables,
           then immediately use other variables that were live across call */
        int ret1 = helper1(v1, v2);
        
        /* v3, v4, f1, d1 are live across the call and in call-clobbered regs */
        v3 = v3 + ret1;  /* Uses v3 which was live across helper1 call */
        v4 = v4 * ret1;  /* Uses v4 which was live across helper1 call */
        
        /* Artificial use to force register pressure */
        asm volatile("" : "+r"(v3), "+r"(v4));
        
        /* Another call creating new insertion point */
        float ret2 = helper3(f1, f2);
        
        /* f3, d1 are live across this call */
        f3 = f3 + ret2;
        d1 = d1 + (double)ret2;
        
        /* Use volatile variables */
        v_vol1 = v3;
        f_vol1 = f3;
        d_vol1 = d1;
        
        /* More calls with mixed arguments */
        double ret3 = helper4(d1, d2);
        d3 = d3 * ret3;
        
        long ret4 = helper5(l1, l2);
        l3 = l3 ^ ret4;
        
        /* Call with many arguments to increase register pressure */
        int ret5 = helper2(v5, v6, v7);
        v8 = v8 + ret5;
        v9 = v9 - ret5;
        
        #ifdef __x86_64__
        /* Force specific register usage on x86 */
        int ret6 = helper_regparm(v8, v9, v10);
        v10 = v10 + ret6;
        #endif
        
        /* Create circular dependencies for next iteration */
        v1 = v10 + i;
        v2 = v1 * 2;
        f1 = (float)v2 * 0.1f;
        d1 = (double)v1 * 0.01;
        l1 = (long)v3 * v4;
    }
    
    /* Compute checksum to prevent dead code elimination */
    long checksum = (long)v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10
                  + (long)(f1 * 100) + (long)(f2 * 100) + (long)(f3 * 100) + (long)(f4 * 100)
                  + (long)(d1 * 100) + (long)(d2 * 100) + (long)(d3 * 100)
                  + l1 + l2 + l3
                  + v_vol1;
    
    printf("Checksum: %ld\n", checksum);
    return (int)(checksum % 100);
}
