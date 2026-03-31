/* caller-save-trigger.c */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Non-inlineable helper functions with different calling conventions */
#ifdef __x86_64__
#define REGPARM3 __attribute__((regparm(3)))
#else
#define REGPARM3
#endif

/* Helper functions declared in separate module or with noinline */
int helper1(int a, int b) __attribute__((noinline));
int helper2(int a, int b) __attribute__((noinline));
float helper3(float a, float b) __attribute__((noinline));
double helper4(double a, double b) __attribute__((noinline));
long helper5(long a, long b) __attribute__((noinline));
int helper6(int a, int b) REGPARM3 __attribute__((noinline));

/* Implementation of helpers (would normally be in separate file) */
int helper1(int a, int b) { return a ^ b; }
int helper2(int a, int b) { return a & ~b; }
float helper3(float a, float b) { return a * 0.5f + b * 0.5f; }
double helper4(double a, double b) { return a * 0.25 + b * 0.75; }
long helper5(long a, long b) { return a % (b | 1); }
int helper6(int a, int b) { return (a << 3) | (b & 7); }

int main(int argc, char **argv) {
    /* Non-constant loop bound to prevent unrolling */
    int N = (argc > 1) ? atoi(argv[1]) : 1000;
    if (N <= 0) N = 1000;
    
    /* Seed RNG to prevent constant propagation */
    srand((unsigned)time(NULL));
    
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
    
    /* Volatile variables to inhibit optimizations */
    volatile int v_volatile = rand() % 100;
    volatile float f_volatile = (float)(rand() % 100) * 0.1f;
    
    /* Main loop with loop-carried dependencies */
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
        f3 = f2 + f1 * 0.5f;
        f4 = f3 * 2.0f - f2;
        
        d1 = d2 * 1.01 + d3;
        d2 = d1 * 0.99 - d3;
        d3 = d2 + d1 * 0.5;
        
        l1 = l2 ^ l3;
        l2 = l1 + l3;
        l3 = l2 - l1;
        
        /* CRITICAL: Function call followed immediately by use of 
           live variables in call-clobbered registers */
        int temp1 = helper1(v1, v2);
        
        /* Immediately use return value with variables live across call */
        v3 = temp1 + v4;  /* v4 was live in call-clobbered register */
        v5 = v3 * v6;     /* v6 was live in call-clobbered register */
        
        /* Artificial use to force register allocation */
        asm volatile("" : "+r"(v3), "+r"(v4));
        
        /* Another function call creating new insertion point */
        float temp2 = helper3(f1, f2);
        
        /* Use return value with variables live across this call */
        f3 = temp2 + f4;  /* f4 was live in call-clobbered register */
        d1 = d2 + d3;     /* d2,d3 were live across call */
        
        /* Mix different types of function calls */
        double temp3 = helper4(d1, d2);
        d3 = temp3 * 0.5 + d1;
        
        /* Function with regparm calling convention (x86 specific) */
        int temp4 = helper6(v7, v8);
        
        /* Complex dependency chain across function calls */
        v9 = temp4 + v10;
        v10 = v9 ^ v1;
        
        /* Use volatile variables to prevent optimizations */
        v_volatile = v1 + v2;
        f_volatile = f1 + f2;
        
        /* Long operation to force register pressure */
        long temp5 = helper5(l1, l2);
        l3 = temp5 + l1 - l2;
        
        /* Another helper call with many live variables */
        int temp6 = helper2(v3, v4);
        v5 = temp6 * v6;
        v6 = v5 + v7;
        
        /* Final dependency to keep all variables live */
        v1 = v10 + v_volatile;
        f1 = f4 + f_volatile;
        d1 = d3 + (double)v_volatile;
        l1 = l3 + (long)v_volatile;
    }
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    checksum += (int)f1 + (int)f2 + (int)f3 + (int)f4;
    checksum += (int)d1 + (int)d2 + (int)d3;
    checksum += (int)l1 + (int)l2 + (int)l3;
    checksum += v_volatile + (int)f_volatile;
    
    printf("Checksum: %d\n", checksum);
    return checksum == 0 ? 1 : 0;
}
