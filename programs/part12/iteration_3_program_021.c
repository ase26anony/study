/* caller_save_test.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Non-inlineable helper functions with different calling conventions */
int __attribute__((noinline)) helper1(int a, int b) {
    return (a * 3 + b) ^ 0x55;
}

int __attribute__((noinline)) helper2(int a, int b, int c) {
    return (a + b * 2 - c) & 0xFF;
}

float __attribute__((noinline)) helper3(float a, float b) {
    return a * 2.5f - b;
}

double __attribute__((noinline)) helper4(double a, double b) {
    return a / 1.7 + b * 0.8;
}

long __attribute__((noinline)) helper5(long a, long b) {
    return (a << 3) | (b & 0xF);
}

/* x86-specific: use regparm to force register passing */
#ifdef __i386__
int __attribute__((regparm(3), noinline)) helper_regparm(int a, int b, int c) {
    return a + b - c;
}
#endif

/* Force register pressure by creating many live values */
int main(int argc, char **argv) {
    int N = (argc > 1) ? atoi(argv[1]) : 1000;
    if (N <= 0) N = 1000;
    
    /* Declare many scalar variables of mixed types */
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    int v11 = 11, v12 = 12, v13 = 13, v14 = 14, v15 = 15;
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f;
    double d1 = 1.11, d2 = 2.22, d3 = 3.33;
    long l1 = 1000L, l2 = 2000L, l3 = 3000L;
    
    /* Use volatile to prevent optimization of key variables */
    volatile int v_volatile = 0;
    
    /* Initialize with non-constant values */
    srand(42);
    v1 += rand() % 10;
    v2 += rand() % 10;
    f1 += (rand() % 100) / 100.0f;
    d1 += (rand() % 100) / 100.0;
    
    /* Main loop with loop-carried dependencies */
    for (int i = 0; i < N; i++) {
        /* Create dense web of data dependencies */
        v1 = v2 + v3;
        v2 = v1 - v4;
        v3 = v2 * v5;
        v4 = v3 / (v6 + 1);
        v5 = v4 ^ v7;
        v6 = v5 | v8;
        v7 = v6 & v9;
        v8 = v7 + v10;
        v9 = v8 - v11;
        v10 = v9 * v12;
        v11 = v10 / (v13 + 1);
        v12 = v11 ^ v14;
        v13 = v12 | v15;
        v14 = v13 & v1;  /* Circular dependency */
        v15 = v14 + v2;
        
        /* Float operations */
        f1 = f2 * 1.5f;
        f2 = f3 + f4;
        f3 = f1 - f2;
        f4 = f3 * 2.0f;
        
        /* Double operations */
        d1 = d2 / 1.3;
        d2 = d3 * 1.7;
        d3 = d1 + d2;
        
        /* Long operations */
        l1 = l2 << 2;
        l2 = l3 >> 1;
        l3 = l1 | l2;
        
        /* 
         * CRITICAL: Function call with many live values.
         * v1-v5 are used as arguments, but v6-v15 remain live
         * in call-clobbered registers across the call.
         */
        int ret1 = helper1(v1, v2);
        
        /* 
         * Immediately use return value with other live variables
         * that were NOT passed to the function (v6, v7, etc.)
         * This creates the precise scenario where save/restore
         * must be inserted between the call and this instruction.
         */
        v6 = ret1 + v6 + v7;  /* v6 and v7 live across call */
        
        /* Artificial use to prevent optimization */
        asm volatile("" : "+r"(v6), "+r"(v7));
        
        /* Another function call with different arguments */
        int ret2 = helper2(v3, v4, v5);
        
        /* Again use return value with variables live across call */
        v8 = ret2 * v8 - v9;  /* v8 and v9 live across call */
        
        /* More operations creating register pressure */
        v10 = helper1(v10, v11);
        v12 = v10 + v12 - v13;
        
        /* Float function call */
        float fret = helper3(f1, f2);
        f3 = fret + f3 * 1.1f;
        
        /* Double function call */
        double dret = helper4(d1, d2);
        d3 = dret - d3 / 2.0;
        
        /* Long function call */
        long lret = helper5(l1, l2);
        l3 = lret ^ l3;
        
        #ifdef __i386__
        /* x86-specific: regparm increases register pressure */
        int rret = helper_regparm(v14, v15, v1);
        v2 = rret + v2;
        #endif
        
        /* Volatile operation to inhibit reordering */
        v_volatile = i;
        
        /* More complex dependency chain */
        v1 = v1 + v_volatile;
        v3 = v3 - v_volatile;
        v5 = v5 ^ v_volatile;
        
        /* Use all variables to keep them live */
        f4 = f4 + (v_volatile % 100) / 100.0f;
        d1 = d1 + (v_volatile % 100) / 100.0;
        l1 = l1 + v_volatile;
    }
    
    /* Compute checksum to prevent dead code elimination */
    long checksum = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                   v11 + v12 + v13 + v14 + v15 +
                   (long)f1 + (long)f2 + (long)f3 + (long)f4 +
                   (long)d1 + (long)d2 + (long)d3 +
                   l1 + l2 + l3;
    
    printf("Checksum: %ld\n", checksum);
    return (checksum > 0) ? 0 : 1;
}
