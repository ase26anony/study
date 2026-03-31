/* caller-save-test.c
 * Designed to trigger uncovered lines 905-913 in caller-save.cc.gcov
 * Compile with: gcc -O2 -fno-omit-frame-pointer -fno-inline -fno-tree-loop-optimize caller-save-test.c -o caller-save-test
 */

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

/* x86-specific regparm attribute to increase register pressure */
#ifdef __i386__
int __attribute__((regparm(3), noinline)) helper_regparm(int a, int b, int c) {
    return (a + b) * c;
}
#endif

/* Force specific instruction insertion context:
 * Function call immediately followed by instruction using value
 * that was live in call-clobbered register before the call */
int main(int argc, char *argv[]) {
    /* Non-constant loop bound to prevent unrolling */
    int N = (argc > 1) ? atoi(argv[1]) : 1000;
    if (N <= 0) N = 1000;
    
    /* Seed RNG to prevent constant propagation */
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
    
    /* Volatile variables to inhibit optimizations */
    volatile int v_vol1 = v1;
    volatile float f_vol1 = f1;
    volatile double d_vol1 = d1;
    
    int checksum = 0;
    
    /* Main loop with loop-carried dependencies */
    for (int i = 0; i < N; i++) {
        /* Create dense web of data dependencies */
        v1 = v2 + v3;
        v2 = v1 - v4;
        v3 = v2 * v5;
        v4 = v3 ^ v6;
        v5 = v4 + v7;
        v6 = v5 - v8;
        v7 = v6 * v9;
        v8 = v7 ^ v10;
        v9 = v8 + v1;
        v10 = v9 - v2;
        
        f1 = f2 * 1.1f + f3;
        f2 = f1 * 0.9f - f4;
        f3 = f2 * 1.2f + f1;
        f4 = f3 * 0.8f - f2;
        
        d1 = d2 * 1.01 + d3;
        d2 = d1 * 0.99 - d3;
        d3 = d2 * 1.02 + d1;
        
        l1 = (l2 << 2) | (l3 & 0xFFFF);
        l2 = (l3 >> 3) ^ l1;
        l3 = (l1 << 1) & l2;
        
        /* CRITICAL SECTION: Trigger caller-save insertion between call and use */
        
        /* Call 1: v1, v2 are arguments, v3 is live across call */
        int r1 = helper1(v1, v2);
        /* Immediate use of v3 which was live in call-clobbered register */
        v3 = v3 + r1;  /* This forces save/restore of v3 around helper1 call */
        
        /* Call 2: f1, f2 are arguments, f3 is live across call */
        float r2 = helper3(f1, f2);
        /* Immediate use of f3 */
        f3 = f3 * r2;
        
        /* Call 3: d1 is argument, d2 is live across call */
        double r3 = helper4(d1, 2.0);
        /* Immediate use of d2 */
        d2 = d2 + r3;
        
        /* Call 4: multiple arguments, many values live across call */
        int r4 = helper2(v4, v5, v6);
        /* v7, v8, v9 were all live across the call */
        v7 = v7 + r4;
        v8 = v8 - r4;
        v9 = v9 * r4;
        
        /* Call 5: long arguments */
        long r5 = helper5(l1, l2);
        /* l3 was live across call */
        l3 = l3 ^ r5;
        
        #ifdef __i386__
        /* Extra register pressure with regparm calling convention */
        int r6 = helper_regparm(v10, v1, v2);
        v10 = v10 + r6;
        #endif
        
        /* Use volatile variables to create artificial register pressure */
        asm volatile("" : "+r"(v_vol1));
        v_vol1 = v1 + v2;
        
        asm volatile("" : "+r"(f_vol1));
        f_vol1 = f1 + f2;
        
        asm volatile("" : "+r"(d_vol1));
        d_vol1 = d1 + d2;
        
        /* Loop-carried dependency to prevent register elimination */
        checksum += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
        checksum += (int)f1 + (int)f2 + (int)f3 + (int)f4;
        checksum += (int)d1 + (int)d2 + (int)d3;
        checksum += (int)(l1 & 0xFFFFFFFF) + (int)(l2 & 0xFFFFFFFF) + (int)(l3 & 0xFFFFFFFF);
    }
    
    /* Final computation to prevent dead code elimination */
    int final_result = checksum % 1000000;
    printf("Result: %d\n", final_result);
    
    return final_result == 0 ? 0 : 1;
}
