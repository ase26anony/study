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
    return a * 0.5f + b * 1.5f;
}

double __attribute__((noinline)) helper4(double a, double b) {
    return a * 2.0 + b * 0.5;
}

long __attribute__((noinline)) helper5(long a, long b) {
    return (a << 3) | (b & 0xFF);
}

/* x86-specific regparm calling convention to increase register pressure */
#ifdef __i386__
int __attribute__((regparm(3))) helper_regparm(int a, int b, int c) {
    return a + b * c;
}
#endif

/* Force register usage with inline asm */
#define FORCE_REGISTER(var) \
    asm volatile("" : "+r"(var))

int main(int argc, char *argv[]) {
    int N = (argc > 1) ? atoi(argv[1]) : 1000;
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
    
    /* Volatile variables to prevent optimization */
    volatile int vol1 = rand() % 50;
    volatile float vol2 = (float)(rand() % 50) * 0.5f;
    
    int i;
    for (i = 0; i < N; i++) {
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
        
        l1 = (l2 << 2) | (l3 & 0xF);
        l2 = (l3 >> 1) ^ l1;
        l3 = (l1 << 1) & l2;
        
        /* Force register usage before call */
        FORCE_REGISTER(v1);
        FORCE_REGISTER(f1);
        FORCE_REGISTER(d1);
        
        /* 
         * CRITICAL: Function call followed immediately by use of 
         * variables that were live in call-clobbered registers
         */
        int result1 = helper1(v1, v2);
        
        /* 
         * This use of v3, f1, d1 creates the scenario where:
         * - v3 was live before helper1 call
         * - v3 is in a call-clobbered register
         * - v3 is used immediately after the call
         * This forces caller-save insertion BETWEEN the call and this instruction
         */
        v3 = result1 + v3 + (int)f1 + (int)d1;
        
        /* More operations to increase pressure */
        f1 = helper3(f1, f2);
        /* Use d1 immediately after call - another insertion point */
        d1 = d1 + helper4(d2, d3);
        
        /* Use volatile variables to prevent reordering */
        v4 = v4 + vol1;
        f2 = f2 * vol2;
        
        /* Another call with mixed arguments */
        int result2 = helper2(v5, v6, v7);
        /* Immediate use of v8 which was live across the call */
        v8 = v8 * result2;
        
        /* Long operations */
        l1 = helper5(l1, l2);
        /* Immediate use of l3 */
        l3 = l3 ^ l1;
        
        #ifdef __i386__
        int result3 = helper_regparm(v9, v10, v1);
        /* Immediate use of v2 */
        v2 = v2 + result3;
        #endif
        
        /* More artificial dependencies */
        v9 = v9 + (int)(f3 * 10.0f);
        v10 = v10 - (int)(d1 * 0.1);
        
        /* Force more register usage */
        FORCE_REGISTER(v5);
        FORCE_REGISTER(f3);
        FORCE_REGISTER(l2);
    }
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    checksum += (int)f1 + (int)f2 + (int)f3 + (int)f4;
    checksum += (int)d1 + (int)d2 + (int)d3;
    checksum += (int)l1 + (int)l2 + (int)l3;
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
