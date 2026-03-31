/* test_optabs_high_operand.c
 * Test program to cover 10/11 operand switch cases in optabs.cc
 * Compile with: gcc -O3 -march=native -fno-tree-vectorize -fprofile-arcs -ftest-coverage test_optabs_high_operand.c -o test_optabs
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Define vector types for portability */
typedef float v4sf __attribute__((vector_size(16)));
typedef int v4si __attribute__((vector_size(16)));
typedef float v8sf __attribute__((vector_size(32)));
typedef int v8si __attribute__((vector_size(32)));

/* Prevent optimization and ensure expansion */
#define NOINLINE __attribute__((noinline, noipa, used))

/* Volatile sink to prevent dead code elimination */
static volatile float volatile_sink;

/* Pattern A: Complex vector shuffle with many operands */
NOINLINE v4sf pattern_a_shuffle(v4sf a, v4sf b, v4sf c, v4sf d, 
                                int mask1, int mask2, int mask3, int mask4) {
    /* Create complex shuffle pattern that may expand to many operands */
    v4sf temp1 = __builtin_shuffle(a, b, (v4si){mask1, mask2, mask3, mask4});
    v4sf temp2 = __builtin_shuffle(c, d, (v4si){mask4, mask3, mask2, mask1});
    
    /* Mix with arithmetic to create dependencies */
    v4sf result = temp1 * temp2 + a - b;
    result = result + c * d;
    
    return result;
}

/* Pattern B: FMA chain creating deep expression tree */
NOINLINE float pattern_b_fma_chain(float a, float b, float c, float d,
                                   float e, float f, float g, float h,
                                   float i, float j, float k, float l) {
    /* Chain of FMA operations - may flatten to many operands */
    float res1 = __builtin_fmaf(a, b, c);
    float res2 = __builtin_fmaf(d, e, __builtin_fmaf(f, g, h));
    float res3 = __builtin_fmaf(i, j, __builtin_fmaf(k, l, res1));
    
    return res1 + res2 + res3;
}

/* Pattern C: Vector reduction with explicit scalarization */
NOINLINE float pattern_c_vector_reduce(v4sf v1, v4sf v2, v4sf v3, v4sf v4) {
    /* Manually extract and sum all elements - creates many extract operations */
    float sum = 0.0f;
    
    /* Extract each element (each extract is an operation) */
    sum += v1[0] + v1[1] + v1[2] + v1[3];
    sum += v2[0] + v2[1] + v2[2] + v2[3];
    sum += v3[0] + v3[1] + v3[2] + v3[3];
    sum += v4[0] + v4[1] + v4[2] + v4[3];
    
    /* Additional arithmetic to create more operands */
    sum = sum * 2.0f - (v1[0] * v2[1]) + (v3[2] * v4[3]);
    
    return sum;
}

/* Pattern D: Conditional vector operations with many comparisons */
NOINLINE v4sf pattern_d_conditional_select(v4sf a, v4sf b, v4sf c, v4sf d,
                                           v4sf e, v4sf f) {
    /* Multiple comparisons and blends */
    v4si cmp1 = a > b;
    v4si cmp2 = c < d;
    v4si cmp3 = e == f;
    
    /* Combine masks - each operation adds operands */
    v4si mask = (cmp1 & cmp2) | cmp3;
    
    /* Conditional select based on complex mask */
    v4sf result = __builtin_shuffle(a, b, mask);
    result = result + __builtin_shuffle(c, d, ~mask);
    
    return result;
}

/* Pattern E: Inline assembly with exactly 11 operands */
NOINLINE int pattern_e_multi_operand_asm(int a, int b, int c, int d, int e,
                                         int f, int g, int h, int i, int j) {
    int result1, result2;
    
    /* Inline asm with 11 total operands (2 outputs, 9 inputs) */
    asm volatile (
        "add %[r1], %[a], %[b]\n\t"
        "add %[r1], %[r1], %[c]\n\t"
        "add %[r1], %[r1], %[d]\n\t"
        "add %[r2], %[e], %[f]\n\t"
        "add %[r2], %[r2], %[g]\n\t"
        "mul %[r1], %[r1], %[r2]\n\t"
        "add %[r1], %[r1], %[h]\n\t"
        "add %[r1], %[r1], %[i]\n\t"
        "sub %[r1], %[r1], %[j]"
        : [r1] "=r" (result1), [r2] "=r" (result2)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c),
          [d] "r" (d), [e] "r" (e), [f] "r" (f),
          [g] "r" (g), [h] "r" (h), [i] "r" (i),
          [j] "r" (j)
        : "cc"
    );
    
    return result1 + result2;
}

/* Pattern F: Vector blend with dynamic mask calculation */
NOINLINE v4sf pattern_f_complex_blend(v4sf a, v4sf b, v4sf c, v4sf d,
                                      float t1, float t2, float t3, float t4) {
    /* Create dynamic blend mask from multiple conditions */
    v4sf thresholds = {t1, t2, t3, t4};
    v4si mask = a > thresholds;
    
    /* Blend using the mask - may expand to many operations */
    v4sf blended = __builtin_shuffle(a, b, mask);
    
    /* Additional blending with other vectors */
    v4si mask2 = c < d;
    v4sf final = __builtin_shuffle(blended, c, mask2);
    
    /* Arithmetic to ensure all operands are used */
    final = final * a + b - c * d;
    
    return final;
}

/* Main test driver with runtime variability */
int main(int argc, char *argv[]) {
    float result = 0.0f;
    
    /* Initialize test vectors with some variability */
    v4sf v1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf v2 = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf v3 = {9.0f, 10.0f, 11.0f, 12.0f};
    v4sf v4 = {13.0f, 14.0f, 15.0f, 16.0f};
    
    /* Use argc to select different patterns at compile time */
    switch (argc % 6) {
        case 0:
            result = pattern_a_shuffle(v1, v2, v3, v4, 1, 3, 0, 2)[0];
            break;
        case 1:
            result = pattern_b_fma_chain(1.1f, 2.2f, 3.3f, 4.4f,
                                        5.5f, 6.6f, 7.7f, 8.8f,
                                        9.9f, 10.1f, 11.11f, 12.12f);
            break;
        case 2:
            result = pattern_c_vector_reduce(v1, v2, v3, v4);
            break;
        case 3:
            result = pattern_d_conditional_select(v1, v2, v3, v4, v1, v2)[0];
            break;
        case 4:
            result = (float)pattern_e_multi_operand_asm(1, 2, 3, 4, 5,
                                                       6, 7, 8, 9, 10);
            break;
        case 5:
            result = pattern_f_complex_blend(v1, v2, v3, v4,
                                           5.0f, 10.0f, 15.0f, 20.0f)[0];
            break;
    }
    
    /* Use result to prevent optimization */
    volatile_sink = result;
    
    /* Print result to ensure code is executed */
    printf("Result: %f\n", result);
    
    return 0;
}

/* Fallback definitions for compilers without certain builtins */
#ifndef __has_builtin
#define __has_builtin(x) 0
#endif

#if !__has_builtin(__builtin_fmaf) && !defined(__builtin_fmaf)
float __builtin_fmaf(float a, float b, float c) {
    return a * b + c;
}
#endif

#if !__has_builtin(__builtin_shuffle) && !defined(__builtin_shuffle)
v4sf __builtin_shuffle(v4sf a, v4sf b, v4si mask) {
    v4sf result;
    for (int i = 0; i < 4; i++) {
        int idx = mask[i] & 0x7;
        result[i] = (idx < 4) ? a[idx] : b[idx - 4];
    }
    return result;
}
#endif
