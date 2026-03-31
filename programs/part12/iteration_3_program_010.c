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
    return a * 0.5f + b * 0.5f;
}

double __attribute__((noinline)) helper4(double a, double b) {
    return a * 2.0 - b;
}

long __attribute__((noinline)) helper5(long a, long b) {
    return (a << 3) | (b & 0xFF);
}

/* x86-specific regparm calling convention to increase register pressure */
#ifdef __i386__
int __attribute__((regparm(3), noinline)) helper_regparm(int a, int b, int c) {
    return (a + b) * c;
}
#endif

/* Force register usage with asm */
#define FORCE_REGISTER(var) asm volatile("" : "+r"(var))

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
    
    /* Volatile variables to prevent optimization */
    volatile int v_vol1 = rand() % 50;
    volatile int v_vol2 = rand() % 50;
    
    for (int i = 0; i < N; i++) {
        /* Create dense web of data dependencies */
        v1 = v2 + v3;
        v2 = v1 ^ v4;
        v3 = v5 * v6;
        v4 = v7 - v8;
        v5 = v9 & v10;
        v6 = v1 | v2;
        v7 = v3 ^ v4;
        v8 = v5 + v6;
        v9 = v7 - v8;
        v10 = v9 * v1;
        
        f1 = f2 * 1.1f;
        f2 = f3 + 0.5f;
        f3 = f4 * 0.9f;
        f4 = f1 - 0.2f;
        
        d1 = d2 * 1.01;
        d2 = d3 + 0.001;
        d3 = d1 * 0.99;
        
        l1 = l2 << 2;
        l2 = l3 >> 1;
        l3 = l1 | l2;
        
        /* Force some variables into registers */
        FORCE_REGISTER(v1);
        FORCE_REGISTER(v2);
        FORCE_REGISTER(f1);
        FORCE_REGISTER(d1);
        
        /* CRITICAL: Function call followed immediately by use of 
           live variables that weren't passed to the function */
        int ret1 = helper1(v1, v2);
        /* v3, v4, v5 are live across the call and must be saved */
        v3 = ret1 + v3;  /* Uses v3 which was live across call */
        v4 = v4 * ret1;  /* Uses v4 which was live across call */
        
        FORCE_REGISTER(v3);
        FORCE_REGISTER(v4);
        
        /* Another call creating multiple insertion points */
        float ret2 = helper3(f1, f2);
        /* f3, f4 are live across this call */
        f3 = ret2 + f3;
        f4 = f4 - ret2;
        
        /* Mixed type calls to pressure different register classes */
        double ret3 = helper4(d1, d2);
        d3 = d3 * ret3;
        
        long ret4 = helper5(l1, l2);
        l3 = l3 ^ ret4;
        
        /* Use volatile variables to inhibit reordering */
        v_vol1 = v_vol1 + v1;
        v_vol2 = v_vol2 * v2;
        
        /* Another helper call with many arguments */
        int ret5 = helper2(v5, v6, v7);
        /* v8, v9, v10 are live across this call */
        v8 = v8 + ret5;
        v9 = v9 - ret5;
        v10 = v10 ^ ret5;
        
        #ifdef __i386__
        /* x86-specific regparm to increase pressure on integer registers */
        int ret6 = helper_regparm(v1, v2, v3);
        v4 = v4 + ret6;
        #endif
        
        /* Create loop-carried dependencies */
        v1 = v1 + i;
        v2 = v2 - i;
        f1 = f1 + (float)i * 0.1f;
        d1 = d1 + (double)i * 0.01;
        l1 = l1 + i;
    }
    
    /* Compute checksum to prevent dead code elimination */
    long checksum = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10
                  + (long)f1 + (long)f2 + (long)f3 + (long)f4
                  + (long)d1 + (long)d2 + (long)d3
                  + l1 + l2 + l3
                  + v_vol1 + v_vol2;
    
    printf("Checksum: %ld\n", checksum);
    return 0;
}
