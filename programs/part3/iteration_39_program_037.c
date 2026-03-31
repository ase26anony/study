/* Main test file to trigger early rematerialization pseudo-register replacement */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Force compiler to use many pseudo-registers */
#define FORCE_REGISTER_PRESSURE 1

/* External helper functions from second compilation unit */
extern struct MultiArg helper1(struct MultiArg a, struct MultiArg b);
extern struct MultiArg helper2(struct MultiArg a, struct MultiArg b);
extern struct MultiArg helper3(struct MultiArg a, struct MultiArg b);

/* Complex structure to force register pressure */
struct MultiArg {
    int a;
    long b;
    float c;
    double d;
    int64_t e;
};

/* Vector types for additional register pressure */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Volatile variables to prevent optimization */
volatile int g_volatile_counter = 1000;
volatile int g_volatile_seed = 42;

/* Noinline function to create register pressure across calls */
__attribute__((noinline, noipa))
static long test_function(int seed) {
    /* Declare many local variables of different types */
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10;
    long b1, b2, b3, b4, b5;
    float c1, c2, c3, c4, c5;
    double d1, d2, d3, d4, d5;
    int64_t e1, e2, e3;
    
    /* Vector variables */
    v4si v1, v2, v3, v4;
    v4sf vf1, vf2, vf3;
    v2df vd1, vd2;
    
    /* Initialize with volatile to prevent constant propagation */
    a1 = seed + g_volatile_seed;
    a2 = a1 * 3;
    a3 = a2 - 17;
    a4 = a3 / 2;
    a5 = a4 ^ 0x55AA55AA;
    
    /* Create chain of dependent operations */
    b1 = (long)a1 * a2;
    b2 = b1 + a3;
    b3 = b2 - a4;
    b4 = b3 * a5;
    b5 = b4 / (a1 + 1);
    
    /* Floating point operations */
    c1 = (float)a1 / 3.14159f;
    c2 = c1 * 2.71828f;
    c3 = c2 - c1;
    c4 = c3 + c2;
    c5 = c4 * c1;
    
    /* Double precision operations */
    d1 = (double)b1 / 3.141592653589793;
    d2 = d1 * 2.718281828459045;
    d3 = d2 - d1;
    d4 = d3 + d2;
    d5 = d4 * d1;
    
    /* 64-bit integer operations */
    e1 = (int64_t)a1 * b1;
    e2 = e1 + (int64_t)a2 * b2;
    e3 = e2 - (int64_t)a3 * b3;
    
    /* Vector operations - create many temporary vector registers */
    v1 = (v4si){a1, a2, a3, a4};
    v2 = (v4si){a5, a1, a2, a3};
    v3 = v1 + v2;
    v4 = v1 * v2 - v3;
    
    vf1 = (v4sf){c1, c2, c3, c4};
    vf2 = (v4sf){c5, c1, c2, c3};
    vf3 = vf1 * vf2 + vf1;
    
    vd1 = (v2df){d1, d2};
    vd2 = (v2df){d3, d4};
    vd1 = vd1 * vd2 - (v2df){d5, d1};
    
    /* Critical section: operations designed to create pseudo-register 
       with multiple uses that might trigger replacement */
    int temp1 = a1 + a2;
    int temp2 = temp1 * a3;      /* temp1 used here */
    int temp3 = temp2 - a4;      /* temp2 used here */
    int temp4 = temp3 ^ temp1;   /* temp1 used again - creates multiple refs */
    int temp5 = temp4 / temp2;   /* temp2 used again */
    
    /* More chains with intermixed types */
    float ftemp1 = (float)temp1 / c1;
    float ftemp2 = ftemp1 * c2 + (float)temp2;
    float ftemp3 = ftemp2 - ftemp1 * c3;  /* ftemp1 used twice */
    
    double dtemp1 = (double)temp3 * d1;
    double dtemp2 = dtemp1 / d2 + (double)temp4;
    double dtemp3 = dtemp2 * dtemp1 - d3; /* dtemp1 used twice */
    
    /* Artificial register pressure with inline asm */
    asm volatile (
        "/* Clobber physical registers to force pseudo-register usage */\n\t"
        "nop"
        : 
        : 
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "memory"
    );
    
    /* Create struct arguments for helper calls */
    struct MultiArg arg1 = {a1, b1, c1, d1, e1};
    struct MultiArg arg2 = {a2, b2, c2, d2, e2};
    struct MultiArg arg3 = {a3, b3, c3, d3, e3};
    
    /* Call helpers to create inter-procedural pressure */
    struct MultiArg res1 = helper1(arg1, arg2);
    struct MultiArg res2 = helper2(arg2, arg3);
    struct MultiArg res3 = helper3(res1, res2);
    
    /* More operations using results */
    a6 = res1.a + res2.a;
    a7 = a6 * res3.a;
    a8 = a7 - temp5;
    a9 = a8 ^ (int)ftemp3;
    a10 = a9 / (int)(dtemp3 * 100.0);
    
    /* Final complex computation using all temporaries */
    long result = (long)a10 * b5 
                + (long)temp1 * temp2 
                + (long)(ftemp1 * 1000.0f)
                + (long)(dtemp1 * 1000.0)
                + v3[0] + v3[1] + v3[2] + v3[3]
                + (long)vf3[0] + (long)vf3[1]
                + (long)vd1[0] + (long)vd1[1]
                + res1.a + res2.a + res3.a;
    
    /* Force serial evaluation with volatile */
    asm volatile ("" : : "r"(result) : "memory");
    
    return result;
}

/* Hot loop to repeatedly call test function */
int main() {
    long total = 0;
    int iterations = g_volatile_counter;
    
    printf("Starting early rematerialization test with %d iterations\n", iterations);
    
    for (int i = 0; i < iterations; i++) {
        /* Vary seed to prevent complete optimization */
        int seed = g_volatile_seed + i;
        
        /* Call test function - should create massive register pressure */
        long result = test_function(seed);
        
        /* Use result to prevent dead code elimination */
        total += result;
        
        /* Occasionally update volatile to prevent loop unrolling */
        if (i % 100 == 0) {
            asm volatile ("" : : "r"(total) : "memory");
        }
    }
    
    printf("Test completed. Total: %ld\n", total);
    
    /* Return non-zero to ensure all code paths are considered */
    return (total != 0) ? 0 : 1;
}
