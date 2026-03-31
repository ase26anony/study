/* caller-save-test.c */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Non-inlineable helper functions with different calling conventions */
int __attribute__((noinline)) helper1(int a, int b) { return a ^ b; }
int __attribute__((noinline)) helper2(int a, int b) { return a + b * 3; }
float __attribute__((noinline)) helper3(float a, float b) { return a - b * 0.5f; }
double __attribute__((noinline)) helper4(double a, double b) { return a / (b + 1.0); }
long __attribute__((noinline)) helper5(long a, long b) { return a % (b | 1); }

/* x86-specific: use regparm to force register passing */
#ifdef __i386__
int __attribute__((regparm(3), noinline)) helper_regparm(int a, int b, int c) {
    return a + b - c;
}
#endif

/* Force register pressure by creating many live values */
int main(int argc, char **argv) {
    int N = (argc > 1) ? atoi(argv[1]) : 1000;
    volatile int seed = time(NULL);
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
    
    float f1 = (float)(rand() % 100) / 10.0f;
    float f2 = (float)(rand() % 100) / 10.0f;
    float f3 = (float)(rand() % 100) / 10.0f;
    float f4 = (float)(rand() % 100) / 10.0f;
    
    double d1 = (double)(rand() % 100) / 5.0;
    double d2 = (double)(rand() % 100) / 5.0;
    double d3 = (double)(rand() % 100) / 5.0;
    
    long l1 = rand() % 100;
    long l2 = rand() % 100;
    long l3 = rand() % 100;
    long l4 = rand() % 100;
    
    /* Create artificial register pressure with inline asm */
    asm volatile("" : "+r"(v1), "+r"(v2), "+r"(v3));
    
    for (int i = 0; i < N; i++) {
        /* Create dense web of data dependencies */
        v1 = v2 + v3;
        v4 = v1 * v5;
        v6 = v4 - v7;
        v8 = v6 ^ v9;
        v10 = v8 | v1;
        
        f1 = f2 * 1.5f;
        f3 = f1 + f4;
        f2 = f3 - f1;
        f4 = f2 / 2.0f;
        
        d1 = d2 * 1.7;
        d3 = d1 + d2;
        d2 = d3 - d1;
        
        l1 = l2 << 2;
        l3 = l1 | l4;
        l2 = l3 ^ l1;
        l4 = l2 + 1;
        
        /* CRITICAL: Call with some live values, keeping others live across call */
        int ret1 = helper1(v1, v2);
        /* v3, v4, v5, v6 remain live across the call in call-clobbered registers */
        v3 = ret1 + v4;  /* Uses v4 which was live across call */
        
        /* Another call immediately after */
        float ret2 = helper3(f1, f2);
        /* f3, f4 remain live across this call */
        f3 = ret2 * f4;  /* Uses f4 which was live across call */
        
        /* Mix types to pressure different register classes */
        double ret3 = helper4(d1, d2);
        d3 = ret3 + d1;  /* d1 was live across call */
        
        /* Use inline asm to create artificial uses preventing optimization */
        asm volatile("" : "+r"(v5), "+r"(v6), "+r"(v7));
        
        /* Another call sequence */
        long ret4 = helper5(l1, l2);
        l3 = ret4 + l4;  /* l4 was live across call */
        
        /* Call with many arguments to pressure argument registers */
        int ret5 = helper2(v8, v9);
        v10 = ret5 + v3;  /* v3 was live across call */
        
        /* Create loop-carried dependencies */
        v2 = v10 + i;
        v5 = v3 * i;
        v7 = v6 - i;
        v9 = v8 ^ i;
        
        f2 = f3 + (float)i;
        d2 = d3 * (double)i;
        l2 = l3 + (long)i;
        
        /* Prevent optimization of the entire computation */
        asm volatile("" : : "r"(v1), "r"(f1), "r"(d1), "r"(l1));
    }
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    checksum += (int)f1 + (int)f2 + (int)f3 + (int)f4;
    checksum += (int)d1 + (int)d2 + (int)d3;
    checksum += (int)l1 + (int)l2 + (int)l3 + (int)l4;
    
    printf("Result: %d (seed: %d)\n", checksum, seed);
    return checksum != 0;
}
