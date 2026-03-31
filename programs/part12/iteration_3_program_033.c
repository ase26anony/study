/* caller-save-trigger.c
 * Compile with: gcc -O2 -fno-omit-frame-pointer -fno-inline -fno-tree-loop-optimize caller-save-trigger.c -o caller-save-test
 * For RTL dumps: gcc -O3 -fno-schedule-insns -fdump-rtl-all caller-save-trigger.c
 */

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

/* Prevent inlining and force actual calls */
__attribute__((noinline)) int helper1(int a, int b) {
    return (a ^ b) + 1;
}

__attribute__((noinline)) REGPARM3 int helper2(int a, int b, int c) {
    return (a * b) - c;
}

__attribute__((noinline)) REGPARM2 float helper3(float a, float b) {
    return a * 0.5f + b * 0.5f;
}

__attribute__((noinline)) double helper4(double a, double b) {
    return a + b * 0.333;
}

__attribute__((noinline)) long helper5(long a, long b) {
    return (a << 3) | (b & 0xFF);
}

__attribute__((noinline)) int helper6(int a, int b, int c, int d) {
    return (a + b) * (c - d);
}

/* External function to prevent interprocedural analysis */
extern int external_func(int) __attribute__((noinline));

int external_func(int x) {
    return x * 7 + 3;
}

int main(int argc, char *argv[]) {
    /* Non-constant loop bound to prevent unrolling */
    int N = (argc > 1) ? atoi(argv[1]) : 1000;
    if (N <= 0) N = 1000;
    
    /* Seed RNG for variable initialization */
    srand(time(NULL));
    
    /* Declare many scalar variables of mixed types to create register pressure */
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
    
    double d1 = (double)(rand() % 100) / 3.0;
    double d2 = (double)(rand() % 100) / 3.0;
    double d3 = (double)(rand() % 100) / 3.0;
    
    long l1 = (long)rand() * rand();
    long l2 = (long)rand() * rand();
    long l3 = (long)rand() * rand();
    
    /* Volatile variables to prevent optimization */
    volatile int v_volatile = rand() % 50;
    volatile float f_volatile = (float)(rand() % 50) / 5.0f;
    
    /* Main loop with loop-carried dependencies */
    for (int i = 0; i < N; i++) {
        /* Create dense web of data dependencies */
        v1 = v2 + v3;
        v2 = v1 - v4;
        v3 = v5 * v6;
        v4 = v7 ^ v8;
        v5 = v9 + v10;
        v6 = v1 * v2;
        
        /* Mix in volatile operations */
        v_volatile = v_volatile + 1;
        v7 = v8 + v_volatile;
        
        /* CRITICAL POINT 1: Function call followed by immediate use of 
           live variables in call-clobbered registers */
        int ret1 = helper1(v1, v2);
        
        /* Immediately use return value with variables that were live
           across the call (v3, v4, v5 must be in registers) */
        v8 = ret1 * v3 + v4 - v5;
        
        /* Artificial use to force register retention */
        asm volatile("" : "+r"(v3), "+r"(v4), "+r"(v5));
        
        /* CRITICAL POINT 2: Another call with different convention */
        int ret2 = helper2(v6, v7, v8);
        
        /* Use return value with other live variables */
        v9 = ret2 + v3 * 2;
        v10 = v4 - ret2 / 2;
        
        /* Float operations to pressure floating-point registers */
        f1 = helper3(f1, f2);
        f2 = f3 * f4 + f1;
        
        /* CRITICAL POINT 3: Call between dependent float operations */
        f3 = helper3(f2, f_volatile);
        
        /* Immediate use of f3 (must be saved/restored if in call-clobbered reg) */
        f4 = f3 * 2.0f + f1;
        
        /* Double precision operations */
        d1 = helper4(d1, d2);
        d2 = d3 + d1 * 0.5;
        
        /* Long integer operations */
        l1 = helper5(l1, l2);
        l2 = l3 ^ l1;
        l3 = helper5(l2, l1);
        
        /* CRITICAL POINT 4: Call with many arguments */
        int ret3 = helper6(v1, v2, v3, v4);
        
        /* Complex dependency chain using the return value */
        v1 = v2 + ret3;
        v2 = v3 * ret3;
        v3 = v4 - ret3;
        v4 = v5 ^ ret3;
        
        /* External function call */
        v5 = external_func(v6);
        
        /* Use v6, v7 immediately after call (must be in registers) */
        v6 = v7 + v5;
        v7 = v8 * v6;
        
        /* More artificial register pressure */
        asm volatile("" : "+r"(v8), "+r"(v9), "+r"(v10));
        
        /* Update volatile for next iteration */
        f_volatile = f_volatile * 1.01f;
    }
    
    /* Compute checksum to prevent dead code elimination */
    long checksum = (long)v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10
                    + (long)(f1 * 100) + (long)(f2 * 100) + (long)(f3 * 100) + (long)(f4 * 100)
                    + (long)d1 + (long)d2 + (long)d3
                    + l1 + l2 + l3;
    
    printf("Checksum: %ld\n", checksum);
    return 0;
}
