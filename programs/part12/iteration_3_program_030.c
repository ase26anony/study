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
    return a * 0.3 + b * 0.7;
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
#define PRESSURE(var) asm volatile("" : "+r"(var))

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
    
    float f1 = (float)rand() / RAND_MAX;
    float f2 = (float)rand() / RAND_MAX;
    float f3 = (float)rand() / RAND_MAX;
    float f4 = (float)rand() / RAND_MAX;
    
    double d1 = (double)rand() / RAND_MAX;
    double d2 = (double)rand() / RAND_MAX;
    double d3 = (double)rand() / RAND_MAX;
    
    long l1 = rand() * rand();
    long l2 = rand() * rand();
    long l3 = rand() * rand();
    
    /* Create artificial register pressure */
    volatile int vol1 = v1;
    volatile float volf = f1;
    
    for (int i = 0; i < N; i++) {
        /* Complex web of data dependencies */
        v1 = v2 + v3;
        v2 = v1 ^ v4;
        v3 = v2 * v5;
        v4 = v3 - v6;
        v5 = v4 | v7;
        v6 = v5 & v8;
        v7 = v6 + v9;
        v8 = v7 ^ v10;
        v9 = v8 * v1;
        v10 = v9 - v2;
        
        f1 = f2 * 1.5f;
        f2 = f1 + f3;
        f3 = f2 * 0.8f;
        f4 = f3 - f1;
        
        d1 = d2 * 1.7;
        d2 = d1 + d3;
        d3 = d2 * 0.6;
        
        l1 = l2 << 2;
        l2 = l1 | l3;
        l3 = l2 >> 1;
        
        /* CRITICAL: Function call followed immediately by use of 
           live variables in call-clobbered registers */
        int ret1 = helper1(v1, v2);
        
        /* This use of v3 must happen immediately after the call,
           forcing potential save/restore insertion between call and use */
        v3 = v3 + ret1;  /* v3 was live across the call */
        
        /* Create register pressure to force spill decisions */
        PRESSURE(v4);
        PRESSURE(v5);
        
        /* Another call with different register types */
        float ret2 = helper3(f1, f2);
        
        /* Immediate use of f3 which was live across call */
        f3 = f3 * ret2;
        
        PRESSURE(f4);
        
        /* Call with double arguments */
        double ret3 = helper4(d1, d2);
        
        /* Immediate use of d3 */
        d3 = d3 + ret3;
        
        /* Mixed type calls to increase pressure */
        int ret4 = helper2(v6, v7, v8);
        v9 = v9 + ret4;  /* v9 live across call */
        
        long ret5 = helper5(l1, l2);
        l3 = l3 ^ ret5;  /* l3 live across call */
        
        #ifdef __x86_64__
        /* Force specific register usage on x86 */
        int ret6 = helper_regparm(v10, v1, v2);
        v4 = v4 + ret6;  /* v4 live across call */
        #endif
        
        /* More arithmetic to keep values alive */
        v1 = v1 + i;
        v2 = v2 - i;
        f1 = f1 + (float)i * 0.1f;
        d1 = d1 + (double)i * 0.01;
        l1 = l1 + i;
        
        /* Artificial memory barrier to prevent reordering */
        asm volatile("" ::: "memory");
    }
    
    /* Compute checksum to prevent elimination */
    long checksum = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    checksum += (long)(f1 * 100) + (long)(f2 * 100) + (long)(f3 * 100) + (long)(f4 * 100);
    checksum += (long)(d1 * 1000) + (long)(d2 * 1000) + (long)(d3 * 1000);
    checksum += l1 + l2 + l3;
    
    printf("Checksum: %ld\n", checksum);
    return 0;
}
