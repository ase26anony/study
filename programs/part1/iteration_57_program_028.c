/* test_multi_operand_expansion.c
 * 
 * This program is designed to trigger GCC's optabs expansion for
 * instructions with 10 or 11 operands. It uses multiple strategies:
 * 1. Vector intrinsics with many operands
 * 2. Inline assembly with many constraints
 * 3. Built-in functions with many parameters
 * 4. Complex expressions that may combine
 */

#include <stdint.h>
#include <string.h>
#include <stdio.h>

/* Prevent dead code elimination */
volatile int g_volatile_sink;

/* Generic fallback for architectures without specific intrinsics */
void generic_multi_operand_test(void) {
    /* Complex expression that might combine into multi-operand instruction */
    int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8, i = 9, j = 10;
    
    /* A chain that could potentially be combined */
    int result = a * b + c * d + e * f + g * h + i * j;
    g_volatile_sink = result;
    
    /* Bitfield operations across multiple words */
    struct {
        unsigned int a : 3;
        unsigned int b : 5;
        unsigned int c : 7;
        unsigned int d : 9;
        unsigned int e : 8;
    } bits = {1, 2, 3, 4, 5};
    
    /* Multiple bitfield extractions in one expression */
    unsigned int bit_result = (bits.a << 0) | (bits.b << 3) | 
                              (bits.c << 8) | (bits.d << 15) | 
                              (bits.e << 24);
    g_volatile_sink = bit_result;
}

/* Inline assembly with many operands - works on all architectures */
void inline_asm_multi_operand(void) {
    int op0 = 0, op1 = 1, op2 = 2, op3 = 3, op4 = 4;
    int op5 = 5, op6 = 6, op7 = 7, op8 = 8, op9 = 9, op10 = 10;
    int result;
    
    /* 11-operand inline asm - forces expansion */
    asm volatile (
        /* Template doesn't matter much - we just need the operand count */
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
        : "r" (op0), "r" (op1), "r" (op2), "r" (op3), 
          "r" (op4), "r" (op5), "r" (op6), "r" (op7),
          "r" (op8), "r" (op9), "r" (op10)
        : "cc"
    );
    
    g_volatile_sink = result;
}

/* Atomic built-in with many parameters */
void atomic_multi_operand(void) {
    long long atomic_var = 0;
    long long expected = 0;
    long long desired = 42;
    long long *ptr = &atomic_var;
    
    /* __atomic_compare_exchange has 6 parameters, which may expand further */
    int success = __atomic_compare_exchange(ptr, &expected, &desired,
                                            0, /* weak */
                                            __ATOMIC_SEQ_CST, 
                                            __ATOMIC_SEQ_CST);
    
    g_volatile_sink = success;
    g_volatile_sink = atomic_var;
}

#ifdef __x86_64__
#include <immintrin.h>

void x86_avx512_multi_operand(void) {
    /* AVX-512 has many instructions with multiple operands */
    __m512i a = _mm512_set1_epi32(1);
    __m512i b = _mm512_set1_epi32(2);
    __m512i c = _mm512_set1_epi32(3);
    __mmask16 mask = 0xAAAA;  /* 1010101010101010 */
    
    /* Fused multiply-add with mask and rounding control */
    __m512d d = _mm512_set1_pd(1.0);
    __m512d e = _mm512_set1_pd(2.0);
    __m512d f = _mm512_set1_pd(3.0);
    
    /* Complex expression that might use multi-operand instructions */
    __m512d result1 = _mm512_fmadd_pd(d, e, f);
    
    /* Store result to prevent elimination */
    double buffer[8];
    _mm512_storeu_pd(buffer, result1);
    g_volatile_sink = (int)buffer[0];
    
    /* Try to trigger masked operation expansion */
    __m512i result2 = _mm512_mask_add_epi32(a, mask, b, c);
    
    int buffer2[16];
    _mm512_storeu_si512(buffer2, result2);
    g_volatile_sink = buffer2[0];
}
#endif

#ifdef __ARM_ARCH
#ifdef __ARM_NEON
#include <arm_neon.h>

void arm_neon_multi_operand(void) {
    /* ARM NEON/SVE may have multi-operand instructions */
    int32x4_t a = vdupq_n_s32(1);
    int32x4_t b = vdupq_n_s32(2);
    int32x4_t c = vdupq_n_s32(3);
    int32x4_t d = vdupq_n_s32(4);
    int32x4_t e = vdupq_n_s32(5);
    
    /* Chain of operations that might combine */
    int32x4_t result = vaddq_s32(a, b);
    result = vmlaq_s32(result, c, d);
    result = vmlaq_s32(result, e, a);
    
    int32_t res_arr[4];
    vst1q_s32(res_arr, result);
    g_volatile_sink = res_arr[0];
}
#endif
#endif

#ifdef __PPC64__
#include <altivec.h>

void powerpc_vsx_multi_operand(void) {
    /* PowerPC VSX/Altivec may have complex permute operations */
    vector int va = {1, 2, 3, 4};
    vector int vb = {5, 6, 7, 8};
    vector int vc = {9, 10, 11, 12};
    vector int vd = {13, 14, 15, 16};
    
    /* Complex expression with multiple operations */
    vector int result = vec_add(va, vb);
    result = vec_madd(result, vc, vd);
    
    int res_arr[4];
    memcpy(res_arr, &result, sizeof(result));
    g_volatile_sink = res_arr[0];
}
#endif

/* Decimal floating point built-ins (if supported) */
#ifdef __DECIMAL_BID_FORMAT__
void decimal_multi_operand(void) {
    /* Some decimal float built-ins take many arguments */
    _Decimal128 d1 = 1.0DL;
    _Decimal128 d2 = 2.0DL;
    _Decimal128 d3 = 3.0DL;
    
    /* Complex decimal expression */
    _Decimal128 result = d1 * d2 + d3;
    
    /* Convert to int for sink */
    g_volatile_sink = (int)result;
}
#endif

/* Vector reduction across multiple registers */
void vector_reduction_test(void) {
    /* Create many variables for a complex reduction */
    int v0 = 1, v1 = 2, v2 = 3, v3 = 4, v4 = 5;
    int v5 = 6, v6 = 7, v7 = 8, v8 = 9, v9 = 10;
    
    /* Complex expression with 10 operands */
    int reduction = v0 + v1 * v2 - v3 / (v4 + 1) + 
                    v5 * v6 - v7 + v8 * v9;
    
    g_volatile_sink = reduction;
    
    /* Another complex chain */
    int chain = ((((v0 * v1) + v2) * v3 + v4) * v5 + v6) * v7 + v8 * v9;
    g_volatile_sink = chain;
}

int main(void) {
    printf("Testing multi-operand instruction expansion...\n");
    
    /* Execute all test functions */
    generic_multi_operand_test();
    inline_asm_multi_operand();
    atomic_multi_operand();
    vector_reduction_test();
    
    /* Architecture-specific tests */
#ifdef __x86_64__
    x86_avx512_multi_operand();
#endif
    
#ifdef __ARM_ARCH
#ifdef __ARM_NEON
    arm_neon_multi_operand();
#endif
#endif
    
#ifdef __PPC64__
    powerpc_vsx_multi_operand();
#endif
    
#ifdef __DECIMAL_BID_FORMAT__
    decimal_multi_operand();
#endif
    
    printf("Tests completed (check RTL dumps for 10/11-operand expansion)\n");
    return g_volatile_sink != 0 ? 0 : 1;
}
