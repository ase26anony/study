/* caller_save_test.c - Test program for GCC caller-save register management */

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
    return (a & 0xFFFF) | (b << 16);
}

/* x86-specific: use regparm to force register parameter passing */
#ifdef __i386__
int __attribute__((regparm(3), noinline)) helper_regparm(int a, int b, int c) {
    return (a + b) * c;
}
#endif

/* Prevent common subexpression elimination */
volatile int global_seed = 42;

int main(int argc, char *argv[]) {
    /* Non-constant loop bound to prevent unrolling */
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
    
    f1 = (float)(rand() % 100) / 10.0f;
    f2 = (float)(rand() % 100) / 10.0f;
    f3 = (float)(rand() % 100) / 10.0f;
    f4 = (float)(rand() % 100) / 10.0f;
    f5 = (float)(rand() % 100) / 10.0f;
    
    d1 = (double)(rand() % 100) / 10.0;
    d2 = (double)(rand() % 100) / 10.0;
    d3 = (double)(rand() % 100) / 10.0;
    d4 = (double)(rand() % 100) / 10.0;
    
    l1 = rand() % 100;
    l2 = rand() % 100;
    l3 = rand() % 100;
    
    /* Main loop with loop-carried dependencies */
    for (int i = 0; i < N; i++) {
        /* Create dense web of data dependencies */
        v1 = v2 + v3 + i;
        v4 = v1 * v5 - v6;
        v7 = v4 ^ v8;
        v9 = v7 + v10;
        v2 = v9 - v3;
        v5 = v2 * v4;
        
        /* Mix in float operations */
        f1 = f2 * 1.1f + (float)v1;
        f3 = f1 + f4 - f5;
        f2 = f3 * 0.9f;
        f4 = f2 - f5;
        
        /* Double precision computations */
        d1 = d2 * 1.5 + (double)v4;
        d3 = d1 - d4;
        d2 = d3 * 0.8;
        
        /* Long integer operations */
        l1 = (l2 << 3) | (l3 & 0xFF);
        l2 = l1 + v7;
        l3 = l2 ^ v8;
        
        /* CRITICAL: Call helper1 with some live variables */
        /* v6 and v8 are passed, but v1, v2, v3, v4, v5, v7, v9, v10 
           remain live in call-clobbered registers */
        int result1 = helper1(v6, v8);
        
        /* Immediately use result with variables that were live across the call */
        /* This creates the need for caller-save insertion between the call
           and this use instruction */
        v10 = result1 + v1 + v2;  /* v1 and v2 were live across helper1 call */
        
        /* Artificial use to prevent optimization */
        asm volatile("" : "+r"(v3), "+r"(v4), "+r"(v5));
        
        /* Second function call with different arguments */
        int result2 = helper2(v3, v4, v5);
        
        /* Use result with other live variables */
        v6 = result2 * v7 / (v9 + 1);
        
        /* Float function call */
        float fresult = helper3(f1, f2);
        f5 = fresult + f3 + f4;  /* f3 and f4 were live across helper3 call */
        
        /* Double function call */
        double dresult = helper4(d1, d2);
        d4 = dresult * d3;  /* d3 was live across helper4 call */
        
        /* Long function call */
        long lresult = helper5(l1, l2);
        l3 = lresult ^ l3;
        
        #ifdef __i386__
        /* x86-specific: regparm calling convention */
        int rp_result = helper_regparm(v1, v2, v3);
        v8 = rp_result + v4 + v5;  /* v4 and v5 were live across call */
        #endif
        
        /* Create more dependencies for next iteration */
        v3 = v10 * 2 - v6;
        v8 = v9 + v7;
        
        /* Use volatile to prevent reordering */
        global_seed = i;
    }
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    checksum += (int)f1 + (int)f2 + (int)f3 + (int)f4 + (int)f5;
    checksum += (int)d1 + (int)d2 + (int)d3 + (int)d4;
    checksum += (int)l1 + (int)l2 + (int)l3;
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
