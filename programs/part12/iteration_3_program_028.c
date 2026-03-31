/* caller-save-trigger.c */
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

/* x86-specific regparm calling convention to increase register pressure */
#ifdef __i386__
int __attribute__((regparm(3), noinline)) helper_regparm(int a, int b, int c) {
    return a + b * c;
}
#endif

/* Force register usage with asm */
#define FORCE_REGISTER(var) asm volatile("" : "+r"(var))

int main(int argc, char *argv[]) {
    int N = (argc > 1) ? atoi(argv[1]) : 1000;
    
    /* Declare many scalar variables of mixed types */
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    double d1 = 1.11, d2 = 2.22, d3 = 3.33, d4 = 4.44, d5 = 5.55;
    long l1 = 100, l2 = 200, l3 = 300, l4 = 400, l5 = 500;
    
    /* Use volatile to prevent optimization */
    volatile int v_volatile = 0;
    
    srand(time(NULL));
    
    for (int i = 0; i < N; i++) {
        /* Create dense web of data dependencies */
        v1 = v2 + v3 + i;
        v2 = v1 * v4 - v5;
        v3 = v2 ^ v6;
        v4 = v3 + v7 * 2;
        v5 = v4 - v8 / 3;
        
        /* Mix in float operations */
        f1 = f2 * 1.5f + (float)v1;
        f2 = f3 - f1 * 0.5f;
        f3 = f4 + (float)v2 * 0.25f;
        f4 = f5 * 2.0f - f2;
        
        /* Double operations */
        d1 = d2 + (double)v3 * 0.1;
        d2 = d3 * 1.1 - d1;
        d3 = d4 + (double)v4 * 0.01;
        
        /* Long operations */
        l1 = l2 << 2;
        l2 = l3 | (l1 & 0xFFFF);
        l3 = l4 + v5 * 2L;
        
        /* CRITICAL: Function call with live values, followed immediately by 
           use of other live values that must be saved across the call */
        int ret1 = helper1(v1, v2);
        
        /* v3, v4, v5 are live here and in call-clobbered registers */
        v6 = v3 + v4 + v5 + ret1;  /* Uses values live across the call */
        FORCE_REGISTER(v6);  /* Force register use */
        
        /* Another function call creating another insertion point */
        float ret2 = helper3(f1, f2);
        
        /* f3, f4 are live here */
        f5 = f3 * f4 + ret2;
        FORCE_REGISTER(f5);
        
        /* More operations to increase pressure */
        v7 = helper2(v6, v5, v4);
        v8 = v7 + v3 * 2;
        
        double ret3 = helper4(d1, d2);
        d4 = d3 * ret3 + 1.0;
        
        long ret4 = helper5(l1, l2);
        l4 = l3 + ret4;
        
        /* Use volatile to create artificial barrier */
        v_volatile = v1 + v2 + v3;
        
        /* More function calls with mixed arguments */
        v9 = helper1(v8, v7);
        v10 = helper2(v9, v8, v7);
        
        /* Create loop-carried dependencies */
        v1 = v10 + i;
        v2 = v9 - i;
        
        /* Force all variables to be live simultaneously */
        asm volatile("" : : "r"(v1), "r"(v2), "r"(v3), "r"(v4), "r"(v5),
                         "r"(v6), "r"(v7), "r"(v8), "r"(v9), "r"(v10),
                         "r"(f1), "r"(f2), "r"(f3), "r"(f4), "r"(f5),
                         "r"(d1), "r"(d2), "r"(d3), "r"(d4),
                         "r"(l1), "r"(l2), "r"(l3), "r"(l4));
    }
    
    /* Compute checksum to prevent elimination */
    long checksum = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10
                  + (long)f1 + (long)f2 + (long)f3 + (long)f4 + (long)f5
                  + (long)d1 + (long)d2 + (long)d3 + (long)d4
                  + l1 + l2 + l3 + l4;
    
    printf("Checksum: %ld\n", checksum);
    return 0;
}
