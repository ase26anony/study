/* Main test file to trigger early rematerialization pseudo-register replacement */
#include <stdint.h>
#include <stdio.h>

/* Forward declarations for helper functions */
struct MultiArg {
    int a, b, c, d;
    float e, f;
    double g, h;
};

/* Helper functions defined in separate compilation unit */
struct MultiArg __attribute__((noinline)) helper1(int a, int b, float c, double d);
struct MultiArg __attribute__((noinline)) helper2(long a, double b, int c, float d);
double __attribute__((noinline)) helper3(struct MultiArg arg1, struct MultiArg arg2);
float __attribute__((noinline)) helper4(int a, int b, int c, int d, int e, int f);

/* Vector types for additional register pressure */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Volatile variables to prevent optimization */
volatile int g_volatile_counter = 1000;
volatile int g_volatile_seed = 12345;

/* Main test function with dense computation */
__attribute__((noinline, optimize("no-inline")))
double test_function(int iterations) {
    /* Declare many local variables of different types */
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10;
    float f1, f2, f3, f4, f5, f6, f7, f8;
    double d1, d2, d3, d4, d5, d6, d7, d8;
    long l1, l2, l3, l4, l5;
    v4si v1, v2, v3, v4;
    v4sf vf1, vf2, vf3;
    
    /* Initialize with volatile to prevent constant propagation */
    a1 = g_volatile_seed;
    f1 = (float)g_volatile_seed * 0.5f;
    d1 = (double)g_volatile_seed * 0.25;
    l1 = (long)g_volatile_seed * 1000L;
    
    /* Vector initialization */
    v1 = (v4si){a1, a1 + 1, a1 + 2, a1 + 3};
    vf1 = (v4sf){f1, f1 * 2.0f, f1 * 3.0f, f1 * 4.0f};
    
    /* Complex chain of interdependent computations */
    /* This creates many pseudo-registers with multiple uses */
    
    /* Block 1: Integer operations with serial dependencies */
    a2 = a1 * 3 + 7;
    a3 = a2 / 2 - a1;          /* a2 used as operand */
    a4 = a3 * a2 + a1;         /* a3 and a2 used as operands */
    a5 = (a4 << 3) | (a3 >> 2); /* a4 and a3 used */
    a6 = a5 ^ a4 ^ a3 ^ a2;    /* Multiple uses of previous values */
    
    /* Block 2: Floating point operations */
    f2 = f1 * 3.14159f + 2.71828f;
    f3 = f2 / f1 - 1.0f;       /* f2 used as operand */
    f4 = f3 * f2 + f1;         /* f3 and f2 used */
    f5 = f4 - f3 * f2 / f1;    /* Multiple uses */
    
    /* Block 3: Double precision operations */
    d2 = d1 * 1.23456789;
    d3 = d2 + d1 / 2.0;        /* d2 used as operand */
    d4 = d3 * d2 - d1;         /* d3 and d2 used */
    d5 = d4 / d3 + d2 * d1;    /* Multiple uses */
    
    /* Block 4: Long integer operations */
    l2 = l1 * 37L;
    l3 = l2 + l1 / 3L;         /* l2 used as operand */
    l4 = l3 * l2 - l1;         /* l3 and l2 used */
    l5 = l4 ^ l3 | l2 & l1;    /* Multiple uses */
    
    /* Block 5: Vector operations - use wide registers */
    v2 = v1 * 2 + (v4si){1, 2, 3, 4};
    v3 = v2 + v1 >> 1;         /* v2 used as operand */
    v4 = v3 * v2 - v1;         /* v3 and v2 used */
    
    vf2 = vf1 * 2.5f + (v4sf){0.1f, 0.2f, 0.3f, 0.4f};
    vf3 = vf2 / vf1 - 1.0f;    /* vf2 used as operand */
    
    /* Inline assembly to clobber physical registers and increase pressure */
    /* Clobber multiple registers to force pseudo-register usage */
    asm volatile(
        "# Artificial register pressure\n"
        "mov r0, %0\n"
        "mov r1, %1\n"
        "add r2, r0, r1\n"
        : 
        : "r" (a6), "r" (a5)
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", 
          "r8", "r9", "r10", "r11", "r12", "memory"
    );
    
    /* More computations after assembly to ensure pseudo-registers are live */
    a7 = a6 + a5 - a4 * a3;
    a8 = a7 | a6 & a5 ^ a4;
    a9 = a8 * 17 + a7 / 3;
    a10 = a9 - a8 + a7 - a6;
    
    f6 = f5 * 2.0f - f4 / 3.0f + f3;
    f7 = f6 + f5 * f4 - f3;
    f8 = f7 / f6 + f5 - f4;
    
    d6 = d5 * 1.5 - d4 / 2.0 + d3;
    d7 = d6 + d5 * d4 - d3;
    d8 = d7 / d6 + d5 - d4;
    
    /* Call helper functions to create inter-procedural pressure */
    struct MultiArg arg1 = helper1(a10, a9, f8, d8);
    struct MultiArg arg2 = helper2(l5, d7, a8, f7);
    
    /* More computations with results */
    double result1 = helper3(arg1, arg2);
    float result2 = helper4(a10, a9, a8, a7, a6, a5);
    
    /* Final computation using all temporaries */
    double final_result = 
        (double)a10 + (double)a9 * 0.1 + (double)a8 * 0.01 +
        (double)a7 * 0.001 + (double)a6 * 0.0001 +
        (double)f8 + (double)f7 * 0.1 + (double)f6 * 0.01 +
        d8 + d7 * 0.1 + d6 * 0.01 +
        (double)l5 * 0.00001 +
        result1 + (double)result2;
    
    /* Extract and sum vector elements */
    int *vp = (int*)&v4;
    float *vfp = (float*)&vf3;
    for (int i = 0; i < 4; i++) {
        final_result += (double)vp[i] * 0.000001;
        final_result += (double)vfp[i] * 0.0000001;
    }
    
    return final_result;
}

int main() {
    double total = 0.0;
    int iterations = g_volatile_counter;
    
    printf("Starting early rematerialization test...\n");
    
    /* Loop to increase chances of triggering the pass */
    for (int i = 0; i < iterations; i++) {
        /* Modify volatile to prevent loop unrolling */
        g_volatile_seed = i * 1103515245 + 12345;
        
        /* Call test function repeatedly */
        double result = test_function(i);
        total += result;
        
        /* Prevent dead code elimination */
        if (i % 100 == 0) {
            printf("Iteration %d: result = %f\n", i, result);
        }
    }
    
    printf("Total: %f\n", total);
    printf("Test completed.\n");
    
    return (int)(total * 0.000001);
}
