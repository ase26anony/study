/* Test program to trigger 10-11 operand instruction patterns in optabs.cc */
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

/* GCC vector extensions for generic approach */
typedef float v8sf __attribute__((vector_size(32)));
typedef double v4df __attribute__((vector_size(32)));
typedef int v8si __attribute__((vector_size(32)));

/* Complex reduction using GCC vector extensions */
FORCE_INLINE v8sf complex_vector_op(v8sf a, v8sf b, v8sf c, v8sf d) {
    /* Complex expression that might expand to multi-operand instructions */
    return a * b + c * d + a * c + b * d;
}

#ifdef __x86_64__
/* AVX-512 intrinsics with many operands */
FORCE_INLINE __m512 avx512_multi_operand_test(__m512 a, __m512 b, __m512 c, 
                                              __m512 d, __m512 e, __mmask16 k) {
    /* Chain of FMA operations - each FMA has 3 inputs + mask = potentially many operands */
    __m512 t1 = _mm512_mask_fmadd_ps(a, k, b, c);
    __m512 t2 = _mm512_mask_fmadd_ps(t1, k, d, e);
    __m512 t3 = _mm512_mask_add_ps(t2, k, a, b);
    __m512 t4 = _mm512_mask_mul_ps(t3, k, c, d);
    
    /* Complex blend with multiple sources */
    return _mm512_mask_blend_ps(k, t1, 
        _mm512_mask_blend_ps(k >> 1, t2,
            _mm512_mask_blend_ps(k >> 2, t3, t4)));
}

/* Test AVX-512 gather with many parameters */
FORCE_INLINE __m512i test_gather(__m512i index, __m512i mask, int scale) {
    int base[1024];
    for (int i = 0; i < 1024; i++) base[i] = i;
    
    /* _mm512_i32gather_epi32 has index, mask, base pointer, scale - 
       during expansion this may involve many operands */
    return _mm512_mask_i32gather_epi32(_mm512_setzero_si512(), 
                                      (__mmask16)mask,
                                      index, 
                                      base, 
                                      scale);
}
#endif

#ifdef __aarch64__
/* ARM NEON/SVE style multi-vector operations */
FORCE_INLINE float32x4x4_t neon_multi_operand_test(float32x4_t a, float32x4_t b,
                                                   float32x4_t c, float32x4_t d) {
    /* Use multiple vector operations that might combine */
    float32x4_t t1 = vfmaq_f32(a, b, c);
    float32x4_t t2 = vfmaq_f32(d, a, b);
    float32x4_t t3 = vmulq_f32(t1, t2);
    float32x4_t t4 = vaddq_f32(vaddq_f32(a, b), vaddq_f32(c, d));
    
    /* Return as a structure - might trigger multi-register operations */
    float32x4x4_t result;
    result.val[0] = t1;
    result.val[1] = t2;
    result.val[2] = t3;
    result.val[3] = t4;
    return result;
}

/* Table lookup with many operands */
FORCE_INLINE uint8x16_t test_tbl(uint8x16_t a, uint8x16_t b, 
                                 uint8x16_t c, uint8x16_t d) {
    /* Create a 4-register table */
    uint8x16x4_t table;
    table.val[0] = a;
    table.val[1] = b;
    table.val[2] = c;
    table.val[3] = d;
    
    /* Index vector */
    uint8x16_t indices = vaddq_u8(a, b);
    
    /* vqtbl4q_u8 uses 5 vector registers as operands */
    return vqtbl4q_u8(table, indices);
}
#endif

/* Inline assembly with many operands - direct test of operand handling */
void inline_asm_multi_operand_test(void) {
    volatile int a = 1, b = 2, c = 3, d = 4, e = 5;
    volatile int f = 6, g = 7, h = 8, i = 9, j = 10;
    volatile int k = 11;
    volatile int result1, result2, result3;
    
    /* 11-operand asm statement */
    __asm__ volatile (
        "mov %0, %1\n\t"
        "add %0, %0, %2\n\t"
        "add %0, %0, %3\n\t"
        "add %0, %0, %4\n\t"
        "add %0, %0, %5\n\t"
        "add %0, %0, %6\n\t"
        "add %0, %0, %7\n\t"
        "add %0, %0, %8\n\t"
        "add %0, %0, %9\n\t"
        "add %0, %0, %10"
        : "=r"(result1)
        : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e), 
          "r"(f), "r"(g), "r"(h), "r"(i), "r"(j)
        : "cc"
    );
    
    /* 10-operand variant */
    __asm__ volatile (
        "imul %0, %1, %2\n\t"
        "imul %3, %4, %5\n\t"
        "add %0, %0, %3\n\t"
        "add %0, %0, %6\n\t"
        "add %0, %0, %7\n\t"
        "add %0, %0, %8\n\t"
        "add %0, %0, %9"
        : "=r"(result2), "=r"(result3)
        : "r"(a), "r"(b), "r"(c), "r"(d), 
          "r"(e), "r"(f), "r"(g), "r"(h)
        : "cc"
    );
    
    printf("ASM results: %d, %d\n", result1, result2);
}

/* Complex built-in usage */
FORCE_INLINE double complex_builtin_test(double a, double b, double c, 
                                         double d, double e) {
    /* Chain of FMA operations - each expands to potentially many operands */
    double t1 = __builtin_fma(a, b, c);
    double t2 = __builtin_fma(d, e, t1);
    double t3 = __builtin_fma(a, c, b);
    double t4 = __builtin_fma(d, b, e);
    
    /* Complex expression to prevent optimization */
    return __builtin_fma(t1, t2, __builtin_fma(t3, t4, a + b + c + d + e));
}

/* OpenMP SIMD reduction with vector types */
void omp_vector_reduction(float* output, const float* input, int n) {
    v8sf sum = {0, 0, 0, 0, 0, 0, 0, 0};
    
    #pragma omp simd reduction(+:sum) aligned(input:32)
    for (int i = 0; i < n; i += 8) {
        v8sf chunk;
        memcpy(&chunk, &input[i], sizeof(v8sf));
        sum = complex_vector_op(sum, chunk, sum, chunk);
    }
    
    memcpy(output, &sum, sizeof(v8sf));
}

/* Main test function */
int main() {
    float input[256];
    float output[8];
    
    /* Initialize input */
    for (int i = 0; i < 256; i++) {
        input[i] = i * 0.1f;
    }
    
    /* Test 1: OpenMP vector reduction */
    omp_vector_reduction(output, input, 256);
    printf("OpenMP reduction result[0] = %f\n", output[0]);
    
    /* Test 2: Inline assembly with many operands */
    inline_asm_multi_operand_test();
    
    /* Test 3: Complex built-in chain */
    double builtin_result = complex_builtin_test(1.0, 2.0, 3.0, 4.0, 5.0);
    printf("Built-in result = %f\n", builtin_result);
    
    /* Test 4: Target-specific intrinsics */
    #ifdef __x86_64__
    {
        __m512 avx_a = _mm512_set1_ps(1.0f);
        __m512 avx_b = _mm512_set1_ps(2.0f);
        __m512 avx_c = _mm512_set1_ps(3.0f);
        __m512 avx_d = _mm512_set1_ps(4.0f);
        __m512 avx_e = _mm512_set1_ps(5.0f);
        __mmask16 mask = 0xAAAA;
        
        __m512 avx_result = avx512_multi_operand_test(avx_a, avx_b, avx_c, 
                                                      avx_d, avx_e, mask);
        float avx_store[16];
        _mm512_storeu_ps(avx_store, avx_result);
        printf("AVX-512 result[0] = %f\n", avx_store[0]);
    }
    #elif defined(__aarch64__)
    {
        float32x4_t neon_a = vdupq_n_f32(1.0f);
        float32x4_t neon_b = vdupq_n_f32(2.0f);
        float32x4_t neon_c = vdupq_n_f32(3.0f);
        float32x4_t neon_d = vdupq_n_f32(4.0f);
        
        float32x4x4_t neon_result = neon_multi_operand_test(neon_a, neon_b, 
                                                           neon_c, neon_d);
        printf("NEON result[0] = %f\n", vgetq_lane_f32(neon_result.val[0], 0));
    }
    #endif
    
    /* Test 5: GCC vector extension complex operations */
    {
        v8sf vec_a = {1, 2, 3, 4, 5, 6, 7, 8};
        v8sf vec_b = {8, 7, 6, 5, 4, 3, 2, 1};
        v8sf vec_c = {2, 3, 4, 5, 6, 7, 8, 9};
        v8sf vec_d = {9, 8, 7, 6, 5, 4, 3, 2};
        
        v8sf vec_result = complex_vector_op(vec_a, vec_b, vec_c, vec_d);
        printf("Vector extension result[0] = %f\n", vec_result[0]);
    }
    
    return 0;
}
