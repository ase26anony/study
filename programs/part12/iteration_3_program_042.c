/* caller-save-test.c */
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
    return a * 0.5f + b * 1.5f;
}

double __attribute__((noinline)) helper4(double a, double b) {
    return a * 2.0 + b * 0.5;
}

long __attribute__((noinline)) helper5(long a, long b) {
    return (a << 3) | (b & 0xFF);
}

/* x86-specific: force use of specific registers */
#ifdef __x86_64__
int __attribute__((regparm(3), noinline)) helper_regparm(int a, int b, int c) {
    return a * b + c;
}
#endif

/* Force register pressure by preventing optimization */
#define PRESSURE(var) asm volatile("" : "+r"(var))

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
    
    /* Use volatile to prevent optimization around calls */
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
        f4 = f1 * f2 - f3;
        
        /* Double operations */
        d1 = d2 * 1.75 + (double)v3;
        d2 = d3 / 2.0 + d1;
        d3 = d1 * d2 - 3.14159;
        
        /* Long operations */
        l1 = l2 << 2 | (l3 & 0xFFF);
        l2 = l3 + l1 * 3;
        l3 = l1 ^ l2;
        
        /* More integer dependencies */
        v6 = v7 * v8 + v9;
        v7 = v6 - v10 * 2;
        v8 = v7 ^ v11;
        v9 = v8 + v12;
        v10 = v9 * v13 - v14;
        
        v11 = v10 + v15;
        v12 = v11 * v1;
        v13 = v12 ^ v2;
        v14 = v13 + v3;
        v15 = v14 * v4 - v5;
        
        /* Create artificial register pressure */
        PRESSURE(v1);
        PRESSURE(v2);
        PRESSURE(f1);
        PRESSURE(d1);
        PRESSURE(l1);
        
        /* 
         * CRITICAL: Function call followed immediately by use of 
         * values that were live in call-clobbered registers
         */
        int result1 = helper1(v1, v2);
        
        /* 
         * v3, v4, f1, d1, l1 were live across the call.
         * This use forces them to be in call-clobbered registers
         * that need saving/restoring.
         */
        v3 = v3 + result1;  /* Uses v3 which was live across call */
        v4 = v4 * result1;  /* Uses v4 which was live across call */
        
        /* More pressure */
        PRESSURE(v3);
        PRESSURE(v4);
        
        /* Another call with different arguments */
        int result2 = helper2(v5, v6, v7);
        
        /* Immediate use of values live across this call */
        v8 = v8 + result2 + v_volatile;
        v9 = v9 * result2;
        
        /* Float call */
        float fresult = helper3(f1, f2);
        
        /* Use float values that were live across call */
        f3 = f3 + fresult;
        f4 = f4 * fresult;
        
        /* Double call */
        double dresult = helper4(d1, d2);
        
        /* Use double values live across call */
        d3 = d3 + dresult;
        
        /* Long call */
        long lresult = helper5(l1, l2);
        
        /* Use long values live across call */
        l3 = l3 ^ lresult;
        
        #ifdef __x86_64__
        /* x86-specific: regparm calling convention */
        int regresult = helper_regparm(v10, v11, v12);
        v13 = v13 + regresult;
        #endif
        
        /* More complex dependency chain */
        v14 = helper1(v13, v14);
        v15 = helper2(v15, v14, v13);
        
        /* Volatile access to prevent reordering */
        v_volatile = i;
    }
    
    /* Compute checksum to prevent dead code elimination */
    long checksum = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                    v11 + v12 + v13 + v14 + v15 +
                    (long)f1 + (long)f2 + (long)f3 + (long)f4 +
                    (long)d1 + (long)d2 + (long)d3 +
                    l1 + l2 + l3;
    
    printf("Checksum: %ld\n", checksum);
    return 0;
}
