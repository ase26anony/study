/* test_multi_operand.c - Test program for GCC optabs 10/11-operand expansion */
#include <stdio.h>
#include <stdint.h>
#include <stdatomic.h>

/* Enable architecture-specific intrinsics */
#if defined(__x86_64__) || defined(__i386__)
#include <immintrin.h>
#include <x86intrin.h>
#elif defined(__ARM_NEON) || defined(__ARM_NEON__)
#include <arm_neon.h>
#elif defined(__aarch64__)
#include <arm_neon.h>
#include <arm_acle.h>
#endif

/* Complex expression that might combine into multi-operand instruction */
static inline int complex_expression(int a, int b, int c, int d, int e,
                                     int f, int g, int h, int i, int j) {
    /* Fused multiply-add chain with 10 operands */
    return a * b + c * d + e * f + g * h + i * j;
}

/* Function using inline assembly with 11 operands */
static inline uint64_t multi_operand_asm(uint64_t a, uint64_t b, uint64_t c,
                                         uint64_t d, uint64_t e, uint64_t f,
                                         uint64_t g, uint64_t h, uint64_t i,
                                         uint64_t j, uint64_t k) {
    uint64_t result;
    
    /* Inline assembly with 11 operands to force optabs expansion */
    __asm__ volatile (
        /* Template doesn't matter much - we care about operand count */
        "add %[res], %[a], %[b]\n\t"
        "add %[res], %[res], %[c]\n\t"
        "add %[res], %[res], %[d]\n\t"
        "add %[res], %[res], %[e]\n\t"
        "add %[res], %[res], %[f]\n\t"
        "add %[res], %[res], %[g]\n\t"
        "add %[res], %[res], %[h]\n\t"
        "add %[res], %[res], %[i]\n\t"
        "add %[res], %[res], %[j]\n\t"
        "add %[res], %[res], %[k]"
        : [res] "=r" (result)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j), [k] "r" (k)
        : "cc"
    );
    
    return result;
}

#if defined(__x86_64__) && defined(__AVX512F__)
/* AVX-512 masked operation with many operands */
static inline __m512d avx512_multi_operand(__m512d a, __m512d b, __m512d c,
                                           __m512d d, __m512d e, __mmask8 k) {
    /* Fused multiply-add with mask and rounding - potentially 10+ operands */
    __m512d t1 = _mm512_mask3_fmadd_pd(a, b, c, k);
    __m512d t2 = _mm512_mask3_fmadd_pd(d, e, t1, k);
    return t2;
}
#endif

#if defined(__aarch64__) && defined(__ARM_FEATURE_SVE)
#include <arm_sve.h>
/* SVE2 intrinsic with lane selection - potentially many operands */
static inline svint32_t sve_multi_lane(svint32_t a, svint32_t b, svint32_t c,
                                       svint32_t d, svint32_t e, svint32_t f,
                                       svint32_t g, svint32_t h) {
    /* Complex SVE operation chain */
    svint32_t t1 = svmla_lane_s32(a, b, c, 0);
    svint32_t t2 = svmla_lane_s32(d, e, f, 1);
    svint32_t t3 = svadd_s32_z(svptrue_b32(), t1, t2);
    svint32_t t4 = svmla_lane_s32(g, h, t3, 2);
    return t4;
}
#endif

/* Atomic operation with many parameters */
static inline int atomic_multi_operand(_Atomic int* ptr, int* expected,
                                       int desired, int weak, int success_memorder,
                                       int failure_memorder) {
    int result = __atomic_compare_exchange(ptr, expected, &desired, weak,
                                           success_memorder, failure_memorder);
    return result;
}

/* Vector reduction across multiple registers */
static inline int vector_reduction(int32_t* data) {
#if defined(__ARM_NEON) || defined(__ARM_NEON__)
    int32x4_t v1 = vld1q_s32(data);
    int32x4_t v2 = vld1q_s32(data + 4);
    int32x4_t v3 = vld1q_s32(data + 8);
    int32x4_t v4 = vld1q_s32(data + 12);
    
    /* Complex vector operation chain */
    int32x4_t sum1 = vaddq_s32(v1, v2);
    int32x4_t sum2 = vaddq_s32(v3, v4);
    int32x4_t sum = vaddq_s32(sum1, sum2);
    
    /* Horizontal reduction */
    int32x2_t sum2d = vadd_s32(vget_low_s32(sum), vget_high_s32(sum));
    int32x2_t final = vpadd_s32(sum2d, sum2d);
    return vget_lane_s32(final, 0);
#elif defined(__x86_64__)
    __m128i v1 = _mm_load_si128((__m128i*)data);
    __m128i v2 = _mm_load_si128((__m128i*)(data + 4));
    __m128i v3 = _mm_load_si128((__m128i*)(data + 8));
    __m128i v4 = _mm_load_si128((__m128i*)(data + 12));
    
    __m128i sum1 = _mm_add_epi32(v1, v2);
    __m128i sum2 = _mm_add_epi32(v3, v4);
    __m128i sum = _mm_add_epi32(sum1, sum2);
    
    /* Horizontal reduction */
    sum = _mm_hadd_epi32(sum, sum);
    sum = _mm_hadd_epi32(sum, sum);
    return _mm_cvtsi128_si32(sum);
#else
    /* Fallback scalar version */
    int total = 0;
    for (int i = 0; i < 16; i++) {
        total += data[i];
    }
    return total;
#endif
}

/* Bit-field manipulation across multiple words */
static inline uint64_t bitfield_operations(uint64_t a, uint64_t b, uint64_t c,
                                           uint64_t d, uint64_t e, uint64_t f,
                                           uint64_t g, uint64_t h, uint64_t i,
                                           uint64_t j) {
    /* Complex bitfield extraction and insertion */
    uint64_t result = 0;
    
    /* Extract bits from each input and combine */
    result |= (a & 0xFF) << 0;
    result |= (b & 0xFF) << 8;
    result |= (c & 0xFF) << 16;
    result |= (d & 0xFF) << 24;
    result |= (e & 0xFF) << 32;
    result |= (f & 0xFF) << 40;
    result |= (g & 0xFF) << 48;
    result |= (h & 0xFF) << 56;
    
    /* More operations to encourage combining */
    result ^= i;
    result += j;
    
    return result;
}

int main() {
    int result = 0;
    
    /* Test 1: Complex arithmetic expression with 10 operands */
    int expr_result = complex_expression(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
    result += expr_result;
    printf("Complex expression result: %d\n", expr_result);
    
    /* Test 2: Inline assembly with 11 operands */
    uint64_t asm_result = multi_operand_asm(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11);
    result += (int)asm_result;
    printf("Assembly 11-operand result: %lu\n", asm_result);
    
    /* Test 3: Bitfield operations with 10 operands */
    uint64_t bitfield_result = bitfield_operations(0xAA, 0xBB, 0xCC, 0xDD,
                                                   0xEE, 0xFF, 0x11, 0x22,
                                                   0x33, 0x44);
    result += (int)bitfield_result;
    printf("Bitfield result: %lx\n", bitfield_result);
    
    /* Test 4: Vector reduction */
    int32_t vector_data[16];
    for (int i = 0; i < 16; i++) {
        vector_data[i] = i + 1;
    }
    int vector_result = vector_reduction(vector_data);
    result += vector_result;
    printf("Vector reduction result: %d\n", vector_result);
    
    /* Test 5: Atomic operation with many parameters */
    _Atomic int atomic_var = 42;
    int expected = 42;
    int desired = 100;
    int atomic_result = atomic_multi_operand(&atomic_var, &expected, desired,
                                             0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    result += atomic_result;
    printf("Atomic operation result: %d (atomic_var=%d)\n", atomic_result, atomic_var);
    
#if defined(__x86_64__) && defined(__AVX512F__)
    /* Test 6: AVX-512 multi-operand intrinsic */
    __m512d avx_a = _mm512_set1_pd(1.0);
    __m512d avx_b = _mm512_set1_pd(2.0);
    __m512d avx_c = _mm512_set1_pd(3.0);
    __m512d avx_d = _mm512_set1_pd(4.0);
    __m512d avx_e = _mm512_set1_pd(5.0);
    __mmask8 mask = 0xFF;
    
    __m512d avx_result = avx512_multi_operand(avx_a, avx_b, avx_c, avx_d, avx_e, mask);
    double avx_sum = _mm512_reduce_add_pd(avx_result);
    result += (int)avx_sum;
    printf("AVX-512 result sum: %f\n", avx_sum);
#endif
    
    /* Additional test: Chain of operations that might combine */
    int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8, i = 9, j = 10, k = 11;
    int chain_result = a * b + c * d + e * f + g * h + i * j + k;
    result += chain_result;
    printf("Operation chain result: %d\n", chain_result);
    
    return result;
}
