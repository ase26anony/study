/* Test program to cover 10-11 operand cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Target-specific includes */
#ifdef __x86_64__
#include <immintrin.h>
#elif defined(__aarch64__)
#include <arm_neon.h>
#endif

/* Force inlining and optimization */
#define HOT __attribute__((hot, always_inline))
#define NOINLINE __attribute__((noinline))

/* GCC vector extensions for generic testing */
typedef float v8sf __attribute__((vector_size(32)));
typedef int v8si __attribute__((vector_size(32)));

/* Complex inline assembly with many operands */
static inline HOT void multi_operand_asm(void) {
    uint64_t a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11;
    
    /* Initialize values */
    a1 = 1; a2 = 2; a3 = 3; a4 = 4; a5 = 5;
    a6 = 6; a7 = 7; a8 = 8; a9 = 9; a10 = 10; a11 = 11;
    
    /* 11-operand inline assembly */
    asm volatile (
        "add %[r1], %[r1], %[r2]\n\t"
        "add %[r3], %[r3], %[r4]\n\t"
        "add %[r5], %[r5], %[r6]\n\t"
        "add %[r7], %[r7], %[r8]\n\t"
        "add %[r9], %[r9], %[r10]\n\t"
        "add %[r11], %[r11], %[r1]"
        : [r1] "+r" (a1), [r3] "+r" (a3), [r5] "+r" (a5),
          [r7] "+r" (a7), [r9] "+r" (a9), [r11] "+r" (a11)
        : [r2] "r" (a2), [r4] "r" (a4), [r6] "r" (a6),
          [r8] "r" (a8), [r10] "r" (a10)
        : "cc"
    );
    
    /* Use results to prevent elimination */
    volatile uint64_t sum = a1 + a3 + a5 + a7 + a9 + a11;
    (void)sum;
}

#ifdef __x86_64__
/* AVX-512 intrinsics that can generate many operands */
static inline HOT __m512 avx512_complex_operation(
    __m512 a, __m512 b, __m512 c, __m512 d,
    __mmask16 k1, __mmask16 k2) {
    
    /* Complex sequence that may expand to many operands */
    __m512 t1 = _mm512_mask_add_ps(a, k1, b, c);
    __m512 t2 = _mm512_mask_mul_ps(b, k2, c, d);
    __m512 t3 = _mm512_mask_fmadd_ps(t1, k1, t2, a);
    __m512 t4 = _mm512_mask3_fmadd_ps(t1, t2, t3, k2);
    
    /* Nested FMA operations */
    __m512 result = _mm512_fmadd_ps(
        _mm512_fmadd_ps(a, b, c),
        _mm512_fmadd_ps(d, t1, t2),
        _mm512_fmadd_ps(t3, t4, _mm512_set1_ps(1.0f))
    );
    
    return result;
}

/* Test AVX-512 mask operations with many operands */
static NOINLINE void test_avx512_many_operands(void) {
    __m512 v1 = _mm512_set1_ps(1.0f);
    __m512 v2 = _mm512_set1_ps(2.0f);
    __m512 v3 = _mm512_set1_ps(3.0f);
    __m512 v4 = _mm512_set1_ps(4.0f);
    __m512 v5 = _mm512_set1_ps(5.0f);
    
    __mmask16 m1 = 0xAAAA;
    __mmask16 m2 = 0x5555;
    
    /* Complex operation that may require many RTL operands */
    __m512 result = avx512_complex_operation(v1, v2, v3, v4, m1, m2);
    
    /* Use result to prevent elimination */
    float res[16];
    _mm512_storeu_ps(res, result);
    volatile float sum = res[0] + res[15];
    (void)sum;
}
#endif

#ifdef __aarch64__
/* ARM NEON/SVE style operations with many operands */
static inline HOT float32x4_t neon_complex_operation(
    float32x4_t a, float32x4_t b, float32x4_t c,
    float32x4_t d, float32x4_t e, uint32x4_t mask) {
    
    /* Complex sequence with lane operations */
    float32x4_t t1 = vfmaq_f32(a, b, c);
    float32x4_t t2 = vfmaq_laneq_f32(d, e, a, 1);
    float32x4_t t3 = vbslq_f32(mask, t1, t2);
    
    /* Multiple FMA operations */
    float32x4_t result = vfmaq_f32(
        vfmaq_f32(t1, t2, t3),
        vfmaq_f32(a, b, c),
        vfmaq_f32(d, e, vdupq_n_f32(2.0f))
    );
    
    return result;
}

static NOINLINE void test_neon_many_operands(void) {
    float32x4_t v1 = vdupq_n_f32(1.0f);
    float32x4_t v2 = vdupq_n_f32(2.0f);
    float32x4_t v3 = vdupq_n_f32(3.0f);
    float32x4_t v4 = vdupq_n_f32(4.0f);
    float32x4_t v5 = vdupq_n_f32(5.0f);
    uint32x4_t mask = vdupq_n_u32(0xFFFFFFFF);
    
    float32x4_t result = neon_complex_operation(v1, v2, v3, v4, v5, mask);
    
    /* Use result */
    volatile float sum = vgetq_lane_f32(result, 0) + vgetq_lane_f32(result, 3);
    (void)sum;
}
#endif

/* GCC vector extensions with complex expressions */
static NOINLINE void test_gcc_vector_many_operands(void) {
    v8sf v1 = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    v8sf v2 = {2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f};
    v8sf v3 = {3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f};
    v8sf v4 = {4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f};
    
    /* Complex expression that may generate many operands */
    v8sf result = v1 * v2 + v3 * v4 + 
                  (v1 + v2) * (v3 - v4) + 
                  __builtin_fma(v1, v2, v3) * v4 +
                  v1 * __builtin_fma(v2, v3, v4);
    
    /* Use result */
    volatile float sum = result[0] + result[7];
    (void)sum;
}

/* OpenMP SIMD reduction with vector types */
static NOINLINE void test_omp_reduction_many_operands(void) {
    #define N 1024
    float array[N];
    
    /* Initialize */
    for (int i = 0; i < N; i++) {
        array[i] = (i % 10) * 0.1f;
    }
    
    float sum = 0.0f;
    float prod = 1.0f;
    
    /* Complex reduction that may expand to many operands */
    #pragma omp simd reduction(+:sum) reduction(*:prod) \
                simdlen(8) aligned(array:32)
    for (int i = 0; i < N; i++) {
        sum += array[i];
        prod *= (array[i] + 1.0f);
        
        /* Additional operations to increase complexity */
        array[i] = __builtin_fma(array[i], sum, prod);
    }
    
    volatile float result = sum + prod;
    (void)result;
}

/* Main function that calls all test cases */
int main(void) {
    printf("Testing multi-operand instruction patterns...\n");
    
    /* Always test inline assembly */
    multi_operand_asm();
    
    /* Test GCC vector extensions */
    test_gcc_vector_many_operands();
    
    /* Test OpenMP reduction */
    test_omp_reduction_many_operands();
    
    /* Target-specific tests */
    #ifdef __x86_64__
    test_avx512_many_operands();
    printf("x86_64 AVX-512 tests completed\n");
    #elif defined(__aarch64__)
    test_neon_many_operands();
    printf("AArch64 NEON tests completed\n");
    #else
    printf("Generic tests completed (no target-specific SIMD)\n");
    #endif
    
    return 0;
}
