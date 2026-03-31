/* Test program to cover 10/11 operand expansion in optabs.cc */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Simple PRNG to generate non-constant values */
static uint64_t prng_state = 0x123456789ABCDEFULL;
static inline uint64_t prng_next(void) {
    prng_state = prng_state * 6364136223846793005ULL + 1442695040888963407ULL;
    return prng_state;
}

/* Initialize values to prevent constant propagation */
static void init_values(uint64_t *vals, int count) {
    for (int i = 0; i < count; i++) {
        vals[i] = prng_next();
    }
}

/* Function for 10 operands */
#ifdef __AVX512F__
#include <immintrin.h>
__attribute__((noinline))
static uint64_t func_10_operands(__m512i a, __m512i b, __m512i c, __m512i d,
                                 __m512i e, __m512i f, __m512i g, __m512i h,
                                 __mmask16 mask, void *addr) {
    /* AVX-512 masked store with many operands */
    __m512i result = _mm512_add_epi64(a, b);
    result = _mm512_add_epi64(result, c);
    result = _mm512_add_epi64(result, d);
    result = _mm512_add_epi64(result, e);
    result = _mm512_add_epi64(result, f);
    result = _mm512_add_epi64(result, g);
    result = _mm512_add_epi64(result, h);
    
    /* Complex operation that might expand to many operands */
    _mm512_mask_storeu_epi64(addr, mask, result);
    
    /* Use result to prevent elimination */
    uint64_t sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += result[i];
    }
    return sum;
}
#elif defined(__ARM_FEATURE_SVE)
#include <arm_sve.h>
__attribute__((noinline))
static uint64_t func_10_operands(svint64_t a, svint64_t b, svint64_t c, 
                                 svint64_t d, svint64_t e, svint64_t f,
                                 svint64_t g, svint64_t h, svbool_t pg,
                                 uint64_t *addr) {
    /* SVE scatter store with many operands */
    svint64_t result = svadd_x(pg, a, b);
    result = svadd_x(pg, result, c);
    result = svadd_x(pg, result, d);
    result = svadd_x(pg, result, e);
    result = svadd_x(pg, result, f);
    result = svadd_x(pg, result, g);
    result = svadd_x(pg, result, h);
    
    /* Complex SVE operation */
    svst1_scatter_u64base_offset_u64(pg, addr, svindex_u64(0, 1), result);
    
    /* Use result */
    return svaddv(pg, result);
}
#else
/* Generic fallback using inline assembly with 10 operands */
__attribute__((noinline))
static uint64_t func_10_operands(uint64_t a0, uint64_t a1, uint64_t a2,
                                 uint64_t a3, uint64_t a4, uint64_t a5,
                                 uint64_t a6, uint64_t a7, uint64_t a8,
                                 uint64_t a9) {
    uint64_t result;
    
    /* Extended inline assembly with 10 input operands */
    __asm__ volatile (
        "/* 10-operand dummy operation */\n\t"
        "add %[r], %[a0], %[a1]\n\t"
        "add %[r], %[r], %[a2]\n\t"
        "add %[r], %[r], %[a3]\n\t"
        "add %[r], %[r], %[a4]\n\t"
        "add %[r], %[r], %[a5]\n\t"
        "add %[r], %[r], %[a6]\n\t"
        "add %[r], %[r], %[a7]\n\t"
        "add %[r], %[r], %[a8]\n\t"
        "add %[r], %[r], %[a9]"
        : [r] "=r" (result)
        : [a0] "r" (a0), [a1] "r" (a1), [a2] "r" (a2),
          [a3] "r" (a3), [a4] "r" (a4), [a5] "r" (a5),
          [a6] "r" (a6), [a7] "r" (a7), [a8] "r" (a8),
          [a9] "r" (a9)
        : "cc"
    );
    
    return result;
}
#endif

/* Function for 11 operands */
#ifdef __AVX512F__
__attribute__((noinline))
static uint64_t func_11_operands(__m512i a, __m512i b, __m512i c, __m512i d,
                                 __m512i e, __m512i f, __m512i g, __m512i h,
                                 __m512i i, __mmask16 mask, void *addr) {
    /* Complex AVX-512 operation with 11 conceptual operands */
    __m512i temp1 = _mm512_add_epi64(a, b);
    __m512i temp2 = _mm512_add_epi64(c, d);
    __m512i temp3 = _mm512_add_epi64(e, f);
    __m512i temp4 = _mm512_add_epi64(g, h);
    
    __m512i result = _mm512_add_epi64(temp1, temp2);
    result = _mm512_add_epi64(result, temp3);
    result = _mm512_add_epi64(result, temp4);
    result = _mm512_add_epi64(result, i);
    
    /* Masked compress store - complex operation with many operands */
    _mm512_mask_compressstoreu_epi64(addr, mask, result);
    
    /* Use result */
    uint64_t sum = 0;
    for (int j = 0; j < 8; j++) {
        sum += result[j];
    }
    return sum;
}
#elif defined(__ARM_FEATURE_SVE)
__attribute__((noinline))
static uint64_t func_11_operands(svint64_t a, svint64_t b, svint64_t c,
                                 svint64_t d, svint64_t e, svint64_t f,
                                 svint64_t g, svint64_t h, svint64_t i,
                                 svbool_t pg, uint64_t *addr) {
    /* Complex SVE operation with 11 operands */
    svint64_t temp1 = svadd_x(pg, a, b);
    svint64_t temp2 = svadd_x(pg, c, d);
    svint64_t temp3 = svadd_x(pg, e, f);
    svint64_t temp4 = svadd_x(pg, g, h);
    
    svint64_t result = svadd_x(pg, temp1, temp2);
    result = svadd_x(pg, result, temp3);
    result = svadd_x(pg, result, temp4);
    result = svadd_x(pg, result, i);
    
    /* SVE scatter with offset - complex multi-operand operation */
    svst1_scatter_u64base_offset_u64(pg, addr, 
                                     svadd_x(pg, svindex_u64(0, 1), 
                                             svdup_u64(0x100)), result);
    
    return svaddv(pg, result);
}
#else
/* Generic fallback using inline assembly with 11 operands */
__attribute__((noinline))
static uint64_t func_11_operands(uint64_t a0, uint64_t a1, uint64_t a2,
                                 uint64_t a3, uint64_t a4, uint64_t a5,
                                 uint64_t a6, uint64_t a7, uint64_t a8,
                                 uint64_t a9, uint64_t a10) {
    uint64_t result;
    
    /* Extended inline assembly with 11 input operands */
    __asm__ volatile (
        "/* 11-operand dummy operation */\n\t"
        "mov %[r], %[a0]\n\t"
        "add %[r], %[r], %[a1]\n\t"
        "add %[r], %[r], %[a2]\n\t"
        "add %[r], %[r], %[a3]\n\t"
        "add %[r], %[r], %[a4]\n\t"
        "add %[r], %[r], %[a5]\n\t"
        "add %[r], %[r], %[a6]\n\t"
        "add %[r], %[r], %[a7]\n\t"
        "add %[r], %[r], %[a8]\n\t"
        "add %[r], %[r], %[a9]\n\t"
        "add %[r], %[r], %[a10]"
        : [r] "=r" (result)
        : [a0] "r" (a0), [a1] "r" (a1), [a2] "r" (a2),
          [a3] "r" (a3), [a4] "r" (a4), [a5] "r" (a5),
          [a6] "r" (a6), [a7] "r" (a7), [a8] "r" (a8),
          [a9] "r" (a9), [a10] "r" (a10)
        : "cc"
    );
    
    return result;
}
#endif

int main(int argc, char *argv[]) {
    /* Initialize with non-constant values */
    uint64_t values[20];
    init_values(values, 20);
    
    /* Use argv to add more variability */
    for (int i = 0; i < argc && i < 20; i++) {
        values[i] ^= (uint64_t)argv[i];
    }
    
    uint64_t result = 0;
    
    /* Call 10-operand function */
    result += func_10_operands(values[0], values[1], values[2], values[3],
                               values[4], values[5], values[6], values[7],
                               values[8], values[9]);
    
    /* Call 11-operand function */
    result += func_11_operands(values[0], values[1], values[2], values[3],
                               values[4], values[5], values[6], values[7],
                               values[8], values[9], values[10]);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %lu\n", (unsigned long)result);
    
    return 0;
}
