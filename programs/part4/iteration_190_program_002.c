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

/* Helper to initialize values from argv */
static uint64_t init_value(int argc, char **argv, int idx) {
    if (idx < argc) {
        return (uint64_t)argv[idx][0] * (idx + 1);
    }
    return prng_next();
}

/* Function targeting 10 operands */
#ifdef __AVX512F__
#include <immintrin.h>
uint64_t func_10_operands(__m512i a, __m512i b, __m512i c, __m512i d,
                          __m512i e, __m512i f, __m512i g, __m512i h,
                          __mmask16 mask, void *addr) {
    /* AVX-512 masked store with many operands */
    __m512i result = _mm512_mask_add_epi32(a, mask, b, c);
    result = _mm512_add_epi32(result, d);
    result = _mm512_add_epi32(result, e);
    result = _mm512_add_epi32(result, f);
    result = _mm512_add_epi32(result, g);
    result = _mm512_add_epi32(result, h);
    
    /* Complex operation that might expand to many operands */
    _mm512_mask_storeu_epi32(addr, mask, result);
    
    /* Use result to prevent optimization */
    uint64_t sum = 0;
    int32_t *ptr = (int32_t*)&result;
    for (int i = 0; i < 16; i++) {
        sum += ptr[i];
    }
    return sum;
}
#elif defined(__ARM_FEATURE_SVE)
#include <arm_sve.h>
uint64_t func_10_operands(svint32_t a, svint32_t b, svint32_t c, svint32_t d,
                          svint32_t e, svint32_t f, svint32_t g, svint32_t h,
                          svbool_t mask, void *addr) {
    /* SVE scatter store with multiple operands */
    svint32_t result = svadd_s32_z(mask, a, b);
    result = svadd_s32_z(mask, result, c);
    result = svadd_s32_z(mask, result, d);
    result = svadd_s32_z(mask, result, e);
    result = svadd_s32_z(mask, result, f);
    result = svadd_s32_z(mask, result, g);
    result = svadd_s32_z(mask, result, h);
    
    /* Complex SVE operation */
    svst1_scatter_s32index_s32(mask, (int32_t*)addr, 
                              svindex_s32(0, 1), result);
    
    /* Use result */
    uint64_t sum = 0;
    int32_t temp[16];
    svbool_t all_mask = svptrue_b32();
    svst1_s32(all_mask, temp, result);
    for (int i = 0; i < 16; i++) {
        sum += temp[i];
    }
    return sum;
}
#else
/* Generic fallback using inline assembly with 10 operands */
uint64_t func_10_operands(uint64_t a0, uint64_t a1, uint64_t a2, uint64_t a3,
                          uint64_t a4, uint64_t a5, uint64_t a6, uint64_t a7,
                          uint64_t a8, uint64_t a9) {
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

/* Function targeting 11 operands */
#ifdef __AVX512F__
uint64_t func_11_operands(__m512i a, __m512i b, __m512i c, __m512i d,
                          __m512i e, __m512i f, __m512i g, __m512i h,
                          __m512i i, __mmask16 mask, void *addr) {
    /* Complex AVX-512 operation chain */
    __m512i temp1 = _mm512_maskz_add_epi64(mask, a, b);
    __m512i temp2 = _mm512_maskz_add_epi64(mask, c, d);
    __m512i temp3 = _mm512_maskz_add_epi64(mask, e, f);
    __m512i temp4 = _mm512_maskz_add_epi64(mask, g, h);
    
    __m512i result = _mm512_add_epi64(temp1, temp2);
    result = _mm512_add_epi64(result, temp3);
    result = _mm512_add_epi64(result, temp4);
    result = _mm512_add_epi64(result, i);
    
    /* Masked compress store - potentially expands to many operands */
    _mm512_mask_compressstoreu_epi64(addr, mask, result);
    
    /* Use result */
    uint64_t sum = 0;
    uint64_t *ptr = (uint64_t*)&result;
    for (int j = 0; j < 8; j++) {
        sum += ptr[j];
    }
    return sum;
}
#elif defined(__ARM_FEATURE_SVE)
uint64_t func_11_operands(svint64_t a, svint64_t b, svint64_t c, svint64_t d,
                          svint64_t e, svint64_t f, svint64_t g, svint64_t h,
                          svint64_t i, svbool_t mask, void *addr) {
    /* SVE operation with 11 conceptual operands */
    svint64_t temp1 = svadd_s64_z(mask, a, b);
    svint64_t temp2 = svadd_s64_z(mask, c, d);
    svint64_t temp3 = svadd_s64_z(mask, e, f);
    svint64_t temp4 = svadd_s64_z(mask, g, h);
    
    svint64_t result = svadd_s64_z(mask, temp1, temp2);
    result = svadd_s64_z(mask, result, temp3);
    result = svadd_s64_z(mask, result, temp4);
    result = svadd_s64_z(mask, result, i);
    
    /* Complex SVE scatter with offset */
    svint64_t offsets = svindex_s64(0, 8);
    svst1_scatter_s64offset_s64(mask, (int64_t*)addr, offsets, result);
    
    /* Use result */
    uint64_t sum = 0;
    int64_t temp[8];
    svbool_t all_mask = svptrue_b64();
    svst1_s64(all_mask, temp, result);
    for (int j = 0; j < 8; j++) {
        sum += temp[j];
    }
    return sum;
}
#else
/* Generic fallback with 11 operands */
uint64_t func_11_operands(uint64_t a0, uint64_t a1, uint64_t a2, uint64_t a3,
                          uint64_t a4, uint64_t a5, uint64_t a6, uint64_t a7,
                          uint64_t a8, uint64_t a9, uint64_t a10) {
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

int main(int argc, char **argv) {
    uint64_t values[20];
    uint64_t final_result = 0;
    
    /* Initialize values with non-constant data */
    for (int i = 0; i < 20; i++) {
        values[i] = init_value(argc, argv, i);
    }
    
    /* Call 10-operand function */
#ifdef __AVX512F__
    __m512i vec0 = _mm512_set_epi64(values[0], values[1], values[2], values[3],
                                   values[4], values[5], values[6], values[7]);
    __m512i vec1 = _mm512_set_epi64(values[1], values[2], values[3], values[4],
                                   values[5], values[6], values[7], values[8]);
    __m512i vec2 = _mm512_set_epi64(values[2], values[3], values[4], values[5],
                                   values[6], values[7], values[8], values[9]);
    __m512i vec3 = _mm512_set_epi64(values[3], values[4], values[5], values[6],
                                   values[7], values[8], values[9], values[10]);
    __m512i vec4 = _mm512_set_epi64(values[4], values[5], values[6], values[7],
                                   values[8], values[9], values[10], values[11]);
    __m512i vec5 = _mm512_set_epi64(values[5], values[6], values[7], values[8],
                                   values[9], values[10], values[11], values[12]);
    __m512i vec6 = _mm512_set_epi64(values[6], values[7], values[8], values[9],
                                   values[10], values[11], values[12], values[13]);
    __m512i vec7 = _mm512_set_epi64(values[7], values[8], values[9], values[10],
                                   values[11], values[12], values[13], values[14]);
    __m512i vec8 = _mm512_set_epi64(values[8], values[9], values[10], values[11],
                                   values[12], values[13], values[14], values[15]);
    
    __mmask16 mask = 0xAAAA; /* 1010101010101010 pattern */
    char buffer[256] __attribute__((aligned(64)));
    
    final_result += func_10_operands(vec0, vec1, vec2, vec3, vec4, 
                                    vec5, vec6, vec7, mask, buffer);
    final_result += func_11_operands(vec0, vec1, vec2, vec3, vec4,
                                    vec5, vec6, vec7, vec8, mask, buffer);
#elif defined(__ARM_FEATURE_SVE)
    /* For ARM SVE, we'd need dynamic allocation for vectors */
    /* Using generic path for demonstration */
    final_result += func_10_operands(values[0], values[1], values[2], values[3],
                                    values[4], values[5], values[6], values[7],
                                    values[8], values[9]);
    final_result += func_11_operands(values[0], values[1], values[2], values[3],
                                    values[4], values[5], values[6], values[7],
                                    values[8], values[9], values[10]);
#else
    /* Generic path using scalar values */
    final_result += func_10_operands(values[0], values[1], values[2], values[3],
                                    values[4], values[5], values[6], values[7],
                                    values[8], values[9]);
    final_result += func_11_operands(values[0], values[1], values[2], values[3],
                                    values[4], values[5], values[6], values[7],
                                    values[8], values[9], values[10]);
#endif
    
    /* Use result to prevent optimization */
    printf("Result: %lu\n", final_result);
    
    return (final_result > 0) ? 0 : 1;
}
