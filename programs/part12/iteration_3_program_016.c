/* caller_save_test.c - Test program to trigger caller-save insertion logic */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Non-inlineable helper functions with different calling conventions */
int __attribute__((noinline)) helper1(int a, int b) { return a + b; }
int __attribute__((noinline)) helper2(int a, int b) { return a - b; }
float __attribute__((noinline)) helper3(float a, float b) { return a * b; }
double __attribute__((noinline)) helper4(double a, double b) { return a / b; }
long __attribute__((noinline)) helper5(long a, long b) { return a ^ b; }

/* Functions with x86 regparm attribute (if on x86) */
#ifdef __i386__
int __attribute__((regparm(3), noinline)) helper_regparm(int a, int b, int c) { 
    return a + b - c; 
}
#endif

/* Function that uses many registers */
int __attribute__((noinline)) complex_helper(int a, int b, int c, int d, 
                                            float e, float f, double g) {
    return a + b - c * d + (int)(e * f) + (int)g;
}

int main(int argc, char *argv[]) {
    /* Non-constant loop bound to prevent optimization */
    int N = (argc > 1) ? atoi(argv[1]) : 1000;
    if (N <= 0) N = 1000;
    
    /* Seed RNG to prevent constant propagation */
    srand(time(NULL));
    
    /* Declare many scalar variables of mixed types to create register pressure */
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
    
    long l1 = (long)rand() * rand();
    long l2 = (long)rand() * rand();
    long l3 = (long)rand() * rand();
    
    /* Mark some variables as volatile to inhibit optimizations */
    volatile int v_volatile = rand() % 100;
    volatile float f_volatile = (float)(rand() % 100) / 10.0f;
    
    /* Main loop with loop-carried dependencies */
    for (int i = 0; i < N; i++) {
        /* Create dense web of data dependencies between variables */
        v1 = v2 + v3;
        v2 = v1 - v4;
        v3 = v2 * v5;
        v4 = v3 / (v6 + 1);
        v5 = v4 ^ v7;
        v6 = v5 | v8;
        v7 = v6 & v9;
        v8 = v7 + v10;
        v9 = v8 - v1;
        v10 = v9 * v2;
        
        f1 = f2 + f3;
        f2 = f1 * f4;
        f3 = f2 - f1;
        f4 = f3 / (f1 + 1.0f);
        
        d1 = d2 * d3;
        d2 = d1 / (d3 + 1.0);
        d3 = d2 - d1;
        
        l1 = l2 ^ l3;
        l2 = l1 | (long)v1;
        l3 = l2 & (long)v2;
        
        /* CRITICAL: Call helper function with some live variables,
           then immediately use other variables that were live across the call */
        
        /* First call - v1, v2 are arguments, v3-v6 are live across call */
        int result1 = helper1(v1, v2);
        
        /* Immediately use variables that were live across the call */
        /* This creates the need for caller-save between call and this use */
        v3 = v3 + result1;  /* v3 was live in call-clobbered register */
        v4 = v4 * result1;  /* v4 was live in call-clobbered register */
        
        /* Use asm volatile to create artificial register pressure */
        asm volatile("" : "+r"(v3), "+r"(v4));
        
        /* Second call - different arguments, more live variables */
        float result2 = helper3(f1, f2);
        
        /* Immediately use more live variables */
        f3 = f3 + result2;
        f4 = f4 - result2;
        
        /* Third call with mixed types */
        double result3 = helper4(d1, d2);
        d3 = d3 * result3;
        
        /* Fourth call - many arguments to use more registers */
        int result4 = complex_helper(v5, v6, v7, v8, f1, f2, d1);
        
        /* Use volatile variables to prevent optimizations */
        v_volatile = v_volatile + result4;
        f_volatile = f_volatile * (float)result4;
        
        /* Create more dependencies for next iteration */
        v5 = v6 + result4;
        v6 = v7 - v_volatile;
        v7 = v8 * (int)f_volatile;
        
        #ifdef __i386__
        /* Use regparm function on x86 to pressure specific registers */
        int result5 = helper_regparm(v9, v10, v1);
        v9 = v9 ^ result5;
        v10 = v10 | result5;
        #endif
        
        /* Final call in the sequence */
        long result6 = helper5(l1, l2);
        l3 = l3 ^ result6;
        
        /* Loop-carried dependency for next iteration */
        v1 = v1 + i;
        v2 = v2 - i;
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
