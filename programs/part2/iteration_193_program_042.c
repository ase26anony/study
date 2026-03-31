/* test_optabs_high_operand_count.c
 * This program is designed to trigger the 10 and 11 operand switch cases
 * in optabs.cc during RTL expansion.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Define vector types for various architectures */
#ifdef __SSE__
#include <xmmintrin.h>
typedef __m128 v4sf;
typedef __m128i v4si;
#else
/* Fallback definitions if SSE not available */
typedef float v4sf __attribute__((vector_size(16)));
typedef int v4si __attribute__((vector_size(16)));
#endif

#ifdef __AVX__
#include <immintrin.h>
typedef __m256 v8sf;
#endif

/* Prevent optimization and ensure expansion */
#define NOINLINE __attribute__((noinline, noipa, used))

/* Volatile sink to prevent dead code elimination */
static volatile int sink;

/* Pattern A: Complex vector shuffle with many operands */
NOINLINE static v4sf pattern_a_shuffle(v4sf a, v4sf b, v4sf c, v4sf d,
                                       int imm1, int imm2, int imm3, int imm4)
{
    /* This creates many operands through multiple shuffle operations */
    v4sf t1 = __builtin_shuffle(a, b, (v4si){imm1, imm2, imm3, imm4});
    v4sf t2 = __builtin_shuffle(c, d, (v4si){imm4, imm3, imm2, imm1});
    v4sf t3 = __builtin_shuffle(t1, t2, (v4si){imm2, imm1, imm4, imm3});
    
    /* Mix with arithmetic to create more operands */
    return t1 + t2 * t3;
}

/* Pattern B: Fused multiply-add chain */
NOINLINE static float pattern_b_fma_chain(float a, float b, float c, float d,
                                          float e, float f, float g, float h,
                                          float i, float j, float k)
{
    /* Deep FMA chain that may expand to many operands */
    float t1 = __builtin_fmaf(a, b, c);
    float t2 = __builtin_fmaf(d, e, f);
    float t3 = __builtin_fmaf(g, h, i);
    float t4 = __builtin_fmaf(t1, t2, t3);
    return __builtin_fmaf(t4, j, k);
}

/* Pattern C: Vector reduction with explicit scalarization */
NOINLINE static float pattern_c_vector_reduce(v4sf v1, v4sf v2, v4sf v3, v4sf v4)
{
    /* Extract each element - creates many extract operations */
    float a1 = ((float*)&v1)[0] + ((float*)&v1)[1];
    float a2 = ((float*)&v1)[2] + ((float*)&v1)[3];
    float b1 = ((float*)&v2)[0] + ((float*)&v2)[1];
    float b2 = ((float*)&v2)[2] + ((float*)&v2)[3];
    float c1 = ((float*)&v3)[0] + ((float*)&v3)[1];
    float c2 = ((float*)&v3)[2] + ((float*)&v3)[3];
    float d1 = ((float*)&v4)[0] + ((float*)&v4)[1];
    float d2 = ((float*)&v4)[2] + ((float*)&v4)[3];
    
    /* Combine all - this creates a complex expression tree */
    return (a1 * a2) + (b1 * b2) - (c1 * c2) / (d1 * d2);
}

/* Pattern D: Conditional vector operations */
NOINLINE static v4sf pattern_d_conditional_select(v4sf a, v4sf b, v4sf c,
                                                  v4sf d, v4sf mask1,
                                                  v4sf mask2, v4sf mask3)
{
    /* Complex conditional logic with many vector operands */
    v4sf cmp1 = a > b;
    v4sf cmp2 = c < d;
    v4sf cmp3 = (a + b) == (c - d);
    
    v4sf blend1 = (cmp1 & mask1) | (~cmp1 & mask2);
    v4sf blend2 = (cmp2 & mask2) | (~cmp2 & mask3);
    v4sf blend3 = (cmp3 & blend1) | (~cmp3 & blend2);
    
    return a * blend1 + b * blend2 + c * blend3;
}

/* Pattern E: Inline assembly with many operands */
NOINLINE static int pattern_e_multi_operand_asm(int a, int b, int c, int d,
                                                int e, int f, int g, int h,
                                                int i, int j, int k)
{
    int result1, result2;
    
    /* 11-operand inline assembly */
    asm volatile (
        "mov %[r1], %[a]\n\t"
        "add %[r1], %[b]\n\t"
        "imul %[r1], %[c]\n\t"
        "add %[r1], %[d]\n\t"
        "sub %[r1], %[e]\n\t"
        "and %[r1], %[f]\n\t"
        "or %[r1], %[g]\n\t"
        "xor %[r1], %[h]\n\t"
        "add %[r1], %[i]\n\t"
        "sub %[r1], %[j]\n\t"
        "mov %[r2], %[r1]\n\t"
        "add %[r2], %[k]"
        : [r1] "=&r" (result1), [r2] "=&r" (result2)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j), [k] "r" (k)
        : "cc"
    );
    
    return result1 + result2;
}

/* Pattern F: Vector permute with immediate calculation */
NOINLINE static v4sf pattern_f_complex_permute(v4sf v0, v4sf v1, v4sf v2,
                                               v4sf v3, int* indices)
{
    /* Dynamic index calculation creates many operands */
    v4sf t0 = __builtin_shuffle(v0, v1, 
        (v4si){indices[0], indices[1], indices[2], indices[3]});
    v4sf t1 = __builtin_shuffle(v2, v3,
        (v4si){indices[4], indices[5], indices[6], indices[7]});
    
    /* More operations to increase operand count */
    v4sf t2 = t0 + t1;
    v4sf t3 = t0 - t1;
    v4sf t4 = t0 * t1;
    v4sf t5 = t2 / (t3 + v4sf{1.0f, 1.0f, 1.0f, 1.0f});
    
    return __builtin_shuffle(t4, t5,
        (v4si){indices[8] & 3, indices[9] & 3, indices[10] & 3, indices[11] & 3});
}

/* Main test driver */
int main(int argc, char** argv)
{
    /* Use argc for runtime variability to prevent constant folding */
    int seed = argc > 1 ? atoi(argv[1]) : 42;
    srand(seed);
    
    /* Initialize test data */
    v4sf v1 = {1.0f + rand() % 100, 2.0f, 3.0f, 4.0f};
    v4sf v2 = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf v3 = {9.0f, 10.0f, 11.0f, 12.0f};
    v4sf v4 = {13.0f, 14.0f, 15.0f, 16.0f};
    
    v4sf mask1 = {0xFFFFFFFF, 0, 0xFFFFFFFF, 0};
    v4sf mask2 = {0, 0xFFFFFFFF, 0, 0xFFFFFFFF};
    v4sf mask3 = {0xFFFFFFFF, 0xFFFFFFFF, 0, 0};
    
    int indices[12];
    for (int i = 0; i < 12; i++) {
        indices[i] = rand() % 8;
    }
    
    float result = 0.0f;
    
    /* Execute all patterns to trigger various expansion paths */
    if (seed % 5 == 0) {
        /* Pattern A - vector shuffle */
        v4sf r = pattern_a_shuffle(v1, v2, v3, v4, 
                                  indices[0], indices[1], indices[2], indices[3]);
        result += ((float*)&r)[0] + ((float*)&r)[1] + ((float*)&r)[2] + ((float*)&r)[3];
    }
    
    if (seed % 5 == 1) {
        /* Pattern B - FMA chain (11 scalar operands) */
        float r = pattern_b_fma_chain(1.1f, 2.2f, 3.3f, 4.4f, 5.5f,
                                      6.6f, 7.7f, 8.8f, 9.9f, 10.1f, 11.11f);
        result += r;
    }
    
    if (seed % 5 == 2) {
        /* Pattern C - vector reduction */
        float r = pattern_c_vector_reduce(v1, v2, v3, v4);
        result += r;
    }
    
    if (seed % 5 == 3) {
        /* Pattern D - conditional select */
        v4sf r = pattern_d_conditional_select(v1, v2, v3, v4, mask1, mask2, mask3);
        result += ((float*)&r)[0] + ((float*)&r)[3];
    }
    
    if (seed % 5 == 4) {
        /* Pattern E - inline assembly with 11 operands */
        int r = pattern_e_multi_operand_asm(seed, seed+1, seed+2, seed+3,
                                           seed+4, seed+5, seed+6, seed+7,
                                           seed+8, seed+9, seed+10);
        result += (float)r;
    }
    
    /* Always execute pattern F - complex permute */
    v4sf rf = pattern_f_complex_permute(v1, v2, v3, v4, indices);
    result += ((float*)&rf)[0] + ((float*)&rf)[1];
    
    /* Use result to prevent optimization */
    sink = (int)result;
    printf("Result: %f\n", result);
    
    return (result > 0.0f) ? 0 : 1;
}
