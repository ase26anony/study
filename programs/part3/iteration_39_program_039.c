/* Main test driver with volatile loop control */
#include <stdint.h>
#include <stdio.h>

/* External helper functions */
extern struct Vec4 helper1(int a, int b, int c, int d);
extern struct Vec4 helper2(struct Vec4 v1, struct Vec4 v2);
extern struct Vec4 helper3(struct Vec4 v, float scalar);
extern double helper4(struct Vec4 v1, struct Vec4 v2, struct Vec4 v3);

/* Volatile to prevent optimization */
volatile int g_iterations = 100;

/* Vector types for register pressure */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Complex struct to force register pressure across calls */
struct Vec4 {
    float x, y, z, w;
};

/* NOINLINE to prevent inlining and preserve register pressure */
__attribute__((noinline)) 
float test_function(int seed) {
    /* Many local variables of different types to create register pressure */
    volatile int v0 = seed; /* volatile to prevent optimization */
    int v1 = v0 + 1;
    int v2 = v1 * 2;
    int v3 = v2 - v1;
    int v4 = v3 << 2;
    int v5 = v4 | 0xFF;
    int v6 = v5 & 0x0F;
    int v7 = v6 ^ v5;
    
    float f0 = (float)v7 * 0.5f;
    float f1 = f0 + 1.0f;
    float f2 = f1 * 2.0f;
    float f3 = f2 / f1;
    float f4 = f3 - f2;
    
    double d0 = (double)f4 * 1.5;
    double d1 = d0 + 2.0;
    double d2 = d1 * 3.0;
    double d3 = d2 / d1;
    double d4 = d3 - d2;
    
    long l0 = (long)d4 * 100L;
    long l1 = l0 + 200L;
    long l2 = l1 * 300L;
    long l3 = l2 / l1;
    long l4 = l3 - l2;
    
    /* Vector operations for wide register pressure */
    v4si vec_int = {v0, v1, v2, v3};
    v4si vec_int2 = {v4, v5, v6, v7};
    v4si vec_int3 = vec_int + vec_int2;
    v4si vec_int4 = vec_int3 * vec_int2;
    v4si vec_int5 = vec_int4 - vec_int3;
    
    v4sf vec_float = {f0, f1, f2, f3};
    v4sf vec_float2 = {f1, f2, f3, f4};
    v4sf vec_float3 = vec_float + vec_float2;
    v4sf vec_float4 = vec_float3 * vec_float2;
    v4sf vec_float5 = vec_float4 - vec_float3;
    
    v2df vec_double = {d0, d1};
    v2df vec_double2 = {d2, d3};
    v2df vec_double3 = vec_double + vec_double2;
    v2df vec_double4 = vec_double3 * vec_double2;
    v2df vec_double5 = vec_double4 - vec_double3;
    
    /* Complex interdependent computations */
    /* Chain 1: a = b + c; d = a * e pattern to create pseudo-register reuse */
    int chain_a = v0 + v1;
    int chain_b = chain_a * v2;
    int chain_c = chain_b - v3;
    int chain_d = chain_c / v4;
    int chain_e = chain_d | v5;
    int chain_f = chain_e & v6;
    int chain_g = chain_f ^ v7;
    
    float chain_h = (float)chain_g + f0;
    float chain_i = chain_h * f1;
    float chain_j = chain_i - f2;
    float chain_k = chain_j / f3;
    float chain_l = chain_k + f4;
    
    /* Inline assembly to clobber physical registers and increase pressure */
    asm volatile(
        "/* Clobber many registers to force pseudo-register usage */\n\t"
        "mov r0, %0\n\t"
        "mov r1, %1\n\t"
        "add r2, r0, r1\n\t"
        : 
        : "r"(chain_a), "r"(chain_b)
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", 
          "r8", "r9", "r10", "r11", "r12", "memory"
    );
    
    /* More computations after assembly to force reloads */
    double chain_m = (double)chain_l + d0;
    double chain_n = chain_m * d1;
    double chain_o = chain_n - d2;
    double chain_p = chain_o / d3;
    double chain_q = chain_p + d4;
    
    long chain_r = (long)chain_q + l0;
    long chain_s = chain_r * l1;
    long chain_t = chain_s - l2;
    long chain_u = chain_t / l3;
    long chain_v = chain_u + l4;
    
    /* Vector chain computations */
    v4si vec_chain1 = vec_int + vec_int2;
    v4si vec_chain2 = vec_chain1 * vec_int3;
    v4si vec_chain3 = vec_chain2 - vec_int4;
    v4si vec_chain4 = vec_chain3 | vec_int5;
    
    v4sf vec_chain5 = vec_float + vec_float2;
    v4sf vec_chain6 = vec_chain5 * vec_float3;
    v4sf vec_chain7 = vec_chain6 - vec_float4;
    v4sf vec_chain8 = vec_chain7 + vec_float5;
    
    v2df vec_chain9 = vec_double + vec_double2;
    v2df vec_chain10 = vec_chain9 * vec_double3;
    v2df vec_chain11 = vec_chain10 - vec_double4;
    v2df vec_chain12 = vec_chain11 + vec_double5;
    
    /* Call helper functions to create inter-procedural pressure */
    struct Vec4 vec1 = helper1(v0, v1, v2, v3);
    struct Vec4 vec2 = helper1(v4, v5, v6, v7);
    struct Vec4 vec3 = helper2(vec1, vec2);
    struct Vec4 vec4 = helper3(vec3, f0);
    
    double helper_result = helper4(vec1, vec2, vec3);
    
    /* Final computation using all variables to ensure they're live */
    float result = 
        (float)chain_v + 
        vec_chain1[0] + vec_chain1[1] + vec_chain1[2] + vec_chain1[3] +
        vec_chain6[0] + vec_chain6[1] + vec_chain6[2] + vec_chain6[3] +
        (float)vec_chain12[0] + (float)vec_chain12[1] +
        vec1.x + vec2.y + vec3.z + vec4.w +
        (float)helper_result;
    
    return result;
}

int main() {
    float total = 0.0f;
    int iterations = g_iterations;
    
    for (int i = 0; i < iterations; i++) {
        /* Call test function repeatedly */
        total += test_function(i);
        
        /* Prevent loop unrolling */
        asm volatile("" : : : "memory");
    }
    
    printf("Result: %f\n", total);
    return (int)total;
}
