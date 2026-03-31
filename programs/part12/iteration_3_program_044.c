/* caller-save-test.c */
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
    return a * 0.5f + b * 1.5f;
}

double __attribute__((noinline)) helper4(double a, double b) {
    return a * 2.0 + b * 0.5;
}

long __attribute__((noinline)) helper5(long a, long b) {
    return (a << 3) | (b & 0xFF);
}

/* x86-specific: use regparm to force specific register usage */
#ifdef __i386__
int __attribute__((regparm(3), noinline)) helper_regparm(int a, int b, int c) {
    return a + b * c;
}
#endif

/* Prevent inlining and force register pressure */
int __attribute__((noinline)) complex_helper(int a, int b, int c, int d, 
                                            float e, float f, double g) {
    volatile int temp = a + b;
    asm volatile("" : "+r"(temp));
    return temp * c - d + (int)(e * f) + (int)g;
}

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
    int v11 = rand() % 100;
    int v12 = rand() % 100;
    int v13 = rand() % 100;
    int v14 = rand() % 100;
    int v15 = rand() % 100;
    
    float f1 = (float)(rand() % 100) * 0.1f;
    float f2 = (float)(rand() % 100) * 0.2f;
    float f3 = (float)(rand() % 100) * 0.3f;
    float f4 = (float)(rand() % 100) * 0.4f;
    float f5 = (float)(rand() % 100) * 0.5f;
    
    double d1 = (double)(rand() % 100) * 0.01;
    double d2 = (double)(rand() % 100) * 0.02;
    double d3 = (double)(rand() % 100) * 0.03;
    
    long l1 = (long)rand() * rand();
    long l2 = (long)rand() * rand();
    long l3 = (long)rand() * rand();
    
    /* Create artificial register pressure with inline asm */
    asm volatile("" : "+r"(v1), "+r"(v2), "+r"(v3));
    
    for (int i = 0; i < N; i++) {
        /* Create dense web of data dependencies */
        v1 = v2 + v3;
        v4 = v1 * v5;
        v6 = v4 - v7;
        v8 = v6 ^ v9;
        v10 = v8 | v11;
        v12 = v10 & v13;
        v14 = v12 + v15;
        v15 = v14 - v1;
        
        f1 = f2 * 1.1f + f3;
        f4 = f1 - f5 * 0.9f;
        f2 = f4 * 2.0f;
        
        d1 = d2 * 3.14 + d3;
        d3 = d1 * 0.5 - d2;
        
        l1 = l2 >> 2;
        l3 = l1 | l2;
        l2 = l3 << 1;
        
        /* CRITICAL: Function call followed immediately by use of 
           variables that were live in call-clobbered registers */
        int ret1 = helper1(v1, v2);
        
        /* This use of v3, v4, v5 must happen immediately after the call.
           They were live across the call and likely in call-clobbered regs. */
        v3 = ret1 + v4 + v5;  /* v3, v4, v5 were live across helper1 call */
        
        /* Insert asm barrier to prevent reordering */
        asm volatile("" : : "r"(v3), "r"(v4), "r"(v5));
        
        /* Another call creating more pressure */
        float ret2 = helper3(f1, f2);
        
        /* Immediate use of f3, f4 which were live across the call */
        f5 = ret2 * f3 + f4;  /* f3, f4 were live across helper3 call */
        
        /* More complex call with many arguments */
        int ret3 = complex_helper(v6, v7, v8, v9, f1, f2, d1);
        
        /* Immediate use of many variables live across the call */
        v10 = ret3 + v11 + v12 + v13;
        v14 = v10 * v15;
        
        /* Use volatile to force register saves */
        volatile int vol_temp = v14;
        (void)vol_temp;
        
        /* Double call sequence */
        double ret4 = helper4(d1, d2);
        d3 = ret4 + d1;  /* d1 was live across helper4 call */
        
        long ret5 = helper5(l1, l2);
        l3 = ret5 ^ l1;  /* l1 was live across helper5 call */
        
        #ifdef __i386__
        int ret6 = helper_regparm(v1, v2, v3);
        v4 = ret6 + v5;  /* v5 was live across helper_regparm call */
        #endif
        
        /* Create loop-carried dependencies */
        v1 = v1 + i;
        v2 = v2 - i;
        f1 = f1 + (float)i * 0.1f;
        d1 = d1 + (double)i * 0.01;
        l1 = l1 + i;
        
        /* Another helper call at the end of loop body */
        int ret7 = helper2(v14, v15, v1);
        v2 = ret7 * v3;  /* v3 was live across helper2 call */
    }
    
    /* Compute checksum to prevent dead code elimination */
    long checksum = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                   v11 + v12 + v13 + v14 + v15 +
                   (long)f1 + (long)f2 + (long)f3 + (long)f4 + (long)f5 +
                   (long)d1 + (long)d2 + (long)d3 +
                   l1 + l2 + l3;
    
    printf("Checksum: %ld\n", checksum);
    return 0;
}
