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

/* x86-specific: force use of specific registers */
#ifdef __x86_64__
int __attribute__((regparm(3), noinline)) helper_regparm(int a, int b, int c) {
    return a + b * c;
}
#endif

/* Force register pressure by preventing optimization */
static void use_var(int v) {
    asm volatile("" : "+r"(v));
}

int main(int argc, char *argv[]) {
    int N = (argc > 1) ? atoi(argv[1]) : 1000;
    if (N <= 0) N = 1000;
    
    /* Seed RNG to prevent constant propagation */
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
    float f1 = (float)(rand() % 100) / 10.0f;
    float f2 = (float)(rand() % 100) / 10.0f;
    float f3 = (float)(rand() % 100) / 10.0f;
    double d1 = (double)(rand() % 100) / 10.0;
    double d2 = (double)(rand() % 100) / 10.0;
    long l1 = (long)rand() * rand();
    long l2 = (long)rand() * rand();
    
    /* Create artificial register pressure */
    volatile int vol1 = rand() % 100;
    volatile int vol2 = rand() % 100;
    
    int checksum = 0;
    
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
        v9 = v8 + v11;
        v10 = v9 - v12;
        v11 = v10 * v13;
        v12 = v11 ^ v14;
        v13 = v12 + v15;
        v14 = v13 - v1;
        v15 = v14 * v2;
        
        /* Mix in float/double/long operations */
        f1 = f2 * 1.1f + f3;
        f2 = f1 * 0.9f - f3;
        f3 = helper3(f1, f2);
        
        d1 = d2 * 1.5 + (double)v1;
        d2 = helper4(d1, d2);
        
        l1 = (l1 << 1) | (l2 & 1);
        l2 = helper5(l1, l2);
        
        /* CRITICAL: Function call with some live variables as arguments,
           followed immediately by use of other live variables */
        int ret1 = helper1(v1, v2);
        
        /* v3, v4, v5 are live across the call and must be saved */
        v3 = v3 + ret1;  /* Uses v3 which was live across helper1 call */
        use_var(v3);     /* Artificial use to force register allocation */
        
        /* Another call creating more pressure */
        int ret2 = helper2(v4, v5, v6);
        
        /* v7, v8 are live across this call */
        v7 = v7 * ret2;
        use_var(v7);
        
        /* Use volatile variables to inhibit optimizations */
        vol1 = vol1 + v8;
        vol2 = vol2 ^ v9;
        
        #ifdef __x86_64__
        /* x86-specific: force use of regparm calling convention */
        int ret3 = helper_regparm(v10, v11, v12);
        v13 = v13 + ret3;  /* v13 live across call */
        #else
        int ret3 = helper1(v10, v11);
        v13 = v13 + ret3 + v12;  /* v12, v13 live across call */
        #endif
        
        /* More operations keeping many values live */
        v14 = v14 + v15 + ret1;
        v15 = v15 - ret2 - ret3;
        
        /* Use all variables in checksum to prevent elimination */
        checksum += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                   v11 + v12 + v13 + v14 + v15 + (int)f1 + (int)f2 + 
                   (int)f3 + (int)d1 + (int)d2 + (int)l1 + (int)l2 +
                   vol1 + vol2;
        
        /* Prevent loop unrolling */
        if (i % 7 == 0) {
            v1 = v1 ^ checksum;
        }
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
