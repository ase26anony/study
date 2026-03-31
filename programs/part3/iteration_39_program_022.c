/* Main test file to trigger early rematerialization pseudo-register replacement */
#include <stdint.h>
#include <stdio.h>

/* Forward declarations for helper functions */
struct MultiReg {
    int a, b, c, d;
    float e, f;
    double g, h;
};

struct MultiReg __attribute__((noinline)) helper1(int a, int b, float c, double d);
struct MultiReg __attribute__((noinline)) helper2(long a, long b, float c, double d);
struct MultiReg __attribute__((noinline)) helper3(int a, int b, int c, int d);
float __attribute__((noinline)) helper4(struct MultiReg m);
double __attribute__((noinline)) helper5(struct MultiReg m);

/* Vector types for additional register pressure */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Volatile variable to prevent optimization */
volatile int loop_counter = 1000;

/* Main test function with high register pressure */
__attribute__((noinline, optimize("no-inline-functions")))
double test_function(int seed) {
    /* Many local variables of different types */
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10;
    float f1, f2, f3, f4, f5, f6, f7, f8;
    double d1, d2, d3, d4, d5, d6;
    long l1, l2, l3, l4;
    v4si v1, v2, v3, v4;
    v4sf vf1, vf2, vf3;
    
    /* Initialize with seed to prevent constant propagation */
    a1 = seed;
    a2 = seed * 2;
    a3 = seed + 1;
    a4 = seed - 1;
    a5 = seed * 3;
    a6 = seed / 2;
    a7 = seed % 7;
    a8 = seed ^ 0xFF;
    a9 = seed << 2;
    a10 = seed >> 1;
    
    f1 = seed * 1.1f;
    f2 = seed * 2.2f;
    f3 = seed * 3.3f;
    f4 = seed * 4.4f;
    f5 = seed * 5.5f;
    f6 = seed * 6.6f;
    f7 = seed * 7.7f;
    f8 = seed * 8.8f;
    
    d1 = seed * 1.11;
    d2 = seed * 2.22;
    d3 = seed * 3.33;
    d4 = seed * 4.44;
    d5 = seed * 5.55;
    d6 = seed * 6.66;
    
    l1 = seed * 100L;
    l2 = seed * 200L;
    l3 = seed * 300L;
    l4 = seed * 400L;
    
    v1 = (v4si){a1, a2, a3, a4};
    v2 = (v4si){a5, a6, a7, a8};
    v3 = (v4si){a9, a10, a1, a2};
    v4 = (v4si){a3, a4, a5, a6};
    
    vf1 = (v4sf){f1, f2, f3, f4};
    vf2 = (v4sf){f5, f6, f7, f8};
    vf3 = (v4sf){f1, f3, f5, f7};
    
    /* Complex chain of interdependent computations */
    /* This creates many pseudo-registers with multiple uses */
    
    /* Chain 1: Integer operations */
    a1 = a2 + a3;           /* Pseudo-reg for a1 created */
    a4 = a1 * a5;           /* a1 used here - potential for remat */
    a6 = a4 - a7;
    a8 = a6 / a9;
    a10 = a8 ^ a1;          /* a1 used again */
    
    /* Chain 2: Float operations with dependencies */
    f1 = f2 + f3;
    f4 = f1 * f5;           /* f1 used */
    f6 = f4 - f7;
    f8 = f6 / f1;           /* f1 used again */
    
    /* Chain 3: Double operations */
    d1 = d2 + d3;
    d4 = d1 * d5;           /* d1 used */
    d6 = d4 - d1;           /* d1 used again */
    
    /* Chain 4: Long operations */
    l1 = l2 + l3;
    l4 = l1 * l2;           /* l1 used */
    l3 = l4 - l1;           /* l1 used again */
    
    /* Vector operations - use wide registers */
    v1 = v1 + v2;
    v3 = v1 * v4;           /* v1 used */
    v2 = v3 - v1;           /* v1 used again */
    
    vf1 = vf1 + vf2;
    vf3 = vf1 * vf2;        /* vf1 used */
    vf2 = vf3 - vf1;        /* vf1 used again */
    
    /* Inline assembly to clobber physical registers */
    /* This forces more pseudo-register usage */
    asm volatile (
        "# Clobber registers to increase pressure\n\t"
        "nop"
        : /* no outputs */
        : /* no inputs */
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
          "r8", "r9", "r10", "r11", "r12", "memory"
    );
    
    /* Call helper functions to create inter-procedural pressure */
    struct MultiReg mr1 = helper1(a1, a2, f1, d1);
    struct MultiReg mr2 = helper2(l1, l2, f2, d2);
    struct MultiReg mr3 = helper3(a3, a4, a5, a6);
    
    /* More computations using helper results */
    float f_result = helper4(mr1) + helper4(mr2) + helper4(mr3);
    double d_result = helper5(mr1) + helper5(mr2) + helper5(mr3);
    
    /* Final complex computation using all temporaries */
    double result = 
        (a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10) * 0.1 +
        (f1 + f2 + f3 + f4 + f5 + f6 + f7 + f8) * 0.2 +
        (d1 + d2 + d3 + d4 + d5 + d6) * 0.3 +
        (l1 + l2 + l3 + l4) * 0.4 +
        f_result * 0.5 +
        d_result * 0.6;
    
    /* Vector reduction */
    int vsum = 0;
    for (int i = 0; i < 4; i++) {
        vsum += v1[i] + v2[i] + v3[i] + v4[i];
    }
    
    float vfsum = 0.0f;
    for (int i = 0; i < 4; i++) {
        vfsum += vf1[i] + vf2[i] + vf3[i];
    }
    
    result += vsum * 0.01 + vfsum * 0.02;
    
    return result;
}

int main() {
    double total = 0.0;
    int iterations = loop_counter;
    
    printf("Starting early rematerialization test...\n");
    
    /* Loop to increase compilation complexity */
    for (int i = 0; i < iterations; i++) {
        /* Use volatile to prevent loop unrolling */
        volatile int seed = i;
        total += test_function(seed);
        
        /* Prevent optimization of loop */
        asm volatile("" : : : "memory");
    }
    
    printf("Result: %f\n", total);
    printf("Test completed.\n");
    
    return (int)(total) % 256;
}
