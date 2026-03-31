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
volatile int global_seed = 42;

int main(int argc, char *argv[]) {
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
    
    int checksum = 0;
    
    /* Main loop with loop-carried dependencies */
    for (int i = 0; i < N; i++) {
        /* Create dense web of data dependencies */
        v1 = v2 + v3;
        v4 = v1 * v5;
        v6 = v4 - v7;
        v8 = v6 ^ v9;  /* XOR creates dependency */
        v10 = v8 | v1; /* OR creates dependency */
        
        f1 = f2 * f3;
        f4 = f1 + f5;
        f2 = f4 - f3;
        
        d1 = d2 / d3;
        d4 = d1 * d2;
        d3 = d4 - d1;
        
        l1 = l2 + l3;
        l2 = l1 - l3;
        l3 = l2 ^ l1;
        
        /* CRITICAL: Call helper1 with some live variables */
        /* v1, v2, v3 are used in computation before call */
        int temp1 = helper1(v1, v2);
        
        /* Immediately use return value with variables live across call */
        /* v4, v5, v6 were NOT passed to helper1, so they must be saved */
        v7 = temp1 + v4 + v5;  /* Uses v4, v5 which were live across call */
        
        /* Artificial register pressure */
        asm volatile("" : "+r"(v6), "+r"(v8));
        
        /* Another call with different arguments */
        float temp2 = helper3(f1, f2);
        
        /* Use return value with variables live across this call */
        /* f3, f4 were NOT passed to helper3 */
        f5 = temp2 * f3 + f4;
        
        /* More arithmetic creating dependencies */
        v9 = v7 * v10;
        v2 = v9 - v6;
        v3 = v2 ^ v8;
        
        /* Call with mixed types */
        double temp3 = helper4(d1, d2);
        d4 = temp3 * d3;
        
        /* REGPARM call on x86 - different register pressure */
        long temp4 = helper5(l1, l2);
        l3 = temp4 + l3;
        
        /* Complex call with many arguments */
        int temp5 = helper6(v1, v2, v3, v4);
        v5 = temp5 + v6 + v7;
        
        /* More artificial uses to prevent optimization */
        asm volatile("" : "+r"(v8), "+r"(v9), "+r"(v10));
        asm volatile("" : "+r"(f1), "+r"(f2));
        asm volatile("" : "+r"(d1), "+r"(d2));
        
        /* Loop-carried dependency */
        v1 = v1 + i;
        f1 = f1 + (float)i;
        d1 = d1 + (double)i;
        l1 = l1 + i;
    }
    
    /* Compute checksum to prevent dead code elimination */
    checksum = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    checksum += (int)f1 + (int)f2 + (int)f3 + (int)f4 + (int)f5;
    checksum += (int)d1 + (int)d2 + (int)d3 + (int)d4;
    checksum += (int)l1 + (int)l2 + (int)l3;
    
    printf("Checksum: %d\n", checksum);
    return checksum != 0 ? 0 : 1;
}

/* Implementation of helper functions in same file (but after main) */
int helper1(int a, int b) {
    return a + b + global_seed;
}

int helper2(int a, int b, int c) {
    return a * b - c;
}

float helper3(float a, float b) {
    return a / (b + 1.0f);
}

double helper4(double a, double b) {
    return a * b - 3.14159;
}

long helper5(long a, long b) {
    return (a << 3) | (b & 0xFF);
}

int helper6(int a, int b, int c, int d) {
    return (a + b) * (c - d);
}
