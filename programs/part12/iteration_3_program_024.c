/* caller-save-test.c */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Non-inlineable helper functions with different calling conventions */
int __attribute__((noinline)) helper1(int a, int b) {
    return a ^ b;
}

int __attribute__((noinline)) helper2(int a, int b, int c) {
    return (a + b) * c;
}

float __attribute__((noinline)) helper3(float a, float b) {
    return a * b - 0.5f;
}

double __attribute__((noinline)) helper4(double a, double b) {
    return a / (b + 1.0);
}

long __attribute__((noinline)) helper5(long a, long b) {
    return (a << 3) | (b & 0xFF);
}

/* x86-specific regparm calling convention */
#ifdef __i386__
int __attribute__((regparm(3), noinline)) helper_regparm(int a, int b, int c) {
    return a + b - c;
}
#endif

/* Force register pressure with mixed types and dependencies */
int main(int argc, char *argv[]) {
    int N = (argc > 1) ? atoi(argv[1]) : 1000;
    volatile int seed = (int)time(NULL);
    srand(seed);
    
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
    
    float f1 = (float)rand() / RAND_MAX;
    float f2 = (float)rand() / RAND_MAX;
    float f3 = (float)rand() / RAND_MAX;
    float f4 = (float)rand() / RAND_MAX;
    
    double d1 = (double)rand() / RAND_MAX;
    double d2 = (double)rand() / RAND_MAX;
    double d3 = (double)rand() / RAND_MAX;
    
    long l1 = rand() % 1000;
    long l2 = rand() % 1000;
    long l3 = rand() % 1000;
    long l4 = rand() % 1000;
    
    /* Artificial use to prevent optimization */
    asm volatile("" : "+r"(v1), "+r"(v2), "+r"(v3));
    
    for (int i = 0; i < N; i++) {
        /* Create dense web of data dependencies */
        v1 = v2 + v3;
        v4 = v1 * v5;
        v6 = v4 ^ v7;
        v8 = v6 - v9;
        v10 = v8 | v1;
        
        f1 = f2 * 1.1f;
        f3 = f1 + f4;
        f2 = f3 - 0.25f;
        f4 = f2 / 2.0f;
        
        d1 = d2 * 1.01;
        d3 = d1 + d2;
        d2 = d3 * 0.99;
        
        l1 = l2 << 2;
        l3 = l1 | l4;
        l2 = l3 ^ 0xABCD;
        l4 = l2 + 1;
        
        /* CRITICAL: Function call with some live variables,
           followed immediately by use of other live variables */
        int ret1 = helper1(v1, v2);
        
        /* v3, v4, f1, d1 are live across the call in call-clobbered regs */
        v3 = ret1 + v4;  /* Uses v4 which was live across call */
        
        /* Insert artificial barrier to prevent reordering */
        asm volatile("" : "+r"(v3), "+r"(v4));
        
        /* Another call creating insertion point */
        float ret2 = helper3(f1, f2);
        
        /* f3, d1 are live across this call */
        f3 = ret2 * d1;  /* Uses d1 which was live across call */
        
        /* More dependencies and calls */
        int ret3 = helper2(v5, v6, v7);
        v8 = ret3 + v9 + v10;
        
        double ret4 = helper4(d2, d3);
        d1 = ret4 * 2.0;
        
        long ret5 = helper5(l1, l2);
        l3 = ret5 ^ l4;
        
        #ifdef __i386__
        int ret6 = helper_regparm(v1, v2, v3);
        v4 = ret6 * v5;
        #endif
        
        /* Create loop-carried dependencies */
        v1 = v1 + i;
        v2 = v2 ^ i;
        f1 = f1 + (float)i * 0.01f;
        d1 = d1 + (double)i * 0.001;
        l1 = l1 + i;
        
        /* More artificial uses to maintain live ranges */
        asm volatile("" : "+r"(v5), "+r"(v6), "+r"(v7));
        asm volatile("" : "+r"(f2), "+r"(f4));
        asm volatile("" : "+r"(d2), "+r"(d3));
        asm volatile("" : "+r"(l2), "+r"(l4));
    }
    
    /* Compute checksum to prevent dead code elimination */
    long checksum = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10
                  + (long)(f1 * 1000) + (long)(f2 * 1000)
                  + (long)(f3 * 1000) + (long)(f4 * 1000)
                  + (long)(d1 * 1000) + (long)(d2 * 1000)
                  + (long)(d3 * 1000) + l1 + l2 + l3 + l4;
    
    printf("Checksum: %ld\n", checksum);
    return 0;
}
