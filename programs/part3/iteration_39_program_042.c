/* Main test driver with register pressure */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* External helper functions */
extern struct LargeStruct helper1(int a, int b, int c, int d);
extern struct LargeStruct helper2(struct LargeStruct s1, struct LargeStruct s2);
extern double helper3(struct LargeStruct s, float f, double d);

/* Volatile to prevent optimization */
volatile int g_volatile_counter = 1000;

/* Vector types for register pressure */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Complex struct to force register pressure across calls */
struct LargeStruct {
    v4si vec_int;
    v4sf vec_float;
    v2df vec_double;
    long long extra[2];
};

/* Force noinline to prevent optimization */
__attribute__((noinline, optimize("O0")))
static struct LargeStruct create_pressure(int base) {
    struct LargeStruct s;
    
    /* Initialize vectors */
    s.vec_int = (v4si){base, base+1, base+2, base+3};
    s.vec_float = (v4sf){base*1.1f, base*1.2f, base*1.3f, base*1.4f};
    s.vec_double = (v2df){base*2.1, base*2.2};
    s.extra[0] = base * 1000LL;
    s.extra[1] = base * 2000LL;
    
    return s;
}

/* Main test function with dense computations */
__attribute__((noinline))
double test_function(int iterations) {
    /* Many local variables of different types */
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10;
    float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    long l1, l2, l3, l4, l5;
    v4si v1, v2, v3, v4;
    v4sf vf1, vf2, vf3, vf4;
    v2df vd1, vd2, vd3;
    
    /* Initialize with volatile to prevent constant propagation */
    volatile int seed = iterations;
    a1 = seed + 1; a2 = seed + 2; a3 = seed + 3; a4 = seed + 4; a5 = seed + 5;
    a6 = seed + 6; a7 = seed + 7; a8 = seed + 8; a9 = seed + 9; a10 = seed + 10;
    
    f1 = seed * 1.1f; f2 = seed * 1.2f; f3 = seed * 1.3f; f4 = seed * 1.4f;
    f5 = seed * 1.5f; f6 = seed * 1.6f; f7 = seed * 1.7f; f8 = seed * 1.8f;
    f9 = seed * 1.9f; f10 = seed * 2.0f;
    
    d1 = seed * 2.1; d2 = seed * 2.2; d3 = seed * 2.3; d4 = seed * 2.4;
    d5 = seed * 2.5; d6 = seed * 2.6; d7 = seed * 2.7; d8 = seed * 2.8;
    d9 = seed * 2.9; d10 = seed * 3.0;
    
    l1 = seed * 100L; l2 = seed * 200L; l3 = seed * 300L; 
    l4 = seed * 400L; l5 = seed * 500L;
    
    /* Vector initialization */
    v1 = (v4si){a1, a2, a3, a4};
    v2 = (v4si){a5, a6, a7, a8};
    v3 = (v4si){a9, a10, a1, a2};
    v4 = (v4si){a3, a4, a5, a6};
    
    vf1 = (v4sf){f1, f2, f3, f4};
    vf2 = (v4sf){f5, f6, f7, f8};
    vf3 = (v4sf){f9, f10, f1, f2};
    vf4 = (v4sf){f3, f4, f5, f6};
    
    vd1 = (v2df){d1, d2};
    vd2 = (v2df){d3, d4};
    vd3 = (v2df){d5, d6};
    
    /* Dense chain of interdependent computations */
    /* This creates many pseudo-registers with multiple uses */
    
    /* Chain 1: Integer operations */
    a1 = a2 + a3;      /* Pseudo-reg for a1 used here */
    a4 = a1 * a5;      /* a1 used again immediately */
    a6 = a4 - a7;
    a8 = a6 / (a1 + 1); /* a1 used third time */
    a9 = a8 | a2;
    a10 = a9 & a3;
    
    /* Chain 2: Float operations with dependencies */
    f1 = f2 * f3;
    f4 = f1 + f5;      /* f1 used as operand */
    f6 = f4 - f7;
    f8 = f6 * f1;      /* f1 used again */
    f9 = f8 / f2;
    f10 = f9 + f1;     /* f1 used third time */
    
    /* Chain 3: Double operations - creates pressure */
    d1 = d2 + d3;
    d4 = d1 * d5;      /* d1 used */
    d6 = d4 - d7;
    d8 = d6 / d1;      /* d1 used again */
    d9 = d8 + d2;
    d10 = d9 * d1;     /* d1 used third time */
    
    /* Chain 4: Mixed operations */
    l1 = (long)a1 * l2;
    l3 = l1 + (long)f1;
    l4 = l3 * (long)d1;
    l5 = l4 / (a1 + 1); /* a1 used again */
    
    /* Vector operations - use wide registers */
    v1 = v1 + v2;
    v3 = v1 * v4;      /* v1 used */
    v2 = v3 - v1;      /* v1 used again */
    v4 = v2 / (v1 + 1); /* v1 used third time */
    
    vf1 = vf1 + vf2;
    vf3 = vf1 * vf4;   /* vf1 used */
    vf2 = vf3 - vf1;   /* vf1 used again */
    vf4 = vf2 / (vf1 + 1.0f); /* vf1 used third time */
    
    vd1 = vd1 + vd2;
    vd3 = vd1 * vd2;   /* vd1 used */
    vd2 = vd3 - vd1;   /* vd1 used again */
    
    /* Artificial register pressure with inline asm */
    /* Clobber many physical registers */
    asm volatile (
        "# Artificial register pressure\n"
        "mov r0, %0\n"
        "mov r1, %1\n"
        "add r0, r0, r1\n"
        : 
        : "r" (a1), "r" (a2)
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
          "r8", "r9", "r10", "r11", "r12", "memory"
    );
    
    /* More computations after asm clobber */
    a1 = a1 + a10;
    f1 = f1 * f10;
    d1 = d1 - d10;
    
    /* Call helper functions for inter-procedural pressure */
    struct LargeStruct s1 = create_pressure(a1);
    struct LargeStruct s2 = create_pressure(a2);
    
    /* Use helper functions from other compilation unit */
    struct LargeStruct s3 = helper1(a1, a2, a3, a4);
    struct LargeStruct s4 = helper2(s1, s2);
    double result = helper3(s3, f1, d1);
    
    /* Final computation using all variables */
    double final_result = 
        (double)a1 + (double)a2 + (double)a3 + (double)a4 + (double)a5 +
        (double)a6 + (double)a7 + (double)a8 + (double)a9 + (double)a10 +
        (double)f1 + (double)f2 + (double)f3 + (double)f4 + (double)f5 +
        (double)f6 + (double)f7 + (double)f8 + (double)f9 + (double)f10 +
        d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9 + d10 +
        (double)l1 + (double)l2 + (double)l3 + (double)l4 + (double)l5 +
        result;
    
    /* Use vectors in final computation */
    int sum_v1 = v1[0] + v1[1] + v1[2] + v1[3];
    int sum_v2 = v2[0] + v2[1] + v2[2] + v2[3];
    int sum_v3 = v3[0] + v3[1] + v3[2] + v3[3];
    int sum_v4 = v4[0] + v4[1] + v4[2] + v4[3];
    
    final_result += sum_v1 + sum_v2 + sum_v3 + sum_v4;
    
    return final_result;
}

int main() {
    double total = 0.0;
    int iterations = g_volatile_counter;
    
    /* Loop to increase compilation complexity */
    for (int i = 0; i < iterations; i++) {
        /* Volatile to prevent loop unrolling */
        volatile int loop_var = i;
        total += test_function(loop_var);
        
        /* Prevent dead code elimination */
        if (total > 1e100) {
            total *= 0.5;
        }
    }
    
    printf("Result: %f\n", total);
    return 0;
}
