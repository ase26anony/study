/* Test program to cover 10/11 operand expansion in optabs.cc */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Simple PRNG to generate non-constant values */
static uint64_t simple_rand(uint64_t seed) {
    return seed * 1103515245 + 12345;
}

/* Function using 10 operands */
#ifdef __AVX512F__
#include <immintrin.h>
int func_10_operands(__m512i a, __m512i b, __m512i c, __m512i d,
                     __m512i e, __m512i f, __m512i g, __m512i h,
                     __m512i i, __m512i j) {
    /* AVX-512 compress store with mask - expands to many operands */
    char buffer[64] __attribute__((aligned(64)));
    __mmask16 mask = 0xAAAA;  /* 1010101010101010 pattern */
    
    /* This intrinsic expands to many operands during RTL expansion */
    _mm512_mask_compressstoreu_epi32(buffer, mask, a);
    
    /* Use all operands to prevent optimization */
    __m512i temp = _mm512_add_epi32(a, b);
    temp = _mm512_add_epi32(temp, c);
    temp = _mm512_add_epi32(temp, d);
    temp = _mm512_add_epi32(temp, e);
    temp = _mm512_add_epi32(temp, f);
    temp = _mm512_add_epi32(temp, g);
    temp = _mm512_add_epi32(temp, h);
    temp = _mm512_add_epi32(temp, i);
    temp = _mm512_add_epi32(temp, j);
    
    /* Extract and return a scalar result */
    return _mm512_reduce_add_epi32(temp);
}
#elif defined(__ARM_FEATURE_SVE)
#include <arm_sve.h>
int func_10_operands(svint32_t a, svint32_t b, svint32_t c, svint32_t d,
                     svint32_t e, svint32_t f, svint32_t g, svint32_t h,
                     svint32_t i, svint32_t j) {
    /* SVE scatter store with predicate - many operands */
    uint64_t base[4] = {0, 1, 2, 3};
    svbool_t pg = svptrue_b32();
    
    /* Complex SVE operation that may expand to many operands */
    svint32_t temp = svadd_x(pg, a, b);
    temp = svadd_x(pg, temp, c);
    temp = svadd_x(pg, temp, d);
    temp = svadd_x(pg, temp, e);
    temp = svadd_x(pg, temp, f);
    temp = svadd_x(pg, temp, g);
    temp = svadd_x(pg, temp, h);
    temp = svadd_x(pg, temp, i);
    temp = svadd_x(pg, temp, j);
    
    /* Reduce to scalar */
    return svaddv(pg, temp);
}
#else
/* Generic fallback using inline assembly with 10 operands */
int func_10_operands(int a, int b, int c, int d, int e,
                     int f, int g, int h, int i, int j) {
    int result;
    
    /* Extended inline assembly with 10 input operands and 1 output */
    __asm__ volatile (
        "/* 10-operand dummy operation */\n\t"
        "add %[out], %[a], %[b]\n\t"
        "add %[out], %[out], %[c]\n\t"
        "add %[out], %[out], %[d]\n\t"
        "add %[out], %[out], %[e]\n\t"
        "add %[out], %[out], %[f]\n\t"
        "add %[out], %[out], %[g]\n\t"
        "add %[out], %[out], %[h]\n\t"
        "add %[out], %[out], %[i]\n\t"
        "add %[out], %[out], %[j]"
        : [out] "=r" (result)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j)
        : "cc"
    );
    
    return result;
}
#endif

/* Function using 11 operands */
#ifdef __AVX512F__
int func_11_operands(__m512i a, __m512i b, __m512i c, __m512i d,
                     __m512i e, __m512i f, __m512i g, __m512i h,
                     __m512i i, __m512i j, __m512i k) {
    /* AVX-512 masked gather - potentially many operands */
    __m512i index = _mm512_set_epi32(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
    __mmask16 mask = 0x5555;  /* 0101010101010101 pattern */
    int base[64] __attribute__((aligned(64)));
    
    /* Initialize base array */
    for (int idx = 0; idx < 64; idx++) base[idx] = idx;
    
    /* This may expand to many operands */
    __m512i gathered = _mm512_mask_i32gather_epi32(a, mask, index, base, 4);
    
    /* Use all operands */
    __m512i temp = _mm512_add_epi32(gathered, b);
    temp = _mm512_add_epi32(temp, c);
    temp = _mm512_add_epi32(temp, d);
    temp = _mm512_add_epi32(temp, e);
    temp = _mm512_add_epi32(temp, f);
    temp = _mm512_add_epi32(temp, g);
    temp = _mm512_add_epi32(temp, h);
    temp = _mm512_add_epi32(temp, i);
    temp = _mm512_add_epi32(temp, j);
    temp = _mm512_add_epi32(temp, k);
    
    return _mm512_reduce_add_epi32(temp);
}
#elif defined(__ARM_FEATURE_SVE)
int func_11_operands(svint32_t a, svint32_t b, svint32_t c, svint32_t d,
                     svint32_t e, svint32_t f, svint32_t g, svint32_t h,
                     svint32_t i, svint32_t j, svint32_t k) {
    /* SVE gather load with multiple operands */
    uint64_t bases[4] = {0, 1, 2, 3};
    svuint64_t offsets = svindex_u64(0, 1);
    svbool_t pg = svptrue_b32();
    
    /* Complex operation using all 11 operands */
    svint32_t temp = svadd_x(pg, a, b);
    temp = svadd_x(pg, temp, c);
    temp = svadd_x(pg, temp, d);
    temp = svadd_x(pg, temp, e);
    temp = svadd_x(pg, temp, f);
    temp = svadd_x(pg, temp, g);
    temp = svadd_x(pg, temp, h);
    temp = svadd_x(pg, temp, i);
    temp = svadd_x(pg, temp, j);
    temp = svadd_x(pg, temp, k);
    
    /* Additional operation to ensure 11 operands are used */
    temp = svmul_x(pg, temp, a);
    
    return svaddv(pg, temp);
}
#else
/* Generic fallback using inline assembly with 11 operands */
int func_11_operands(int a, int b, int c, int d, int e,
                     int f, int g, int h, int i, int j, int k) {
    int result;
    
    /* Extended inline assembly with 11 input operands */
    __asm__ volatile (
        "/* 11-operand dummy operation */\n\t"
        "mov %[out], %[a]\n\t"
        "add %[out], %[out], %[b]\n\t"
        "add %[out], %[out], %[c]\n\t"
        "add %[out], %[out], %[d]\n\t"
        "add %[out], %[out], %[e]\n\t"
        "add %[out], %[out], %[f]\n\t"
        "add %[out], %[out], %[g]\n\t"
        "add %[out], %[out], %[h]\n\t"
        "add %[out], %[out], %[i]\n\t"
        "add %[out], %[out], %[j]\n\t"
        "add %[out], %[out], %[k]"
        : [out] "=r" (result)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j), [k] "r" (k)
        : "cc"
    );
    
    return result;
}
#endif

int main(int argc, char *argv[]) {
    /* Initialize variables with non-constant values */
    uint64_t seed = (argc > 1) ? (uint64_t)argv[1][0] : 123456789;
    
    /* Generate 11 different values */
    int vals[11];
    for (int idx = 0; idx < 11; idx++) {
        seed = simple_rand(seed);
        vals[idx] = (int)(seed & 0x7FFFFFFF);  /* Positive values only */
    }
    
    /* Call both functions with appropriate number of arguments */
    int result1 = func_10_operands(vals[0], vals[1], vals[2], vals[3],
                                   vals[4], vals[5], vals[6], vals[7],
                                   vals[8], vals[9]);
    
    int result2 = func_11_operands(vals[0], vals[1], vals[2], vals[3],
                                   vals[4], vals[5], vals[6], vals[7],
                                   vals[8], vals[9], vals[10]);
    
    /* Combine results to prevent dead code elimination */
    int final_result = result1 + result2;
    
    printf("Result: %d\n", final_result);
    
    /* Use all values to prevent optimization */
    for (int idx = 0; idx < 11; idx++) {
        if (vals[idx] == 0) {
            printf("Unexpected zero at index %d\n", idx);
        }
    }
    
    return (final_result > 0) ? 0 : 1;
}
