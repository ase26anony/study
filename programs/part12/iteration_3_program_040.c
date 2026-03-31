/* caller-save-test.c
 * Compile with: gcc -O2 -fno-omit-frame-pointer -fno-inline -fno-tree-loop-optimize caller-save-test.c -o caller-save-test
 * For RTL dumps: gcc -O3 -fno-schedule-insns -fdump-rtl-all caller-save-test.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Non-inlineable helper functions with different calling conventions */
#ifdef __x86_64__
#define REGPARM2 __attribute__((regparm(2)))
#define REGPARM3 __attribute__((regparm(3)))
#else
#define REGPARM2
#define REGPARM3
#endif

/* Prevent inlining and optimization */
#define NOINLINE __attribute__((noinline))

/* Helper functions implemented in separate compilation unit or with noinline */
NOINLINE int helper1(int a, int b);
NOINLINE float helper2(float a, float b);
NOINLINE double helper3(double a, double b);
NOINLINE long helper4(long a, long b);
NOINLINE REGPARM2 int helper5(int a, int b, int c);
NOINLINE REGPARM3 float helper6(float a, float b, float c, float d);

/* Implementations (would normally be in separate file) */
NOINLINE int helper1(int a, int b) {
    volatile int result = a + b;
    return result;
}

NOINLINE float helper2(float a, float b) {
    volatile float result = a * b;
    return result;
}

NOINLINE double helper3(double a, double b) {
    volatile double result = a - b;
    return result;
}

NOINLINE long helper4(long a, long b) {
    volatile long result = a ^ b;
    return result;
}

NOINLINE REGPARM2 int helper5(int a, int b, int c) {
    volatile int result = (a * b) / (c + 1);
    return result;
}

NOINLINE REGPARM3 float helper6(float a, float b, float c, float d) {
    volatile float result = (a + b) * (c - d);
    return result;
}

int main(int argc, char *argv[]) {
    int N = (argc > 1) ? atoi(argv[1]) : 1000;
    if (N <= 0) N = 1000;
    
    /* Seed RNG for variable initialization */
    srand(time(NULL));
    
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
    
    float f1 = (float)(rand() % 100) / 10.0f;
    float f2 = (float)(rand() % 100) / 10.0f;
    float f3 = (float)(rand() % 100) / 10.0f;
    float f4 = (float)(rand() % 100) / 10.0f;
    float f5 = (float)(rand() % 100) / 10.0f;
    
    double d1 = (double)(rand() % 100) / 5.0;
    double d2 = (double)(rand() % 100) / 5.0;
    double d3 = (double)(rand() % 100) / 5.0;
    
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
        v2 = v4 - v5;
        v3 = v6 * v7;
        v4 = v8 ^ v9;
        v5 = v10 + i;
        
        f1 = f2 * f3;
        f2 = f4 / (f5 + 1.0f);
        f3 = f1 - f2;
        
        d1 = d2 + d3;
        d2 = d1 * 1.01;
        d3 = d2 - 0.5;
        
        l1 = l2 | l3;
        l2 = l1 ^ i;
        l3 = l2 & 0xFF;
        
        /* CRITICAL: Call helper1 with some live variables, 
           leaving others live in call-clobbered registers */
        int ret1 = helper1(v1, v2);
        
        /* Immediately use return value with variables that were 
           live across the call (v3, v4 in call-clobbered regs) */
        v6 = ret1 + v3 + v4;  /* v3, v4 must be saved/restored */
        
        /* Artificial use to force register allocation */
        asm volatile("" : "+r"(v3), "+r"(v4));
        
        /* Another call with different types */
        float ret2 = helper2(f1, f2);
        
        /* Use return value with variables live across call */
        f4 = ret2 * f3 * f5;  /* f3, f5 must be saved/restored */
        
        /* More arithmetic to keep variables live */
        v7 = v8 * v9;
        v8 = v10 ^ v1;
        v9 = v2 + v3;
        v10 = v4 - v5;
        
        /* Call with regparm convention (x86 specific) */
        int ret3 = helper5(v5, v6, v7);
        
        /* Use return value immediately after call */
        v1 = v2 + ret3 + v8;  /* v2, v8 live across call */
        
        /* Double precision call */
        double ret4 = helper3(d1, d2);
        d3 = ret4 + d1 + d2;  /* d1, d2 live across call */
        
        /* Long integer call */
        long ret5 = helper4(l1, l2);
        l3 = ret5 ^ l1 ^ l2;  /* l1, l2 live across call */
        
        /* Mixed call with many arguments */
        float ret6 = helper6(f1, f2, f3, f4);
        f5 = ret6 * 2.0f + f1;  /* f1 live across call */
        
        /* Volatile operations to inhibit optimization */
        v_volatile = v1 + v2;
        f_volatile = f1 + f2;
        
        /* More complex dependency chain */
        v2 = v3 * v4 + v5;
        v3 = v6 / (v7 + 1);
        v4 = v8 ^ v9 | v10;
        v5 = v1 + i;
        
        f3 = f4 * f5 - f1;
        f4 = f2 / (f3 + 0.1f);
        
        d1 = d2 * 1.5 - d3;
        d2 = d1 + 0.25;
        
        l1 = l2 << 2;
        l2 = l3 >> 1;
        l3 = l1 | l2;
    }
    
    /* Compute checksum to prevent dead code elimination */
    long checksum = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    checksum += (long)(f1 + f2 + f3 + f4 + f5);
    checksum += (long)(d1 + d2 + d3);
    checksum += l1 + l2 + l3;
    
    printf("Checksum: %ld\n", checksum);
    
    return 0;
}
