/* Test program to cover 10-11 operand cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Force inline expansion */
#define FORCE_INLINE __attribute__((always_inline)) inline

/* Target-specific includes */
#ifdef __x86_64__
#include <immintrin.h>
#include <x86intrin.h>
#elif defined(__aarch64__)
#include <arm_neon.h>
#include <arm_acle.h>
#endif

/* GCC vector extensions for complex operations */
typedef float v8sf __attribute__((vector_size(32)));
typedef double v4df __attribute__((vector_size(32)));
typedef int v8si __attribute__((vector_size(32)));

/* Complex reduction using GCC vector extensions */
FORCE_INLINE v8sf complex_vector_operation(v8sf a, v8sf b, v8sf c, v8sf d) {
    /* Complex expression that might expand to multi-operand instructions */
    return a * b + c * d + (a + b) * (c - d) + a * c + b * d;
}

/* Function using many operands through nested operations */
FORCE_INLINE float multi_operand_fma_chain(float a, float b, float c, 
                                          float d, float e, float f,
                                          float g, float h, float i,
                                          float j, float k) {
    /* Chain of FMA operations - each FMA has 3 operands, nesting creates many */
    float t1 = __builtin_fmaf(a, b, c);
    float t2 = __builtin_fmaf(d, e, f);
    float t3 = __builtin_fmaf(g, h, i);
    float t4 = __builtin_fmaf(t1, t2, t3);
    return __builtin_fmaf(t4, j, k);
}

#ifdef __x86_64__
/* AVX-512 mask operations with many operands */
FORCE_INLINE __m512 avx512_complex_op(__m512 a, __m512 b, __m512 c,
                                      __m512 d, __m512 e, __mmask16 k) {
    /* Complex sequence that might use 10-11 operand patterns */
    __m512 t1 = _mm512_mask_add_ps(a, k, b, c);
    __m512 t2 = _mm512_mask_mul_ps(b, k, c, d);
    __m512 t3 = _mm512_mask_fmadd_ps(c, k, d, e);
    __m512 t4 = _mm512_mask_fnmadd_ps(d, k, e, t1);
    return _mm512_mask_fmadd_ps(t1, k, t2, _mm512_mask_add_ps(t3, k, t4, a));
}

/* Test AVX-512 gather with complex addressing */
FORCE_INLINE __m512i test_gather(__m512i index, __m512i mask, int scale) {
    int base[1024];
    for (int i = 0; i < 1024; i++) base[i] = i;
    
    /* _mm512_i32gather_epi32 has multiple operands during expansion */
    return _mm512_mask_i32gather_epi32(_mm512_setzero_si512(), 
                                      (__mmask16)mask,
                                      index, 
                                      base, 
                                      scale);
}
#endif

#ifdef __aarch64__
/* ARM NEON/SVE style operations with many operands */
FORCE_INLINE float32x4_t neon_complex_op(float32x4_t a, float32x4_t b,
                                         float32x4_t c, float32x4_t d,
                                         float32x4_t e, float32x4_t f) {
    /* Complex sequence using multiple operations */
    float32x4_t t1 = vfmaq_f32(a, b, c);
    float32x4_t t2 = vfmaq_f32(d, e, f);
    float32x4_t t3 = vmulq_f32(t1, t2);
    float32x4_t t4 = vaddq_f32(vmulq_f32(a, b), vmulq_f32(c, d));
    return vfmaq_f32(t3, t4, vaddq_f32(e, f));
}

/* Table lookup with many operands */
FORCE_INLINE int8x16_t test_tbl(int8x16_t a, int8x16_t b, 
                                int8x16_t c, int8x16_t d) {
    /* vqtbl4q_s8 expands to multiple operands */
    int8x16x4_t table = {a, b, c, d};
    return vqtbl4q_s8(table, a);
}
#endif

/* Inline assembly with exactly 11 operands */
FORCE_INLINE void asm_11_operands(uint64_t *a, uint64_t *b, uint64_t *c,
                                  uint64_t *d, uint64_t *e, uint64_t *f,
                                  uint64_t *g, uint64_t *h, uint64_t *i,
                                  uint64_t *j, uint64_t *k) {
    /* Extended asm with 11 operands (5 outputs + 6 inputs) */
    asm volatile (
        "mov %[out1], %[in1]\n\t"
        "add %[out2], %[in2], %[in3]\n\t"
        "sub %[out3], %[in4], %[in5]\n\t"
        "and %[out4], %[in6], %[out1]\n\t"
        "or  %[out5], %[out2], %[out3]"
        : [out1] "=r" (*a), [out2] "=r" (*b), [out3] "=r" (*c),
          [out4] "=r" (*d), [out5] "=r" (*e)
        : [in1] "r" (*f), [in2] "r" (*g), [in3] "r" (*h),
          [in4] "r" (*i), [in5] "r" (*j), [in6] "r" (*k)
        : "cc"
    );
}

/* OpenMP SIMD reduction with vector types */
void omp_vector_reduction(v8sf *array, int n, v8sf *result) {
    v8sf sum = {0};
    
    #pragma omp simd reduction(+:sum)
    for (int i = 0; i < n; i++) {
        /* Complex operation that might expand to multi-operand pattern */
        sum = sum + array[i] * array[i] + 
              (array[i] * 2.0f) - (array[i] / 3.0f);
    }
    
    *result = sum;
}

/* Main test function */
int main() {
    /* Test GCC vector extensions */
    v8sf vec1 = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    v8sf vec2 = {2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f};
    v8sf vec3 = {3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f};
    v8sf vec4 = {4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f};
    
    v8sf result_vec = complex_vector_operation(vec1, vec2, vec3, vec4);
    
    /* Test FMA chain */
    float fma_result = multi_operand_fma_chain(1.1f, 2.2f, 3.3f, 4.4f, 5.5f,
                                               6.6f, 7.7f, 8.8f, 9.9f,
                                               10.10f, 11.11f);
    
    #ifdef __x86_64__
    /* Test AVX-512 operations if available */
    if (__builtin_cpu_supports("avx512f")) {
        __m512 avx_vec1 = _mm512_set1_ps(1.0f);
        __m512 avx_vec2 = _mm512_set1_ps(2.0f);
        __m512 avx_vec3 = _mm512_set1_ps(3.0f);
        __m512 avx_vec4 = _mm512_set1_ps(4.0f);
        __m512 avx_vec5 = _mm512_set1_ps(5.0f);
        __mmask16 mask = 0xAAAA;
        
        __m512 avx_result = avx512_complex_op(avx_vec1, avx_vec2, avx_vec3,
                                              avx_vec4, avx_vec5, mask);
        
        /* Test gather */
        __m512i index = _mm512_set_epi32(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
        __m512i gather_result = test_gather(index, _mm512_set1_epi32(-1), 4);
        
        /* Use results to prevent optimization */
        float avx_store[16];
        _mm512_storeu_ps(avx_store, avx_result);
        printf("AVX-512 result: %f\n", avx_store[0]);
    }
    #endif
    
    #ifdef __aarch64__
    /* Test ARM NEON operations */
    float32x4_t neon1 = {1.0f, 2.0f, 3.0f, 4.0f};
    float32x4_t neon2 = {2.0f, 3.0f, 4.0f, 5.0f};
    float32x4_t neon3 = {3.0f, 4.0f, 5.0f, 6.0f};
    float32x4_t neon4 = {4.0f, 5.0f, 6.0f, 7.0f};
    float32x4_t neon5 = {5.0f, 6.0f, 7.0f, 8.0f};
    float32x4_t neon6 = {6.0f, 7.0f, 8.0f, 9.0f};
    
    float32x4_t neon_result = neon_complex_op(neon1, neon2, neon3,
                                              neon4, neon5, neon6);
    
    float neon_store[4];
    vst1q_f32(neon_store, neon_result);
    printf("NEON result: %f\n", neon_store[0]);
    #endif
    
    /* Test inline assembly with 11 operands */
    uint64_t asm_vars[11] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
    asm_11_operands(&asm_vars[0], &asm_vars[1], &asm_vars[2],
                    &asm_vars[3], &asm_vars[4], &asm_vars[5],
                    &asm_vars[6], &asm_vars[7], &asm_vars[8],
                    &asm_vars[9], &asm_vars[10]);
    
    /* Test OpenMP reduction */
    v8sf omp_array[100];
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 8; j++) {
            omp_array[i][j] = (float)(i + j);
        }
    }
    
    v8sf omp_result;
    omp_vector_reduction(omp_array, 100, &omp_result);
    
    /* Use results to prevent dead code elimination */
    float result_store[8];
    memcpy(result_store, &result_vec, sizeof(result_vec));
    
    printf("Results: %f %f\n", result_store[0], fma_result);
    printf("Assembly results: %lu %lu\n", asm_vars[0], asm_vars[1]);
    
    return (int)(result_store[0] + fma_result + asm_vars[0]);
}
