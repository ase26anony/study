/* Test program to trigger 10/11-operand instruction expansion in GCC optabs.cc */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Architecture detection */
#if defined(__x86_64__) || defined(__i386__)
#define TARGET_X86 1
#include <immintrin.h>
#include <x86intrin.h>
#endif

#if defined(__aarch64__) || defined(__arm__)
#define TARGET_ARM 1
#include <arm_neon.h>
#ifdef __ARM_FEATURE_SVE
#include <arm_sve.h>
#endif
#endif

#if defined(__powerpc__) || defined(__PPC__)
#define TARGET_PPC 1
#include <altivec.h>
#endif

/* Complex expression that might combine into multi-operand instruction */
static inline int complex_expression(int a, int b, int c, int d, int e,
                                     int f, int g, int h, int i, int j) {
    /* This expression might be combined into a single multi-operand
       instruction at high optimization levels */
    return a * b + c * d + e * f + g * h + i * j;
}

/* Function using inline assembly with 11 operands */
static inline void multi_operand_asm(void) {
    int64_t in1 = 1, in2 = 2, in3 = 3, in4 = 4, in5 = 5;
    int64_t in6 = 6, in7 = 7, in8 = 8, in9 = 9, in10 = 10;
    int64_t out1, out2;
    
    /* Inline assembly with exactly 11 operands to trigger the 11-operand case */
    asm volatile (
        /* Simple template - the important part is the operand count */
        "add %[out1], %[in1], %[in2]\n\t"
        "add %[out2], %[in3], %[in4]\n\t"
        : [out1] "=r" (out1), [out2] "=r" (out2)
        : [in1] "r" (in1), [in2] "r" (in2), [in3] "r" (in3),
          [in4] "r" (in4), [in5] "r" (in5), [in6] "r" (in6),
          [in7] "r" (in7), [in8] "r" (in8), [in9] "r" (in9),
          [in10] "r" (in10)
        : "cc"
    );
    
    (void)out1; (void)out2; /* Prevent unused variable warnings */
}

/* Function using atomic built-in with many parameters */
static inline int atomic_multi_operand(void) {
    int64_t val = 42;
    int64_t expected = 42;
    int64_t desired = 43;
    int weak = 0;
    int success_memorder = __ATOMIC_SEQ_CST;
    int failure_memorder = __ATOMIC_SEQ_CST;
    
    /* __atomic_compare_exchange has 6 parameters, which when expanded
       with memory order parameters might reach 10+ operands */
    int result = __atomic_compare_exchange(&val, &expected, &desired,
                                          weak, success_memorder,
                                          failure_memorder);
    return result ? (int)val : (int)expected;
}

#ifdef TARGET_X86
/* AVX-512 intrinsics that can generate multi-operand instructions */
static inline __m512i avx512_multi_operand(__m512i a, __m512i b, __m512i c,
                                          __m512i d, __m512i e, __m512i f,
                                          __m512i g, __m512i h, __m512i i,
                                          __m512i j) {
    /* Chain of operations that might be combined */
    __m512i t1 = _mm512_add_epi32(a, b);
    __m512i t2 = _mm512_add_epi32(c, d);
    __m512i t3 = _mm512_add_epi32(e, f);
    __m512i t4 = _mm512_add_epi32(g, h);
    __m512i t5 = _mm512_add_epi32(i, j);
    
    __m512i sum1 = _mm512_add_epi32(t1, t2);
    __m512i sum2 = _mm512_add_epi32(t3, t4);
    __m512i sum3 = _mm512_add_epi32(sum1, sum2);
    
    return _mm512_add_epi32(sum3, t5);
}

/* AVX-512 masked operation with many operands */
static inline __m512d avx512_masked_fma(__m512d a, __m512d b, __m512d c,
                                       __m512d d, __m512d e, __m512d f,
                                       __mmask8 k) {
    /* This could potentially expand to a multi-operand FMA instruction */
    __m512d t1 = _mm512_fmadd_pd(a, b, c);
    __m512d t2 = _mm512_fmadd_pd(d, e, f);
    __m512d result = _mm512_add_pd(t1, t2);
    
    /* Masked operation adds extra operands */
    return _mm512_mask_mov_pd(a, k, result);
}
#endif

#ifdef TARGET_ARM
/* ARM NEON/SVE operations with lane selection - can have many operands */
static inline int32x4_t neon_multi_lane(int32x4_t a, int32x4_t b, int32x4_t c,
                                       int32x4_t d, int32x4_t e, int32x4_t f,
                                       int32x4_t g, int32x4_t h) {
    /* Operations with lane selection increase operand count */
    int32x4_t t1 = vmlaq_laneq_s32(a, b, c, 0);
    int32x4_t t2 = vmlaq_laneq_s32(d, e, f, 1);
    int32x4_t t3 = vmlaq_laneq_s32(g, h, a, 2);
    
    int32x4_t sum1 = vaddq_s32(t1, t2);
    return vaddq_s32(sum1, t3);
}

#ifdef __ARM_FEATURE_SVE
/* SVE2 intrinsics with complex patterns */
static inline svint32_t sve_multi_operand(svint32_t a, svint32_t b, svint32_t c,
                                         svint32_t d, svint32_t e, svint32_t f,
                                         svbool_t pg) {
    /* SVE operations with predicate can have many operands */
    svint32_t t1 = svmla_x(pg, a, b, c);
    svint32_t t2 = svmla_x(pg, d, e, f);
    return svadd_x(pg, t1, t2);
}
#endif
#endif

#ifdef TARGET_PPC
/* PowerPC Altivec operations */
static inline vector signed int ppc_multi_operand(vector signed int a,
                                                 vector signed int b,
                                                 vector signed int c,
                                                 vector signed int d,
                                                 vector signed int e,
                                                 vector signed int f) {
    /* Complex permute and compute operations */
    vector signed int t1 = vec_madd(a, b, c);
    vector signed int t2 = vec_madd(d, e, f);
    return vec_add(t1, t2);
}
#endif

/* Main driver function */
int main(void) {
    int result = 0;
    
    /* 1. Complex arithmetic expression with 10+ variables */
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int f = 6, g = 7, h = 8, i = 9, j = 10;
    
    result += complex_expression(a, b, c, d, e, f, g, h, i, j);
    
    /* 2. Inline assembly with 11 operands */
    multi_operand_asm();
    
    /* 3. Atomic operation with many parameters */
    result += atomic_multi_operand();
    
#ifdef TARGET_X86
    /* 4. AVX-512 operations */
    if (__builtin_cpu_supports("avx512f")) {
        __m512i vec1 = _mm512_set1_epi32(1);
        __m512i vec2 = _mm512_set1_epi32(2);
        __m512i vec3 = _mm512_set1_epi32(3);
        __m512i vec4 = _mm512_set1_epi32(4);
        __m512i vec5 = _mm512_set1_epi32(5);
        __m512i vec6 = _mm512_set1_epi32(6);
        __m512i vec7 = _mm512_set1_epi32(7);
        __m512i vec8 = _mm512_set1_epi32(8);
        __m512i vec9 = _mm512_set1_epi32(9);
        __m512i vec10 = _mm512_set1_epi32(10);
        
        __m512i vec_result = avx512_multi_operand(vec1, vec2, vec3, vec4, vec5,
                                                 vec6, vec7, vec8, vec9, vec10);
        
        /* Extract a single value to add to result */
        int32_t extracted[16];
        _mm512_storeu_si512(extracted, vec_result);
        result += extracted[0];
        
        /* Masked operation */
        __m512d dbl1 = _mm512_set1_pd(1.0);
        __m512d dbl2 = _mm512_set1_pd(2.0);
        __m512d dbl3 = _mm512_set1_pd(3.0);
        __m512d dbl4 = _mm512_set1_pd(4.0);
        __m512d dbl5 = _mm512_set1_pd(5.0);
        __m512d dbl6 = _mm512_set1_pd(6.0);
        
        __m512d dbl_result = avx512_masked_fma(dbl1, dbl2, dbl3, dbl4, dbl5,
                                              dbl6, 0xFF);
        
        double dbl_extracted[8];
        _mm512_storeu_pd(dbl_extracted, dbl_result);
        result += (int)dbl_extracted[0];
    }
#endif

#ifdef TARGET_ARM
    /* 5. ARM NEON operations */
    int32x4_t neon1 = vdupq_n_s32(1);
    int32x4_t neon2 = vdupq_n_s32(2);
    int32x4_t neon3 = vdupq_n_s32(3);
    int32x4_t neon4 = vdupq_n_s32(4);
    int32x4_t neon5 = vdupq_n_s32(5);
    int32x4_t neon6 = vdupq_n_s32(6);
    int32x4_t neon7 = vdupq_n_s32(7);
    int32x4_t neon8 = vdupq_n_s32(8);
    
    int32x4_t neon_result = neon_multi_lane(neon1, neon2, neon3, neon4,
                                           neon5, neon6, neon7, neon8);
    
    int32_t neon_extracted[4];
    vst1q_s32(neon_extracted, neon_result);
    result += neon_extracted[0];
#endif

    /* 6. Another complex expression inline */
    int k = 11, l = 12, m = 13, n = 14, o = 15;
    int p = 16, q = 17, r = 18, s = 19, t = 20;
    
    /* Expression with 10 multiplication operations */
    result += a*k + b*l + c*m + d*n + e*o + f*p + g*q + h*r + i*s + j*t;
    
    printf("Result: %d\n", result);
    return result != 0 ? 0 : 1;
}
