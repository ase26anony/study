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

/* x86-specific regparm calling convention */
#ifdef __i386__
int __attribute__((regparm(3), noinline)) helper_regparm(int a, int b, int c) {
    return a + b * c;
}
#else
int __attribute__((noinline)) helper_regparm(int a, int b, int c) {
    return a + b * c;
}
#endif

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
    float f2 = (float)(rand() % 100) * 0.1f;
    float f3 = (float)(rand() % 100) * 0.1f;
    float f4 = (float)(rand() % 100) * 0.1f;
    
    double d1 = (double)(rand() % 100) * 0.01;
    double d2 = (double)(rand() % 100) * 0.01;
    double d3 = (double)(rand() % 100) * 0.01;
    
    long l1 = (long)rand() * rand();
    long l2 = (long)rand() * rand();
    long l3 = (long)rand() * rand();
    
    /* Mark some variables as volatile to inhibit optimizations */
    volatile int v_volatile = rand() % 100;
    
    for (int i = 0; i < N; i++) {
        /* Create dense web of data dependencies */
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
        
        f1 = f2 * 1.1f + f3;
        f2 = f1 * 0.9f - f4;
        f3 = f2 + f4 * 0.5f;
        f4 = f3 * 2.0f - f1;
        
        d1 = d2 * 1.01 + d3;
        d2 = d1 * 0.99 - d3;
        d3 = d2 + d1 * 0.5;
        
        l1 = l2 << 2;
        l2 = l3 >> 1;
        l3 = l1 | l2;
        
        /* CRITICAL: Function call with some live variables as arguments,
           followed immediately by use of other live variables that were
           NOT passed to the function (live across call in call-clobbered regs) */
        int ret1 = helper1(v1, v2);
        
        /* v3, v4, v5 are live across the call and must be saved/restored */
        v3 = v3 + ret1;  /* Uses v3 which was live across helper1 call */
        v4 = v4 * ret1;  /* Uses v4 which was live across helper1 call */
        
        /* Artificial use to prevent optimization */
        asm volatile("" : "+r"(v5));
        
        /* Another function call creating another insertion point */
        float ret2 = helper3(f1, f2);
        
        /* f3 is live across this call */
        f3 = f3 + ret2;
        
        /* Use volatile variable */
        v_volatile = v_volatile + 1;
        
        /* More complex chain with regparm function */
        int ret3 = helper_regparm(v6, v7, v8);
        
        /* v9, v10 are live across this call */
        v9 = v9 ^ ret3;
        v10 = v10 | ret3;
        
        /* Double precision call */
        double ret4 = helper4(d1, d2);
        
        /* d3 is live across this call */
        d3 = d3 * ret4;
        
        /* Long integer call */
        long ret5 = helper5(l1, l2);
        
        /* l3 is live across this call */
        l3 = l3 & ret5;
        
        /* Final helper call in the sequence */
        int ret6 = helper2(v9, v10, v1);
        
        /* v2 is live across this call - forces save/restore insertion
           between the call and this use */
        v2 = v2 + ret6;  /* This use should trigger the target insertion */
        
        /* Create artificial dependencies for next iteration */
        v1 = v1 + i;
        f1 = f1 + (float)i * 0.1f;
        d1 = d1 + (double)i * 0.01;
        l1 = l1 + i;
    }
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    checksum += (int)f1 + (int)f2 + (int)f3 + (int)f4;
    checksum += (int)d1 + (int)d2 + (int)d3;
    checksum += (int)l1 + (int)l2 + (int)l3;
    checksum += v_volatile;
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
