/* Test program to cover 10/11-operand expansion in optabs.cc */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Simple PRNG to generate non-constant values */
static uint32_t prng_state = 123456789;
static uint32_t prng() {
    prng_state = prng_state * 1103515245 + 12345;
    return prng_state;
}

/* Helper to initialize arrays with non-constant values */
static void init_values(uint64_t *arr, int count, int argc, char **argv) {
    for (int i = 0; i < count; i++) {
        if (argc > i + 1) {
            arr[i] = strtoull(argv[i + 1], NULL, 0);
        } else {
            arr[i] = prng() | ((uint64_t)prng() << 32);
        }
    }
}

/* Function targeting 10-operand expansion */
#ifdef __AVX512F__
#include <immintrin.h>
uint64_t func_10_operands(__m512i a, __m512i b, __m512i c, __m512i d,
                          __m512i e, __m512i f, __m512i g, __m512i h,
                          __mmask16 mask, void *ptr) {
    /* AVX-512 masked store with many operands */
    __m512i result = _mm512_mask_add_epi32(a, mask, b, c);
    result = _mm512_mask_add_epi32(result, mask, d, e);
    result = _mm512_mask_add_epi32(result, mask, f, g);
    result = _mm512_mask_add_epi32(result, mask, result, h);
    
    /* Store result to prevent optimization */
    _mm512_mask_storeu_epi32(ptr, mask, result);
    
    /* Return a checksum */
    uint64_t sum = 0;
    int32_t *p = (int32_t*)&result;
    for (int i = 0; i < 16; i++) {
        sum += p[i];
    }
    return sum;
}
#elif defined(__ARM_FEATURE_SVE)
#include <arm_sve.h>
uint64_t func_10_operands(svint32_t a, svint32_t b, svint32_t c, svint32_t d,
                          svint32_t e, svint32_t f, svint32_t g, svint32_t h,
                          svbool_t mask, void *ptr) {
    /* SVE scatter store with many operands */
    svint32_t result = svadd_m(mask, a, b);
    result = svadd_m(mask, result, c);
    result = svadd_m(mask, result, d);
    result = svadd_m(mask, result, e);
    result = svadd_m(mask, result, f);
    result = svadd_m(mask, result, g);
    result = svadd_m(mask, result, h);
    
    /* Use the result */
    svst1(mask, (int32_t*)ptr, result);
    
    /* Return a checksum */
    uint64_t sum = 0;
    int32_t temp[16];
    svbool_t all_mask = svptrue_b32();
    svst1(all_mask, temp, result);
    for (int i = 0; i < 16; i++) {
        sum += temp[i];
    }
    return sum;
}
#else
/* Generic fallback using inline assembly with 10 operands */
uint64_t func_10_operands(uint64_t a, uint64_t b, uint64_t c, uint64_t d,
                          uint64_t e, uint64_t f, uint64_t g, uint64_t h,
                          uint64_t i, uint64_t j) {
    uint64_t result1, result2, result3;
    
    /* Extended inline assembly with 10 explicit operands */
    __asm__ volatile (
        /* Dummy operations that use all 10 input operands */
        "mov %[r1], %[a] \n\t"
        "add %[r1], %[b] \n\t"
        "add %[r1], %[c] \n\t"
        "mov %[r2], %[d] \n\t"
        "add %[r2], %[e] \n\t"
        "add %[r2], %[f] \n\t"
        "mov %[r3], %[g] \n\t"
        "add %[r3], %[h] \n\t"
        "add %[r3], %[i] \n\t"
        "add %[r3], %[j] \n\t"
        "add %[r1], %[r2] \n\t"
        "add %[r1], %[r3] \n\t"
        : [r1] "=&r" (result1), [r2] "=&r" (result2), [r3] "=&r" (result3)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j)
        : "cc"
    );
    
    return result1 + result2 + result3;
}
#endif

/* Function targeting 11-operand expansion */
#ifdef __AVX512F__
uint64_t func_11_operands(__m512i a, __m512i b, __m512i c, __m512i d,
                          __m512i e, __m512i f, __m512i g, __m512i h,
                          __m512i i, __mmask16 mask, void *ptr) {
    /* Complex AVX-512 operation with 11 operands */
    __m512i temp1 = _mm512_maskz_add_epi64(mask, a, b);
    __m512i temp2 = _mm512_maskz_add_epi64(mask, c, d);
    __m512i temp3 = _mm512_maskz_add_epi64(mask, e, f);
    __m512i temp4 = _mm512_maskz_add_epi64(mask, g, h);
    
    __m512i result = _mm512_mask_add_epi64(temp1, mask, temp2, temp3);
    result = _mm512_mask_add_epi64(result, mask, result, temp4);
    result = _mm512_mask_add_epi64(result, mask, result, i);
    
    /* Store and compute checksum */
    _mm512_mask_storeu_epi64(ptr, mask, result);
    
    uint64_t sum = 0;
    uint64_t *p = (uint64_t*)&result;
    for (int j = 0; j < 8; j++) {
        sum += p[j];
    }
    return sum;
}
#elif defined(__ARM_FEATURE_SVE)
uint64_t func_11_operands(svint64_t a, svint64_t b, svint64_t c, svint64_t d,
                          svint64_t e, svint64_t f, svint64_t g, svint64_t h,
                          svint64_t i, svbool_t mask, void *ptr) {
    /* SVE operation with 11 operands */
    svint64_t temp1 = svadd_m(mask, a, b);
    svint64_t temp2 = svadd_m(mask, c, d);
    svint64_t temp3 = svadd_m(mask, e, f);
    svint64_t temp4 = svadd_m(mask, g, h);
    
    svint64_t result = svadd_m(mask, temp1, temp2);
    result = svadd_m(mask, result, temp3);
    result = svadd_m(mask, result, temp4);
    result = svadd_m(mask, result, i);
    
    /* Store and compute checksum */
    svst1(mask, (int64_t*)ptr, result);
    
    uint64_t sum = 0;
    int64_t temp[8];
    svbool_t all_mask = svptrue_b64();
    svst1(all_mask, temp, result);
    for (int j = 0; j < 8; j++) {
        sum += temp[j];
    }
    return sum;
}
#else
/* Generic fallback using inline assembly with 11 operands */
uint64_t func_11_operands(uint64_t a, uint64_t b, uint64_t c, uint64_t d,
                          uint64_t e, uint64_t f, uint64_t g, uint64_t h,
                          uint64_t i, uint64_t j, uint64_t k) {
    uint64_t result1, result2, result3, result4;
    
    /* Extended inline assembly with 11 explicit operands */
    __asm__ volatile (
        /* Dummy operations that use all 11 input operands */
        "mov %[r1], %[a] \n\t"
        "add %[r1], %[b] \n\t"
        "mov %[r2], %[c] \n\t"
        "add %[r2], %[d] \n\t"
        "mov %[r3], %[e] \n\t"
        "add %[r3], %[f] \n\t"
        "mov %[r4], %[g] \n\t"
        "add %[r4], %[h] \n\t"
        "add %[r1], %[i] \n\t"
        "add %[r2], %[j] \n\t"
        "add %[r3], %[k] \n\t"
        "add %[r1], %[r2] \n\t"
        "add %[r1], %[r3] \n\t"
        "add %[r1], %[r4] \n\t"
        : [r1] "=&r" (result1), [r2] "=&r" (result2),
          [r3] "=&r" (result3), [r4] "=&r" (result4)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j), [k] "r" (k)
        : "cc"
    );
    
    return result1 + result2 + result3 + result4;
}
#endif

int main(int argc, char **argv) {
    uint64_t values[20];
    uint64_t result = 0;
    
    /* Initialize with non-constant values */
    init_values(values, 20, argc, argv);
    
    /* Call 10-operand function */
#ifdef __AVX512F__
    __m512i v1 = _mm512_set_epi64(values[0], values[1], values[2], values[3],
                                  values[4], values[5], values[6], values[7]);
    __m512i v2 = _mm512_set_epi64(values[1], values[2], values[3], values[4],
                                  values[5], values[6], values[7], values[8]);
    __m512i v3 = _mm512_set_epi64(values[2], values[3], values[4], values[5],
                                  values[6], values[7], values[8], values[9]);
    __m512i v4 = _mm512_set_epi64(values[3], values[4], values[5], values[6],
                                  values[7], values[8], values[9], values[10]);
    __m512i v5 = _mm512_set_epi64(values[4], values[5], values[6], values[7],
                                  values[8], values[9], values[10], values[11]);
    __m512i v6 = _mm512_set_epi64(values[5], values[6], values[7], values[8],
                                  values[9], values[10], values[11], values[12]);
    __m512i v7 = _mm512_set_epi64(values[6], values[7], values[8], values[9],
                                  values[10], values[11], values[12], values[13]);
    __m512i v8 = _mm512_set_epi64(values[7], values[8], values[9], values[10],
                                  values[11], values[12], values[13], values[14]);
    __mmask16 mask = 0xAAAA;  /* 1010101010101010 pattern */
    char buffer[256];
    
    result += func_10_operands(v1, v2, v3, v4, v5, v6, v7, v8, mask, buffer);
    
    /* Call 11-operand function */
    __m512i v9 = _mm512_set_epi64(values[8], values[9], values[10], values[11],
                                  values[12], values[13], values[14], values[15]);
    result += func_11_operands(v1, v2, v3, v4, v5, v6, v7, v8, v9, mask, buffer);
#elif defined(__ARM_FEATURE_SVE)
    /* ARM SVE implementation would go here */
    /* For brevity, using generic path for non-AVX512 */
    result += func_10_operands(values[0], values[1], values[2], values[3],
                               values[4], values[5], values[6], values[7],
                               values[8], values[9]);
    result += func_11_operands(values[0], values[1], values[2], values[3],
                               values[4], values[5], values[6], values[7],
                               values[8], values[9], values[10]);
#else
    /* Generic path using scalar values */
    result += func_10_operands(values[0], values[1], values[2], values[3],
                               values[4], values[5], values[6], values[7],
                               values[8], values[9]);
    result += func_11_operands(values[0], values[1], values[2], values[3],
                               values[4], values[5], values[6], values[7],
                               values[8], values[9], values[10]);
#endif
    
    printf("Result: %lu\n", (unsigned long)result);
    return (int)(result & 0x7FFFFFFF);
}
