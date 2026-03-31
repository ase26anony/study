/* main.c - Primary test file with hot loop and register pressure */
#include <stdint.h>
#include <stdio.h>

/* Forward declarations from helper.c */
struct MultiReg {
    int a, b, c, d;
    float e, f;
    double g, h;
};

struct MultiReg __attribute__((noinline)) helper1(int a, int b, float c, double d);
struct MultiReg __attribute__((noinline)) helper2(long a, long b, float c, double d);
struct MultiReg __attribute__((noinline)) helper3(int a, float b, double c, long d);
double __attribute__((noinline)) helper4(struct MultiReg m1, struct MultiReg m2);

/* Volatile to prevent optimization */
volatile int loop_counter = 1000;

/* Vector types for register pressure */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Test function with extreme register pressure */
double __attribute__((noinline)) test_function(int seed) {
    /* Many local variables of different types */
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10;
    float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    long l1, l2, l3, l4, l5, l6, l7, l8, l9, l10;
    
    /* Vector variables */
    v4si v1, v2, v3, v4, v5;
    v4sf vf1, vf2, vf3, vf4, vf5;
    v2df vd1, vd2, vd3, vd4, vd5;
    
    /* Initialize with seed */
    a1 = seed;
    f1 = seed * 1.1f;
    d1 = seed * 1.1;
    l1 = seed * 1000L;
    
    /* Complex chain of interdependent computations */
    /* First chain: integer operations */
    a2 = a1 * 3 + 7;
    a3 = a2 / 2 - a1;
    a4 = a3 ^ a2;
    a5 = a4 | a3;
    a6 = a5 & a4;
    a7 = a6 << 3;
    a8 = a7 >> 1;
    a9 = a8 % 17;
    a10 = a9 * a8 + a7 - a6;
    
    /* Second chain: float operations */
    f2 = f1 * 2.3f + 1.1f;
    f3 = f2 / 1.7f - f1;
    f4 = f3 * f2;
    f5 = f4 / f3;
    f6 = f5 + f4 - f3;
    f7 = f6 * 2.0f;
    f8 = f7 / 3.0f;
    f9 = f8 + f7 - f6;
    f10 = f9 * f8 / f7;
    
    /* Third chain: double operations */
    d2 = d1 * 3.14159 + 2.71828;
    d3 = d2 / 1.41421 - d1;
    d4 = d3 * d2;
    d5 = d4 / d3;
    d6 = d5 + d4 - d3;
    d7 = d6 * 1.61803;
    d8 = d7 / 2.30258;
    d9 = d8 + d7 - d6;
    d10 = d9 * d8 / d7;
    
    /* Fourth chain: long operations */
    l2 = l1 * 3L + 7L;
    l3 = l2 / 2L - l1;
    l4 = l3 ^ l2;
    l5 = l4 | l3;
    l6 = l5 & l4;
    l7 = l6 << 3;
    l8 = l7 >> 1;
    l9 = l8 % 17L;
    l10 = l9 * l8 + l7 - l6;
    
    /* Vector operations - creates wide register pressure */
    v1 = (v4si){a1, a2, a3, a4};
    v2 = (v4si){a5, a6, a7, a8};
    v3 = v1 + v2;
    v4 = v1 * v2;
    v5 = v3 - v4;
    
    vf1 = (v4sf){f1, f2, f3, f4};
    vf2 = (v4sf){f5, f6, f7, f8};
    vf3 = vf1 + vf2;
    vf4 = vf1 * vf2;
    vf5 = vf3 - vf4;
    
    vd1 = (v2df){d1, d2};
    vd2 = (v2df){d3, d4};
    vd3 = vd1 + vd2;
    vd4 = vd1 * vd2;
    vd5 = vd3 - vd4;
    
    /* Artificial register pressure via inline assembly */
    /* Clobber many registers to force pseudo-register usage */
    asm volatile (
        "# Artificial register clobber\n"
        "mov r0, %0\n"
        "mov r1, %1\n"
        :
        : "r" (a10), "r" (l10)
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
          "r8", "r9", "r10", "r11", "r12", "memory"
    );
    
    /* More interdependent operations after assembly */
    int b1 = a10 + l10;
    float b2 = f10 + d10;
    double b3 = d10 * f10;
    long b4 = l10 ^ a10;
    
    /* Critical pattern: pseudo-register used as both source and dest */
    /* This creates the DF_REF pattern we want to trigger */
    int temp1 = b1 * 2;
    int temp2 = temp1 + b1;      /* temp1 used as source */
    temp1 = temp2 / 3;           /* temp1 used as dest, creating multiple refs */
    temp2 = temp1 * 4;
    temp1 = temp2 - temp1;       /* Another source/dest pattern */
    
    float ftemp1 = b2 * 1.5f;
    float ftemp2 = ftemp1 + b2;
    ftemp1 = ftemp2 / 2.0f;
    ftemp2 = ftemp1 * 3.0f;
    ftemp1 = ftemp2 - ftemp1;
    
    /* Call helper functions to increase inter-procedural pressure */
    struct MultiReg m1 = helper1(temp1, temp2, ftemp1, b3);
    struct MultiReg m2 = helper2(b4, l10, ftemp2, d10);
    struct MultiReg m3 = helper3(a10, f10, d10, l10);
    
    /* More computations with struct results */
    double result1 = helper4(m1, m2);
    double result2 = helper4(m2, m3);
    double result3 = helper4(m3, m1);
    
    /* Final complex computation using all temporaries */
    double final_result = 
        (a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10) * 0.01 +
        (f1 + f2 + f3 + f4 + f5 + f6 + f7 + f8 + f9 + f10) * 0.1 +
        (d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9 + d10) +
        (l1 + l2 + l3 + l4 + l5 + l6 + l7 + l8 + l9 + l10) * 0.001 +
        result1 + result2 + result3 +
        v5[0] + v5[1] + v5[2] + v5[3] +
        vf5[0] + vf5[1] + vf5[2] + vf5[3] +
        vd5[0] + vd5[1] +
        b1 + b2 + b3 + b4 +
        temp1 + temp2 + ftemp1 + ftemp2;
    
    return final_result;
}

int main() {
    double total = 0.0;
    int iterations = loop_counter;
    
    /* Hot loop to trigger optimization passes */
    for (int i = 0; i < iterations; i++) {
        total += test_function(i);
        
        /* Prevent loop unrolling */
        asm volatile ("# Loop barrier" : : : "memory");
    }
    
    /* Use result to prevent dead code elimination */
    volatile double sink __attribute__((unused)) = total;
    
    printf("Result: %f\n", total);
    return 0;
}
