/* test_optabs_coverage.c - Test program to cover 10/11 operand switch cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent optimizations from removing our test code */
#define KEEP(expr) do { \
    volatile static int keep_counter = 0; \
    keep_counter += (int)(expr); \
} while(0)

/* Architecture detection and fallbacks */
#if defined(__x86_64__) || defined(__i386__)
    #define TARGET_X86 1
    #include <x86intrin.h>
#elif defined(__aarch64__) || defined(__arm__)
    #define TARGET_ARM 1
    #include <arm_neon.h>
#else
    #define TARGET_GENERIC 1
#endif

/* Generic vector types for portability */
typedef float v4sf __attribute__((vector_size(16)));
typedef int v4si __attribute__((vector_size(16)));
typedef float v8sf __attribute__((vector_size(32)));
typedef int v8si __attribute__((vector_size(32)));

/* Pattern A: Complex vector shuffle with many operands */
__attribute__((noinline, noipa))
v4sf pattern_a_shuffle_many_operands(v4sf a, v4sf b, v4sf c, v4sf d, 
                                     int mask1, int mask2, int mask3, int mask4) {
    /* This should expand to many operands due to multiple shuffle operations */
    v4sf t1 = __builtin_shuffle(a, b, (v4si){mask1, mask2, mask3, mask4});
    v4sf t2 = __builtin_shuffle(c, d, (v4si){mask4, mask3, mask2, mask1});
    v4sf t3 = __builtin_shuffle(t1, t2, (v4si){mask2, mask1, mask4, mask3});
    
    /* Chain operations to create deep expression tree */
    return t1 + t2 * t3 - a / b;
}

/* Pattern B: FMA chain creating deep expression tree */
__attribute__((noinline, noipa))
float pattern_b_fma_chain(float a, float b, float c, float d, 
                         float e, float f, float g, float h) {
    /* Each FMA expands to multiple operands, chain creates deep tree */
    float t1 = __builtin_fmaf(a, b, c);
    float t2 = __builtin_fmaf(d, e, f);
    float t3 = __builtin_fmaf(g, h, t1);
    float t4 = __builtin_fmaf(t1, t2, t3);
    float t5 = __builtin_fmaf(t2, t3, t4);
    
    return __builtin_fmaf(t3, t4, t5) + __builtin_fmaf(t4, t5, t1);
}

/* Pattern C: Vector reduction with explicit scalarization */
__attribute__((noinline, noipa))
float pattern_c_vector_reduction(v4sf v1, v4sf v2, v4sf v3, v4sf v4) {
    /* Extract each element - each extract is an operation */
    float sum = 0.0f;
    
    /* Unrolled extraction and summation - creates many operands */
    sum += v1[0] + v1[1] + v1[2] + v1[3];
    sum += v2[0] + v2[1] + v2[2] + v2[3];
    sum += v3[0] + v3[1] + v3[2] + v3[3];
    sum += v4[0] + v4[1] + v4[2] + v4[3];
    
    /* Additional arithmetic to ensure expansion */
    sum = sum * 2.0f - (v1[0] * v2[1]) + (v3[2] / v4[3]);
    
    return sum;
}

/* Pattern D: Conditional vector operations with many comparisons */
__attribute__((noinline, noipa))
v4sf pattern_d_conditional_blend(v4sf a, v4sf b, v4sf c, v4sf d,
                                 v4sf mask1, v4sf mask2) {
    /* Multiple comparisons and blends */
    v4sf cmp1 = a > b;
    v4sf cmp2 = c < d;
    v4sf cmp3 = (a + b) == (c - d);
    
    /* Complex blend operations */
    v4sf t1 = __builtin_shuffle(a, b, (v4si){0, 2, 1, 3});
    v4sf t2 = __builtin_shuffle(c, d, (v4si){3, 1, 2, 0});
    
    /* Blend based on multiple conditions */
    v4sf result = (cmp1 & mask1) ? t1 : t2;
    result = (cmp2 & mask2) ? result + a : result - b;
    result = cmp3 ? result * c : result / d;
    
    return result;
}

/* Pattern E: Inline assembly with exactly 11 operands */
__attribute__((noinline, noipa))
int pattern_e_asm_many_operands(int a, int b, int c, int d, int e,
                               int f, int g, int h, int i, int j) {
    int result1, result2;
    
    /* Inline asm with 11 total operands (2 outputs, 9 inputs) */
    asm volatile (
        "add %[r1], %[a], %[b]\n\t"
        "add %[r1], %[r1], %[c]\n\t"
        "add %[r1], %[r1], %[d]\n\t"
        "add %[r2], %[e], %[f]\n\t"
        "add %[r2], %[r2], %[g]\n\t"
        "add %[r2], %[r2], %[h]\n\t"
        "mul %[r1], %[r1], %[r2]\n\t"
        "add %[r1], %[r1], %[i]\n\t"
        "add %[r1], %[r1], %[j]"
        : [r1] "=r" (result1), [r2] "=r" (result2)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c),
          [d] "r" (d), [e] "r" (e), [f] "r" (f),
          [g] "r" (g), [h] "r" (h), [i] "r" (i),
          [j] "r" (j)
        : "cc"
    );
    
    return result1 + result2;
}

/* Pattern F: Vector operations that might use expand_mult_highpart */
__attribute__((noinline, noipa))
v4si pattern_f_vector_mult_high(v4si a, v4si b, v4si c, v4si d) {
    /* Complex vector multiplications that might trigger highpart expansion */
    v4si t1 = a * b;
    v4si t2 = c * d;
    v4si t3 = t1 + t2;
    v4si t4 = t1 - t2;
    
    /* Mixed-size operations */
    v4si result = (t3 >> 2) | (t4 << 2);
    result = result & a | result & b;
    result = result ^ c ^ d;
    
    /* Additional arithmetic chain */
    return result + (t1 * 3) - (t2 / 2) + (t3 & t4);
}

/* Main test driver */
int main(int argc, char *argv[]) {
    float checksum = 0.0f;
    
    /* Use argc to add variability and ensure all paths are compiled */
    int variant = argc > 1 ? atoi(argv[1]) % 6 : 0;
    
    /* Initialize test data */
    v4sf vec1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec2 = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf vec3 = {9.0f, 10.0f, 11.0f, 12.0f};
    v4sf vec4 = {13.0f, 14.0f, 15.0f, 16.0f};
    v4si ivec1 = {1, 2, 3, 4};
    v4si ivec2 = {5, 6, 7, 8};
    v4si ivec3 = {9, 10, 11, 12};
    v4si ivec4 = {13, 14, 15, 16};
    
    /* Execute different patterns based on variant */
    switch (variant) {
        case 0:
            checksum += pattern_a_shuffle_many_operands(vec1, vec2, vec3, vec4, 0, 2, 1, 3)[0];
            break;
        case 1:
            checksum += pattern_b_fma_chain(1.1f, 2.2f, 3.3f, 4.4f, 5.5f, 6.6f, 7.7f, 8.8f);
            break;
        case 2:
            checksum += pattern_c_vector_reduction(vec1, vec2, vec3, vec4);
            break;
        case 3:
            checksum += pattern_d_conditional_blend(vec1, vec2, vec3, vec4, vec1, vec2)[0];
            break;
        case 4:
            checksum += pattern_e_asm_many_operands(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
            break;
        case 5:
            checksum += pattern_f_vector_mult_high(ivec1, ivec2, ivec3, ivec4)[0];
            break;
    }
    
    /* Additional test to ensure all patterns are compiled */
    if (argc > 2) {
        /* Force compilation of all patterns by calling them */
        KEEP(pattern_a_shuffle_many_operands(vec1, vec2, vec3, vec4, 0, 2, 1, 3)[0]);
        KEEP(pattern_b_fma_chain(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f));
        KEEP(pattern_c_vector_reduction(vec1, vec2, vec3, vec4));
        KEEP(pattern_d_conditional_blend(vec1, vec2, vec3, vec4, vec1, vec2)[0]);
        KEEP(pattern_e_asm_many_operands(1, 2, 3, 4, 5, 6, 7, 8, 9, 10));
        KEEP(pattern_f_vector_mult_high(ivec1, ivec2, ivec3, ivec4)[0]);
    }
    
    printf("Checksum: %f\n", checksum);
    return (int)checksum;
}

/* Fallback implementations for architectures without certain builtins */
#ifndef __FP_FAST_FMAF
float __builtin_fmaf(float x, float y, float z) {
    return x * y + z;
}
#endif
