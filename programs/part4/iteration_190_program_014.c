/* Test program to cover 10/11 operand expansion in optabs.cc */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Simple PRNG to generate non-constant values */
static uint64_t prng_state = 123456789;

static uint64_t prng_next(void) {
    prng_state = prng_state * 1103515245 + 12345;
    return prng_state;
}

/* Generic fallback using inline assembly with many operands */
#ifdef GENERIC_FALLBACK

/* Function for 10 operands */
static int func_10_operands(int a0, int a1, int a2, int a3, int a4,
                            int a5, int a6, int a7, int a8, int a9) {
    int result;
    /* Extended inline assembly with 10 input operands and 1 output */
    __asm__ volatile (
        "/* dummy 10-operand assembly */\n\t"
        "add %1, %2, %0\n\t"
        "add %3, %0, %0\n\t"
        "add %4, %0, %0\n\t"
        "add %5, %0, %0\n\t"
        "add %6, %0, %0\n\t"
        "add %7, %0, %0\n\t"
        "add %8, %0, %0\n\t"
        "add %9, %0, %0\n\t"
        "add %10, %0, %0"
        : "=r" (result)
        : "r" (a0), "r" (a1), "r" (a2), "r" (a3), "r" (a4),
          "r" (a5), "r" (a6), "r" (a7), "r" (a8), "r" (a9)
        : "cc"
    );
    return result + a0 + a1; /* Ensure result is used */
}

/* Function for 11 operands */
static int func_11_operands(int a0, int a1, int a2, int a3, int a4,
                            int a5, int a6, int a7, int a8, int a9,
                            int a10) {
    int result;
    /* Extended inline assembly with 11 input operands and 1 output */
    __asm__ volatile (
        "/* dummy 11-operand assembly */\n\t"
        "add %1, %2, %0\n\t"
        "add %3, %0, %0\n\t"
        "add %4, %0, %0\n\t"
        "add %5, %0, %0\n\t"
        "add %6, %0, %0\n\t"
        "add %7, %0, %0\n\t"
        "add %8, %0, %0\n\t"
        "add %9, %0, %0\n\t"
        "add %10, %0, %0\n\t"
        "add %11, %0, %0"
        : "=r" (result)
        : "r" (a0), "r" (a1), "r" (a2), "r" (a3), "r" (a4),
          "r" (a5), "r" (a6), "r" (a7), "r" (a8), "r" (a9),
          "r" (a10)
        : "cc"
    );
    return result + a0 + a1 + a10; /* Ensure result is used */
}

#elif defined(__AVX512F__) && defined(__AVX512VL__)
/* x86 AVX-512 implementation using high-operand-count intrinsics */
#include <immintrin.h>

/* Function for 10 operands using AVX-512 mask compress/store */
static int func_10_operands(__m512i v0, __m512i v1, __m512i v2, 
                           __m512i v3, __m512i v4, __m512i v5,
                           __m512i v6, __m512i v7, __m512i v8,
                           __m512i v9) {
    /* Create a mask from the first vector */
    __mmask16 mask = _mm512_cmpeq_epi32_mask(v0, v1);
    
    /* Use a volatile array to prevent optimization */
    volatile int32_t dest[16] __attribute__((aligned(64)));
    
    /* _mm512_mask_compressstoreu_epi32 has implicit operands */
    _mm512_mask_compressstoreu_epi32((void*)dest, mask, v2);
    
    /* Combine results to ensure they're used */
    __m512i result = _mm512_add_epi32(v3, v4);
    result = _mm512_add_epi32(result, v5);
    result = _mm512_add_epi32(result, v6);
    result = _mm512_add_epi32(result, v7);
    result = _mm512_add_epi32(result, v8);
    result = _mm512_add_epi32(result, v9);
    
    /* Extract and return a scalar */
    return _mm512_reduce_add_epi32(result) + dest[0];
}

/* Function for 11 operands using multiple AVX-512 operations */
static int func_11_operands(__m512i v0, __m512i v1, __m512i v2,
                           __m512i v3, __m512i v4, __m512i v5,
                           __m512i v6, __m512i v7, __m512i v8,
                           __m512i v9, __m512i v10) {
    /* Complex sequence with many operands */
    __m512i temp1 = _mm512_add_epi32(v0, v1);
    __m512i temp2 = _mm512_add_epi32(v2, v3);
    __m512i temp3 = _mm512_add_epi32(v4, v5);
    __m512i temp4 = _mm512_add_epi32(v6, v7);
    __m512i temp5 = _mm512_add_epi32(v8, v9);
    
    /* Blend operations with masks - potentially many operands */
    __m512i result = _mm512_mask_blend_epi32(0xAAAA, temp1, temp2);
    result = _mm512_mask_add_epi32(result, 0x5555, temp3, temp4);
    result = _mm512_mask_mul_epi32(result, 0xFFFF, result, temp5);
    result = _mm512_mask_add_epi32(result, 0x0F0F, result, v10);
    
    return _mm512_reduce_add_epi32(result);
}

#elif defined(__ARM_FEATURE_SVE)
/* ARM SVE implementation */
#include <arm_sve.h>

/* Function for 10 operands using SVE scatter/gather */
static int func_10_operands(svint32_t v0, svint32_t v1, svint32_t v2,
                           svint32_t v3, svint32_t v4, svint32_t v5,
                           svint32_t v6, svint32_t v7, svint32_t v8,
                           svint32_t v9) {
    /* Create predicate */
    svbool_t pg = svcmpgt(svptrue_b32(), v0, v1);
    
    /* Create base offsets */
    svint32_t bases = svadd_x(pg, v2, v3);
    
    /* Complex scatter operation with many implicit operands */
    volatile int32_t buffer[256] __attribute__((aligned(64)));
    
    /* Multiple operations to ensure expansion */
    svint32_t data = svadd_x(pg, v4, v5);
    data = svadd_x(pg, data, v6);
    data = svadd_x(pg, data, v7);
    data = svadd_x(pg, data, v8);
    data = svadd_x(pg, data, v9);
    
    /* Scatter store - may expand to many operands */
    svst1_scatter_s32index(pg, buffer, bases, data);
    
    /* Reduce to scalar */
    return svaddv(pg, data) + buffer[0];
}

/* Function for 11 operands */
static int func_11_operands(svint32_t v0, svint32_t v1, svint32_t v2,
                           svint32_t v3, svint32_t v4, svint32_t v5,
                           svint32_t v6, svint32_t v7, svint32_t v8,
                           svint32_t v9, svint32_t v10) {
    svbool_t pg = svptrue_b32();
    
    /* Chain of operations using all operands */
    svint32_t result = svadd_x(pg, v0, v1);
    result = svmla_x(pg, result, v2, v3);
    result = svmla_x(pg, result, v4, v5);
    result = svmla_x(pg, result, v6, v7);
    result = svmla_x(pg, result, v8, v9);
    result = svadd_x(pg, result, v10);
    
    return svaddv(pg, result);
}

#else
/* Default to generic inline assembly if no specific intrinsics available */
#define GENERIC_FALLBACK 1
#include __FILE__
#endif

int main(int argc, char *argv[]) {
    /* Initialize variables with non-constant values */
    int vals[20];
    
    /* Use argv for some values, PRNG for others to ensure variability */
    for (int i = 0; i < 20; i++) {
        if (i < argc && i < 10) {
            vals[i] = (argv[i][0] ? argv[i][0] : 1) + i;
        } else {
            vals[i] = (int)(prng_next() % 1000) + 1;
        }
    }
    
    /* Call 10-operand function */
    int result10 = func_10_operands(vals[0], vals[1], vals[2], vals[3],
                                   vals[4], vals[5], vals[6], vals[7],
                                   vals[8], vals[9]);
    
    /* Call 11-operand function */
    int result11 = func_11_operands(vals[0], vals[1], vals[2], vals[3],
                                   vals[4], vals[5], vals[6], vals[7],
                                   vals[8], vals[9], vals[10]);
    
    /* Combine results to prevent dead code elimination */
    int final_result = result10 + result11;
    
    /* Print result to ensure side effects */
    printf("Result: %d\n", final_result);
    
    return (final_result > 0) ? 0 : 1;
}
