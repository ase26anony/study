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

/* Create artificial register pressure with mixed types */
int main(int argc, char *argv[]) {
    int N = (argc > 1) ? atoi(argv[1]) : 1000;
    srand(time(NULL));
    
    /* Declare many scalar variables of mixed types */
    volatile int v1 = rand() % 100;
    volatile int v2 = rand() % 100;
    volatile int v3 = rand() % 100;
    volatile int v4 = rand() % 100;
    volatile int v5 = rand() % 100;
    int v6 = rand() % 100;
    int v7 = rand() % 100;
    int v8 = rand() % 100;
    int v9 = rand() % 100;
    int v10 = rand() % 100;
    float f1 = (float)(rand() % 100) / 10.0f;
    float f2 = (float)(rand() % 100) / 10.0f;
    float f3 = (float)(rand() % 100) / 10.0f;
    double d1 = (double)(rand() % 100) / 5.0;
    double d2 = (double)(rand() % 100) / 5.0;
    long l1 = (long)rand() * rand();
    long l2 = (long)rand() * rand();
    long l3 = (long)rand() * rand();
    int v11 = rand() % 100;
    int v12 = rand() % 100;
    
    /* Create complex data dependencies */
    for (int i = 0; i < N; i++) {
        /* First computation block - creates live values */
        v1 = v2 + v3;
        v4 = v1 * v5;
        v6 = v4 - v7;
        v8 = v6 ^ v9;
        v10 = v8 | v11;
        
        /* Use volatile variables to force register usage */
        asm volatile("" : "+r"(v1), "+r"(v2), "+r"(v3));
        
        /* CRITICAL: Call with some live values, leaving others live across call */
        int ret1 = helper1(v1, v2);
        
        /* Immediately use return value with variables live across the call */
        /* This creates the precise insertion point between call and use */
        v3 = ret1 + v4;  /* v4 was live in call-clobbered register across helper1 */
        
        /* More computations to keep values live */
        f1 = helper3(f1, f2);
        v5 = v3 * v6;
        
        /* Second call - creates another insertion point */
        int ret2 = helper2(v5, v6, v7);
        
        /* Again, immediately use return with live-across-call values */
        v7 = ret2 + v8;  /* v8 was live across helper2 */
        
        /* Mixed type operations to pressure different register classes */
        d1 = helper4(d1, d2);
        v9 = (int)d1 + v10;
        
        /* Third call with different types */
        l1 = helper5(l2, l3);
        
        /* Use result with other live values */
        v11 = (int)l1 + v12;
        
        /* Loop-carried dependencies to prevent register elimination */
        v12 = v11 + i;
        f2 = f1 * 1.1f;
        f3 = f2 + 0.5f;
        d2 = d1 * 0.9;
        l2 = l1 >> 2;
        l3 = l2 + i;
        
        /* More artificial register pressure */
        asm volatile("" : "+r"(v4), "+r"(v5), "+r"(v6));
        
        #ifdef __x86_64__
        /* x86-specific: force regparm calling convention */
        int ret3 = helper_regparm(v7, v8, v9);
        v10 = ret3 + v11;  /* v11 live across call */
        #endif
        
        /* Another call sequence */
        float ret4 = helper3(f2, f3);
        f1 = ret4 + 2.0f;
        
        /* Final computation block */
        v1 = v2 - v3;
        v2 = v4 ^ v5;
        v3 = v6 & v7;
        v4 = v8 | v9;
        v5 = v10 * v11;
    }
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 + v11 + v12
                   + (int)f1 + (int)f2 + (int)f3 + (int)d1 + (int)d2
                   + (int)l1 + (int)l2 + (int)l3;
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
