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

/* Function declarations - prevent inlining */
int helper1(int a, int b) __attribute__((noinline));
int helper2(int a, int b, int c) __attribute__((noinline));
float helper3(float a, float b) __attribute__((noinline));
double helper4(double a, double b) __attribute__((noinline));
long helper5(long a, long b) REGPARM3 __attribute__((noinline));
int helper6(int a, int b, int c, int d) __attribute__((noinline));

/* Global volatile to prevent optimization */
volatile int global_seed = 12345;

int main(int argc, char **argv) {
    int N = (argc > 1) ? atoi(argv[1]) : 1000;
    
    /* Declare many scalar variables of mixed types */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    float f1, f2, f3, f4, f5;
    double d1, d2, d3, d4;
    long l1, l2, l3;
    
    /* Initialize with non-constant values */
    srand(global_seed);
    v1 = rand() % 100;
    v2 = rand() % 100;
    v3 = rand() % 100;
    v4 = rand() % 100;
    v5 = rand() % 100;
    v6 = rand() % 100;
    v7 = rand() % 100;
    v8 = rand() % 100;
    v9 = rand() % 100;
    v10 = rand() % 100;
    
    f1 = (float)rand() / RAND_MAX;
    f2 = (float)rand() / RAND_MAX;
    f3 = (float)rand() / RAND_MAX;
    f4 = (float)rand() / RAND_MAX;
    f5 = (float)rand() / RAND_MAX;
    
    d1 = (double)rand() / RAND_MAX;
    d2 = (double)rand() / RAND_MAX;
    d3 = (double)rand() / RAND_MAX;
    d4 = (double)rand() / RAND_MAX;
    
    l1 = rand() % 100;
    l2 = rand() % 100;
    l3 = rand() % 100;
    
    /* Main loop with non-constant trip count */
    for (int i = 0; i < N; i++) {
        /* Create dense web of data dependencies */
        v1 = v2 + v3;
        v4 = v1 * v5;
        v6 = v4 - v7;
        v8 = v6 ^ v9;
        v10 = v8 | v2;
        
        f1 = f2 + f3;
        f4 = f1 * f5;
        f2 = f4 - f3;
        
        d1 = d2 + d3;
        d4 = d1 * d2;
        d3 = d4 - d1;
        
        l1 = l2 + l3;
        l2 = l1 ^ l3;
        l3 = l2 - l1;
        
        /* CRITICAL: Function call with some live variables as arguments,
           followed immediately by use of other live variables */
        int ret1 = helper1(v1, v2);
        
        /* v3, v4, v5 are live across the call and used immediately after */
        v3 = v3 + ret1;  /* Uses v3 which was live across helper1 call */
        v4 = v4 * ret1;  /* Uses v4 which was live across helper1 call */
        
        /* Artificial register pressure */
        asm volatile("" : "+r"(v5), "+r"(v6));
        
        /* Another function call - creates another insertion point */
        float ret2 = helper3(f1, f2);
        
        /* f3, f4 are live across this call */
        f3 = f3 + ret2;
        f4 = f4 * ret2;
        
        /* More arithmetic creating loop-carried dependencies */
        v5 = helper2(v3, v4, v5);
        v6 = v5 + v6 + v7;
        
        d1 = helper4(d1, d2);
        d3 = d1 + d3 + d4;
        
        /* Function with regparm calling convention (x86 specific) */
        long ret3 = helper5(l1, l2);
        l3 = l3 + ret3;
        
        /* Complex function call with many arguments */
        int ret4 = helper6(v8, v9, v10, v1);
        v2 = v2 + ret4;
        
        /* More artificial uses to keep variables live */
        asm volatile("" : "+r"(v7), "+r"(v8), "+r"(v9));
        asm volatile("" : "+r"(f5), "+r"(d2), "+r"(d4));
        
        /* Shuffle values to create more dependencies */
        int tmp = v1;
        v1 = v10;
        v10 = v9;
        v9 = v8;
        v8 = v7;
        v7 = v6;
        v6 = v5;
        v5 = v4;
        v4 = v3;
        v3 = v2;
        v2 = tmp;
    }
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    checksum += (int)(f1 + f2 + f3 + f4 + f5);
    checksum += (int)(d1 + d2 + d3 + d4);
    checksum += (int)(l1 + l2 + l3);
    
    printf("Checksum: %d\n", checksum);
    return 0;
}

/* Implementation of helper functions - in same file but after main
   to make inlining less likely even without noinline attribute */

int helper1(int a, int b) {
    /* Simple operation that compiler can't optimize away */
    return (a + b) ^ 0x1234;
}

int helper2(int a, int b, int c) {
    return (a * b) + c;
}

float helper3(float a, float b) {
    return a - b * 1.5f;
}

double helper4(double a, double b) {
    return a / (b + 1.0);
}

long helper5(long a, long b) {
    return (a << 3) | (b & 0xFF);
}

int helper6(int a, int b, int c, int d) {
    return (a & b) | (c ^ d);
}
