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
    return a * 0.5f + b * 0.5f;
}

double __attribute__((noinline)) helper4(double a, double b) {
    return a * 0.3 + b * 0.7;
}

long __attribute__((noinline)) helper5(long a, long b) {
    return (a << 3) | (b & 0xFF);
}

/* x86-specific: Use regparm to force register argument passing */
#ifdef __i386__
int __attribute__((regparm(3), noinline)) helper_regparm(int a, int b, int c) {
    return a + b * c;
}
#endif

/* Prevent common subexpression elimination */
volatile int global_seed = 42;

int main(int argc, char *argv[]) {
    int N = (argc > 1) ? atoi(argv[1]) : 1000;
    if (N <= 0) N = 1000;
    
    /* Declare many scalar variables of mixed types */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    float f1, f2, f3, f4, f5;
    double d1, d2, d3, d4;
    long l1, l2, l3;
    
    /* Initialize with non-constant values */
    srand(time(NULL));
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
    
    f1 = (float)rand() / RAND_MAX;
    f2 = (float)rand() / RAND_MAX;
    f3 = (float)rand() / RAND_MAX;
    f4 = (float)rand() / RAND_MAX;
    f5 = (float)rand() / RAND_MAX;
    
    d1 = (double)rand() / RAND_MAX;
    d2 = (double)rand() / RAND_MAX;
    d3 = (double)rand() / RAND_MAX;
    d4 = (double)rand() / RAND_MAX;
    
    l1 = rand();
    l2 = rand();
    l3 = rand();
    
    /* Create artificial register pressure with asm */
    asm volatile("" : "+r"(v1), "+r"(v2), "+r"(v3));
    
    for (int i = 0; i < N; i++) {
        /* Create dense web of data dependencies */
        v1 = v2 + v3 + global_seed;
        v4 = v1 * v5 - v6;
        v7 = helper1(v4, v2);  /* Function call 1 */
        
        /* CRITICAL: v8 is live across the call in a call-clobbered register */
        v8 = v7 * v3;  /* Uses return value immediately */
        
        /* v9, v10 remain live across next call */
        v9 = v8 + v4 + v1;
        v10 = helper2(v9, v5, v6);  /* Function call 2 */
        
        /* Immediately use v10 with other live variables */
        v2 = v10 * v3 + v7;
        
        /* Float operations to use FP call-clobbered registers */
        f1 = f2 * 1.5f + f3;
        f4 = helper3(f1, f2);  /* Function call 3 */
        
        /* f5 is live across call */
        f5 = f4 + f3 * 2.0f;
        f2 = helper3(f5, f4);  /* Another call */
        
        /* Double operations */
        d1 = d2 * 0.8 + d3;
        d4 = helper4(d1, d2);  /* Function call 4 */
        
        /* d3 is live across call */
        d3 = d4 * 0.6 + d1;
        
        /* Long operations */
        l1 = l2 ^ l3;
        l2 = helper5(l1, l3);  /* Function call 5 */
        
        /* l3 is live across call */
        l3 = l2 << 2 | l1;
        
        /* Mix types to prevent optimization */
        v3 = (int)f1 + (int)d1 + (int)l1;
        v5 = helper1(v3, v8);  /* Another call */
        
        /* Create loop-carried dependencies */
        v6 = v5 + v6 + i;  /* v6 depends on its previous value */
        v7 = v6 * v7 - i;
        
        /* More artificial register pressure */
        asm volatile("" : "+r"(v4), "+r"(v9), "+r"(v10));
        
        /* Force spill by using all variables after calls */
        f3 = (float)v4 * 0.1f + f5;
        d2 = (double)v9 * 0.01 + d4;
        
        /* Another call with many live registers */
        v1 = helper2(v1, v2, v3);
        
        /* Immediately use result with other live values */
        v4 = v1 + v8 + v9;  /* v8 and v9 were live across the call */
        
        #ifdef __i386__
        /* Use regparm calling convention on x86 */
        v5 = helper_regparm(v4, v6, v7);
        #endif
        
        /* Prevent optimization of loop */
        global_seed = (global_seed * 1103515245 + 12345) & 0x7fffffff;
    }
    
    /* Compute checksum to prevent dead code elimination */
    long checksum = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10
                  + (long)f1 + (long)f2 + (long)f3 + (long)f4 + (long)f5
                  + (long)d1 + (long)d2 + (long)d3 + (long)d4
                  + l1 + l2 + l3;
    
    printf("Checksum: %ld\n", checksum);
    return 0;
}
