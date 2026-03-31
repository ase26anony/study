/* Main test driver with hot loop to induce register pressure */
#include <stdint.h>
#include <stdlib.h>

/* Volatile to prevent optimization */
volatile int g_iterations = 1000;

/* Vector types for register pressure */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Complex struct for inter-procedural pressure */
struct MultiReg {
    v4si vec_int;
    v4sf vec_float;
    v2df vec_double;
    long long ints[4];
    double floats[4];
};

/* Forward declarations for helper functions in other file */
struct MultiReg __attribute__((noinline)) helper1(struct MultiReg a, struct MultiReg b);
struct MultiReg __attribute__((noinline)) helper2(struct MultiReg a, struct MultiReg b, struct MultiReg c);
v4si __attribute__((noinline)) vector_op(v4si a, v4si b, v4si c);
v2df __attribute__((noinline)) double_vector_op(v2df a, v2df b);

/* Main test function with dense computations */
long long __attribute__((noinline, optimize("O3"))) test_function(int seed) {
    /* Declare many variables of different types */
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10;
    float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    long long l1, l2, l3, l4, l5, l6, l7, l8, l9, l10;
    v4si v1, v2, v3, v4, v5;
    v4sf vf1, vf2, vf3;
    v2df vd1, vd2, vd3;
    
    /* Initialize with seed to prevent constant propagation */
    a1 = seed * 1; a2 = seed * 2; a3 = seed * 3; a4 = seed * 4; a5 = seed * 5;
    a6 = seed * 6; a7 = seed * 7; a8 = seed * 8; a9 = seed * 9; a10 = seed * 10;
    
    f1 = seed * 1.1f; f2 = seed * 2.2f; f3 = seed * 3.3f; f4 = seed * 4.4f;
    f5 = seed * 5.5f; f6 = seed * 6.6f; f7 = seed * 7.7f; f8 = seed * 8.8f;
    f9 = seed * 9.9f; f10 = seed * 10.1f;
    
    d1 = seed * 1.11; d2 = seed * 2.22; d3 = seed * 3.33; d4 = seed * 4.44;
    d5 = seed * 5.55; d6 = seed * 6.66; d7 = seed * 7.77; d8 = seed * 8.88;
    d9 = seed * 9.99; d10 = seed * 10.11;
    
    l1 = seed * 100LL; l2 = seed * 200LL; l3 = seed * 300LL; l4 = seed * 400LL;
    l5 = seed * 500LL; l6 = seed * 600LL; l7 = seed * 700LL; l8 = seed * 800LL;
    l9 = seed * 900LL; l10 = seed * 1000LL;
    
    /* Vector initialization */
    v1 = (v4si){a1, a2, a3, a4};
    v2 = (v4si){a5, a6, a7, a8};
    v3 = (v4si){a9, a10, a1, a2};
    v4 = (v4si){a3, a4, a5, a6};
    v5 = (v4si){a7, a8, a9, a10};
    
    vf1 = (v4sf){f1, f2, f3, f4};
    vf2 = (v4sf){f5, f6, f7, f8};
    vf3 = (v4sf){f9, f10, f1, f2};
    
    vd1 = (v2df){d1, d2};
    vd2 = (v2df){d3, d4};
    vd3 = (v2df){d5, d6};
    
    /* Long chain of interdependent computations */
    /* This creates many pseudo-registers with multiple uses */
    
    /* Chain 1: Integer operations */
    a1 = a2 + a3;      /* Pseudo-reg for a2 used here */
    a4 = a1 * a5;      /* a1 used as operand and destination in chain */
    a6 = a4 - a7;
    a8 = a6 / (a9 ? a9 : 1);
    a10 = a8 ^ a1;     /* a1 used again */
    
    /* Chain 2: Float operations with dependencies */
    f1 = f2 * f3;
    f4 = f1 + f5;      /* f1 used as operand */
    f6 = f4 - f7;
    f8 = f6 * f9;
    f10 = f8 / f1;     /* f1 used again */
    
    /* Chain 3: Double operations */
    d1 = d2 + d3;
    d4 = d1 * d5;      /* d1 used */
    d6 = d4 - d7;
    d8 = d6 / d9;
    d10 = d8 + d1;     /* d1 used again */
    
    /* Chain 4: Long long operations */
    l1 = l2 + l3;
    l4 = l1 * l5;      /* l1 used */
    l6 = l4 - l7;
    l8 = l6 / (l9 ? l9 : 1);
    l10 = l8 ^ l1;     /* l1 used again */
    
    /* Vector operations - use wide registers */
    v1 = v1 + v2;
    v3 = v1 * v4;      /* v1 used */
    v5 = v3 - v2;
    
    vf1 = vf1 + vf2;
    vf3 = vf1 * vf2;   /* vf1 used */
    
    vd1 = vd1 + vd2;
    vd3 = vd1 * vd2;   /* vd1 used */
    
    /* Artificial register pressure with inline assembly */
    /* Clobber many physical registers */
    asm volatile(
        "# Artificial register pressure\n"
        "mov r0, %0\n"
        "mov r1, %1\n"
        "add r2, r0, r1\n"
        :
        : "r" (a1), "r" (a2)
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
          "r8", "r9", "r10", "r11", "r12", "memory"
    );
    
    /* More complex interdependent computations */
    for (int i = 0; i < 4; i++) {
        a1 = a1 + a2 * a3;
        a2 = a2 - a4 / (a5 ? a5 : 1);
        a3 = a3 ^ a6;
        a4 = a4 | a7;
        a5 = a5 & a8;
        
        /* Use the same variable as both source and destination */
        f1 = f1 * f2 + f3;     /* f1 used as source and destination */
        f2 = f2 - f4 * f5;
        f3 = f3 / f6 + f7;
        
        d1 = d1 + d2 * d3;     /* d1 used as source and destination */
        d2 = d2 - d4 / d5;
        
        l1 = l1 ^ l2 | l3;     /* l1 used as source and destination */
        l2 = l2 + l4 * l5;
    }
    
    /* Call helper functions for inter-procedural pressure */
    struct MultiReg s1 = {v1, vf1, vd1, {l1, l2, l3, l4}, {d1, d2, d3, d4}};
    struct MultiReg s2 = {v2, vf2, vd2, {l5, l6, l7, l8}, {d5, d6, d7, d8}};
    struct MultiReg s3 = {v3, vf3, vd3, {l9, l10, l1, l2}, {d9, d10, d1, d2}};
    
    struct MultiReg r1 = helper1(s1, s2);
    struct MultiReg r2 = helper2(s1, s2, s3);
    
    /* Vector function calls */
    v1 = vector_op(v1, v2, v3);
    vd1 = double_vector_op(vd1, vd2);
    
    /* Final computation using all variables to ensure they're live */
    long long result = 
        (long long)a1 + (long long)a2 + (long long)a3 + (long long)a4 + (long long)a5 +
        (long long)a6 + (long long)a7 + (long long)a8 + (long long)a9 + (long long)a10 +
        (long long)f1 + (long long)f2 + (long long)f3 + (long long)f4 + (long long)f5 +
        (long long)f6 + (long long)f7 + (long long)f8 + (long long)f9 + (long long)f10 +
        (long long)d1 + (long long)d2 + (long long)d3 + (long long)d4 + (long long)d5 +
        (long long)d6 + (long long)d7 + (long long)d8 + (long long)d9 + (long long)d10 +
        l1 + l2 + l3 + l4 + l5 + l6 + l7 + l8 + l9 + l10 +
        v1[0] + v1[1] + v1[2] + v1[3] +
        (long long)vf1[0] + (long long)vf1[1] + (long long)vf1[2] + (long long)vf1[3] +
        (long long)vd1[0] + (long long)vd1[1] +
        r1.vec_int[0] + r2.vec_int[1];
    
    return result;
}

int main() {
    volatile int seed = 42;
    long long total = 0;
    
    /* Hot loop to trigger optimization passes */
    for (int i = 0; i < g_iterations; i++) {
        total += test_function(seed + i);
        
        /* Prevent loop unrolling from reducing pressure */
        if (i % 7 == 0) {
            asm volatile("" : : : "memory");
        }
    }
    
    /* Use result to prevent dead code elimination */
    if (total > 0) {
        return 0;
    } else {
        return 1;
    }
}
