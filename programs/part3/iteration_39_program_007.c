/* Main test driver with register pressure */
#include <stdint.h>
#include <stdio.h>

/* Force no optimization on these helpers */
#define NOINLINE __attribute__((noinline))
#define VOLATILE_VAR(x) volatile int x

/* Vector types for register pressure */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Complex struct for inter-procedural pressure */
struct MultiReg {
    v4si vec;
    double dbl;
    long long ll;
    int i;
    float f;
};

/* External helper functions */
NOINLINE struct MultiReg helper1(struct MultiReg a, struct MultiReg b);
NOINLINE struct MultiReg helper2(struct MultiReg a, struct MultiReg b, struct MultiReg c);
NOINLINE v4si vector_op(v4si a, v4si b, v4si c);
NOINLINE double mixed_ops(double a, double b, float c, int d, long e);

/* Volatile to prevent optimization */
VOLATILE_VAR(iterations) = 100;

/* Main test function with high register pressure */
NOINLINE long long test_function(int seed) {
    /* Declare many variables of different types */
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10;
    float f1, f2, f3, f4, f5, f6, f7, f8;
    double d1, d2, d3, d4, d5, d6;
    long l1, l2, l3, l4, l5;
    v4si v1, v2, v3, v4, v5;
    v4sf vf1, vf2, vf3;
    v2df vd1, vd2;
    
    /* Initialize with seed */
    a1 = seed;
    f1 = seed * 1.1f;
    d1 = seed * 2.2;
    l1 = seed * 3;
    
    /* Complex chain of interdependent computations */
    /* This creates many pseudo-registers with multiple uses */
    
    /* Chain 1: Integer operations */
    a2 = a1 * 3 + 7;
    a3 = a2 / 2 - a1;
    a4 = a3 << 3 | a2;
    a5 = (a4 ^ a3) & 0xFF;
    a6 = a5 * a4 - a3 + a2;
    a7 = a6 % 17 + a5;
    a8 = a7 << 1 >> 2;
    a9 = a8 * 3 + a7 - a6;
    a10 = a9 & 0xFFFF | a8;
    
    /* Chain 2: Float operations with dependencies on integers */
    f2 = f1 * a2 + 1.5f;
    f3 = f2 / a3 - 0.25f;
    f4 = f3 * f2 + a4;
    f5 = f4 - f3 * 2.0f;
    f6 = f5 / (a5 + 1.0f);
    f7 = f6 * 3.14159f + a6;
    f8 = f7 - f6 / 2.0f;
    
    /* Chain 3: Double operations */
    d2 = d1 * a7 + f2;
    d3 = d2 / (a8 + 1.0) - f3;
    d4 = d3 * 2.71828 + a9;
    d5 = d4 - d3 / 3.0;
    d6 = d5 * d4 + f7;
    
    /* Chain 4: Long operations */
    l2 = l1 * a10 + (long)d2;
    l3 = l2 >> 2 | (long)a3;
    l4 = l3 * 3 - l2;
    l5 = l4 + (long)f8 * 2;
    
    /* Vector operations - use wide registers */
    v1 = (v4si){a1, a2, a3, a4};
    v2 = (v4si){a5, a6, a7, a8};
    v3 = v1 + v2 * 2;
    v4 = v3 - v1 >> 1;
    v5 = v4 * v3 | v2;
    
    vf1 = (v4sf){f1, f2, f3, f4};
    vf2 = (v4sf){f5, f6, f7, f8};
    vf3 = vf1 * 1.5f + vf2;
    
    vd1 = (v2df){d1, d2};
    vd2 = (v2df){d3, d4};
    vd1 = vd1 * 2.0 + vd2;
    
    /* Artificial register pressure with inline asm */
    /* Clobber many registers to force spilling */
    asm volatile (
        "# Artificial register pressure\n"
        "mov r0, %0\n"
        "mov r1, %1\n"
        : 
        : "r" (a10), "r" (l5)
        : "r0", "r1", "r2", "r3", "r4", "r5", 
          "r6", "r7", "r8", "r9", "r10", "memory"
    );
    
    /* More computations after asm to force rematerialization */
    int b1 = a10 + l5;
    int b2 = b1 * 2 - a9;
    float b3 = f8 + b2;
    double b4 = d6 * b3;
    long b5 = l5 + (long)b4;
    
    /* Create struct for inter-procedural calls */
    struct MultiReg s1 = {v5, d5, l5, a10, f8};
    struct MultiReg s2 = {v4, d4, l4, a9, f7};
    struct MultiReg s3 = {v3, d3, l3, a8, f6};
    
    /* Call helper functions - creates cross-function pressure */
    struct MultiReg r1 = helper1(s1, s2);
    struct MultiReg r2 = helper2(s1, s2, s3);
    
    /* Use vector operations */
    v4si vr = vector_op(v1, v2, v3);
    
    /* More mixed operations */
    double dr = mixed_ops(d1, d2, f1, a1, l1);
    
    /* Final computation using all variables */
    long long result = 
        (long long)a10 + (long long)l5 + 
        (long long)(f8 * 1000) + (long long)(d6 * 1000) +
        vr[0] + vr[1] + vr[2] + vr[3] +
        (long long)r1.ll + (long long)r2.ll +
        (long long)(dr * 1000) + b5;
    
    /* Another asm to prevent optimization */
    asm volatile (
        "# Final barrier\n"
        : 
        : 
        : "memory"
    );
    
    return result;
}

int main() {
    long long total = 0;
    int i;
    
    /* Loop to increase execution time and coverage */
    for (i = 0; i < iterations; i++) {
        total += test_function(i);
        
        /* Prevent loop unrolling */
        asm volatile (
            "# Loop barrier\n"
            : 
            : 
            : "memory"
        );
    }
    
    /* Use result to prevent dead code elimination */
    printf("Result: %lld\n", total);
    
    return (int)(total % 1000);
}
