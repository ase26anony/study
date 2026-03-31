/* caller-save-trigger.c */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Non-inlineable helper functions with different calling conventions */
#ifdef __x86_64__
#define REGPARM3 __attribute__((regparm(3)))
#define REGPARM2 __attribute__((regparm(2)))
#else
#define REGPARM3
#define REGPARM2
#endif

/* Helper functions declared in separate compilation unit or noinline */
int helper1(int a, int b) __attribute__((noinline));
int helper2(int a, int b) __attribute__((noinline));
float helper3(float a, float b) __attribute__((noinline));
double helper4(double a, double b) __attribute__((noinline));
long helper5(long a, long b) __attribute__((noinline, REGPARM3));
int helper6(int a, int b, int c) __attribute__((noinline, REGPARM2));

/* Implementations (would normally be in separate file) */
int helper1(int a, int b) {
    asm volatile("" : : "r"(a), "r"(b));
    return a ^ b;
}

int helper2(int a, int b) {
    asm volatile("" : : "r"(a), "r"(b));
    return a + b * 3;
}

float helper3(float a, float b) {
    asm volatile("" : : "r"(a), "r"(b));
    return a * 0.5f + b;
}

double helper4(double a, double b) {
    asm volatile("" : : "r"(a), "r"(b));
    return a * 2.0 + b * 0.75;
}

long helper5(long a, long b) {
    asm volatile("" : : "r"(a), "r"(b));
    return (a << 3) | (b & 0xFF);
}

int helper6(int a, int b, int c) {
    asm volatile("" : : "r"(a), "r"(b), "r"(c));
    return a * b - c;
}

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
    float f1 = (float)(rand() % 100) * 0.1f;
    float f2 = (float)(rand() % 100) * 0.1f;
    float f3 = (float)(rand() % 100) * 0.1f;
    float f4 = (float)(rand() % 100) * 0.1f;
    double d1 = (double)(rand() % 100) * 0.01;
    double d2 = (double)(rand() % 100) * 0.01;
    double d3 = (double)(rand() % 100) * 0.01;
    long l1 = (long)rand() * rand();
    long l2 = (long)rand() * rand();
    long l3 = (long)rand() * rand();
    
    /* Volatile variables to inhibit optimizations */
    volatile int v_vol1 = 0;
    volatile float f_vol1 = 0.0f;
    volatile double d_vol1 = 0.0;
    
    /* Main loop with loop-carried dependencies */
    for (int i = 0; i < N; i++) {
        /* Create dense web of data dependencies */
        v1 = v2 + v3;
        v2 = v1 * v4 - v5;
        v3 = v6 ^ v7;
        v4 = v8 | v9;
        v5 = v10 & v1;
        
        f1 = f2 * 1.5f + f3;
        f2 = f4 - f1 * 0.3f;
        f3 = f1 + f2;
        
        d1 = d2 * 2.5 + d3;
        d2 = d1 * 0.8 - d3;
        
        l1 = l2 << 2;
        l2 = l3 >> 1;
        l3 = l1 | l2;
        
        /* CRITICAL: Function call followed immediately by use of 
           variables that were live in call-clobbered registers */
        
        /* Call 1: Pass some variables, others remain live */
        int r1 = helper1(v1, v2);
        
        /* IMMEDIATE use of variables that were live across the call */
        v6 = v3 + r1;      /* v3 was live in call-clobbered register */
        v7 = v4 * r1;      /* v4 was live in call-clobbered register */
        
        /* Artificial use to force register allocation */
        asm volatile("" : "+r"(v3), "+r"(v4));
        
        /* Call 2: Different calling convention */
        long r2 = helper5(l1, l2);
        
        /* IMMEDIATE use of live variables */
        l3 = l3 + r2;      /* l3 was live */
        v8 = (int)(l3 & 0xFF);
        
        /* Call 3: Float operation */
        float r3 = helper3(f1, f2);
        
        /* IMMEDIATE use with dependency chain */
        f4 = f3 + r3;      /* f3 was live */
        d3 = (double)f4 * 0.25;
        
        /* Call 4: Double with more arguments */
        double r4 = helper4(d1, d2);
        
        /* Complex immediate use creating pressure */
        d1 = d3 + r4 * 2.0;
        v9 = (int)d1 + v5;  /* v5 was live */
        
        /* Call 5: Three arguments */
        int r5 = helper6(v6, v7, v8);
        
        /* More immediate uses */
        v10 = v9 + r5;
        v_vol1 = v10;      /* Volatile store */
        
        /* Call 6: Mixed use */
        int r6 = helper2(v10, v1);
        
        /* Final dependency to keep everything alive */
        v1 = v2 + v3 + v4 + v5 + r6;
        f1 = f2 + f3 + f4 + (float)r6;
        d2 = d1 + d3 + (double)r6;
        l1 = l2 + l3 + r6;
        
        /* Update volatile variables to prevent optimization */
        f_vol1 = f1;
        d_vol1 = d2;
        
        /* Create loop-carried dependencies for next iteration */
        v2 += i;
        v3 ^= i;
        f2 += (float)i * 0.1f;
        d3 += (double)i * 0.01;
        l2 += i;
    }
    
    /* Compute checksum to prevent dead code elimination */
    long checksum = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10
                  + (long)f1 + (long)f2 + (long)f3 + (long)f4
                  + (long)d1 + (long)d2 + (long)d3
                  + l1 + l2 + l3
                  + v_vol1 + (long)f_vol1 + (long)d_vol1;
    
    printf("Checksum: %ld\n", checksum);
    return (int)(checksum % 100);
}
