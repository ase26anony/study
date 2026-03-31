/* caller-save-trigger.c */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Non-inlineable helper functions with different calling conventions */
int __attribute__((noinline)) helper1(int a, int b) {
    return a ^ b;
}

int __attribute__((noinline)) helper2(int a, int b, int c) {
    return a + b - c;
}

float __attribute__((noinline)) helper3(float a, float b) {
    return a * b;
}

double __attribute__((noinline)) helper4(double a, double b) {
    return a / (b + 1.0);
}

long __attribute__((noinline)) helper5(long a, long b) {
    return a & ~b;
}

/* x86-specific: use regparm to force register argument passing */
#ifdef __i386__
int __attribute__((regparm(3), noinline)) helper_regparm(int a, int b, int c) {
    return (a * b) | c;
}
#endif

int main(int argc, char *argv[]) {
    int N = (argc > 1) ? atoi(argv[1]) : 1000;
    srand(time(NULL));
    
    /* Declare many scalar variables of mixed types */
    volatile int v1 = rand() % 100;
    int v2 = rand() % 100;
    volatile int v3 = rand() % 100;
    int v4 = rand() % 100;
    volatile float f1 = (float)(rand() % 100) / 10.0f;
    float f2 = (float)(rand() % 100) / 10.0f;
    volatile double d1 = (double)(rand() % 100) / 10.0;
    double d2 = (double)(rand() % 100) / 10.0;
    volatile long l1 = rand() % 100;
    long l2 = rand() % 100;
    int v5 = rand() % 100;
    volatile int v6 = rand() % 100;
    int v7 = rand() % 100;
    volatile int v8 = rand() % 100;
    float f3 = (float)(rand() % 100) / 10.0f;
    volatile float f4 = (float)(rand() % 100) / 10.0f;
    double d3 = (double)(rand() % 100) / 10.0;
    volatile double d4 = (double)(rand() % 100) / 10.0;
    long l3 = rand() % 100;
    volatile long l4 = rand() % 100;
    int v9 = rand() % 100;
    volatile int v10 = rand() % 100;
    
    /* Loop with non-constant trip count */
    for (int i = 0; i < N; i++) {
        /* Create dense web of data dependencies */
        v1 = v2 + v3;
        v4 = v1 * v5;
        v6 = v4 ^ v7;
        v8 = v6 - v9;
        v10 = v8 | v3;
        
        f1 = f2 * 1.1f;
        f2 = f1 + f3;
        f3 = f4 - 0.5f;
        f4 = f3 * f2;
        
        d1 = d2 / 1.01;
        d2 = d1 + d3;
        d3 = d4 * 0.99;
        d4 = d3 - d2;
        
        l1 = l2 << 1;
        l2 = l1 | l3;
        l3 = l4 ^ 0xAA;
        l4 = l3 & l2;
        
        /* CRITICAL: Function call followed immediately by use of 
           live variables in call-clobbered registers */
        int ret1 = helper1(v1, v2);
        
        /* This use of v3 (which was live across the call) forces
           caller-save insertion between the call and this instruction */
        v3 = ret1 + v3;  /* v3 is live across helper1 call */
        
        /* Insert artificial register pressure */
        asm volatile("" : "+r"(v3), "+r"(v4));
        
        /* Another call creating multiple insertion points */
        float ret2 = helper3(f1, f2);
        
        /* Use f3 which was live across helper3 call */
        f3 = ret2 - f3;  /* f3 is live across helper3 call */
        
        asm volatile("" : "+r"(f3), "+r"(f4));
        
        /* Mix of calls with different signatures */
        double ret3 = helper4(d1, d2);
        d3 = ret3 * d3;  /* d3 is live across helper4 call */
        
        long ret4 = helper5(l1, l2);
        l3 = ret4 ^ l3;  /* l3 is live across helper5 call */
        
        /* Call with many arguments to increase register pressure */
        int ret5 = helper2(v5, v6, v7);
        v8 = ret5 + v8;  /* v8 is live across helper2 call */
        
        #ifdef __i386__
        int ret6 = helper_regparm(v9, v10, v1);
        v2 = ret6 - v2;  /* v2 is live across helper_regparm call */
        #endif
        
        /* Loop-carried dependencies */
        v5 = v5 + i;
        v7 = v7 ^ i;
        v9 = v9 - i;
        
        f2 = f2 + (float)i * 0.1f;
        d2 = d2 + (double)i * 0.01;
        l2 = l2 + i;
    }
    
    /* Compute checksum to prevent dead code elimination */
    long checksum = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10
                  + (long)f1 + (long)f2 + (long)f3 + (long)f4
                  + (long)d1 + (long)d2 + (long)d3 + (long)d4
                  + l1 + l2 + l3 + l4;
    
    printf("Checksum: %ld\n", checksum);
    return 0;
}
