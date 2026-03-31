/* Main test driver with register pressure */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Volatile to prevent optimization */
volatile int g_iterations = 1000;

/* Forward declarations for helper functions */
struct LargeStruct {
    int a, b, c, d;
    float e, f;
    double g, h;
};

/* Vector types for register pressure */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Helper functions in separate compilation unit */
struct LargeStruct __attribute__((noinline)) helper1(int a, int b, float c, double d);
v4si __attribute__((noinline)) vector_op1(v4si a, v4si b, v4si c);
v4sf __attribute__((noinline)) vector_op2(v4sf a, v4sf b, v4sf c);
double __attribute__((noinline)) complex_calc(double a, double b, double c, double d);

/* Main test function designed to create maximum register pressure */
int __attribute__((noinline, optimize("O3"))) test_function(int seed) {
    /* Declare many variables of different types to use pseudo-registers */
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10;
    float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    long l1, l2, l3, l4, l5;
    
    /* Vector variables for wide register pressure */
    v4si v1, v2, v3, v4, v5;
    v4sf vf1, vf2, vf3, vf4;
    v2df vd1, vd2, vd3;
    
    /* Initialize with seed to prevent constant propagation */
    a1 = seed;
    f1 = seed * 1.1f;
    d1 = seed * 1.234567;
    l1 = seed * 1000L;
    
    /* Create complex dependency chain to force serial evaluation */
    /* This creates many pseudo-registers for intermediate values */
    
    /* Integer chain - each depends on previous */
    a2 = a1 * 3 + 7;
    a3 = a2 / 2 - a1;
    a4 = a3 * a2 + a1;
    a5 = a4 ^ a3 | a2;
    a6 = a5 << 3;
    a7 = a6 >> 1;
    a8 = a7 + a6 - a5;
    a9 = a8 * a7 / (a6 + 1);
    a10 = a9 % (a8 + 2);
    
    /* Float chain with mixed operations */
    f2 = f1 * 2.3f + 1.0f;
    f3 = f2 / 1.7f - f1;
    f4 = f3 * f2 + f1;
    f5 = f4 / (f3 + 0.1f);
    f6 = f5 * 3.14159f;
    f7 = f6 + f5 - f4;
    f8 = f7 * f6 / f5;
    f9 = f8 - f7 + f6;
    f10 = f9 * 2.0f;
    
    /* Double precision chain */
    d2 = d1 * 1.234 + 5.678;
    d3 = d2 / 2.345 - d1;
    d4 = d3 * d2 + d1;
    d5 = d4 / (d3 + 0.001);
    d6 = d5 * 3.14159265358979;
    d7 = d6 + d5 - d4;
    d8 = d7 * d6 / d5;
    d9 = d8 - d7 + d6;
    d10 = d9 * 2.0;
    
    /* Long integer chain */
    l2 = l1 * 3L + 7L;
    l3 = l2 / 2L - l1;
    l4 = l3 * l2 + l1;
    l5 = l4 ^ l3 | l2;
    
    /* Vector operations - these use wide registers */
    v1 = (v4si){a1, a2, a3, a4};
    v2 = (v4si){a5, a6, a7, a8};
    v3 = v1 + v2;
    v4 = v1 * v2 - v3;
    v5 = v4 >> 1;
    
    vf1 = (v4sf){f1, f2, f3, f4};
    vf2 = (v4sf){f5, f6, f7, f8};
    vf3 = vf1 + vf2;
    vf4 = vf1 * vf2 - vf3;
    
    vd1 = (v2df){d1, d2};
    vd2 = (v2df){d3, d4};
    vd3 = vd1 + vd2;
    
    /* Artificial register pressure with inline assembly */
    /* Clobber many registers to force spilling */
    asm volatile (
        "# Artificial register pressure\n"
        "mov r0, %0\n"
        "mov r1, %1\n"
        "add r2, r0, r1\n"
        :
        : "r" (a10), "r" (l5)
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
          "r8", "r9", "r10", "r11", "r12", "memory"
    );
    
    /* More complex operations with function calls */
    /* These create cross-register dependencies */
    struct LargeStruct ls = helper1(a10, l5, f10, d10);
    
    /* Vector operations that might trigger rematerialization */
    v4si v6 = vector_op1(v3, v4, v5);
    v4sf vf5 = vector_op2(vf3, vf4, (v4sf){f9, f10, f1, f2});
    double d11 = complex_calc(d6, d7, d8, d9);
    
    /* Create a loop-like pattern with adjacent uses */
    /* This is designed to create DF_REF patterns that might trigger replacement */
    int temp1, temp2, temp3, temp4, temp5;
    
    /* Chain where each result is used immediately in next operation */
    temp1 = a1 + a2 + a3;
    temp2 = temp1 * f1;  /* temp1 used as operand and destination */
    temp3 = temp2 - d1;
    temp4 = temp3 | l1;
    temp5 = temp4 ^ temp1;  /* Mix earlier result */
    
    /* More chains to increase pressure */
    float ftemp1, ftemp2, ftemp3;
    ftemp1 = f1 + f2 + f3;
    ftemp2 = ftemp1 * d1;
    ftemp3 = ftemp2 / temp2;
    
    double dtemp1, dtemp2, dtemp3;
    dtemp1 = d1 + d2 + d3;
    dtemp2 = dtemp1 * f1;
    dtemp3 = dtemp2 / temp3;
    
    /* Final computation using all variables to keep them live */
    int result = 
        a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10 +
        (int)f1 + (int)f2 + (int)f3 + (int)f4 + (int)f5 +
        (int)f6 + (int)f7 + (int)f8 + (int)f9 + (int)f10 +
        (int)d1 + (int)d2 + (int)d3 + (int)d4 + (int)d5 +
        (int)d6 + (int)d7 + (int)d8 + (int)d9 + (int)d10 +
        ls.a + ls.b + ls.c + ls.d +
        v6[0] + v6[1] + v6[2] + v6[3] +
        (int)vf5[0] + (int)vf5[1] + (int)vf5[2] + (int)vf5[3] +
        (int)d11 + temp5 + (int)ftemp3 + (int)dtemp3;
    
    return result;
}

int main() {
    int total = 0;
    int iterations = g_iterations;
    
    printf("Starting early rematerialization test...\n");
    
    /* Loop to increase compilation complexity */
    for (int i = 0; i < iterations; i++) {
        /* Use volatile to prevent loop unrolling */
        volatile int seed = i * 1234567;
        total += test_function(seed);
        
        /* Prevent optimization of loop */
        asm volatile("" : : : "memory");
    }
    
    printf("Result: %d\n", total);
    return total != 0 ? 0 : 1;
}
