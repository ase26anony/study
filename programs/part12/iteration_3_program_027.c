/* caller_save_test.c */
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

/* x86-specific regparm calling convention */
#ifdef __i386__
int __attribute__((regparm(3), noinline)) helper_regparm(int a, int b, int c) {
    return a + b * c;
}
#else
int __attribute__((noinline)) helper_regparm(int a, int b, int c) {
    return a + b * c;
}
#endif

int main(int argc, char *argv[]) {
    int N = (argc > 1) ? atoi(argv[1]) : 1000;
    if (N <= 0) N = 1000;
    
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
    
    float f1 = (float)(rand() % 100) * 0.1f;
    float f2 = (float)(rand() % 100) * 0.2f;
    float f3 = (float)(rand() % 100) * 0.3f;
    float f4 = (float)(rand() % 100) * 0.4f;
    
    double d1 = (double)(rand() % 100) * 0.01;
    double d2 = (double)(rand() % 100) * 0.02;
    double d3 = (double)(rand() % 100) * 0.03;
    
    long l1 = (long)rand() * rand();
    long l2 = (long)rand() * rand();
    long l3 = (long)rand() * rand();
    
    /* Mark some as volatile to prevent optimization */
    volatile int v_volatile = rand() % 50;
    volatile float f_volatile = (float)(rand() % 50) * 0.5f;
    
    for (int i = 0; i < N; i++) {
        /* Create dense web of data dependencies */
        v1 = v2 + v3;
        v4 = v1 * v5;
        v6 = v4 - v7;
        v8 = v6 ^ v9;
        v10 = v8 | v2;
        
        f1 = f2 * 1.5f + f3;
        f2 = f1 * 0.8f - f4;
        f3 = f2 + f4 * 2.0f;
        f4 = f3 * 0.3f;
        
        d1 = d2 * 1.7 + d3;
        d2 = d1 * 0.6 - d3;
        d3 = d2 * 2.1 + d1;
        
        l1 = l2 << 2;
        l2 = l3 >> 1;
        l3 = l1 | l2;
        
        /* CRITICAL: Function call with some live variables as arguments,
           followed immediately by use of other live variables */
        int ret1 = helper1(v1, v2);
        /* v3, v4, v5 are live across the call and used immediately after */
        v3 = v3 + ret1;  /* Uses v3 which was live across helper1 call */
        v4 = v4 * ret1;  /* Uses v4 which was live across helper1 call */
        
        /* Use asm volatile to create artificial register pressure */
        asm volatile("" : "+r"(v5), "+r"(v6));
        
        /* Another call with different arguments */
        int ret2 = helper2(v3, v4, v5);
        /* f1, f2 are live across this call */
        f1 = f1 + (float)ret2 * 0.1f;
        f2 = f2 - (float)ret2 * 0.2f;
        
        /* Float function call */
        float fret = helper3(f1, f2);
        /* d1, d2 are live across this call */
        d1 = d1 + (double)fret;
        d2 = d2 - (double)fret * 0.5;
        
        /* Double function call */
        double dret = helper4(d1, d2);
        /* l1, l2 are live across this call */
        l1 = l1 + (long)dret;
        l2 = l2 ^ (long)dret;
        
        /* Long function call */
        long lret = helper5(l1, l2);
        /* v6, v7 are live across this call */
        v6 = v6 + (int)lret;
        v7 = v7 - (int)lret;
        
        /* Regparm function call (x86 specific) */
        int rret = helper_regparm(v6, v7, v8);
        /* v9, v10 are live across this call */
        v9 = v9 ^ rret;
        v10 = v10 & rret;
        
        /* Use volatile variables to prevent reordering */
        v_volatile = v_volatile + v1;
        f_volatile = f_volatile + f1;
        
        /* More complex dependency chain */
        v1 = v10 + v9;
        v2 = v1 * v8;
        v3 = v2 - v7;
        v4 = v3 ^ v6;
        v5 = v4 | v5;
        
        /* Another call in the middle of computations */
        ret1 = helper1(v5, v6);
        /* v7, v8 must be saved/restored around this call */
        v7 = v7 + ret1;
        v8 = v8 * ret1;
        
        /* Final mixing */
        v9 = (v7 << 3) | (v8 & 0xFF);
        v10 = (v9 ^ v10) + i;
    }
    
    /* Compute checksum to prevent dead code elimination */
    long checksum = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10
                  + (long)f1 + (long)f2 + (long)f3 + (long)f4
                  + (long)d1 + (long)d2 + (long)d3
                  + l1 + l2 + l3
                  + v_volatile + (long)f_volatile;
    
    printf("Checksum: %ld\n", checksum);
    return 0;
}
