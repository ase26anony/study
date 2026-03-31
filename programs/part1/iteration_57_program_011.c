/* Test program to trigger 10/11-operand instruction expansion in GCC optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Architecture detection */
#if defined(__x86_64__) || defined(__i386__)
#define HAS_X86 1
#include <immintrin.h>
#include <x86intrin.h>
#elif defined(__aarch64__) || defined(__arm__)
#define HAS_ARM 1
#include <arm_neon.h>
#ifdef __ARM_FEATURE_SVE
#include <arm_sve.h>
#endif
#elif defined(__powerpc__) || defined(__PPC__)
#define HAS_PPC 1
#include <altivec.h>
#endif

/* Atomic operations header */
#include <stdatomic.h>

/* Complex expression to force instruction combining */
static inline int complex_expression_10_operands(int a, int b, int c, int d, int e,
                                                  int f, int g, int h, int i, int j) {
    /* This expression might be combined into a multi-operand instruction */
    return a * b + c * d + e * f + g * h + i * j;
}

static inline int complex_expression_11_operands(int a, int b, int c, int d, int e,
                                                 int f, int g, int h, int i, int j, int k) {
    /* 11-operand expression */
    return a * b + c * d + e * f + g * h + i * j + k;
}

#ifdef HAS_X86
/* AVX-512 intrinsics with many operands */
__m512d test_avx512_11_operand(__m512d a, __m512d b, __m512d c, 
                               __mmask8 k, int rounding) {
    /* _mm512_mask3_fmadd_round_pd has 11 parameters in RTL expansion:
       1. Result
       2. a
       3. b
       4. c
       5. k
       6. rounding
       Plus implicit operands for masking and rounding control
    */
    return _mm512_mask3_fmadd_round_pd(a, b, c, k, rounding);
}

__m512i test_avx512_10_operand(__m512i a, __m512i b, __m512i c, __m512i d,
                               __mmask16 k1, __mmask16 k2) {
    /* Complex permute and blend operation */
    __m512i t1 = _mm512_add_epi32(a, b);
    __m512i t2 = _mm512_add_epi32(c, d);
    return _mm512_mask_blend_epi32(k1, t1, _mm512_mask_blend_epi32(k2, t2, a));
}
#endif

#ifdef HAS_ARM
/* ARM SVE2 intrinsics with lane selection */
#ifdef __ARM_FEATURE_SVE
svint32_t test_sve_10_operand(svint32_t a, svint32_t b, svint32_t c,
                              svint32_t d, svint32_t e, svint32_t f,
                              uint64_t lane1, uint64_t lane2) {
    /* Simulate complex SVE operation with multiple lane selections */
    svint32_t t1 = svmla_lane_s32(a, b, c, lane1);
    return svmla_lane_s32(t1, d, e, lane2);
}
#endif

/* ARM Neon with multiple vector registers */
int32x4_t test_neon_10_operand(int32x4_t a, int32x4_t b, int32x4_t c,
                               int32x4_t d, int32x4_t e, int32x4_t f,
                               int32x4_t g, int32x4_t h, int32x4_t i,
                               int32x4_t j) {
    /* Chain of operations that might combine */
    int32x4_t t1 = vmlaq_s32(a, b, c);
    int32x4_t t2 = vmlaq_s32(d, e, f);
    int32x4_t t3 = vmlaq_s32(g, h, i);
    return vaddq_s32(vaddq_s32(t1, t2), vaddq_s32(t3, j));
}
#endif

#ifdef HAS_PPC
/* PowerPC Altivec/VSX with complex permute */
vector signed int test_vsx_10_operand(vector signed int a, vector signed int b,
                                      vector signed int c, vector signed int d,
                                      vector signed int e, vector signed int f,
                                      vector signed int g, vector signed int h,
                                      vector unsigned char perm) {
    /* Complex permute and arithmetic */
    vector signed int t1 = vec_add(a, b);
    vector signed int t2 = vec_add(c, d);
    vector signed int t3 = vec_add(e, f);
    vector signed int t4 = vec_perm(t1, t2, perm);
    return vec_add(vec_add(t3, t4), vec_add(g, h));
}
#endif

/* Inline assembly with exactly 11 operands */
static int inline_asm_11_operands(int a, int b, int c, int d, int e,
                                  int f, int g, int h, int i, int j, int k) {
    int result;
    /* This asm has 11 operands: 10 inputs + 1 output */
    __asm__ volatile (
        "# 11-operand assembly block\n\t"
        "mov %1, %0\n\t"  /* Simple move, but operand count matters */
        : "=r" (result)
        : "r" (a), "r" (b), "r" (c), "r" (d), "r" (e),
          "r" (f), "r" (g), "r" (h), "r" (i), "r" (j),
          "m" (k)  /* Memory operand adds complexity */
        : "cc"
    );
    return result;
}

/* Inline assembly with exactly 10 operands */
static int inline_asm_10_operands(int a, int b, int c, int d, int e,
                                  int f, int g, int h, int i, int j) {
    int result;
    __asm__ volatile (
        "# 10-operand assembly block\n\t"
        "add %1, %2, %0\n\t"
        : "=r" (result)
        : "r" (a), "r" (b), "r" (c), "r" (d), "r" (e),
          "r" (f), "r" (g), "r" (h), "r" (i)
        : "cc"
    );
    return result;
}

/* Atomic operation with many parameters */
int test_atomic_11_operand(_Atomic int* ptr, int* expected, int desired,
                           int memorder1, int memorder2, int weak) {
    /* __atomic_compare_exchange has 6 C parameters but expands to more in RTL */
    return __atomic_compare_exchange_n(ptr, expected, desired, weak,
                                       memorder1, memorder2);
}

/* Decimal floating-point built-in (if supported) */
#ifdef __DECIMAL_BID_FORMAT__
__bid128 test_decimal_10_operand(__bid128 a, __bid128 b, __bid128 c,
                                 __bid128 d, __bid128 e, __bid128 f,
                                 __bid128 g, __bid128 h, __bid128 i,
                                 unsigned int round) {
    /* Complex decimal arithmetic chain */
    __bid128 t1 = __bid128_add(a, b);
    __bid128 t2 = __bid128_mul(c, d);
    __bid128 t3 = __bid128_div(e, f);
    __bid128 t4 = __bid128_fma(g, h, i);
    return __bid128_add(__bid128_add(t1, t2), __bid128_add(t3, t4));
}
#endif

/* Vector reduction across multiple registers */
#ifdef HAS_X86
int test_vector_reduction_10(__m256i v1, __m256i v2, __m256i v3, __m256i v4,
                             __m256i v5, __m256i v6, __m256i v7, __m256i v8) {
    /* Horizontal reduction across 8 vectors */
    __m256i sum1 = _mm256_add_epi32(v1, v2);
    __m256i sum2 = _mm256_add_epi32(v3, v4);
    __m256i sum3 = _mm256_add_epi32(v5, v6);
    __m256i sum4 = _mm256_add_epi32(v7, v8);
    
    __m256i sum12 = _mm256_add_epi32(sum1, sum2);
    __m256i sum34 = _mm256_add_epi32(sum3, sum4);
    __m256i total = _mm256_add_epi32(sum12, sum34);
    
    /* Extract and sum all elements */
    int result = 0;
    int temp[8];
    _mm256_storeu_si256((__m256i*)temp, total);
    for (int i = 0; i < 8; i++) {
        result += temp[i];
    }
    return result;
}
#endif

int main() {
    int result = 0;
    
    /* Test complex expressions */
    result += complex_expression_10_operands(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
    result += complex_expression_11_operands(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11);
    
    /* Test inline assembly */
    result += inline_asm_10_operands(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
    result += inline_asm_11_operands(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11);
    
    /* Test atomic operation */
    _Atomic int atomic_var = 42;
    int expected = 42;
    int desired = 100;
    result += test_atomic_11_operand(&atomic_var, &expected, desired,
                                     __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST, 0);
    
#ifdef HAS_X86
    /* Test AVX-512 if available */
    if (__builtin_cpu_supports("avx512f")) {
        __m512d avx_a = _mm512_set1_pd(1.0);
        __m512d avx_b = _mm512_set1_pd(2.0);
        __m512d avx_c = _mm512_set1_pd(3.0);
        __m512d avx_result = test_avx512_11_operand(avx_a, avx_b, avx_c, 0xFF, _MM_FROUND_TO_NEAREST_INT);
        
        __m512i avxi_a = _mm512_set1_epi32(1);
        __m512i avxi_b = _mm512_set1_epi32(2);
        __m512i avxi_c = _mm512_set1_epi32(3);
        __m512i avxi_d = _mm512_set1_epi32(4);
        __m512i avxi_result = test_avx512_10_operand(avxi_a, avxi_b, avxi_c, avxi_d, 0xFFFF, 0xAAAA);
        
        /* Extract some value to prevent elimination */
        double temp[8];
        _mm512_storeu_pd(temp, avx_result);
        result += (int)temp[0];
    }
    
    /* Test vector reduction */
    __m256i v1 = _mm256_set1_epi32(1);
    __m256i v2 = _mm256_set1_epi32(2);
    __m256i v3 = _mm256_set1_epi32(3);
    __m256i v4 = _mm256_set1_epi32(4);
    __m256i v5 = _mm256_set1_epi32(5);
    __m256i v6 = _mm256_set1_epi32(6);
    __m256i v7 = _mm256_set1_epi32(7);
    __m256i v8 = _mm256_set1_epi32(8);
    result += test_vector_reduction_10(v1, v2, v3, v4, v5, v6, v7, v8);
#endif
    
#ifdef HAS_ARM
    /* Test ARM Neon */
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
    
    int32x4_t neon_result = test_neon_10_operand(neon_a, neon_b, neon_c, neon_d,
                                                 neon_e, neon_f, neon_g, neon_h,
                                                 neon_i, neon_j);
    
    int32_t neon_temp[4];
    vst1q_s32(neon_temp, neon_result);
    result += neon_temp[0];
#endif
    
    printf("Result: %d\n", result);
    return result != 0 ? 0 : 1;
}
