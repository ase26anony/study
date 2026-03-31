/* Test program to cover 10-11 operand cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

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

/* GCC vector extensions for fallback */
typedef float v8sf __attribute__((vector_size(32)));
typedef int v8si __attribute__((vector_size(32)));
typedef double v4df __attribute__((vector_size(32)));

/* Complex reduction with many operands */
FORCE_INLINE
float complex_reduction(float *data, int n) {
    float sum = 0.0f;
    #pragma omp simd reduction(+:sum)
    for (int i = 0; i < n; i++) {
        sum += data[i] * data[i] + data[i] * 2.0f;
    }
    return sum;
}

#ifdef __x86_64__
/* AVX-512 intrinsics with many operands */
FORCE_INLINE
__m512 test_avx512_many_operands(__m512 a, __m512 b, __m512 c, 
                                  __m512 d, __m512 e, __mmask16 k) {
    /* Chain multiple FMA operations - each expands to many operands */
    __m512 t1 = _mm512_mask_fmadd_ps(a, k, b, c);
    __m512 t2 = _mm512_mask_fmadd_ps(t1, k, d, e);
    __m512 t3 = _mm512_mask_add_ps(t2, k, t2, a);
    __m512 t4 = _mm512_mask_mul_ps(t3, k, t3, b);
    
    /* Complex blend with multiple sources */
    __m512 result = _mm512_mask_blend_ps(k, t1, 
        _mm512_mask_blend_ps(k >> 1, t2,
            _mm512_mask_blend_ps(k >> 2, t3, t4)));
    
    return result;
}

/* Test AVX-512 gather with many parameters */
FORCE_INLINE
__m512i test_avx512_gather(__m512i index, __m512i src, 
                           __mmask16 k, const int *base) {
    /* Gather with scale, displacement, mask, etc. */
    return _mm512_mask_i32gather_epi32(src, k, index, base, 4);
}
#endif

#ifdef __aarch64__
/* ARM NEON/SVE style with many operands */
FORCE_INLINE
int32x4_t test_neon_many_operands(int32x4_t a, int32x4_t b, 
                                   int32x4_t c, int32x4_t d,
                                   int32x4_t e, int32x4_t f) {
    /* Complex sequence with lane operations */
    int32x4_t t1 = vmlaq_s32(a, b, c);
    int32x4_t t2 = vmlaq_s32(t1, d, e);
    int32x4_t t3 = vaddq_s32(t2, f);
    
    /* Table lookup with multiple registers */
    const int8x16_t table = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    uint8x16_t indices = vreinterpretq_u8_s32(t3);
    int8x16_t result = vqtbl1q_s8(table, indices);
    
    return vreinterpretq_s32_s8(result);
}
#endif

/* Generic vector operations using GCC extensions */
FORCE_INLINE
v8sf test_gcc_vector_many_ops(v8sf a, v8sf b, v8sf c, 
                               v8sf d, v8sf e, v8sf f) {
    /* Complex expression that might expand to many operands */
    v8sf t1 = a * b + c;
    v8sf t2 = d * e + f;
    v8sf t3 = t1 * t2 + a;
    v8sf t4 = t3 * b + c;
    v8sf t5 = t4 * d + e;
    
    /* Blend-like operation */
    v8sf mask = a > b;
    v8sf result = (mask & t3) | (~mask & t5);
    
    return result;
}

/* Inline assembly with exactly 11 operands */
FORCE_INLINE
void test_many_operand_asm(uint64_t *out1, uint64_t *out2, uint64_t *out3,
                           uint64_t in1, uint64_t in2, uint64_t in3,
                           uint64_t in4, uint64_t in5, uint64_t in6) {
    /* 11 operands: 3 outputs, 6 inputs, 2 clobbers = 11 total */
    asm volatile (
        "mov %0, %3\n\t"
        "add %0, %4\n\t"
        "mov %1, %5\n\t"
        "sub %1, %6\n\t"
        "mov %2, %7\n\t"
        "xor %2, %8\n\t"
        : "=r"(*out1), "=r"(*out2), "=r"(*out3)  /* 3 outputs */
        : "r"(in1), "r"(in2), "r"(in3),          /* 6 inputs */
          "r"(in4), "r"(in5), "r"(in6)
        : "cc", "memory"                         /* 2 clobbers */
    );
}

/* Built-in functions with complex expressions */
FORCE_INLINE
double test_builtin_fma_chain(double a, double b, double c,
                              double d, double e, double f) {
    /* Chain of FMA operations - each expands to 3-4 operands */
    double t1 = __builtin_fma(a, b, c);
    double t2 = __builtin_fma(d, e, f);
    double t3 = __builtin_fma(t1, t2, a);
    double t4 = __builtin_fma(t3, b, c);
    double t5 = __builtin_fma(t4, d, e);
    
    return t5;
}

/* Main test function with hot attribute to encourage complex expansion */
__attribute__((hot, noinline))
void run_all_tests() {
    const int N = 1024;
    float *data = (float*)malloc(N * sizeof(float));
    
    /* Initialize data */
    for (int i = 0; i < N; i++) {
        data[i] = (float)i / N;
    }
    
    /* Test 1: Complex reduction (may generate many operands) */
    float sum = complex_reduction(data, N);
    printf("Reduction sum: %f\n", sum);
    
    /* Test 2: Target-specific intrinsics */
    #ifdef __x86_64__
    if (__builtin_cpu_supports("avx512f")) {
        __m512 av = _mm512_set1_ps(1.0f);
        __m512 bv = _mm512_set1_ps(2.0f);
        __m512 cv = _mm512_set1_ps(3.0f);
        __m512 dv = _mm512_set1_ps(4.0f);
        __m512 ev = _mm512_set1_ps(5.0f);
        __mmask16 mask = 0xAAAA;
        
        __m512 result = test_avx512_many_operands(av, bv, cv, dv, ev, mask);
        float res_arr[16];
        _mm512_storeu_ps(res_arr, result);
        printf("AVX-512 result[0]: %f\n", res_arr[0]);
        
        /* Test gather */
        int base_arr[64];
        for (int i = 0; i < 64; i++) base_arr[i] = i;
        __m512i indices = _mm512_set_epi32(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
        __m512i src = _mm512_set1_epi32(0);
        __m512i gathered = test_avx512_gather(indices, src, mask, base_arr);
        int gather_arr[16];
        _mm512_storeu_epi32(gather_arr, gathered);
        printf("Gather result[0]: %d\n", gather_arr[0]);
    }
    #elif defined(__aarch64__)
    int32x4_t a = vdupq_n_s32(1);
    int32x4_t b = vdupq_n_s32(2);
    int32x4_t c = vdupq_n_s32(3);
    int32x4_t d = vdupq_n_s32(4);
    int32x4_t e = vdupq_n_s32(5);
    int32x4_t f = vdupq_n_s32(6);
    
    int32x4_t neon_result = test_neon_many_operands(a, b, c, d, e, f);
    int32_t neon_arr[4];
    vst1q_s32(neon_arr, neon_result);
    printf("NEON result[0]: %d\n", neon_arr[0]);
    #endif
    
    /* Test 3: GCC vector extensions */
    v8sf gcc_a = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    v8sf gcc_b = {2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f};
    v8sf gcc_c = {3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f};
    v8sf gcc_d = {4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f};
    v8sf gcc_e = {5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f};
    v8sf gcc_f = {6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f};
    
    v8sf gcc_result = test_gcc_vector_many_ops(gcc_a, gcc_b, gcc_c, 
                                               gcc_d, gcc_e, gcc_f);
    float gcc_arr[8];
    __builtin_memcpy(gcc_arr, &gcc_result, sizeof(gcc_result));
    printf("GCC vector result[0]: %f\n", gcc_arr[0]);
    
    /* Test 4: Inline assembly with many operands */
    uint64_t out1, out2, out3;
    test_many_operand_asm(&out1, &out2, &out3, 
                          100, 200, 300, 400, 500, 600);
    printf("Assembly result: %lu, %lu, %lu\n", out1, out2, out3);
    
    /* Test 5: Built-in FMA chain */
    double fma_result = test_builtin_fma_chain(1.0, 2.0, 3.0, 
                                               4.0, 5.0, 6.0);
    printf("FMA chain result: %f\n", fma_result);
    
    free(data);
}

int main() {
    /* Run tests multiple times to ensure code paths are taken */
    for (int i = 0; i < 10; i++) {
        run_all_tests();
    }
    return 0;
}
