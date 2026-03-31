#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Architecture-specific headers */
#if defined(__x86_64__) || defined(__i386__)
#include <immintrin.h>
#include <x86intrin.h>
#elif defined(__aarch64__) || defined(__arm__)
#include <arm_neon.h>
#include <arm_acle.h>
#ifdef __ARM_FEATURE_SVE
#include <arm_sve.h>
#endif
#elif defined(__powerpc64__) || defined(__powerpc__)
#include <altivec.h>
#endif

/* Complex expression that might combine into multi-operand instruction */
static inline int complex_expression(int a, int b, int c, int d, int e,
                                     int f, int g, int h, int i, int j) {
    /* This could potentially be combined into a single instruction
       with many operands on some architectures */
    return a * b + c * d + e * f + g * h + i * j;
}

/* Function using inline assembly with 11 operands */
static inline void eleven_operand_asm(void) {
    long op1 = 1, op2 = 2, op3 = 3, op4 = 4, op5 = 5;
    long op6 = 6, op7 = 7, op8 = 8, op9 = 9, op10 = 10;
    long result;
    
    /* Inline assembly with exactly 11 operands to trigger the case */
    asm volatile (
        "/* 11-operand assembly block */\n\t"
        "mov %0, %1\n\t"
        : "=r" (result)
        : "r" (op1), "r" (op2), "r" (op3), "r" (op4), "r" (op5),
          "r" (op6), "r" (op7), "r" (op8), "r" (op9), "r" (op10)
        : "memory"
    );
    
    (void)result; /* Prevent unused variable warning */
}

/* Function using atomic built-in with many parameters */
static inline int atomic_multi_operand(void) {
    _Atomic int atomic_var = 0;
    int expected = 0;
    int desired = 42;
    int weak = 0;
    int success_memorder = __ATOMIC_SEQ_CST;
    int failure_memorder = __ATOMIC_SEQ_CST;
    
    /* __atomic_compare_exchange has 6 parameters, which when expanded
       might become 10+ operands depending on the architecture */
    return __atomic_compare_exchange_n(&atomic_var, &expected, desired,
                                       weak, success_memorder, failure_memorder);
}

#if defined(__x86_64__) || defined(__i386__)
/* x86 AVX-512 intrinsics that can use many operands */
static inline __m512i avx512_multi_operand(__m512i a, __m512i b, __m512i c,
                                           __m512i d, __m512i e, __m512i f,
                                           __mmask16 k1, __mmask16 k2) {
    /* AVX-512 masked operations can have many operands:
       dest, src1, src2, src3, mask, rounding control, etc. */
    __m512i temp1 = _mm512_mask_add_epi32(a, k1, b, c);
    __m512i temp2 = _mm512_mask_mul_epi32(d, k2, e, f);
    return _mm512_add_epi32(temp1, temp2);
}

/* FMA operation that might combine */
static inline __m512d avx512_fma_chain(__m512d a, __m512d b, __m512d c,
                                       __m512d d, __m512d e, __m512d f,
                                       __m512d g, __m512d h, __m512d i) {
    /* Chain of FMA operations that might be combined */
    __m512d result = _mm512_fmadd_pd(a, b, c);
    result = _mm512_fmadd_pd(result, d, e);
    result = _mm512_fmadd_pd(result, f, g);
    result = _mm512_fmadd_pd(result, h, i);
    return result;
}
#endif

#if defined(__aarch64__) || defined(__arm__)
/* ARM NEON/SVE operations with many operands */
#ifdef __ARM_FEATURE_SVE
static inline svint32_t sve_multi_operand(svint32_t a, svint32_t b, svint32_t c,
                                          svint32_t d, svint32_t e, svint32_t f,
                                          svint32_t g, svint32_t h) {
    /* SVE2 operations like svmla_lane can have many operands */
    svint32_t temp1 = svmla_lane_s32(a, b, c, 0);
    svint32_t temp2 = svmla_lane_s32(d, e, f, 1);
    return svadd_s32(temp1, temp2);
}
#endif

/* ARM NEON operations */
static inline int32x4_t neon_multi_operand(int32x4_t a, int32x4_t b, int32x4_t c,
                                           int32x4_t d, int32x4_t e, int32x4_t f) {
    /* vmla_lane can have multiple vector registers and lane indices */
    int32x4_t temp1 = vmlaq_lane_s32(a, b, vget_low_s32(c), 0);
    int32x4_t temp2 = vmlaq_lane_s32(d, e, vget_high_s32(f), 1);
    return vaddq_s32(temp1, temp2);
}
#endif

#if defined(__powerpc64__) || defined(__powerpc__)
/* PowerPC Altivec/VSX operations */
static inline vector int altivec_multi_operand(vector int a, vector int b,
                                               vector int c, vector int d,
                                               vector int e, vector int f) {
    /* Complex permute and compute operations */
    vector int temp1 = vec_madd(a, b, c);
    vector int temp2 = vec_madd(d, e, f);
    return vec_add(temp1, temp2);
}
#endif

/* Main driver function */
int main(void) {
    int result = 0;
    
    /* 1. Complex expression with 10+ operands */
    result += complex_expression(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
    
    /* 2. Inline assembly with 11 operands */
    eleven_operand_asm();
    
    /* 3. Atomic operation with many parameters */
    result += atomic_multi_operand();
    
    /* 4. Architecture-specific vector operations */
#if defined(__x86_64__) || defined(__i386__)
    if (__builtin_cpu_supports("avx512f")) {
        __m512i vec1 = _mm512_set1_epi32(1);
        __m512i vec2 = _mm512_set1_epi32(2);
        __m512i vec3 = _mm512_set1_epi32(3);
        __m512i vec4 = _mm512_set1_epi32(4);
        __m512i vec5 = _mm512_set1_epi32(5);
        __m512i vec6 = _mm512_set1_epi32(6);
        
        __mmask16 mask1 = 0xAAAA;
        __mmask16 mask2 = 0x5555;
        
        __m512i vec_result = avx512_multi_operand(vec1, vec2, vec3, vec4, vec5, vec6, mask1, mask2);
        
        /* Extract one element to prevent dead code elimination */
        int temp[16];
        _mm512_storeu_si512(temp, vec_result);
        result += temp[0];
    }
#endif
    
#if defined(__aarch64__) || defined(__arm__)
    int32x4_t neon1 = vdupq_n_s32(1);
    int32x4_t neon2 = vdupq_n_s32(2);
    int32x4_t neon3 = vdupq_n_s32(3);
    int32x4_t neon4 = vdupq_n_s32(4);
    int32x4_t neon5 = vdupq_n_s32(5);
    int32x4_t neon6 = vdupq_n_s32(6);
    
    int32x4_t neon_result = neon_multi_operand(neon1, neon2, neon3, neon4, neon5, neon6);
    
    /* Extract one element */
    int32_t neon_temp[4];
    vst1q_s32(neon_temp, neon_result);
    result += neon_temp[0];
#endif
    
    /* 5. Another complex expression that might chain operations */
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int f = 6, g = 7, h = 8, i = 9, j = 10, k = 11;
    
    /* Expression with 11 variables that might be combined */
    int chain_result = a * b + c * d + e * f + g * h + i * j + k;
    result += chain_result;
    
    printf("Result: %d\n", result);
    return result != 0 ? 0 : 1;
}
