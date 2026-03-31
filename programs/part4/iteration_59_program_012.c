/* Test program to cover 10 and 11-operand cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Force aggressive optimization and inline expansion */
#define ALWAYS_INLINE __attribute__((always_inline)) inline
#define HOT __attribute__((hot))

/* Target-specific intrinsics */
#ifdef __AVX512F__
#include <immintrin.h>
#elif defined(__ARM_NEON)
#include <arm_neon.h>
#endif

/* GCC vector extensions for generic testing */
typedef float v8sf __attribute__((vector_size(32)));
typedef double v4df __attribute__((vector_size(32)));

/* Complex inline assembly with many operands */
static void test_many_operand_asm(void) {
    /* 11-operand inline assembly */
    int64_t out1, out2, out3, out4;
    int64_t in1 = 1, in2 = 2, in3 = 3, in4 = 4, in5 = 5, in6 = 6;
    
    asm volatile (
        "mov %0, %5\n\t"
        "add %0, %6\n\t"
        "mov %1, %7\n\t"
        "add %1, %8\n\t"
        "mov %2, %9\n\t"
        "add %2, %10\n\t"
        "mov %3, %5\n\t"
        "sub %3, %6"
        : "=r"(out1), "=r"(out2), "=r"(out3), "=r"(out4)
        : "0"(out1), "r"(in1), "r"(in2), "r"(in3), "r"(in4), "r"(in5), "r"(in6)
        : "cc"
    );
    
    printf("ASM result: %ld %ld %ld %ld\n", out1, out2, out3, out4);
}

/* AVX-512 specific test with mask operations */
#ifdef __AVX512F__
ALWAYS_INLINE HOT
__m512 test_avx512_multi_operand(__m512 a, __m512 b, __m512 c, 
                                 __m512 d, __mmask16 k) {
    /* Complex expression that might expand to multi-operand pattern */
    __m512 t1 = _mm512_mask_add_ps(a, k, b, c);
    __m512 t2 = _mm512_mask_mul_ps(t1, k, d, a);
    __m512 t3 = _mm512_mask_fmadd_ps(t2, k, b, c, d);
    __m512 t4 = _mm512_mask_sub_ps(t3, k, t1, t2);
    
    /* Nested FMA operations */
    __m512 result = _mm512_fmadd_ps(t4, t1, 
                     _mm512_fmadd_ps(t2, t3,
                       _mm512_fmadd_ps(a, b, c)));
    
    return result;
}
#endif

/* ARM NEON/AArch64 specific test */
#ifdef __ARM_NEON
ALWAYS_INLINE HOT
int32x4_t test_neon_multi_operand(int32x4_t a, int32x4_t b, int32x4_t c,
                                  int32x4_t d, int32x4_t e) {
    /* Complex lane operations and permutations */
    int32x4_t t1 = vaddq_s32(a, b);
    int32x4_t t2 = vmlaq_s32(t1, c, d);
    int32x4_t t3 = vqdmulhq_s32(t2, e);
    
    /* Multiple vector operations in one expression */
    int32x4_t result = vaddq_s32(
        vmulq_s32(t1, t2),
        vaddq_s32(
            vmulq_s32(t3, a),
            vaddq_s32(
                vmulq_s32(b, c),
                vmulq_s32(d, e)
            )
        )
    );
    
    return result;
}
#endif

/* GCC vector extensions test */
ALWAYS_INLINE HOT
v8sf test_gcc_vector_multi_operand(v8sf a, v8sf b, v8sf c, v8sf d, v8sf e) {
    /* Complex expression with many vector operands */
    v8sf t1 = a + b * c;
    v8sf t2 = d - e * a;
    v8sf t3 = t1 * t2 + c * d;
    v8sf t4 = t3 - a * b + d * e;
    
    /* Nested operations that might require multi-operand expansion */
    v8sf result = t1 * t2 + t3 * t4 + a * b * c * d * e;
    
    return result;
}

/* Built-in FMA test */
ALWAYS_INLINE HOT
double test_builtin_fma_chain(double a, double b, double c, 
                              double d, double e, double f) {
    /* Chain of FMA operations - each expands to 3 operands */
    double t1 = __builtin_fma(a, b, c);
    double t2 = __builtin_fma(d, e, f);
    double t3 = __builtin_fma(t1, t2, a);
    double t4 = __builtin_fma(b, c, d);
    double t5 = __builtin_fma(e, f, t3);
    
    /* Complex expression forcing expansion */
    double result = __builtin_fma(
        __builtin_fma(a, b, c),
        __builtin_fma(d, e, f),
        __builtin_fma(t1, t2, t3)
    );
    
    return result;
}

/* OpenMP SIMD reduction with vector types */
void test_omp_simd_reduction(float* result, const float* a, 
                             const float* b, int n) {
    v8sf sum = {0};
    
    #pragma omp simd reduction(+:sum)
    for (int i = 0; i < n; i += 8) {
        v8sf va = *(v8sf*)(a + i);
        v8sf vb = *(v8sf*)(b + i);
        sum = sum + va * vb + va - vb;
    }
    
    *(v8sf*)result = sum;
}

/* Main test driver */
HOT int main(void) {
    printf("Testing multi-operand instruction patterns\n");
    
    /* Test inline assembly with many operands */
    test_many_operand_asm();
    
    /* Test GCC vector extensions */
    v8sf va = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    v8sf vb = {2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f};
    v8sf vc = {3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f};
    v8sf vd = {4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f};
    v8sf ve = {5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f};
    
    v8sf vresult = test_gcc_vector_multi_operand(va, vb, vc, vd, ve);
    printf("Vector result[0] = %f\n", vresult[0]);
    
    /* Test built-in FMA chain */
    double fma_result = test_builtin_fma_chain(1.0, 2.0, 3.0, 4.0, 5.0, 6.0);
    printf("FMA result = %f\n", fma_result);
    
    /* Test OpenMP SIMD reduction */
    const int N = 1024;
    float* a = malloc(N * sizeof(float));
    float* b = malloc(N * sizeof(float));
    float* result = malloc(8 * sizeof(float));
    
    for (int i = 0; i < N; i++) {
        a[i] = i * 0.1f;
        b[i] = i * 0.2f;
    }
    
    test_omp_simd_reduction(result, a, b, N);
    printf("OpenMP reduction result[0] = %f\n", result[0]);
    
    free(a);
    free(b);
    free(result);
    
    /* Target-specific tests */
#ifdef __AVX512F__
    printf("Testing AVX-512 multi-operand patterns\n");
    __m512 avx_a = _mm512_set1_ps(1.0f);
    __m512 avx_b = _mm512_set1_ps(2.0f);
    __m512 avx_c = _mm512_set1_ps(3.0f);
    __m512 avx_d = _mm512_set1_ps(4.0f);
    __mmask16 mask = 0xFFFF;
    
    __m512 avx_result = test_avx512_multi_operand(avx_a, avx_b, avx_c, avx_d, mask);
    float avx_res[16];
    _mm512_storeu_ps(avx_res, avx_result);
    printf("AVX-512 result[0] = %f\n", avx_res[0]);
#endif
    
#ifdef __ARM_NEON
    printf("Testing NEON multi-operand patterns\n");
    int32x4_t neon_a = vdupq_n_s32(1);
    int32x4_t neon_b = vdupq_n_s32(2);
    int32x4_t neon_c = vdupq_n_s32(3);
    int32x4_t neon_d = vdupq_n_s32(4);
    int32x4_t neon_e = vdupq_n_s32(5);
    
    int32x4_t neon_result = test_neon_multi_operand(neon_a, neon_b, neon_c, neon_d, neon_e);
    int32_t neon_res[4];
    vst1q_s32(neon_res, neon_result);
    printf("NEON result[0] = %d\n", neon_res[0]);
#endif
    
    return 0;
}
