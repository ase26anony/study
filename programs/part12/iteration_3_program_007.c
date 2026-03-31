/* caller-save-test.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Non-inlineable helper functions with mixed calling conventions */
#ifdef __x86_64__
#define REGPARM2 __attribute__((regparm(2)))
#define REGPARM3 __attribute__((regparm(3)))
#else
#define REGPARM2
#define REGPARM3
#endif

/* Prevent inlining and optimization */
#define NOINLINE __attribute__((noinline))
#define USED __attribute__((used))

/* Helper functions declared in separate TU (but implemented here for simplicity) */
NOINLINE int helper1(int a, int b) REGPARM2;
NOINLINE float helper2(float a, float b, float c) REGPARM3;
NOINLINE double helper3(double a, double b);
NOINLINE long helper4(long a, long b, long c, long d);
NOINLINE int helper5(int a, int b, int c, int d, int e);

/* Implementations (would normally be in separate file) */
NOINLINE int helper1(int a, int b) {
    volatile int result = a ^ b;  /* Prevent optimization */
    asm volatile("" : "+r"(result));
    return result + 1;
}

NOINLINE float helper2(float a, float b, float c) {
    volatile float result = a * b + c;
    asm volatile("" : "+r"(result));
    return result * 1.1f;
}

NOINLINE double helper3(double a, double b) {
    volatile double result = a / (b + 1.0);
    asm volatile("" : "+r"(result));
    return result - 0.5;
}

NOINLINE long helper4(long a, long b, long c, long d) {
    volatile long result = (a & b) | (c ^ d);
    asm volatile("" : "+r"(result));
    return result << 2;
}

NOINLINE int helper5(int a, int b, int c, int d, int e) {
    volatile int result = a + b - c * d / (e + 1);
    asm volatile("" : "+r"(result));
    return result & 0xFF;
}

int main(int argc, char **argv) {
    /* Non-constant loop bound to prevent unrolling */
    int N = (argc > 1) ? atoi(argv[1]) : 1000;
    if (N < 10) N = 10;
    
    /* Seed RNG for variable initialization */
    srand(N);
    
    /* Declare many scalar variables of mixed types - all potentially live */
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
    
    float f1 = (rand() % 100) * 0.1f;
    float f2 = (rand() % 100) * 0.1f;
    float f3 = (rand() % 100) * 0.1f;
    float f4 = (rand() % 100) * 0.1f;
    float f5 = (rand() % 100) * 0.1f;
    
    double d1 = (rand() % 100) * 0.01;
    double d2 = (rand() % 100) * 0.01;
    double d3 = (rand() % 100) * 0.01;
    
    long l1 = rand() % 100;
    long l2 = rand() % 100;
    long l3 = rand() % 100;
    
    /* Create artificial register pressure with volatile asm */
    volatile int pressure_var1 = 0;
    volatile int pressure_var2 = 0;
    volatile float pressure_var3 = 0.0f;
    volatile double pressure_var4 = 0.0;
    
    /* Main loop with loop-carried dependencies */
    for (int i = 0; i < N; i++) {
        /* Create dense web of data dependencies */
        v1 = v2 + v3;
        v2 = v1 ^ v4;
        v3 = v5 * v6;
        v4 = v7 & v8;
        v5 = v9 | v10;
        
        f1 = f2 * 1.5f + f3;
        f2 = f4 - f5 * 0.5f;
        f3 = f1 / (f2 + 0.001f);
        
        d1 = d2 * 3.14159 + d3;
        d2 = d1 / (d3 + 1.0);
        
        l1 = l2 << 3;
        l2 = l3 >> 1;
        l3 = l1 ^ l2;
        
        /* CRITICAL: Function call with some live variables as arguments,
           followed immediately by use of other live variables that must
           be preserved across the call */
        
        /* Call 1: Pass some variables, keep others live */
        int r1 = helper1(v1, v2);
        /* Immediately use variables that were NOT passed but are live */
        v6 = v3 + v4 + r1;  /* v3, v4 live across call in call-clobbered regs */
        
        /* Artificial use to prevent reordering */
        asm volatile("" : "+r"(v3), "+r"(v4));
        
        /* Call 2: Different types, different calling convention */
        float r2 = helper2(f1, f2, f3);
        /* Immediately use other live floats */
        f4 = f5 * r2 + 2.0f;  /* f5 live across call */
        
        /* Call 3: Double precision */
        double r3 = helper3(d1, d2);
        /* Use d3 which was live across call */
        d3 = d2 + r3 * 0.5;  /* d2 already used, but ensure d3 is live */
        
        /* Call 4: Many arguments */
        long r4 = helper4(l1, l2, l3, i);
        /* Use all long variables immediately after */
        l1 = l2 + l3 + r4;
        
        /* Call 5: Many integer arguments */
        int r5 = helper5(v5, v6, v7, v8, v9);
        /* Use v10 which was live across call */
        v10 = v9 + r5;
        
        /* More dependencies to keep everything alive */
        v7 = v6 ^ v10;
        v8 = v7 * v1;
        v9 = v8 / (v2 + 1);
        
        f5 = f4 * 0.9f + f1;
        
        /* Update pressure variables to inhibit optimizations */
        pressure_var1 = v1 + v10;
        pressure_var2 = v2 * v9;
        pressure_var3 = f2 + f5;
        pressure_var4 = d1 * d3;
        
        /* Use asm to create artificial register pressure */
        asm volatile("" 
            : "+r"(v1), "+r"(v2), "+r"(v3), "+r"(v4), "+r"(v5)
            : "r"(f1), "r"(f2), "r"(d1), "r"(d2), "r"(l1), "r"(l2));
    }
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    checksum += (int)(f1 + f2 + f3 + f4 + f5);
    checksum += (int)(d1 + d2 + d3);
    checksum += (int)(l1 + l2 + l3);
    checksum += pressure_var1 + pressure_var2 + (int)pressure_var3 + (int)pressure_var4;
    
    printf("Checksum: %d\n", checksum);
    return checksum & 0xFF;
}
