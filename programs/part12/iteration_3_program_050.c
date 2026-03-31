/* caller-save-trigger.c */
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

/* x86-specific: force use of specific registers */
#ifdef __x86_64__
int __attribute__((regparm(3), noinline)) helper_regparm(int a, int b, int c) {
    return a + b - c;
}
#endif

/* Force register pressure by preventing optimization */
static void use_var(int v) {
    asm volatile("" : "+r"(v));
}

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
    int v11 = rand() % 100;
    int v12 = rand() % 100;
    int v13 = rand() % 100;
    int v14 = rand() % 100;
    int v15 = rand() % 100;
    float f1 = (float)rand() / RAND_MAX;
    float f2 = (float)rand() / RAND_MAX;
    float f3 = (float)rand() / RAND_MAX;
    double d1 = (double)rand() / RAND_MAX;
    double d2 = (double)rand() / RAND_MAX;
    long l1 = rand() * 1000L;
    long l2 = rand() * 1000L;
    
    /* Create artificial dependencies between variables */
    for (int i = 0; i < N; i++) {
        /* Complex web of data dependencies */
        v1 = v2 + v3;
        v4 = v1 * v5;
        v6 = v4 - v7;
        v8 = v6 ^ v9;
        v10 = v8 | v11;
        v12 = v10 & v13;
        v14 = v12 + v15;
        v15 = v14 - v1;
        
        f1 = f2 * 1.1f + f3;
        f2 = f1 * 0.9f - f3;
        f3 = helper3(f1, f2);
        
        d1 = d2 * 1.5 + (double)v1;
        d2 = helper4(d1, d2);
        
        l1 = l2 << 2;
        l2 = helper5(l1, l2);
        
        /* CRITICAL: Function call with some live variables,
           immediately followed by use of other live variables */
        int ret1 = helper1(v1, v2);
        
        /* This use of v3 must happen immediately after the call,
           forcing potential caller-save insertion between the call
           and this instruction */
        v3 = ret1 + v3;  /* v3 was live across the call! */
        use_var(v3);     /* Prevent optimization */
        
        /* Another call with different arguments */
        int ret2 = helper2(v4, v5, v6);
        
        /* Immediate use of another variable live across the call */
        v7 = ret2 * v7;  /* v7 was live across the call! */
        use_var(v7);
        
        /* Mix in more calls with floating point */
        float ret3 = helper3(f1, f2);
        f1 = ret3 + f3;  /* f3 was live across the call! */
        
        /* x86-specific calling convention to pressure registers */
        #ifdef __x86_64__
        int ret4 = helper_regparm(v8, v9, v10);
        v11 = ret4 + v11;  /* v11 was live across the call! */
        #endif
        
        /* Create loop-carried dependencies */
        v2 = v1 + i;
        v5 = v4 - i;
        v9 = v8 ^ i;
        v13 = v12 | i;
        
        /* More arithmetic to keep all variables active */
        v1 = v1 + 1;
        v2 = v2 - 1;
        v3 = v3 * 2;
        v4 = v4 / 2;
        v5 = v5 ^ 0x55;
        v6 = v6 | 0xAA;
        v7 = v7 & 0xFF;
        v8 = v8 << 1;
        v9 = v9 >> 1;
        v10 = v10 + v11;
        v11 = v11 - v12;
        v12 = v12 * v13;
        v13 = v13 ^ v14;
        v14 = v14 | v15;
        v15 = v15 & v1;
        
        /* Use volatile asm to prevent reordering around calls */
        asm volatile("" : "+r"(v1), "+r"(v2), "+r"(v3), "+r"(v4));
        asm volatile("" : "+r"(v5), "+r"(v6), "+r"(v7), "+r"(v8));
    }
    
    /* Compute checksum to prevent dead code elimination */
    long checksum = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                   v11 + v12 + v13 + v14 + v15 + (long)f1 + (long)f2 +
                   (long)f3 + (long)d1 + (long)d2 + l1 + l2;
    
    printf("Checksum: %ld\n", checksum);
    return 0;
}
