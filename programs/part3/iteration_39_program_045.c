/* Main test file to trigger early rematerialization replacement logic */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Forward declarations for helper functions */
struct MultiReg {
    int a, b, c, d;
    float e, f;
    double g, h;
};

struct MultiReg __attribute__((noinline)) helper1(int a, float b, double c);
struct MultiReg __attribute__((noinline)) helper2(long a, double b, int c);
struct MultiReg __attribute__((noinline)) helper3(struct MultiReg in1, struct MultiReg in2);
float __attribute__((noinline)) helper4(struct MultiReg m);
double __attribute__((noinline)) helper5(int a, int b, int c, int d, int e, int f);

/* Vector types for register pressure */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Volatile variables to prevent optimization */
volatile int g_volatile_counter = 1000;
volatile float g_volatile_float = 3.14159f;
volatile double g_volatile_double = 2.71828;

/* Main test function with dense computation */
__attribute__((noinline, optimize("O3")))
double test_function(int seed) {
    /* Declare many local variables of different types */
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10;
    float f1, f2, f3, f4, f5, f6, f7, f8;
    double d1, d2, d3, d4, d5, d6, d7, d8;
    long l1, l2, l3, l4;
    v4si v1, v2, v3, v4;
    v4sf vf1, vf2;
    v2df vd1, vd2;
    
    /* Initialize with volatile values to prevent constant propagation */
    a1 = seed + g_volatile_counter;
    f1 = g_volatile_float + seed;
    d1 = g_volatile_double * seed;
    l1 = (long)seed * g_volatile_counter;
    
    /* Vector initialization */
    v1 = (v4si){a1, a1 + 1, a1 + 2, a1 + 3};
    vf1 = (v4sf){f1, f1 + 1.0f, f1 + 2.0f, f1 + 3.0f};
    vd1 = (v2df){d1, d1 + 1.0};
    
    /* Complex chain of interdependent computations */
    /* This creates many pseudo-registers with multiple uses */
    
    /* Block 1: Integer operations with serial dependencies */
    a2 = a1 * 3 + 7;
    a3 = a2 / 2 - a1;          /* a2 used as operand and destination */
    a4 = a3 << 3 | a2;         /* a3 used as operand */
    a5 = (a4 ^ a3) & a2;       /* a4 used as operand */
    a6 = a5 + a4 - a3 * a2;    /* a5 used as operand */
    
    /* Block 2: Floating point operations */
    f2 = f1 * 2.5f;
    f3 = f2 / 1.7f + f1;       /* f2 used as operand */
    f4 = f3 * f2 - f1;         /* f3 used as operand */
    f5 = f4 + f3 / f2;         /* f4 used as operand */
    
    /* Block 3: Double precision operations */
    d2 = d1 * 1.5;
    d3 = d2 / 0.7 + d1;        /* d2 used as operand */
    d4 = d3 * d2 - d1;         /* d3 used as operand */
    d5 = d4 + d3 / d2;         /* d4 used as operand */
    
    /* Block 4: Long integer operations */
    l2 = l1 * 3L + 11L;
    l3 = l2 / 2L - l1;         /* l2 used as operand */
    l4 = l3 << 2 | l2;         /* l3 used as operand */
    
    /* Block 5: Vector operations - use wide registers */
    v2 = v1 + (v4si){1, 2, 3, 4};
    v3 = v2 * v1 - (v4si){5, 6, 7, 8};  /* v2 used as operand */
    v4 = v3 & v2 | v1;                   /* v3 used as operand */
    
    vf2 = vf1 * (v4sf){2.0f, 3.0f, 4.0f, 5.0f};
    vd2 = vd1 + (v2df){0.5, 1.5};
    
    /* Block 6: Mixed type operations causing conversions */
    a7 = (int)(f3 * d3) + a6;
    a8 = (int)l3 + a7 * 2;
    f6 = (float)a8 / f4 + d4;
    d6 = (double)f5 * d5 + (double)a5;
    
    /* Block 7: More complex dependencies */
    a9 = ((a8 << 3) + (a7 >> 2)) * a6;
    a10 = (a9 ^ a8) & (a7 | a6);
    
    f7 = (f6 * 3.14f) / (f5 + 1.0f) - f4;
    f8 = f7 * f6 + f5 / f4;
    
    d7 = (d6 * 2.71) / (d5 + 1.0) - d4;
    d8 = d7 * d6 + d5 / d4;
    
    /* Artificial register pressure with inline assembly */
    /* Clobber many registers to force spilling */
    asm volatile (
        "# Artificial register clobber\n"
        "mov r0, %0\n"
        "mov r1, %1\n"
        : 
        : "r" (a10), "r" (f8)
        : "r0", "r1", "r2", "r3", "r4", "r5", 
          "r6", "r7", "r8", "r9", "r10", "memory"
    );
    
    /* Call helper functions to create inter-procedural pressure */
    struct MultiReg mr1 = helper1(a9, f7, d7);
    struct MultiReg mr2 = helper2(l4, d8, a10);
    
    /* Use results in further computations */
    float f_helper = helper4(mr1);
    double d_helper = helper5(mr1.a, mr1.b, mr2.a, mr2.b, a9, a10);
    
    /* Final computation using all temporaries */
    double result = 
        (double)a1 + (double)a2 + (double)a3 + (double)a4 + (double)a5 +
        (double)a6 + (double)a7 + (double)a8 + (double)a9 + (double)a10 +
        (double)f1 + (double)f2 + (double)f3 + (double)f4 + (double)f5 +
        (double)f6 + (double)f7 + (double)f8 +
        d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 +
        (double)l1 + (double)l2 + (double)l3 + (double)l4 +
        (double)v1[0] + (double)v2[1] + (double)v3[2] + (double)v4[3] +
        (double)vf1[0] + (double)vd1[0] +
        f_helper + d_helper;
    
    return result;
}

int main() {
    double total = 0.0;
    int iterations = g_volatile_counter;
    
    printf("Starting early rematerialization test...\n");
    
    /* Loop to increase compilation complexity */
    for (int i = 0; i < iterations; i++) {
        /* Use volatile to prevent loop elimination */
        if (g_volatile_counter > 0) {
            total += test_function(i);
            
            /* Mix in some conditional logic */
            if (i % 100 == 0) {
                total -= test_function(i / 2);
            }
        }
    }
    
    printf("Result: %f\n", total);
    printf("Test completed.\n");
    
    return (int)total % 256;
}
