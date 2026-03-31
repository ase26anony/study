/* Test program for GCC optabs.cc 10/11 operand expansion coverage */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Simple PRNG to generate non-constant values */
static uint64_t prng_state = 123456789;

static uint64_t prng_next(void) {
    prng_state = prng_state * 1103515245 + 12345;
    return prng_state;
}

/* Initialize values from argv to prevent constant propagation */
static void init_values(uint64_t *vals, int argc, char **argv) {
    for (int i = 0; i < 11; i++) {
        if (argc > i + 1) {
            vals[i] = strtoull(argv[i + 1], NULL, 0);
        } else {
            vals[i] = prng_next();
        }
    }
}

/* Function for 10 operands */
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
    
    /* Store with mask - expands to many operands */
    _mm512_mask_storeu_epi32(ptr, mask, result);
    
    /* Use result to prevent optimization */
    uint64_t sum = 0;
    int32_t *arr = (int32_t*)&result;
    for (int i = 0; i < 16; i++) {
        sum += arr[i];
    }
    return sum;
}
#elif defined(__ARM_FEATURE_SVE)
#include <arm_sve.h>
uint64_t func_10_operands(svint32_t a, svint32_t b, svint32_t c, svint32_t d,
                          svint32_t e, svint32_t f, svint32_t g, svint32_t h,
                          svbool_t mask, void *ptr) {
    /* SVE operations with predicate */
    svint32_t result = svadd_m(mask, a, b);
    result = svadd_m(mask, result, c);
    result = svadd_m(mask, result, d);
    result = svadd_m(mask, result, e);
    result = svadd_m(mask, result, f);
    result = svadd_m(mask, result, g);
    result = svadd_m(mask, result, h);
    
    /* Scatter store with many operands */
    svuint64_t indices = svindex_u64(0, 1);
    svst1_scatter_u64base_s32(mask, (int32_t*)ptr, indices, result);
    
    /* Use result */
    int32_t temp[svcntw()];
    svst1(mask, temp, result);
    
    uint64_t sum = 0;
    for (int i = 0; i < svcntw(); i++) {
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
    
    /* Extended inline assembly with 10 input operands and 3 outputs */
    asm volatile (
        /* Dummy operations that use all operands */
        "mov %[r1], %[a] \n\t"
        "add %[r1], %[r1], %[b] \n\t"
        "add %[r1], %[r1], %[c] \n\t"
        "mov %[r2], %[d] \n\t"
        "add %[r2], %[r2], %[e] \n\t"
        "add %[r2], %[r2], %[f] \n\t"
        "mov %[r3], %[g] \n\t"
        "add %[r3], %[r3], %[h] \n\t"
        "add %[r3], %[r3], %[i] \n\t"
        "add %[r3], %[r3], %[j] \n\t"
        : [r1] "=&r" (result1), [r2] "=&r" (result2), [r3] "=&r" (result3)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j)
        : "cc"
    );
    
    /* Combine results to prevent optimization */
    return result1 + result2 + result3;
}
#endif

/* Function for 11 operands */
#ifdef __AVX512F__
uint64_t func_11_operands(__m512i a, __m512i b, __m512i c, __m512i d,
                          __m512i e, __m512i f, __m512i g, __m512i h,
                          __m512i i, __mmask16 mask, void *ptr) {
    /* Complex AVX-512 operation chain */
    __m512i temp1 = _mm512_maskz_add_epi64(mask, a, b);
    __m512i temp2 = _mm512_maskz_add_epi64(mask, c, d);
    __m512i temp3 = _mm512_maskz_add_epi64(mask, e, f);
    __m512i temp4 = _mm512_maskz_add_epi64(mask, g, h);
    
    /* Blend with many operands */
    __m512i result = _mm512_mask_blend_epi64(mask, temp1, temp2);
    result = _mm512_mask_blend_epi64(mask, result, temp3);
    result = _mm512_mask_blend_epi64(mask, result, temp4);
    result = _mm512_mask_blend_epi64(mask, result, i);
    
    /* Compress store with mask - may expand to many operands */
    _mm512_mask_compressstoreu_epi64(ptr, mask, result);
    
    /* Use result */
    uint64_t sum = 0;
    uint64_t *arr = (uint64_t*)&result;
    for (int j = 0; j < 8; j++) {
        sum += arr[j];
    }
    return sum;
}
#elif defined(__ARM_FEATURE_SVE)
uint64_t func_11_operands(svint64_t a, svint64_t b, svint64_t c, svint64_t d,
                          svint64_t e, svint64_t f, svint64_t g, svint64_t h,
                          svint64_t i, svbool_t mask, void *ptr) {
    /* SVE scatter store with offset - many operands */
    svint64_t result = svadd_m(mask, a, b);
    result = svadd_m(mask, result, c);
    result = svadd_m(mask, result, d);
    result = svadd_m(mask, result, e);
    result = svadd_m(mask, result, f);
    result = svadd_m(mask, result, g);
    result = svadd_m(mask, result, h);
    result = svadd_m(mask, result, i);
    
    /* Complex scatter with base and offset */
    svuint64_t bases = svdup_u64((uint64_t)ptr);
    svuint64_t offsets = svindex_u64(0, 8);
    svst1_scatter_u64offset_s64(mask, bases, offsets, result);
    
    /* Use result */
    int64_t temp[svcntd()];
    svst1(mask, temp, result);
    
    uint64_t sum = 0;
    for (int j = 0; j < svcntd(); j++) {
        sum += temp[j];
    }
    return sum;
}
#else
/* Generic fallback with 11 operands */
uint64_t func_11_operands(uint64_t a, uint64_t b, uint64_t c, uint64_t d,
                          uint64_t e, uint64_t f, uint64_t g, uint64_t h,
                          uint64_t i, uint64_t j, uint64_t k) {
    uint64_t result1, result2, result3, result4;
    
    /* Inline assembly with 11 input operands */
    asm volatile (
        /* Use all 11 operands in various combinations */
        "mov %[r1], %[a] \n\t"
        "add %[r1], %[r1], %[b] \n\t"
        "add %[r1], %[r1], %[c] \n\t"
        "mov %[r2], %[d] \n\t"
        "add %[r2], %[r2], %[e] \n\t"
        "add %[r2], %[r2], %[f] \n\t"
        "mov %[r3], %[g] \n\t"
        "add %[r3], %[r3], %[h] \n\t"
        "add %[r3], %[r3], %[i] \n\t"
        "mov %[r4], %[j] \n\t"
        "add %[r4], %[r4], %[k] \n\t"
        : [r1] "=&r" (result1), [r2] "=&r" (result2),
          [r3] "=&r" (result3), [r4] "=&r" (result4)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j), [k] "r" (k)
        : "cc"
    );
    
    /* Combine all results */
    return result1 + result2 + result3 + result4;
}
#endif

int main(int argc, char **argv) {
    uint64_t values[11];
    uint64_t result = 0;
    
    /* Initialize with non-constant values */
    init_values(values, argc, argv);
    
    /* Buffer for store operations */
    char buffer[1024] __attribute__((aligned(64)));
    
#ifdef __AVX512F__ || defined(__ARM_FEATURE_SVE)
    /* For vector intrinsics, we need to prepare vector values */
    #ifdef __AVX512F__
    __m512i vecs[9];
    __mmask16 mask = 0xAAAA;  /* Alternating mask */
    for (int i = 0; i < 9; i++) {
        uint64_t data[8];
        for (int j = 0; j < 8; j++) {
            data[j] = values[i] + j;
        }
        vecs[i] = _mm512_loadu_si512(data);
    }
    
    /* Call 10-operand function */
    result += func_10_operands(vecs[0], vecs[1], vecs[2], vecs[3],
                               vecs[4], vecs[5], vecs[6], vecs[7],
                               mask, buffer);
    
    /* Call 11-operand function */
    result += func_11_operands(vecs[0], vecs[1], vecs[2], vecs[3],
                               vecs[4], vecs[5], vecs[6], vecs[7],
                               vecs[8], mask, buffer + 64);
    #elif defined(__ARM_FEATURE_SVE)
    /* Similar setup for ARM SVE */
    svbool_t mask = svwhilelt_b32(0, svcntw());
    svint32_t vecs32[8];
    svint64_t vecs64[9];
    
    for (int i = 0; i < 8; i++) {
        int32_t data[svcntw()];
        for (int j = 0; j < svcntw(); j++) {
            data[j] = values[i] + j;
        }
        vecs32[i] = svld1(mask, data);
    }
    
    for (int i = 0; i < 9; i++) {
        int64_t data[svcntd()];
        for (int j = 0; j < svcntd(); j++) {
            data[j] = values[i] + j;
        }
        vecs64[i] = svld1(svwhilelt_b64(0, svcntd()), data);
    }
    
    result += func_10_operands(vecs32[0], vecs32[1], vecs32[2], vecs32[3],
                               vecs32[4], vecs32[5], vecs32[6], vecs32[7],
                               mask, buffer);
    
    result += func_11_operands(vecs64[0], vecs64[1], vecs64[2], vecs64[3],
                               vecs64[4], vecs64[5], vecs64[6], vecs64[7],
                               vecs64[8], svwhilelt_b64(0, svcntd()), buffer + 256);
    #endif
#else
    /* Generic scalar path */
    result += func_10_operands(values[0], values[1], values[2], values[3],
                               values[4], values[5], values[6], values[7],
                               values[8], values[9]);
    
    result += func_11_operands(values[0], values[1], values[2], values[3],
                               values[4], values[5], values[6], values[7],
                               values[8], values[9], values[10]);
#endif
    
    /* Print result to prevent optimization */
    printf("Result: %lu\n", (unsigned long)result);
    
    /* Also use buffer to prevent dead store elimination */
    volatile char *volatile_buffer = buffer;
    return vol_buffer[0] + vol_buffer[64] + vol_buffer[128];
}
