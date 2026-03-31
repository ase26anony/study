/* caller-save-test.c */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Non-inlineable helper functions with different calling conventions */
#ifdef __x86_64__
#define REGPARM3 __attribute__((regparm(3)))
#else
#define REGPARM3
#endif

/* Helper functions declared in separate compilation unit or noinline */
int helper1(int a, int b) __attribute__((noinline));
float helper2(float a, float b) __attribute__((noinline));
double helper3(double a, double b) __attribute__((noinline));
long helper4(long a, long b) __attribute__((noinline));
int helper5(int a, int b, int c) REGPARM3 __attribute__((noinline));

/* Implementations (would normally be in separate file) */
int helper1(int a, int b) {
    volatile int result = a + b;  /* Prevent optimization */
    return result;
}

float helper2(float a, float b) {
    volatile float result = a * b;
    return result;
}

double helper3(double a, double b) {
    volatile double result = a - b;
    return result;
}

long helper4(long a, long b) {
    volatile long result = a ^ b;
    return result;
}

int helper5(int a, int b, int c) REGPARM3 {
    volatile int result = (a * b) / (c + 1);
    return result;
}

int main(int argc, char *argv[]) {
    /* Non-constant loop bound to prevent unrolling */
    int N = (argc > 1) ? atoi(argv[1]) : 1000;
    if (N <= 0) N = 1000;
    
    /* Seed RNG for variable initialization */
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
    
    float f1 = (float)(rand() % 100) / 10.0f;
    float f2 = (float)(rand() % 100) / 10.0f;
    float f3 = (float)(rand() % 100) / 10.0f;
    float f4 = (float)(rand() % 100) / 10.0f;
    
    double d1 = (double)(rand() % 100) / 10.0;
    double d2 = (double)(rand() % 100) / 10.0;
    double d3 = (double)(rand() % 100) / 10.0;
    
    long l1 = rand() % 100;
    long l2 = rand() % 100;
    long l3 = rand() % 100;
    
    /* Volatile variables to prevent optimization */
    volatile int v_volatile = 0;
    volatile float f_volatile = 0.0f;
    
    /* Main loop with loop-carried dependencies */
    for (int i = 0; i < N; i++) {
        /* Create dense web of data dependencies */
        v1 = v2 + v3;
        v2 = v1 * v4;
        v3 = v5 - v6;
        v4 = v7 ^ v8;
        v5 = v9 | v10;
        
        f1 = f2 * f3;
        f2 = f1 + f4;
        f3 = f2 - f1;
        
        d1 = d2 * d3;
        d2 = d1 - d3;
        
        l1 = l2 ^ l3;
        l2 = l1 + i;
        
        /* CRITICAL: Function call with some live variables as arguments,
           leaving others live in call-clobbered registers */
        int ret1 = helper1(v1, v2);
        
        /* Immediately use return value with variables that were live
           across the call (v3, v4, v5 remain live in call-clobbered regs) */
        v6 = ret1 + v3 + v4 + v5;  /* v3, v4, v5 were live across helper1 call */
        
        /* Artificial use to force register pressure */
        asm volatile("" : "+r"(v3), "+r"(v4), "+r"(v5));
        
        /* Another function call with different types */
        float ret2 = helper2(f1, f2);
        
        /* Use return value with variables live across this call */
        f4 = ret2 * f3;  /* f3 was live across helper2 call */
        
        /* Force f3 to stay in register */
        asm volatile("" : "+r"(f3));
        
        /* Third function call with double */
        double ret3 = helper3(d1, d2);
        
        /* Use return value */
        d3 = ret3 + d2;  /* d2 was live across helper3 call */
        
        /* Fourth function call with long */
        long ret4 = helper4(l1, l2);
        
        /* Use return value */
        l3 = ret4 ^ l2;  /* l2 was live across helper4 call */
        
        /* Fifth function call with regparm convention (x86 specific) */
        int ret5 = helper5(v6, v7, v8);
        
        /* Complex use pattern to keep many values live */
        v7 = v8 + ret5;
        v8 = v9 * v10;
        v9 = v7 ^ v8;
        v10 = v9 + i;
        
        /* Volatile operations to prevent reordering */
        v_volatile = v1 + v2;
        f_volatile = f1 + f2;
        
        /* More arithmetic to increase pressure */
        v1 = v1 + 1;
        v2 = v2 * 2;
        v3 = v3 - 1;
        v4 = v4 ^ 0x55;
        v5 = v5 | 0xAA;
        
        f1 = f1 * 1.1f;
        f2 = f2 / 1.1f;
        d1 = d1 * 1.01;
        d2 = d2 / 1.01;
        
        l1 = l1 << 1;
        l2 = l2 >> 1;
        l3 = l3 + 1;
    }
    
    /* Compute checksum to prevent dead code elimination */
    long checksum = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10
                  + (long)f1 + (long)f2 + (long)f3 + (long)f4
                  + (long)d1 + (long)d2 + (long)d3
                  + l1 + l2 + l3
                  + v_volatile + (long)f_volatile;
    
    printf("Checksum: %ld\n", checksum);
    return 0;
}
