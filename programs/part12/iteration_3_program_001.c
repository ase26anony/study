/* caller-save-trigger.c */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Non-inlineable helper functions with different calling conventions */
int __attribute__((noinline)) helper1(int a, int b) {
    return a ^ b;
}

int __attribute__((noinline, regparm(3))) helper2(int a, int b, int c) {
    return (a + b) * c;
}

float __attribute__((noinline)) helper3(float a, float b) {
    return a - b;
}

double __attribute__((noinline)) helper4(double a, double b) {
    return a / (b + 1.0);
}

long __attribute__((noinline)) helper5(long a, long b) {
    return a % (b | 1);
}

/* Another set of helpers to increase call density */
int __attribute__((noinline)) helper6(int a, int b, int c, int d) {
    return (a * b) + (c - d);
}

float __attribute__((noinline)) helper7(float a, float b, float c) {
    return a * b + c;
}

/* Force register pressure with mixed types */
int main(int argc, char *argv[]) {
    int N = (argc > 1) ? atoi(argv[1]) : 1000;
    volatile int seed = (int)time(NULL);
    srand(seed);
    
    /* Declare many scalar variables with mixed types */
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
    float f5 = (float)(rand() % 100) / 10.0f;
    
    double d1 = (double)(rand() % 100) / 5.0;
    double d2 = (double)(rand() % 100) / 5.0;
    double d3 = (double)(rand() % 100) / 5.0;
    
    long l1 = rand() % 100;
    long l2 = rand() % 100;
    long l3 = rand() % 100;
    long l4 = rand() % 100;
    
    int i;
    for (i = 0; i < N; i++) {
        /* Create dense web of data dependencies */
        v1 = v2 + v3;
        v4 = v1 * v5;
        v6 = v4 - v7;
        v8 = v6 ^ v9;
        v10 = v8 | v2;
        
        f1 = f2 * 1.5f;
        f3 = f1 + f4;
        f5 = f3 - f2;
        
        d1 = d2 * 2.0;
        d3 = d1 / 1.7;
        
        l1 = l2 + 3;
        l3 = l1 * 2;
        l4 = l3 ^ l2;
        
        /* CRITICAL: Call with some live variables, leaving others live across call */
        /* v1, v2, v3 are used in call, but v4, v5, v6 remain live in call-clobbered regs */
        int ret1 = helper1(v1, v2);
        
        /* Immediately use return value with variables that were live across the call */
        /* This forces save/restore insertion BETWEEN the call and this instruction */
        v4 = ret1 + v4;  /* v4 was live in call-clobbered register across helper1 call */
        
        /* Artificial use to prevent optimization */
        asm volatile("" : "+r"(v5), "+r"(v6));
        
        /* Another call with different convention */
        int ret2 = helper2(v3, v4, v5);
        
        /* Use return value with other live variables */
        v7 = ret2 * v6;  /* v6 was live across helper2 call */
        
        /* Float/double calls */
        float fret1 = helper3(f1, f2);
        f4 = fret1 + f3;  /* f3 was live across helper3 call */
        
        double dret1 = helper4(d1, d2);
        d3 = dret1 * d3;  /* d3 was live across helper4 call */
        
        /* Long call */
        long lret1 = helper5(l1, l2);
        l4 = lret1 + l3;  /* l3 was live across helper5 call */
        
        /* More complex call with many args */
        int ret3 = helper6(v8, v9, v10, v1);
        v2 = ret3 - v7;  /* v7 was live across helper6 call */
        
        /* Float call with multiple args */
        float fret2 = helper7(f4, f5, f1);
        f2 = fret2 * f3;  /* f3 was live across helper7 call */
        
        /* Create loop-carried dependencies */
        v3 = v2 + i;
        v5 = v4 * (i + 1);
        v9 = v8 ^ i;
        
        f2 = f1 + (float)i;
        d2 = d1 * (double)i;
        l2 = l1 + i;
        
        /* More artificial register pressure */
        asm volatile("" : "+r"(v10), "+r"(f5), "+r"(d3), "+r"(l4));
    }
    
    /* Compute checksum to prevent elimination */
    long checksum = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10
                  + (long)f1 + (long)f2 + (long)f3 + (long)f4 + (long)f5
                  + (long)d1 + (long)d2 + (long)d3
                  + l1 + l2 + l3 + l4;
    
    printf("Checksum: %ld\n", checksum);
    return 0;
}
