/* Test program to cover 10/11 operand expansion in optabs.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Simple PRNG to generate non-constant values */
static unsigned int seed = 12345;
static unsigned int prng(void) {
    seed = seed * 1103515245 + 12345;
    return seed;
}

/* Helper to convert string arguments to integers */
static int arg_to_int(const char *arg, int idx) {
    if (arg && *arg) return atoi(arg) + idx;
    return prng() % 100 + idx;
}

/* Generic fallback using inline assembly with many operands */
#ifndef TARGET_SPECIFIC_INTRINSIC

/* Function for 10 operands */
static int func_10_operands(int a0, int a1, int a2, int a3, int a4,
                           int a5, int a6, int a7, int a8, int a9) {
    int result;
    /* Extended inline assembly with 10 input operands and 1 output */
    __asm__ volatile (
        "/* Dummy 10-operand assembly template */\n\t"
        "add %1, %2, %0\n\t"
        "add %0, %3, %0\n\t"
        "add %0, %4, %0\n\t"
        "add %0, %5, %0\n\t"
        "add %0, %6, %0\n\t"
        "add %0, %7, %0\n\t"
        "add %0, %8, %0\n\t"
        "add %0, %9, %0\n\t"
        "add %0, %10, %0"
        : "=r" (result)
        : "r" (a0), "r" (a1), "r" (a2), "r" (a3),
          "r" (a4), "r" (a5), "r" (a6), "r" (a7),
          "r" (a8), "r" (a9)
        : "cc"
    );
    return result;
}

/* Function for 11 operands */
static int func_11_operands(int a0, int a1, int a2, int a3, int a4,
                           int a5, int a6, int a7, int a8, int a9,
                           int a10) {
    int result;
    /* Extended inline assembly with 11 input operands and 1 output */
    __asm__ volatile (
        "/* Dummy 11-operand assembly template */\n\t"
        "add %1, %2, %0\n\t"
        "add %0, %3, %0\n\t"
        "add %0, %4, %0\n\t"
        "add %0, %5, %0\n\t"
        "add %0, %6, %0\n\t"
        "add %0, %7, %0\n\t"
        "add %0, %8, %0\n\t"
        "add %0, %9, %0\n\t"
        "add %0, %10, %0\n\t"
        "add %0, %11, %0"
        : "=r" (result)
        : "r" (a0), "r" (a1), "r" (a2), "r" (a3),
          "r" (a4), "r" (a5), "r" (a6), "r" (a7),
          "r" (a8), "r" (a9), "r" (a10)
        : "cc"
    );
    return result;
}

#else
/* Target-specific implementations using high-operand-count intrinsics */

#ifdef __AVX512F__
#include <immintrin.h>

/* AVX-512 implementation for 10 operands */
static int func_10_operands(int a0, int a1, int a2, int a3, int a4,
                           int a5, int a6, int a7, int a8, int a9) {
    /* Create vector data from scalar inputs */
    __m512i v0 = _mm512_set1_epi32(a0);
    __m512i v1 = _mm512_set1_epi32(a1);
    __m512i v2 = _mm512_set1_epi32(a2);
    __m512i v3 = _mm512_set1_epi32(a3);
    __m512i v4 = _mm512_set1_epi32(a4);
    __m512i v5 = _mm512_set1_epi32(a5);
    __m512i v6 = _mm512_set1_epi32(a6);
    __m512i v7 = _mm512_set1_epi32(a7);
    __m512i v8 = _mm512_set1_epi32(a8);
    __m512i v9 = _mm512_set1_epi32(a9);
    
    /* Complex sequence of operations using many operands */
    __m512i sum = _mm512_add_epi32(v0, v1);
    sum = _mm512_add_epi32(sum, v2);
    sum = _mm512_add_epi32(sum, v3);
    sum = _mm512_add_epi32(sum, v4);
    sum = _mm512_add_epi32(sum, v5);
    sum = _mm512_add_epi32(sum, v6);
    sum = _mm512_add_epi32(sum, v7);
    sum = _mm512_add_epi32(sum, v8);
    sum = _mm512_add_epi32(sum, v9);
    
    /* Reduce to scalar */
    return _mm512_reduce_add_epi32(sum);
}

/* AVX-512 implementation for 11 operands */
static int func_11_operands(int a0, int a1, int a2, int a3, int a4,
                           int a5, int a6, int a7, int a8, int a9,
                           int a10) {
    /* Similar to 10-operand version but with one more */
    __m512i v0 = _mm512_set1_epi32(a0);
    __m512i v1 = _mm512_set1_epi32(a1);
    __m512i v2 = _mm512_set1_epi32(a2);
    __m512i v3 = _mm512_set1_epi32(a3);
    __m512i v4 = _mm512_set1_epi32(a4);
    __m512i v5 = _mm512_set1_epi32(a5);
    __m512i v6 = _mm512_set1_epi32(a6);
    __m512i v7 = _mm512_set1_epi32(a7);
    __m512i v8 = _mm512_set1_epi32(a8);
    __m512i v9 = _mm512_set1_epi32(a9);
    __m512i v10 = _mm512_set1_epi32(a10);
    
    __m512i sum = _mm512_add_epi32(v0, v1);
    sum = _mm512_add_epi32(sum, v2);
    sum = _mm512_add_epi32(sum, v3);
    sum = _mm512_add_epi32(sum, v4);
    sum = _mm512_add_epi32(sum, v5);
    sum = _mm512_add_epi32(sum, v6);
    sum = _mm512_add_epi32(sum, v7);
    sum = _mm512_add_epi32(sum, v8);
    sum = _mm512_add_epi32(sum, v9);
    sum = _mm512_add_epi32(sum, v10);
    
    return _mm512_reduce_add_epi32(sum);
}

#elif defined(__ARM_FEATURE_SVE)
#include <arm_sve.h>

/* ARM SVE implementation */
static int func_10_operands(int a0, int a1, int a2, int a3, int a4,
                           int a5, int a6, int a7, int a8, int a9) {
    /* Create predicate and vectors from scalars */
    svbool_t pg = svptrue_b32();
    svint32_t v0 = svdup_s32(a0);
    svint32_t v1 = svdup_s32(a1);
    svint32_t v2 = svdup_s32(a2);
    svint32_t v3 = svdup_s32(a3);
    svint32_t v4 = svdup_s32(a4);
    svint32_t v5 = svdup_s32(a5);
    svint32_t v6 = svdup_s32(a6);
    svint32_t v7 = svdup_s32(a7);
    svint32_t v8 = svdup_s32(a8);
    svint32_t v9 = svdup_s32(a9);
    
    /* Chain of operations using many operands */
    svint32_t sum = svadd_s32_z(pg, v0, v1);
    sum = svadd_s32_z(pg, sum, v2);
    sum = svadd_s32_z(pg, sum, v3);
    sum = svadd_s32_z(pg, sum, v4);
    sum = svadd_s32_z(pg, sum, v5);
    sum = svadd_s32_z(pg, sum, v6);
    sum = svadd_s32_z(pg, sum, v7);
    sum = svadd_s32_z(pg, sum, v8);
    sum = svadd_s32_z(pg, sum, v9);
    
    /* Reduce to scalar */
    return svaddv_s32(pg, sum);
}

static int func_11_operands(int a0, int a1, int a2, int a3, int a4,
                           int a5, int a6, int a7, int a8, int a9,
                           int a10) {
    svbool_t pg = svptrue_b32();
    svint32_t v0 = svdup_s32(a0);
    svint32_t v1 = svdup_s32(a1);
    svint32_t v2 = svdup_s32(a2);
    svint32_t v3 = svdup_s32(a3);
    svint32_t v4 = svdup_s32(a4);
    svint32_t v5 = svdup_s32(a5);
    svint32_t v6 = svdup_s32(a6);
    svint32_t v7 = svdup_s32(a7);
    svint32_t v8 = svdup_s32(a8);
    svint32_t v9 = svdup_s32(a9);
    svint32_t v10 = svdup_s32(a10);
    
    svint32_t sum = svadd_s32_z(pg, v0, v1);
    sum = svadd_s32_z(pg, sum, v2);
    sum = svadd_s32_z(pg, sum, v3);
    sum = svadd_s32_z(pg, sum, v4);
    sum = svadd_s32_z(pg, sum, v5);
    sum = svadd_s32_z(pg, sum, v6);
    sum = svadd_s32_z(pg, sum, v7);
    sum = svadd_s32_z(pg, sum, v8);
    sum = svadd_s32_z(pg, sum, v9);
    sum = svadd_s32_z(pg, sum, v10);
    
    return svaddv_s32(pg, sum);
}

#endif
#endif /* TARGET_SPECIFIC_INTRINSIC */

int main(int argc, char *argv[]) {
    /* Initialize 11 variables with non-constant values */
    int vars[11];
    
    for (int i = 0; i < 11; i++) {
        vars[i] = arg_to_int((argc > i + 1) ? argv[i + 1] : NULL, i);
    }
    
    /* Call both functions with overlapping but different operand counts */
    int result1 = func_10_operands(vars[0], vars[1], vars[2], vars[3], vars[4],
                                  vars[5], vars[6], vars[7], vars[8], vars[9]);
    
    int result2 = func_11_operands(vars[0], vars[1], vars[2], vars[3], vars[4],
                                  vars[5], vars[6], vars[7], vars[8], vars[9],
                                  vars[10]);
    
    /* Combine results to prevent dead code elimination */
    int final_result = result1 + result2;
    
    printf("Result: %d\n", final_result);
    return 0;
}
