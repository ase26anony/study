/* caller-save-test.c */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Non-inlineable helper functions with different calling conventions */
int __attribute__((noinline)) helper1(int a, int b) {
    return (a ^ b) * 31;
}

int __attribute__((noinline)) helper2(int a, int b, int c) {
    return (a + b) * c;
}

float __attribute__((noinline)) helper3(float a, float b) {
    return a * b - 1.5f;
}

double __attribute__((noinline)) helper4(double a, double b) {
    return a / (b + 1.0);
}

long __attribute__((noinline)) helper5(long a, long b) {
    return (a << 3) | (b & 0xFF);
}

/* x86-specific regparm calling convention */
#ifdef __i386__
int __attribute__((regparm(3), noinline)) helper_regparm(int a, int b, int c) {
    return a * b + c;
}
#else
int __attribute__((noinline)) helper_regparm(int a, int b, int c) {
    return a * b + c;
}
#endif

/* Force register pressure with volatile asm */
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
    
    /* Create artificial dependencies between variables */
    for (int i = 0; i < N; i++) {
        /* Complex web of data dependencies */
        v1 = v2 + v3;
        v4 = v1 * v5;
        v6 = v4 ^ v7;
        v8 = v6 - v9;
        v10 = v8 / (v2 + 1);
        
        f1 = f2 * 1.1f;
        f3 = f1 + f4;
        f2 = f3 - 0.5f;
        f4 = f2 * f1;
        
        d1 = d2 * 1.01;
        d3 = d1 + d2;
        d2 = d3 / 2.0;
        
        l1 = l2 << 2;
        l3 = l1 | l2;
        l2 = l3 ^ 0xABCD;
        
        /* CRITICAL: Call helper with some live variables, 
           leaving others live in call-clobbered registers */
        int ret1 = helper1(v1, v2);
        
        /* Immediately use return value with variables that were 
           live across the call (in call-clobbered registers) */
        v3 = ret1 + v4;  /* v4 was live across helper1 call */
        v5 = v3 * v6;    /* v6 was live across helper1 call */
        
        /* Apply register pressure */
        PRESSURE(v7);
        PRESSURE(v8);
        
        /* Another call creating a new insertion point */
        float ret2 = helper3(f1, f2);
        
        /* Use return value with variables live across this call */
        f4 = ret2 + f3;  /* f3 was live across helper3 call */
        
        /* Mix different types and calling conventions */
        double ret3 = helper4(d1, d2);
        d3 = ret3 * d1;  /* d1 was live across helper4 call */
        
        int ret4 = helper2(v5, v6, v7);
        v8 = ret4 ^ v9;  /* v9 was live across helper2 call */
        
        long ret5 = helper5(l1, l2);
        l3 = ret5 & l2;  /* l2 was live across helper5 call */
        
        /* Call with regparm convention (x86 specific) */
        int ret6 = helper_regparm(v8, v9, v10);
        v1 = ret6 - v2;  /* v2 was live across helper_regparm call */
        
        /* Create loop-carried dependencies */
        v2 = v1 + i;
        v9 = v8 * i;
        v7 = v6 ^ i;
        
        /* More register pressure */
        PRESSURE(f4);
        PRESSURE(d3);
        PRESSURE(l3);
        
        /* Additional calls to increase caller-save opportunities */
        if (i % 3 == 0) {
            int temp = helper1(v3, v4);
            v5 = temp + v6;  /* v6 live across call */
        }
        
        if (i % 5 == 0) {
            float temp = helper3(f2, f3);
            f1 = temp * f4;  /* f4 live across call */
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    long checksum = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    checksum += (long)(f1 * 1000) + (long)(f2 * 1000) + 
                (long)(f3 * 1000) + (long)(f4 * 1000);
    checksum += (long)(d1 * 1000) + (long)(d2 * 1000) + (long)(d3 * 1000);
    checksum += l1 + l2 + l3;
    
    printf("Checksum: %ld\n", checksum);
    return 0;
}
