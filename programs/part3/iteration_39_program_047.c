/* Main test driver with volatile loop control */
#include <stdint.h>
#include <stdio.h>

/* External helper functions */
extern struct Vec4 helper1(int a, int b, int c, int d);
extern struct Vec4 helper2(struct Vec4 v1, struct Vec4 v2);
extern struct Vec4 helper3(struct Vec4 v, float scalar);
extern double helper4(struct Vec4 v);

/* Vector type for wide register pressure */
typedef float v4f __attribute__((vector_size(16)));
typedef int v4i __attribute__((vector_size(16)));

/* Complex struct for parameter passing pressure */
struct Vec4 {
    float x, y, z, w;
    int flags;
};

/* Volatile to prevent optimization */
volatile int g_iterations = 100;

/* NOINLINE test function with massive register pressure */
__attribute__((noinline, noipa))
float test_function(int seed) {
    /* Declare many variables of different types */
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10;
    float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    double d1, d2, d3, d4, d5;
    long l1, l2, l3, l4;
    v4f vec1, vec2, vec3, vec4;
    v4i ivec1, ivec2;
    
    /* Initialize with seed */
    a1 = seed;
    f1 = seed * 0.5f;
    d1 = seed * 0.25;
    l1 = seed * 100L;
    
    /* Complex interdependent computations - forces many pseudo-registers */
    /* First chain: integer operations */
    a2 = a1 * 3 + 7;
    a3 = a2 / 2 - a1;
    a4 = a3 << 3;
    a5 = a4 ^ a2;
    a6 = a5 | a3;
    a7 = a6 & 0xFF;
    a8 = a7 + a4 - a2;
    a9 = a8 * a3 / (a1 + 1);
    a10 = a9 % 17;
    
    /* Second chain: floating point operations */
    f2 = f1 * 2.0f + 1.0f;
    f3 = f2 / 1.5f - f1;
    f4 = f3 * f2;
    f5 = f4 / (f3 + 0.001f);
    f6 = f5 * f4 - f3;
    f7 = f6 + f2 * 0.5f;
    f8 = f7 / (f4 + 1.0f);
    f9 = f8 * f5 - f6;
    f10 = f9 + f1;
    
    /* Third chain: double precision */
    d2 = d1 * 1.5 + 0.25;
    d3 = d2 / 0.75 - d1;
    d4 = d3 * d2 + d1;
    d5 = d4 / (d3 + 0.0001);
    
    /* Fourth chain: long integers */
    l2 = l1 * 3L + 1000L;
    l3 = l2 / 2L - l1;
    l4 = l3 << 2;
    
    /* Vector operations - uses wide registers */
    vec1 = (v4f){f1, f2, f3, f4};
    vec2 = (v4f){f5, f6, f7, f8};
    vec3 = vec1 + vec2;
    vec4 = vec1 * vec2 - vec3;
    
    ivec1 = (v4i){a1, a2, a3, a4};
    ivec2 = (v4i){a5, a6, a7, a8};
    ivec1 = ivec1 + ivec2;
    
    /* Critical pattern: variable used as both operand and destination
       This creates the DF_REF pattern we want to trigger */
    float temp = f1;
    temp = temp * f2 + f3;  /* temp used as source and dest */
    f1 = temp * 0.5f;       /* f1 used again */
    temp = f1 + f4;         /* Another use pattern */
    
    /* More complex use patterns to increase pseudo-register pressure */
    int chain1 = a1;
    chain1 = chain1 + a2 * 2;
    int chain2 = chain1;
    chain2 = chain2 - a3 / 3;
    int chain3 = chain2;
    chain3 = chain3 | a4;
    int chain4 = chain3;
    chain4 = chain4 & a5;
    
    /* Artificial register pressure via inline asm clobber */
    asm volatile("" 
                 : 
                 : 
                 : "r0", "r1", "r2", "r3", "r4", "r5", 
                   "r6", "r7", "r8", "r9", "r10", "r11",
                   "r12", "memory");
    
    /* Call helper functions for inter-procedural pressure */
    struct Vec4 v1 = helper1(a1, a2, a3, a4);
    struct Vec4 v2 = helper2(v1, (struct Vec4){f1, f2, f3, f4, a10});
    struct Vec4 v3 = helper3(v2, f10);
    double result_d = helper4(v3);
    
    /* Final computation using all variables to keep them live */
    float final_result = 
        (a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10) * 0.01f +
        (f1 + f2 + f3 + f4 + f5 + f6 + f7 + f8 + f9 + f10) +
        (float)(d1 + d2 + d3 + d4 + d5) * 0.1f +
        (float)(l1 + l2 + l3 + l4) * 0.001f +
        vec1[0] + vec2[1] + vec3[2] + vec4[3] +
        (float)ivec1[0] * 0.01f +
        (float)result_d +
        temp + (float)chain4 * 0.001f;
    
    return final_result;
}

int main() {
    float total = 0.0f;
    int iterations = g_iterations;
    
    /* Loop to increase compilation complexity */
    for (int i = 0; i < iterations; i++) {
        /* Mix different seeds to create varying register pressure patterns */
        total += test_function(i);
        total += test_function(i * 3 + 1);
        total += test_function(i * 7 - 2);
    }
    
    /* Use result to prevent dead code elimination */
    if (total > 1000.0f) {
        printf("Result: %f\n", total);
    }
    
    return 0;
}
