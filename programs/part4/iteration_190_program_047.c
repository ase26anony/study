/* Test program to cover 10/11 operand expansion in optabs.cc */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Simple PRNG to generate non-constant values */
static unsigned int seed = 12345;
static unsigned int prng(void) {
    seed = seed * 1103515245 + 12345;
    return seed;
}

/* Generic fallback using inline assembly with many operands */
#ifdef GENERIC_FALLBACK

/* Function for 10 operands */
int func_10_operands(int a0, int a1, int a2, int a3, int a4,
                     int a5, int a6, int a7, int a8, int a9) {
    int result;
    /* Extended inline assembly with 10 input operands and 1 output */
    __asm__ volatile (
        "/* Dummy 10-operand operation %0 = %1 + %2 + ... + %10 */\n\t"
        "add %0, %1, %2\n\t"
        "add %0, %0, %3\n\t"
        "add %0, %0, %4\n\t"
        "add %0, %0, %5\n\t"
        "add %0, %0, %6\n\t"
        "add %0, %0, %7\n\t"
        "add %0, %0, %8\n\t"
        "add %0, %0, %9\n\t"
        "add %0, %0, %10"
        : "=r" (result)
        : "r" (a0), "r" (a1), "r" (a2), "r" (a3), "r" (a4),
          "r" (a5), "r" (a6), "r" (a7), "r" (a8), "r" (a9)
        : "cc"
    );
    return result;
}

/* Function for 11 operands */
int func_11_operands(int a0, int a1, int a2, int a3, int a4,
                     int a5, int a6, int a7, int a8, int a9,
                     int a10) {
    int result;
    /* Extended inline assembly with 11 input operands and 1 output */
    __asm__ volatile (
        "/* Dummy 11-operand operation %0 = sum of all inputs */\n\t"
        "add %0, %1, %2\n\t"
        "add %0, %0, %3\n\t"
        "add %0, %0, %4\n\t"
        "add %0, %0, %5\n\t"
        "add %0, %0, %6\n\t"
        "add %0, %0, %7\n\t"
        "add %0, %0, %8\n\t"
        "add %0, %0, %9\n\t"
        "add %0, %0, %10\n\t"
        "add %0, %0, %11"
        : "=r" (result)
        : "r" (a0), "r" (a1), "r" (a2), "r" (a3), "r" (a4),
          "r" (a5), "r" (a6), "r" (a7), "r" (a8), "r" (a9),
          "r" (a10)
        : "cc"
    );
    return result;
}

#elif defined(__AVX512F__) && defined(__AVX512VL__)
/* x86 AVX-512 implementation with high operand count intrinsics */
#include <immintrin.h>

/* AVX-512 mask compress/store intrinsic can involve many operands */
int func_10_operands(__m512i a0, __m512i a1, __m512i a2, __m512i a3,
                     __m512i a4, __m512i a5, __m512i a6, __m512i a7,
                     __m512i a8, __m512i a9) {
    /* Create a volatile buffer to prevent optimization */
    static volatile uint32_t buffer[16] __attribute__((aligned(64)));
    
    /* Use multiple operations that collectively use all operands */
    __m512i sum = _mm512_add_epi32(a0, a1);
    sum = _mm512_add_epi32(sum, a2);
    sum = _mm512_add_epi32(sum, a3);
    sum = _mm512_add_epi32(sum, a4);
    sum = _mm512_add_epi32(sum, a5);
    sum = _mm512_add_epi32(sum, a6);
    sum = _mm512_add_epi32(sum, a7);
    sum = _mm512_add_epi32(sum, a8);
    sum = _mm512_add_epi32(sum, a9);
    
    /* Use mask compress store which has multiple operands */
    __mmask16 mask = 0xAAAA;  /* Alternating mask */
    _mm512_mask_compressstoreu_epi32((void*)buffer, mask, sum);
    
    /* Extract and return a scalar result */
    return _mm512_reduce_add_epi32(sum) + buffer[0];
}

int func_11_operands(__m512i a0, __m512i a1, __m512i a2, __m512i a3,
                     __m512i a4, __m512i a5, __m512i a6, __m512i a7,
                     __m512i a8, __m512i a9, __m512i a10) {
    /* Similar to above but with 11 operands */
    static volatile uint32_t buffer[16] __attribute__((aligned(64)));
    
    __m512i sum = _mm512_add_epi32(a0, a1);
    sum = _mm512_add_epi32(sum, a2);
    sum = _mm512_add_epi32(sum, a3);
    sum = _mm512_add_epi32(sum, a4);
    sum = _mm512_add_epi32(sum, a5);
    sum = _mm512_add_epi32(sum, a6);
    sum = _mm512_add_epi32(sum, a7);
    sum = _mm512_add_epi32(sum, a8);
    sum = _mm512_add_epi32(sum, a9);
    sum = _mm512_add_epi32(sum, a10);
    
    __mmask16 mask = 0x5555;  /* Different alternating mask */
    _mm512_mask_compressstoreu_epi32((void*)buffer, mask, sum);
    
    return _mm512_reduce_add_epi32(sum) + buffer[1];
}

#elif defined(__ARM_FEATURE_SVE)
/* ARM SVE implementation */
#include <arm_sve.h>

/* SVE scatter store with predicate can involve many operands */
int func_10_operands(svint32_t a0, svint32_t a1, svint32_t a2, svint32_t a3,
                     svint32_t a4, svint32_t a5, svint32_t a6, svint32_t a7,
                     svint32_t a8, svint32_t a9) {
    /* Create predicate and base addresses */
    svbool_t pg = svptrue_b32();
    uint64_t bases[svcntw()] __attribute__((aligned(64)));
    volatile uint32_t buffer[256] __attribute__((aligned(64)));
    
    /* Initialize bases */
    for (int i = 0; i < svcntw(); i++) {
        bases[i] = (uint64_t)(&buffer[i * 8]);
    }
    
    /* Combine all inputs */
    svint32_t sum = svadd_z(pg, a0, a1);
    sum = svadd_z(pg, sum, a2);
    sum = svadd_z(pg, sum, a3);
    sum = svadd_z(pg, sum, a4);
    sum = svadd_z(pg, sum, a5);
    sum = svadd_z(pg, sum, a6);
    sum = svadd_z(pg, sum, a7);
    sum = svadd_z(pg, sum, a8);
    sum = svadd_z(pg, sum, a9);
    
    /* Scatter store with multiple operands */
    svst1_scatter_u64base_s32(pg, svld1_u64(pg, bases), sum);
    
    /* Reduce to scalar */
    return svaddv(pg, sum);
}

int func_11_operands(svint32_t a0, svint32_t a1, svint32_t a2, svint32_t a3,
                     svint32_t a4, svint32_t a5, svint32_t a6, svint32_t a7,
                     svint32_t a8, svint32_t a9, svint32_t a10) {
    svbool_t pg = svptrue_b32();
    uint64_t bases[svcntw()] __attribute__((aligned(64)));
    volatile uint32_t buffer[256] __attribute__((aligned(64)));
    
    for (int i = 0; i < svcntw(); i++) {
        bases[i] = (uint64_t)(&buffer[i * 8]);
    }
    
    svint32_t sum = svadd_z(pg, a0, a1);
    sum = svadd_z(pg, sum, a2);
    sum = svadd_z(pg, sum, a3);
    sum = svadd_z(pg, sum, a4);
    sum = svadd_z(pg, sum, a5);
    sum = svadd_z(pg, sum, a6);
    sum = svadd_z(pg, sum, a7);
    sum = svadd_z(pg, sum, a8);
    sum = svadd_z(pg, sum, a9);
    sum = svadd_z(pg, sum, a10);
    
    svst1_scatter_u64base_s32(pg, svld1_u64(pg, bases), sum);
    
    return svaddv(pg, sum);
}

#else
/* Default to generic inline assembly if no specific intrinsics available */
#define GENERIC_FALLBACK 1
#include __FILE__  /* Include self to use generic implementation */
#endif

int main(int argc, char *argv[]) {
    /* Initialize many variables with non-constant values */
    int vals[20];
    
    /* Use argv for some variability, PRNG for the rest */
    for (int i = 0; i < 20; i++) {
        if (i < argc && i < 10) {
            vals[i] = argv[i][0];  /* Use first char of each arg */
        } else {
            vals[i] = prng() & 0xFF;
        }
    }
    
    /* Call both functions with overlapping but different operand sets */
    int result1 = func_10_operands(vals[0], vals[1], vals[2], vals[3], vals[4],
                                   vals[5], vals[6], vals[7], vals[8], vals[9]);
    
    int result2 = func_11_operands(vals[1], vals[2], vals[3], vals[4], vals[5],
                                   vals[6], vals[7], vals[8], vals[9], vals[10],
                                   vals[11]);
    
    /* Combine results to prevent dead code elimination */
    int final_result = result1 + result2;
    
    /* Print result to ensure side effect */
    printf("Result: %d\n", final_result);
    
    return 0;
}
