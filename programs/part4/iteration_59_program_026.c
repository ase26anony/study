/* Test program to cover 10/11 operand cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* For x86 AVX-512 */
#ifdef __AVX512F__
#include <immintrin.h>
#endif

/* For ARM NEON */
#ifdef __ARM_NEON
#include <arm_neon.h>
#endif

/* GCC vector extensions for generic testing */
typedef float v8sf __attribute__((vector_size(32)));
typedef int v8si __attribute__((vector_size(32)));

/* Hot function attribute to encourage complex instruction patterns */
__attribute__((hot, noinline))
static float complex_vector_operations(void) {
    float result = 0.0f;
    
#ifdef __AVX512F__
    /* AVX-512 mask operations can generate many operands */
    __m512 a = _mm512_set1_ps(1.0f);
    __m512 b = _mm512_set1_ps(2.0f);
    __m512 c = _mm512_set1_ps(3.0f);
    __m512 d = _mm512_set1_ps(4.0f);
    __mmask16 k = 0xAAAA;
    
    /* Complex FMA chain with masking - may expand to many operands */
    __m512 t1 = _mm512_mask_fmadd_ps(a, k, b, c);
    __m512 t2 = _mm512_mask_fnmadd_ps(t1, k, c, d);
    __m512 t3 = _mm512_mask3_fmadd_ps(a, b, t2, k);
    
    /* Blend operations with multiple sources */
    __m512 blended = _mm512_mask_blend_ps(k, t1, t2);
    blended = _mm512_mask_blend_ps(k >> 1, blended, t3);
    
    /* Reduction */
    result = _mm512_reduce_add_ps(blended);
#endif

#ifdef __ARM_NEON
    /* ARM NEON multi-vector operations */
    float32x4x4_t vec4 = vld1q_f32_x4((const float32_t[]){
        1.0f, 2.0f, 3.0f, 4.0f,
        5.0f, 6.0f, 7.0f, 8.0f,
        9.0f, 10.0f, 11.0f, 12.0f,
        13.0f, 14.0f, 15.0f, 16.0f
    });
    
    /* Complex operations on multiple vectors */
    float32x4_t sum = vaddq_f32(vec4.val[0], vec4.val[1]);
    sum = vmlaq_f32(sum, vec4.val[2], vec4.val[3]);
    sum = vfmaq_f32(sum, vaddq_f32(vec4.val[0], vec4.val[2]), 
                    vsubq_f32(vec4.val[1], vec4.val[3]));
    
    /* Horizontal reduction */
    result = vaddvq_f32(sum);
#endif

    /* GCC vector extensions with complex expressions */
    v8sf v1 = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    v8sf v2 = {8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f};
    v8sf v3 = {2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f};
    v8sf v4 = {9.0f, 8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f};
    
    /* Complex expression that might expand to multi-operand pattern */
    v8sf complex_expr = v1 * v2 + v3 * v4 - (v1 + v2) * (v3 - v4);
    
    /* Force use of all vectors in a way that prevents optimization */
    for (int i = 0; i < 8; i++) {
        result += complex_expr[i];
    }
    
    return result;
}

/* Inline assembly with many operands */
__attribute__((noinline))
static uint64_t multi_operand_asm(void) {
    uint64_t a = 1, b = 2, c = 3, d = 4, e = 5;
    uint64_t f = 6, g = 7, h = 8, i = 9, j = 10;
    uint64_t k = 11, result = 0;
    
    /* 11-operand inline assembly */
    asm volatile (
        "add %[a], %[b], %[tmp1]\n\t"
        "add %[c], %[d], %[tmp2]\n\t"
        "add %[e], %[f], %[tmp3]\n\t"
        "add %[g], %[h], %[tmp4]\n\t"
        "add %[i], %[j], %[tmp5]\n\t"
        "add %[tmp1], %[tmp2], %[tmp6]\n\t"
        "add %[tmp3], %[tmp4], %[tmp7]\n\t"
        "add %[tmp5], %[k], %[tmp8]\n\t"
        "add %[tmp6], %[tmp7], %[tmp9]\n\t"
        "add %[tmp8], %[tmp9], %[result]"
        : [result] "=r" (result),
          [tmp1] "=&r" (a), [tmp2] "=&r" (b), [tmp3] "=&r" (c),
          [tmp4] "=&r" (d), [tmp5] "=&r" (e), [tmp6] "=&r" (f),
          [tmp7] "=&r" (g), [tmp8] "=&r" (h), [tmp9] "=&r" (i)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j), [k] "r" (k)
        : "cc"
    );
    
    return result;
}

/* Complex built-in usage */
__attribute__((noinline))
static double complex_builtins(double x, double y, double z) {
    /* Chain of FMA operations that might expand to multi-operand patterns */
    double t1 = __builtin_fma(x, y, z);
    double t2 = __builtin_fma(y, z, x);
    double t3 = __builtin_fma(z, x, y);
    double t4 = __builtin_fma(t1, t2, t3);
    double t5 = __builtin_fma(t2, t3, t1);
    
    /* Complex expression preventing early folding */
    return __builtin_fma(t4, t5, __builtin_fma(x, y, __builtin_fma(y, z, __builtin_fma(z, x, 0.0))));
}

/* OpenMP SIMD reduction with vector types */
__attribute__((noinline))
static float omp_vector_reduction(void) {
    float sum = 0.0f;
    float data[1024];
    
    for (int i = 0; i < 1024; i++) {
        data[i] = (i % 8) * 0.125f;
    }
    
    #pragma omp simd reduction(+:sum) simdlen(8)
    for (int i = 0; i < 1024; i++) {
        sum += data[i] * data[(i + 1) % 1024] - data[(i + 2) % 1024];
    }
    
    return sum;
}

int main(void) {
    float vec_result = complex_vector_operations();
    uint64_t asm_result = multi_operand_asm();
    double builtin_result = complex_builtins(1.1, 2.2, 3.3);
    float omp_result = omp_vector_reduction();
    
    /* Use results to prevent optimization */
    printf("Results: %f, %lu, %f, %f\n", 
           vec_result, 
           (unsigned long)asm_result,
           builtin_result,
           omp_result);
    
    return (int)(vec_result + asm_result + builtin_result + omp_result);
}
