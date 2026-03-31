/* Test program to trigger 10/11-operand expansion in optabs.cc */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Architecture detection */
#if defined(__x86_64__) || defined(__i386__)
#define X86_ARCH 1
#include <immintrin.h>
#include <x86intrin.h>
#elif defined(__aarch64__) || defined(__arm__)
#define ARM_ARCH 1
#include <arm_neon.h>
#ifdef __ARM_FEATURE_SVE
#include <arm_sve.h>
#endif
#elif defined(__powerpc__) || defined(__PPC__)
#define PPC_ARCH 1
#include <altivec.h>
#endif

/* For atomic operations */
#include <stdatomic.h>

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
    
    /* Inline assembly with 11 operands (1 output + 10 inputs) */
    __asm__ volatile (
        /* Simple operation that uses all operands */
        "add %[out], %[a], %[b]\n\t"
        "add %[out], %[out], %[c]\n\t"
        "add %[out], %[out], %[d]\n\t"
        "add %[out], %[out], %[e]\n\t"
        "add %[out], %[out], %[f]\n\t"
        "add %[out], %[out], %[g]\n\t"
        "add %[out], %[out], %[h]\n\t"
        "add %[out], %[out], %[i]\n\t"
        "add %[out], %[out], %[j]\n\t"
        "add %[out], %[out], %[k]"
        : [out] "=r" (result)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j), [k] "r" (k)
        : "cc"
    );
    
    return result;
}

#if X86_ARCH
/* AVX-512 intrinsic with many operands */
static inline __m512d avx512_multi_operand(__m512d a, __m512d b, __m512d c,
                                           __m512d d, __m512d e, __m512d f,
                                           __mmask8 k, int rc) {
    /* Fused multiply-add with mask and rounding control */
    /* This could expand to instruction with many operands */
    __m512d t1 = _mm512_fmadd_pd(a, b, c);
    __m512d t2 = _mm512_fmadd_pd(d, e, f);
    
    /* Masked operation with rounding control */
    return _mm512_mask_add_pd(t1, k, t1, t2);
}

/* Complex vector expression */
static inline __m512i vector_chain(__m512i v0, __m512i v1, __m512i v2,
                                   __m512i v3, __m512i v4, __m512i v5,
                                   __m512i v6, __m512i v7, __m512i v8,
                                   __m512i v9) {
    /* Chain of operations that might combine */
    __m512i t0 = _mm512_add_epi32(v0, v1);
    __m512i t1 = _mm512_add_epi32(v2, v3);
    __m512i t2 = _mm512_add_epi32(v4, v5);
    __m512i t3 = _mm512_add_epi32(v6, v7);
    __m512i t4 = _mm512_add_epi32(v8, v9);
    
    __m512i sum = _mm512_add_epi32(t0, t1);
    sum = _mm512_add_epi32(sum, t2);
    sum = _mm512_add_epi32(sum, t3);
    return _mm512_add_epi32(sum, t4);
}
#endif

#if ARM_ARCH
/* ARM SVE/SVE2 style multi-operand operation simulation */
static inline int32x4_t arm_multi_lane(int32x4_t a, int32x4_t b, int32x4_t c,
                                       int32x4_t d, int32x4_t e, int32x4_t f,
                                       int32x4_t g, int32x4_t h, int lane) {
    /* Simulate complex lane operation across multiple vectors */
    int32x4_t t1 = vmlaq_laneq_s32(a, b, c, lane);
    int32x4_t t2 = vmlaq_laneq_s32(d, e, f, lane);
    return vaddq_s32(t1, t2);
}

/* Multiple vector dot product simulation */
static inline int32x4_t multi_dot_product(int32x4_t a, int32x4_t b, int32x4_t c,
                                          int32x4_t d, int32x4_t e, int32x4_t f,
                                          int32x4_t g, int32x4_t h, int32x4_t i,
                                          int32x4_t j) {
    /* Chain of multiply-add operations */
    int32x4_t acc = vmulq_s32(a, b);
    acc = vmlaq_s32(acc, c, d);
    acc = vmlaq_s32(acc, e, f);
    acc = vmlaq_s32(acc, g, h);
    acc = vmlaq_s32(acc, i, j);
    return acc;
}
#endif

#if PPC_ARCH
/* PowerPC Altivec complex permute */
static inline vector signed int ppc_complex_permute(vector signed int a,
                                                    vector signed int b,
                                                    vector signed int c,
                                                    vector signed int d,
                                                    vector signed int e,
                                                    vector signed int f,
                                                    vector signed int g,
                                                    vector signed int h) {
    /* Complex vector permutation and arithmetic */
    vector signed int t1 = vec_add(a, b);
    vector signed int t2 = vec_add(c, d);
    vector signed int t3 = vec_add(e, f);
    vector signed int t4 = vec_add(g, h);
    
    vector signed int perm1 = vec_perm(t1, t2, (vector unsigned char){0,1,2,3,4,5,6,7,16,17,18,19,20,21,22,23});
    vector signed int perm2 = vec_perm(t3, t4, (vector unsigned char){0,1,2,3,4,5,6,7,16,17,18,19,20,21,22,23});
    
    return vec_add(perm1, perm2);
}
#endif

/* Atomic operation with many parameters */
static inline int atomic_multi_operand(_Atomic int* ptr, int expected, 
                                       int desired, int* weak, int success_memorder,
                                       int failure_memorder) {
    int result = 0;
    /* __atomic_compare_exchange has 6 parameters, which might expand further */
    __atomic_compare_exchange(ptr, &expected, &desired, *weak, 
                             success_memorder, failure_memorder);
    return result;
}

/* Bit-field manipulation across multiple words */
static inline uint64_t bitfield_ops(uint64_t a, uint64_t b, uint64_t c,
                                    uint64_t d, uint64_t e, uint64_t f,
                                    uint64_t g, uint64_t h, uint64_t i,
                                    uint64_t j) {
    /* Complex bit-field extraction and insertion */
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
    
    /* Additional operations */
    result ^= i;
    result += j;
    
    return result;
}

int main() {
    int result = 0;
    
    /* Test 1: Complex expression with 10 operands */
    result += complex_expression(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
    
    /* Test 2: Inline assembly with 11 operands */
    result += multi_operand_asm(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11);
    
    /* Test 3: Bit-field operations */
    result += bitfield_ops(0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
                           0x11, 0x22, 0x33, 0x44);
    
#if X86_ARCH
    /* Test 4: AVX-512 operations */
    if (__builtin_cpu_supports("avx512f")) {
        __m512d avx_a = _mm512_set1_pd(1.0);
        __m512d avx_b = _mm512_set1_pd(2.0);
        __m512d avx_c = _mm512_set1_pd(3.0);
        __m512d avx_d = _mm512_set1_pd(4.0);
        __m512d avx_e = _mm512_set1_pd(5.0);
        __m512d avx_f = _mm512_set1_pd(6.0);
        
        __m512d avx_result = avx512_multi_operand(avx_a, avx_b, avx_c,
                                                  avx_d, avx_e, avx_f,
                                                  0xFF, _MM_FROUND_TO_NEAREST_INT);
        double avx_sum[8];
        _mm512_storeu_pd(avx_sum, avx_result);
        result += (int)avx_sum[0];
    }
#endif
    
#if ARM_ARCH
    /* Test 5: ARM NEON operations */
    int32x4_t neon_a = vdupq_n_s32(1);
    int32x4_t neon_b = vdupq_n_s32(2);
    int32x4_t neon_c = vdupq_n_s32(3);
    int32x4_t neon_d = vdupq_n_s32(4);
    int32x4_t neon_e = vdupq_n_s32(5);
    int32x4_t neon_f = vdupq_n_s32(6);
    int32x4_t neon_g = vdupq_n_s32(7);
    int32x4_t neon_h = vdupq_n_s32(8);
    int32x4_t neon_i = vdupq_n_s32(9);
    int32x4_t neon_j = vdupq_n_s32(10);
    
    int32x4_t neon_result = multi_dot_product(neon_a, neon_b, neon_c, neon_d,
                                              neon_e, neon_f, neon_g, neon_h,
                                              neon_i, neon_j);
    int32_t neon_sum[4];
    vst1q_s32(neon_sum, neon_result);
    result += neon_sum[0];
#endif
    
#if PPC_ARCH
    /* Test 6: PowerPC Altivec operations */
    vector signed int ppc_a = (vector signed int){1, 2, 3, 4};
    vector signed int ppc_b = (vector signed int){5, 6, 7, 8};
    vector signed int ppc_c = (vector signed int){9, 10, 11, 12};
    vector signed int ppc_d = (vector signed int){13, 14, 15, 16};
    vector signed int ppc_e = (vector signed int){17, 18, 19, 20};
    vector signed int ppc_f = (vector signed int){21, 22, 23, 24};
    vector signed int ppc_g = (vector signed int){25, 26, 27, 28};
    vector signed int ppc_h = (vector signed int){29, 30, 31, 32};
    
    vector signed int ppc_result = ppc_complex_permute(ppc_a, ppc_b, ppc_c, ppc_d,
                                                       ppc_e, ppc_f, ppc_g, ppc_h);
    int ppc_sum[4];
    vec_st(ppc_result, 0, ppc_sum);
    result += ppc_sum[0];
#endif
    
    /* Test 7: Atomic operation */
    _Atomic int atomic_var = 42;
    int expected = 42;
    int desired = 100;
    int weak = 0;
    result += atomic_multi_operand(&atomic_var, expected, desired, &weak,
                                   memory_order_seq_cst, memory_order_seq_cst);
    
    printf("Result: %d\n", result);
    return result == 0 ? 0 : 1;
}
