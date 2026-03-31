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

/* Function for 10 operands */
#ifdef __AVX512F__
#include <immintrin.h>
int func_10_operands(__m512i a, __m512i b, __m512i c, __m512i d,
                     __m512i e, __m512i f, __m512i g, __m512i h,
                     __m512i i, __m512i j) {
    /* Use AVX-512 mask compress store which can expand to many operands */
    __mmask16 mask = 0xAAAA;  /* 1010101010101010 */
    char buffer[64] __attribute__((aligned(64)));
    
    /* This intrinsic often expands to many operands */
    _mm512_mask_compressstoreu_epi32(buffer, mask, a);
    
    /* Use result to prevent optimization */
    __m512i result = _mm512_add_epi32(a, b);
    result = _mm512_add_epi32(result, c);
    result = _mm512_add_epi32(result, d);
    result = _mm512_add_epi32(result, e);
    result = _mm512_add_epi32(result, f);
    result = _mm512_add_epi32(result, g);
    result = _mm512_add_epi32(result, h);
    result = _mm512_add_epi32(result, i);
    result = _mm512_add_epi32(result, j);
    
    /* Extract and return a scalar */
    return _mm512_reduce_add_epi32(result);
}
#elif defined(__ARM_FEATURE_SVE)
#include <arm_sve.h>
int func_10_operands(svint32_t a, svint32_t b, svint32_t c, svint32_t d,
                     svint32_t e, svint32_t f, svint32_t g, svint32_t h,
                     svint32_t i, svint32_t j) {
    /* SVE scatter store with many operands */
    svbool_t pg = svptrue_b32();
    uint64_t base = 0x1000;
    
    /* Complex SVE operation that may need many operands */
    svint32_t result = svadd_m(pg, a, b);
    result = svadd_m(pg, result, c);
    result = svadd_m(pg, result, d);
    result = svadd_m(pg, result, e);
    result = svadd_m(pg, result, f);
    result = svadd_m(pg, result, g);
    result = svadd_m(pg, result, h);
    result = svadd_m(pg, result, i);
    result = svadd_m(pg, result, j);
    
    /* Reduce to scalar */
    return svaddv(pg, result);
}
#else
/* Generic fallback using inline assembly with 10 operands */
int func_10_operands(int a, int b, int c, int d, int e,
                     int f, int g, int h, int i, int j) {
    int result1, result2, result3;
    
    /* Extended inline assembly with 10 input operands and 3 outputs */
    asm volatile (
        "/* 10-operand dummy operation */\n\t"
        "add %[r1], %[a], %[b]\n\t"
        "add %[r2], %[c], %[d]\n\t"
        "add %[r3], %[e], %[f]\n\t"
        "add %[r1], %[r1], %[g]\n\t"
        "add %[r2], %[r2], %[h]\n\t"
        "add %[r3], %[r3], %[i]\n\t"
        "add %[r1], %[r1], %[j]\n\t"
        "add %[r1], %[r1], %[r2]\n\t"
        "add %[out], %[r1], %[r3]"
        : [out] "=r" (result1), [r1] "=&r" (result2), [r2] "=&r" (result3)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j)
        : "cc"
    );
    
    return result1;
}
#endif

/* Function for 11 operands */
#ifdef __AVX512F__
int func_11_operands(__m512i a, __m512i b, __m512i c, __m512i d,
                     __m512i e, __m512i f, __m512i g, __m512i h,
                     __m512i i, __m512i j, __m512i k) {
    /* Complex AVX-512 operation with mask and multiple sources */
    __mmask16 mask = 0x5555;  /* 0101010101010101 */
    __m512i temp1, temp2, temp3;
    
    /* Operations that might expand to 11 operands */
    temp1 = _mm512_mask_add_epi32(a, mask, b, c);
    temp2 = _mm512_mask_add_epi32(d, mask, e, f);
    temp3 = _mm512_mask_add_epi32(g, mask, h, i);
    
    /* Combine results */
    __m512i result = _mm512_add_epi32(temp1, temp2);
    result = _mm512_add_epi32(result, temp3);
    result = _mm512_add_epi32(result, j);
    result = _mm512_add_epi32(result, k);
    
    return _mm512_reduce_add_epi32(result);
}
#elif defined(__ARM_FEATURE_SVE)
int func_11_operands(svint32_t a, svint32_t b, svint32_t c, svint32_t d,
                     svint32_t e, svint32_t f, svint32_t g, svint32_t h,
                     svint32_t i, svint32_t j, svint32_t k) {
    /* SVE operation with predicate and multiple vectors */
    svbool_t pg = svptrue_b32();
    
    svint32_t temp1 = svadd_m(pg, a, b);
    svint32_t temp2 = svadd_m(pg, c, d);
    svint32_t temp3 = svadd_m(pg, e, f);
    
    svint32_t result = svadd_m(pg, temp1, temp2);
    result = svadd_m(pg, result, temp3);
    result = svadd_m(pg, result, g);
    result = svadd_m(pg, result, h);
    result = svadd_m(pg, result, i);
    result = svadd_m(pg, result, j);
    result = svadd_m(pg, result, k);
    
    return svaddv(pg, result);
}
#else
/* Generic fallback using inline assembly with 11 operands */
int func_11_operands(int a, int b, int c, int d, int e,
                     int f, int g, int h, int i, int j, int k) {
    int result1, result2, result3, result4;
    
    /* Extended inline assembly with 11 input operands and 4 outputs */
    asm volatile (
        "/* 11-operand dummy operation */\n\t"
        "add %[r1], %[a], %[b]\n\t"
        "add %[r2], %[c], %[d]\n\t"
        "add %[r3], %[e], %[f]\n\t"
        "add %[r4], %[g], %[h]\n\t"
        "add %[r1], %[r1], %[i]\n\t"
        "add %[r2], %[r2], %[j]\n\t"
        "add %[r3], %[r3], %[k]\n\t"
        "add %[r1], %[r1], %[r2]\n\t"
        "add %[r1], %[r1], %[r3]\n\t"
        "add %[out], %[r1], %[r4]"
        : [out] "=r" (result1), [r1] "=&r" (result2), 
          [r2] "=&r" (result3), [r3] "=&r" (result4)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j), [k] "r" (k)
        : "cc"
    );
    
    return result1;
}
#endif

int main(int argc, char *argv[]) {
    /* Initialize variables with non-constant values */
    int vals[20];
    
    /* Use argv for some values, PRNG for others to avoid constant propagation */
    for (int i = 0; i < 20; i++) {
        if (i < argc && i < 10) {
            vals[i] = argv[i][0];  /* First char of each argument */
        } else {
            vals[i] = (int)(prng_next() & 0xFF);
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
    
    return final_result != 0 ? 0 : 1;
}
