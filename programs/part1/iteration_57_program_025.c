/* Test program to trigger 10/11-operand expansion in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Architecture detection */
#if defined(__x86_64__) || defined(__i386__)
#define TARGET_X86 1
#include <immintrin.h>
#include <x86intrin.h>
#elif defined(__aarch64__) || defined(__arm__)
#define TARGET_ARM 1
#include <arm_neon.h>
#ifdef __ARM_FEATURE_SVE
#include <arm_sve.h>
#endif
#elif defined(__powerpc__) || defined(__PPC__)
#define TARGET_PPC 1
#include <altivec.h>
#endif

/* Atomic operations header */
#include <stdatomic.h>

/* Complex expression to force combining */
static inline int complex_expression(int a, int b, int c, int d, int e,
                                     int f, int g, int h, int i, int j) {
    /* This expression might be combined into multi-operand instruction */
    return a * b + c * d + e * f + g * h + i * j;
}

/* Function using many operands in inline assembly */
static void multi_operand_asm(void) {
    /* 11 operands in inline assembly */
    int op1 = 1, op2 = 2, op3 = 3, op4 = 4, op5 = 5;
    int op6 = 6, op7 = 7, op8 = 8, op9 = 9, op10 = 10;
    int result;
    
    /* Inline assembly with 11 operands - forces expansion */
    __asm__ volatile (
        /* Simple template, but many operands */
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
        : "=r" (result)
        : "r" (op1), "r" (op2), "r" (op3), "r" (op4), "r" (op5),
          "r" (op6), "r" (op7), "r" (op8), "r" (op9), "r" (op10)
        : "cc"
    );
    
    printf("ASM result: %d\n", result);
}

/* Atomic operation with many parameters */
static void atomic_multi_operand(void) {
    _Atomic int atomic_var = 42;
    int expected = 42;
    int desired = 100;
    int weak = 0;
    
    /* __atomic_compare_exchange has 6 parameters, which might expand
       to more operands when considering memory ordering */
    int success = __atomic_compare_exchange_n(&atomic_var, &expected, desired,
                                              weak, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    
    printf("Atomic exchange %s\n", success ? "succeeded" : "failed");
}

#if TARGET_X86
/* AVX-512 operations with many operands */
static __m512i avx512_multi_operand(__m512i a, __m512i b, __m512i c,
                                    __m512i d, __m512i e, __m512i f,
                                    __mmask16 mask) {
    /* Chain of operations that might combine */
    __m512i t1 = _mm512_add_epi32(a, b);
    __m512i t2 = _mm512_add_epi32(c, d);
    __m512i t3 = _mm512_add_epi32(e, f);
    
    /* Masked operation with multiple operands */
    __m512i result = _mm512_mask_add_epi32(t1, mask, t2, t3);
    
    /* FMA with rounding control - up to 11 operands in expansion */
    #ifdef __AVX512F__
    __m512d da = _mm512_set1_pd(1.0);
    __m512d db = _mm512_set1_pd(2.0);
    __m512d dc = _mm512_set1_pd(3.0);
    __mmask8 dmask = 0xFF;
    
    /* _mm512_mask3_fmadd_round_pd has implicit rounding mode operand */
    __m512d dresult = _mm512_mask3_fmadd_round_pd(da, db, dc, dmask, _MM_FROUND_TO_NEAREST_INT);
    
    /* Convert and add to integer result */
    __m512i dconv = _mm512_cvtpd_epi32(dresult);
    result = _mm512_add_epi32(result, dconv);
    #endif
    
    return result;
}
#endif

#if TARGET_ARM
/* ARM SVE/NEON operations with lane selection */
static int32x4_t neon_multi_operand(int32x4_t a, int32x4_t b, int32x4_t c,
                                    int32x4_t d, int32x4_t e, int32x4_t f) {
    /* Operations that use multiple vector registers and lanes */
    int32x4_t t1 = vmlaq_s32(a, b, c);
    int32x4_t t2 = vmlaq_s32(d, e, f);
    
    /* Lane operations add extra operands */
    int32x4_t result = vmlaq_laneq_s32(t1, t2, t1, 1);
    
    #ifdef __ARM_FEATURE_SVE
    /* SVE has instructions with many operands */
    svint32_t sv_a = svld1_s32(svptrue_b32(), (const int32_t*)&a);
    svint32_t sv_b = svld1_s32(svptrue_b32(), (const int32_t*)&b);
    svint32_t sv_c = svld1_s32(svptrue_b32(), (const int32_t*)&c);
    
    /* Complex SVE operation - hypothetical but shows pattern */
    /* svmla_lane has vector, vector, vector, lane index operands */
    #endif
    
    return result;
}
#endif

#if TARGET_PPC
/* PowerPC Altivec operations */
static vector int altivec_multi_operand(vector int a, vector int b,
                                        vector int c, vector int d) {
    /* Complex permute and compute */
    vector int t1 = vec_add(a, b);
    vector int t2 = vec_add(c, d);
    
    /* Permute with multiple control vectors */
    vector unsigned char perm = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    vector int result = vec_perm(t1, t2, perm);
    
    /* Multiply-add with permutation */
    result = vec_madd((vector float)result, (vector float)t1, (vector float)t2);
    
    return result;
}
#endif

/* Decimal floating point operations (if supported) */
#ifdef __DECIMAL_BID_FORMAT__
static _Decimal128 decimal_multi_operand(_Decimal128 a, _Decimal128 b,
                                         _Decimal128 c, _Decimal128 d) {
    /* Decimal operations might expand to multi-operand instructions */
    _Decimal128 t1 = __bid128_add(a, b);
    _Decimal128 t2 = __bid128_mul(c, d);
    _Decimal128 result = __bid128_add(t1, t2);
    
    /* Fused multiply-add with rounding */
    result = __bid128_fma(a, b, c);
    
    return result;
}
#endif

/* Bitfield operations across multiple words */
static uint64_t bitfield_multi_operand(uint64_t a, uint64_t b, uint64_t c,
                                       uint64_t d, uint64_t e) {
    /* Complex bitfield manipulation */
    uint64_t result = a;
    
    /* Insert bits from multiple sources */
    result = (result & ~0xFF00000000000000ULL) | ((b & 0xFF) << 56);
    result = (result & ~0x00FF000000000000ULL) | ((c & 0xFF) << 48);
    result = (result & ~0x0000FF0000000000ULL) | ((d & 0xFF) << 40);
    result = (result & ~0x000000FF00000000ULL) | ((e & 0xFF) << 32);
    
    /* Extract and combine */
    uint64_t ext1 = (a >> 32) & 0xFFFF;
    uint64_t ext2 = (b >> 16) & 0xFFFF;
    uint64_t ext3 = (c >> 0) & 0xFFFF;
    
    result |= (ext1 << 16) | (ext2 << 8) | ext3;
    
    return result;
}

int main(void) {
    int sum = 0;
    
    printf("Testing multi-operand expansion patterns\n");
    
    /* 1. Complex expression with 10+ variables */
    int vars[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    sum += complex_expression(vars[0], vars[1], vars[2], vars[3], vars[4],
                              vars[5], vars[6], vars[7], vars[8], vars[9]);
    
    /* 2. Inline assembly with many operands */
    multi_operand_asm();
    
    /* 3. Atomic operation */
    atomic_multi_operand();
    
    /* 4. Architecture-specific vector operations */
    #if TARGET_X86
    {
        __m512i v1 = _mm512_set1_epi32(1);
        __m512i v2 = _mm512_set1_epi32(2);
        __m512i v3 = _mm512_set1_epi32(3);
        __m512i v4 = _mm512_set1_epi32(4);
        __m512i v5 = _mm512_set1_epi32(5);
        __m512i v6 = _mm512_set1_epi32(6);
        
        __m512i vresult = avx512_multi_operand(v1, v2, v3, v4, v5, v6, 0xFFFF);
        int32_t res_arr[16];
        _mm512_storeu_si512(res_arr, vresult);
        sum += res_arr[0];
    }
    #elif TARGET_ARM
    {
        int32x4_t v1 = {1, 2, 3, 4};
        int32x4_t v2 = {5, 6, 7, 8};
        int32x4_t v3 = {9, 10, 11, 12};
        int32x4_t v4 = {13, 14, 15, 16};
        int32x4_t v5 = {17, 18, 19, 20};
        int32x4_t v6 = {21, 22, 23, 24};
        
        int32x4_t vresult = neon_multi_operand(v1, v2, v3, v4, v5, v6);
        sum += vgetq_lane_s32(vresult, 0);
    }
    #elif TARGET_PPC
    {
        vector int v1 = {1, 2, 3, 4};
        vector int v2 = {5, 6, 7, 8};
        vector int v3 = {9, 10, 11, 12};
        vector int v4 = {13, 14, 15, 16};
        
        vector int vresult = altivec_multi_operand(v1, v2, v3, v4);
        sum += ((int*)&vresult)[0];
    }
    #endif
    
    /* 5. Bitfield operations */
    sum += bitfield_multi_operand(0x123456789ABCDEF0ULL,
                                  0xFEDCBA9876543210ULL,
                                  0xAAAAAAAAAAAAAAAAULL,
                                  0x5555555555555555ULL,
                                  0xFFFFFFFFFFFFFFFFULL);
    
    /* 6. Decimal floating point if available */
    #ifdef __DECIMAL_BID_FORMAT__
    {
        _Decimal128 d1 = 1.23456789DL;
        _Decimal128 d2 = 9.87654321DL;
        _Decimal128 d3 = 5.55555555DL;
        _Decimal128 d4 = 3.33333333DL;
        
        _Decimal128 dresult = decimal_multi_operand(d1, d2, d3, d4);
        sum += (int)((double)dresult);
    }
    #endif
    
    printf("Final sum: %d\n", sum);
    return sum != 0 ? 0 : 1;
}
