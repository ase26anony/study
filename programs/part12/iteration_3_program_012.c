/* caller-save-test.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Non-inlineable helper functions with different calling conventions */
#ifdef __x86_64__
#define REGPARM1 __attribute__((regparm(1)))
#define REGPARM2 __attribute__((regparm(2)))
#define REGPARM3 __attribute__((regparm(3)))
#else
#define REGPARM1
#define REGPARM2
#define REGPARM3
#endif

/* Prevent inlining and optimization */
#define NOINLINE __attribute__((noinline,noipa))

/* Helper functions that will be called */
NOINLINE REGPARM1 int helper1(int a, int b);
NOINLINE REGPARM2 float helper2(float a, float b, float c);
NOINLINE REGPARM3 double helper3(double a, double b, double c, double d);
NOINLINE int helper4(int a, int b, int c, int d, int e);
NOINLINE float helper5(float a, float b, float c, float d, float e, float f);

/* Implementations (could be in separate file) */
NOINLINE REGPARM1 int helper1(int a, int b) {
    asm volatile("" : : "r"(a), "r"(b) : "memory");
    return (a ^ b) + 1;
}

NOINLINE REGPARM2 float helper2(float a, float b, float c) {
    asm volatile("" : : "r"(a), "r"(b), "r"(c) : "memory");
    return a * 0.5f + b * 0.3f + c * 0.2f;
}

NOINLINE REGPARM3 double helper3(double a, double b, double c, double d) {
    asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d) : "memory");
    return (a + b) * (c - d);
}

NOINLINE int helper4(int a, int b, int c, int d, int e) {
    asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e) : "memory");
    return a + b - c + d - e;
}

NOINLINE float helper5(float a, float b, float c, float d, float e, float f) {
    asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e), "r"(f) : "memory");
    return a + b - c + d - e + f;
}

int main(int argc, char **argv) {
    int N = (argc > 1) ? atoi(argv[1]) : 1000;
    
    /* Declare many scalar variables of mixed types */
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    volatile float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    volatile float f6 = 6.6f, f7 = 7.7f, f8 = 8.8f, f9 = 9.9f, f10 = 10.10f;
    volatile double d1 = 1.11, d2 = 2.22, d3 = 3.33, d4 = 4.44, d5 = 5.55;
    volatile long l1 = 100, l2 = 200, l3 = 300, l4 = 400, l5 = 500;
    
    /* Additional non-volatile variables for more register pressure */
    int nv1 = 11, nv2 = 12, nv3 = 13, nv4 = 14, nv5 = 15;
    float nf1 = 11.11f, nf2 = 12.12f, nf3 = 13.13f;
    double nd1 = 11.111, nd2 = 12.222;
    
    /* Initialize with some randomness to prevent constant propagation */
    int seed = (argc > 2) ? atoi(argv[2]) : 42;
    srand(seed);
    v1 += rand() % 10;
    f1 += (rand() % 100) / 100.0f;
    d1 += (rand() % 100) / 1000.0;
    
    for (int i = 0; i < N; i++) {
        /* Create dense web of data dependencies */
        nv1 = v1 + v2 - v3;
        nv2 = v4 * v5 + nv1;
        nv3 = v6 ^ v7 | v8;
        nv4 = v9 - v10 + nv2;
        nv5 = nv3 * nv4;
        
        nf1 = f1 * 2.0f + f2;
        nf2 = f3 / 1.5f - f4;
        nf3 = f5 + f6 * f7;
        
        nd1 = d1 * 1.1 + d2;
        nd2 = d3 - d4 / 2.0;
        
        l1 = l2 + l3 - l4;
        l2 = l5 * 3 + l1;
        
        /* CRITICAL: Call helper1 with some live variables,
           then immediately use other variables that were live across call */
        int r1 = helper1(nv1, nv2);
        /* v3, v4, f1, f2 remain live in call-clobbered registers */
        v3 = v3 + r1;  /* Uses v3 which was live across helper1 call */
        v4 = v4 * r1;  /* Uses v4 which was live across helper1 call */
        
        /* Force register pressure with inline asm */
        asm volatile("" : "+r"(v5), "+r"(v6), "+r"(v7));
        
        /* Call helper2 with float args */
        float r2 = helper2(nf1, nf2, nf3);
        /* f4, f5, d1 remain live */
        f4 = f4 + r2;
        f5 = f5 - r2;
        
        /* More dependencies */
        d1 = nd1 + nd2;
        d2 = d1 * 0.9;
        
        /* Call helper3 with double args */
        double r3 = helper3(d1, d2, d3, d4);
        /* d5, l1, l2 remain live */
        d5 = d5 + r3;
        l1 = l1 + (long)r3;
        
        /* Call helper4 with many int args - forces register pressure */
        int r4 = helper4(v8, v9, v10, nv3, nv4);
        /* nv5, l3, l4 remain live */
        nv5 = nv5 ^ r4;
        l3 = l3 + r4;
        
        /* Call helper5 with many float args */
        float r5 = helper5(f6, f7, f8, f9, f10, nf1);
        /* nf2, nf3, l5 remain live */
        nf2 = nf2 * r5;
        l5 = l5 - (long)r5;
        
        /* Create loop-carried dependencies */
        v1 = v2 + i;
        v2 = v3 - i;
        v6 = v7 * (i % 10 + 1);
        v7 = v8 + (i % 5);
        
        f1 = f2 + (i * 0.1f);
        f2 = f3 - (i * 0.05f);
        
        d3 = d4 + (i * 0.01);
        d4 = d5 - (i * 0.005);
        
        l4 = l5 + i;
        l5 = l1 - (i * 2);
        
        /* Artificial memory barrier to prevent reordering */
        asm volatile("" ::: "memory");
    }
    
    /* Compute checksum to prevent dead code elimination */
    long checksum = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                    (long)f1 + (long)f2 + (long)f3 + (long)f4 + (long)f5 +
                    (long)f6 + (long)f7 + (long)f8 + (long)f9 + (long)f10 +
                    (long)d1 + (long)d2 + (long)d3 + (long)d4 + (long)d5 +
                    l1 + l2 + l3 + l4 + l5 +
                    nv1 + nv2 + nv3 + nv4 + nv5 +
                    (long)nf1 + (long)nf2 + (long)nf3 +
                    (long)nd1 + (long)nd2;
    
    printf("Checksum: %ld\n", checksum);
    return 0;
}
