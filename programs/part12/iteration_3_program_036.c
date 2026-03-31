/* caller-save-test.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Non-inlineable helper functions with different calling conventions */
#ifdef __x86_64__
#define REGPARM2 __attribute__((regparm(2)))
#define REGPARM3 __attribute__((regparm(3)))
#else
#define REGPARM2
#define REGPARM3
#endif

/* Prevent inlining and optimization */
#define NOINLINE __attribute__((noinline, noclone))

/* Helper functions that will be called across live ranges */
NOINLINE REGPARM2 int helper1(int a, int b);
NOINLINE REGPARM3 int helper2(int a, int b, int c);
NOINLINE float helper3(float a, float b);
NOINLINE double helper4(double a, double b);
NOINLINE long helper5(long a, long b);

/* Implementations (could be in separate file) */
NOINLINE REGPARM2 int helper1(int a, int b) {
    asm volatile("" : : "r"(a), "r"(b) : "memory");
    return (a ^ b) + 1;
}

NOINLINE REGPARM3 int helper2(int a, int b, int c) {
    asm volatile("" : : "r"(a), "r"(b), "r"(c) : "memory");
    return (a & b) | c;
}

NOINLINE float helper3(float a, float b) {
    asm volatile("" : : "r"(a), "r"(b) : "memory");
    return a * 0.5f + b * 1.5f;
}

NOINLINE double helper4(double a, double b) {
    asm volatile("" : : "r"(a), "r"(b) : "memory");
    return a * 0.25 + b * 0.75;
}

NOINLINE long helper5(long a, long b) {
    asm volatile("" : : "r"(a), "r"(b) : "memory");
    return (a << 3) | (b & 0x7F);
}

int main(int argc, char **argv) {
    /* Non-constant loop bound to prevent unrolling */
    int N = (argc > 1) ? atoi(argv[1]) : 1000;
    if (N < 10) N = 10;
    
    /* Declare many scalar variables with mixed types */
    /* Integers */
    volatile int v1 = 1;
    int v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    
    /* Floating point */
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f;
    double d1 = 1.11, d2 = 2.22, d3 = 3.33, d4 = 4.44;
    
    /* Long integers */
    long l1 = 1001L, l2 = 2002L, l3 = 3003L, l4 = 4004L;
    
    /* Use argc to create data dependencies */
    v1 += argc; v2 += argc; v3 += argc;
    f1 += argc; d1 += argc; l1 += argc;
    
    /* Main loop with loop-carried dependencies */
    for (int i = 0; i < N; i++) {
        /* Create dense web of data dependencies */
        v2 = v1 + v3;
        v4 = v2 * v5 - v6;
        v7 = v4 ^ v8;
        v9 = v7 | v10;
        v10 = v9 & v3;
        
        /* Mix in floating point operations */
        f2 = f1 * 2.0f + (float)v2;
        f3 = f2 / 1.5f - (float)v4;
        f4 = f3 + f1 * 0.3f;
        
        d2 = d1 * 1.5 + (double)v3;
        d3 = d2 / 2.0 - (double)v5;
        d4 = d3 + d1 * 0.7;
        
        l2 = l1 << 2 | (v6 & 0xFF);
        l3 = l2 >> 1 + v7;
        l4 = l3 ^ l1;
        
        /* CRITICAL: Call function with some live variables,
           leaving others live in call-clobbered registers */
        int ret1 = helper1(v2, v3);
        
        /* Immediately use return value with variables that were
           live across the call (in call-clobbered registers) */
        v1 = ret1 + v4 + v5;  /* v4, v5 were live across helper1 call */
        
        /* Artificial use to force register pressure */
        asm volatile("" : "+r"(v6), "+r"(v7), "+r"(v8));
        
        /* Another call with different arguments */
        int ret2 = helper2(v6, v7, v8);
        
        /* Use return value with other live variables */
        v9 = ret2 * v10 + v1;  /* v10, v1 were live across helper2 call */
        
        /* Floating point calls */
        float fret3 = helper3(f2, f3);
        f1 = fret3 + f4 * 2.0f;  /* f4 was live across helper3 call */
        
        /* More arithmetic to keep values alive */
        v3 = v1 + v9;
        v5 = v2 * v3 - v4;
        v6 = v5 ^ v7;
        
        /* Double precision call */
        double dret4 = helper4(d2, d3);
        d1 = dret4 + d4 * 0.5;  /* d4 was live across helper4 call */
        
        /* Long integer call */
        long lret5 = helper5(l2, l3);
        l1 = lret5 + l4 * 2;  /* l4 was live across helper5 call */
        
        /* Create circular dependencies for next iteration */
        v1 = v1 + 1;
        v2 = v2 - v1;
        f1 = f1 + 0.1f;
        d1 = d1 - 0.01;
        l1 = l1 ^ 0x5555;
        
        /* Prevent loop invariant code motion */
        if (i % 7 == 0) {
            v10 = helper1(v9, v8);
        }
        if (i % 13 == 0) {
            f4 = helper3(f3, f2);
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    checksum += (int)f1 + (int)f2 + (int)f3 + (int)f4;
    checksum += (int)d1 + (int)d2 + (int)d3 + (int)d4;
    checksum += (int)l1 + (int)l2 + (int)l3 + (int)l4;
    
    printf("Checksum: %d\n", checksum);
    return checksum == 0 ? 1 : 0;
}
