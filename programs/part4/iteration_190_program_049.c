/* Test program to cover 10/11 operand expansion in optabs.cc */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Simple PRNG to generate non-constant values */
static uint64_t seed = 123456789;

static uint64_t rand_val(void) {
    seed = seed * 1103515245 + 12345;
    return seed;
}

/* Function prototypes */
int func_10_operands(uint64_t a, uint64_t b, uint64_t c, uint64_t d,
                     uint64_t e, uint64_t f, uint64_t g, uint64_t h,
                     uint64_t i, uint64_t j);

int func_11_operands(uint64_t a, uint64_t b, uint64_t c, uint64_t d,
                     uint64_t e, uint64_t f, uint64_t g, uint64_t h,
                     uint64_t i, uint64_t j, uint64_t k);

/* Architecture-specific implementations */
#ifdef __AVX512F__
#include <immintrin.h>

/* AVX-512 implementation with many operands */
int func_10_operands(uint64_t a, uint64_t b, uint64_t c, uint64_t d,
                     uint64_t e, uint64_t f, uint64_t g, uint64_t h,
                     uint64_t i, uint64_t j) {
    /* Create vector from inputs */
    __m512i v1 = _mm512_set_epi64(a, b, c, d, e, f, g, h);
    __m512i v2 = _mm512_set_epi64(i, j, a, b, c, d, e, f);
    
    /* Complex operation with mask - may expand to many operands */
    __mmask8 mask = (__mmask8)((a ^ b ^ c ^ d) & 0xFF);
    __m512i result = _mm512_mask_add_epi64(v1, mask, v1, v2);
    
    /* Extract and combine results */
    uint64_t res[8];
    _mm512_storeu_si512(res, result);
    
    int sum = 0;
    for (int idx = 0; idx < 8; idx++) {
        sum += (int)(res[idx] & 0xFFFFFFFF);
    }
    return sum;
}

int func_11_operands(uint64_t a, uint64_t b, uint64_t c, uint64_t d,
                     uint64_t e, uint64_t f, uint64_t g, uint64_t h,
                     uint64_t i, uint64_t j, uint64_t k) {
    /* Even more complex AVX-512 operation */
    __m512i v1 = _mm512_set_epi64(a, b, c, d, e, f, g, h);
    __m512i v2 = _mm512_set_epi64(i, j, k, a, b, c, d, e);
    __m512i v3 = _mm512_set_epi64(f, g, h, i, j, k, a, b);
    
    /* Multiple masks for complex operation */
    __mmask8 mask1 = (__mmask8)((a ^ c ^ e ^ g) & 0xFF);
    __mmask8 mask2 = (__mmask8)((b ^ d ^ f ^ h) & 0xFF);
    
    /* Chained operations that may require many operands when expanded */
    __m512i temp = _mm512_mask_add_epi64(v1, mask1, v1, v2);
    __m512i result = _mm512_mask_sub_epi64(temp, mask2, temp, v3);
    
    /* Extract results */
    uint64_t res[8];
    _mm512_storeu_si512(res, result);
    
    int sum = 0;
    for (int idx = 0; idx < 8; idx++) {
        sum += (int)(res[idx] & 0xFFFF);
    }
    return sum;
}

#elif defined(__ARM_FEATURE_SVE)
#include <arm_sve.h>

/* ARM SVE implementation */
int func_10_operands(uint64_t a, uint64_t b, uint64_t c, uint64_t d,
                     uint64_t e, uint64_t f, uint64_t g, uint64_t h,
                     uint64_t i, uint64_t j) {
    /* SVE vectors with predicate - may require many operands */
    svuint64_t v1 = svdup_u64(a);
    svuint64_t v2 = svdup_u64(b);
    svuint64_t v3 = svdup_u64(c);
    svuint64_t v4 = svdup_u64(d);
    svuint64_t v5 = svdup_u64(e);
    svuint64_t v6 = svdup_u64(f);
    svuint64_t v7 = svdup_u64(g);
    svuint64_t v8 = svdup_u64(h);
    
    /* Create predicate */
    svbool_t pg = svwhilelt_b64(0, 8);
    
    /* Complex SVE operation */
    svuint64_t sum1 = svadd_u64_x(pg, v1, v2);
    svuint64_t sum2 = svadd_u64_x(pg, v3, v4);
    svuint64_t sum3 = svadd_u64_x(pg, v5, v6);
    svuint64_t sum4 = svadd_u64_x(pg, v7, v8);
    
    svuint64_t result = svadd_u64_x(pg, 
                      svadd_u64_x(pg, sum1, sum2),
                      svadd_u64_x(pg, sum3, sum4));
    
    /* Extract result */
    uint64_t res;
    svst1_u64(pg, &res, result);
    return (int)(res & 0xFFFFFFFF);
}

int func_11_operands(uint64_t a, uint64_t b, uint64_t c, uint64_t d,
                     uint64_t e, uint64_t f, uint64_t g, uint64_t h,
                     uint64_t i, uint64_t j, uint64_t k) {
    /* Similar SVE implementation with 11 inputs */
    svuint64_t v1 = svdup_u64(a);
    svuint64_t v2 = svdup_u64(b);
    svuint64_t v3 = svdup_u64(c);
    svuint64_t v4 = svdup_u64(d);
    svuint64_t v5 = svdup_u64(e);
    svuint64_t v6 = svdup_u64(f);
    svuint64_t v7 = svdup_u64(g);
    svuint64_t v8 = svdup_u64(h);
    svuint64_t v9 = svdup_u64(i);
    
    svbool_t pg = svwhilelt_b64(0, 8);
    
    /* Multiple operations chained together */
    svuint64_t sum1 = svadd_u64_x(pg, v1, v2);
    svuint64_t sum2 = svadd_u64_x(pg, v3, v4);
    svuint64_t sum3 = svadd_u64_x(pg, v5, v6);
    svuint64_t sum4 = svadd_u64_x(pg, v7, v8);
    svuint64_t sum5 = svadd_u64_x(pg, v9, svdup_u64(j));
    
    svuint64_t result = svadd_u64_x(pg,
                      svadd_u64_x(pg, sum1, sum2),
                      svadd_u64_x(pg, sum3, svadd_u64_x(pg, sum4, sum5)));
    
    uint64_t res;
    svst1_u64(pg, &res, result);
    return (int)(res & 0xFFFFFFFF);
}

#else
/* Generic fallback using inline assembly with many operands */

int func_10_operands(uint64_t a, uint64_t b, uint64_t c, uint64_t d,
                     uint64_t e, uint64_t f, uint64_t g, uint64_t h,
                     uint64_t i, uint64_t j) {
    uint64_t result1, result2, result3, result4;
    
    /* Extended inline assembly with 10 input operands and 4 outputs */
    asm volatile (
        /* Dummy operations that use all inputs */
        "add %[r1], %[a], %[b]\n\t"
        "add %[r2], %[c], %[d]\n\t"
        "add %[r3], %[e], %[f]\n\t"
        "add %[r4], %[g], %[h]\n\t"
        "add %[r1], %[r1], %[i]\n\t"
        "add %[r2], %[r2], %[j]\n\t"
        "add %[r1], %[r1], %[r2]\n\t"
        "add %[r3], %[r3], %[r4]\n\t"
        "add %[r1], %[r1], %[r3]"
        : [r1] "=r" (result1),
          [r2] "=r" (result2),
          [r3] "=r" (result3),
          [r4] "=r" (result4)
        : [a] "r" (a),
          [b] "r" (b),
          [c] "r" (c),
          [d] "r" (d),
          [e] "r" (e),
          [f] "r" (f),
          [g] "r" (g),
          [h] "r" (h),
          [i] "r" (i),
          [j] "r" (j)
        : "cc"
    );
    
    return (int)((result1 + result2 + result3 + result4) & 0xFFFFFFFF);
}

int func_11_operands(uint64_t a, uint64_t b, uint64_t c, uint64_t d,
                     uint64_t e, uint64_t f, uint64_t g, uint64_t h,
                     uint64_t i, uint64_t j, uint64_t k) {
    uint64_t result1, result2, result3, result4, result5;
    
    /* Extended inline assembly with 11 input operands and 5 outputs */
    asm volatile (
        /* Dummy operations using all 11 inputs */
        "add %[r1], %[a], %[b]\n\t"
        "add %[r2], %[c], %[d]\n\t"
        "add %[r3], %[e], %[f]\n\t"
        "add %[r4], %[g], %[h]\n\t"
        "add %[r5], %[i], %[j]\n\t"
        "add %[r1], %[r1], %[k]\n\t"
        "add %[r2], %[r2], %[r1]\n\t"
        "add %[r3], %[r3], %[r2]\n\t"
        "add %[r4], %[r4], %[r3]\n\t"
        "add %[r5], %[r5], %[r4]\n\t"
        "add %[r1], %[r1], %[r5]"
        : [r1] "=r" (result1),
          [r2] "=r" (result2),
          [r3] "=r" (result3),
          [r4] "=r" (result4),
          [r5] "=r" (result5)
        : [a] "r" (a),
          [b] "r" (b),
          [c] "r" (c),
          [d] "r" (d),
          [e] "r" (e),
          [f] "r" (f),
          [g] "r" (g),
          [h] "r" (h),
          [i] "r" (i),
          [j] "r" (j),
          [k] "r" (k)
        : "cc"
    );
    
    return (int)((result1 + result2 + result3 + result4 + result5) & 0xFFFFFFFF);
}
#endif

int main(int argc, char *argv[]) {
    /* Initialize 11 variables with non-constant values */
    uint64_t vars[11];
    
    /* Use argv for some variability, PRNG for the rest */
    for (int idx = 0; idx < 11; idx++) {
        if (idx < argc && argv[idx] != NULL) {
            vars[idx] = (uint64_t)argv[idx][0] * 100 + idx;
        } else {
            vars[idx] = rand_val() % 1000;
        }
    }
    
    /* Call both functions to trigger different expansion paths */
    int result1 = func_10_operands(vars[0], vars[1], vars[2], vars[3],
                                   vars[4], vars[5], vars[6], vars[7],
                                   vars[8], vars[9]);
    
    int result2 = func_11_operands(vars[0], vars[1], vars[2], vars[3],
                                   vars[4], vars[5], vars[6], vars[7],
                                   vars[8], vars[9], vars[10]);
    
    /* Combine results to prevent dead code elimination */
    int final_result = result1 + result2;
    
    printf("Result: %d\n", final_result);
    
    /* Use result in a conditional to prevent optimization */
    if (final_result > 1000000) {
        printf("Large result detected\n");
    }
    
    return final_result & 0xFF;
}
