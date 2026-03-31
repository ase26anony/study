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
    return a * 2.0 - b;
}

long __attribute__((noinline)) helper5(long a, long b) {
    return (a << 3) | (b & 0xFF);
}

/* x86-specific regparm calling convention */
#ifdef __i386__
int __attribute__((regparm(3), noinline)) helper_regparm(int a, int b, int c) {
    return (a + b) * c;
}
#else
int __attribute__((noinline)) helper_regparm(int a, int b, int c) {
    return (a + b) * c;
}
#endif

/* Force register pressure by using many live variables across calls */
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
    float f2 = (float)(rand() % 100) * 0.1f;
    float f3 = (float)(rand() % 100) * 0.1f;
    double d1 = (double)(rand() % 100) * 0.01;
    double d2 = (double)(rand() % 100) * 0.01;
    long l1 = (long)rand() * rand();
    long l2 = (long)rand() * rand();
    
    /* Volatile variables to prevent optimization */
    volatile int v_vol1 = v1;
    volatile float f_vol1 = f1;
    
    for (int i = 0; i < N; i++) {
        /* Create dense web of data dependencies */
        v1 = v2 + v3;
        v2 = v1 - v4;
        v3 = v5 * v6;
        v4 = v7 ^ v8;
        v5 = v9 + v10;
        v6 = v11 - v12;
        v7 = v13 * v14;
        v8 = v15 ^ v1;
        
        /* Mix in float operations */
        f1 = f2 * 1.1f;
        f2 = f3 + 0.5f;
        f3 = f1 * 0.9f;
        
        /* Double operations */
        d1 = d2 * 1.01;
        d2 = d1 - 0.5;
        
        /* Long operations */
        l1 = l2 << 2;
        l2 = l1 >> 1;
        
        /* CRITICAL: Function call with some live variables as arguments,
           leaving others live in call-clobbered registers */
        int ret1 = helper1(v1, v2);
        
        /* Immediately use return value with variables that were live
           across the call (v3, v4, v5, etc.) - this creates the need
           for caller-save insertion between the call and this use */
        v9 = ret1 + v3 + v4;  /* v3, v4 were live across helper1 call */
        
        /* Artificial use to force register allocation */
        asm volatile("" : "+r"(v5), "+r"(v6));
        
        /* Another function call */
        int ret2 = helper2(v5, v6, v7);
        
        /* Use return value with more live variables */
        v10 = ret2 * v8 - v9;
        
        /* Float function call */
        float fret1 = helper3(f1, f2);
        
        /* Use float return with live float variable */
        f3 = fret1 + f3 * 0.8f;
        
        /* Double function call */
        double dret1 = helper4(d1, d2);
        
        /* Use double return */
        d2 = dret1 * 0.99;
        
        /* Long function call */
        long lret1 = helper5(l1, l2);
        
        /* Use long return */
        l1 = lret1 + 1;
        
        /* Function with regparm calling convention (x86) */
        int ret3 = helper_regparm(v10, v11, v12);
        
        /* Use return value with many live variables */
        v13 = ret3 + v14 + v15 + v1;
        
        /* More arithmetic to keep all variables live */
        v14 = v13 * 2;
        v15 = v14 / 3;
        v11 = v15 + v10;
        v12 = v11 - v9;
        
        /* Volatile accesses to prevent reordering */
        v_vol1 = v1;
        f_vol1 = f1;
        
        /* Create loop-carried dependencies */
        v1 = v1 + i;
        v2 = v2 - i;
        v3 = v3 * (i % 10 + 1);
    }
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                   v11 + v12 + v13 + v14 + v15 +
                   (int)f1 + (int)f2 + (int)f3 +
                   (int)d1 + (int)d2 +
                   (int)(l1 & 0xFFFFFFFF) + (int)(l2 & 0xFFFFFFFF);
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
