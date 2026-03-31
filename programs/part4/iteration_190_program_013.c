/* Test program for covering 10/11 operand expansion in optabs.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Simple PRNG to generate non-constant values */
static unsigned int prng_state = 123456789;
static unsigned int prng() {
    prng_state = prng_state * 1103515245 + 12345;
    return prng_state;
}

/* Function to use values from argv to initialize variables */
static unsigned long get_value_from_argv(int argc, char **argv, int idx) {
    if (argc > idx && argv[idx]) {
        return (unsigned long)strtoul(argv[idx], NULL, 0);
    }
    return prng() + idx * 7919; /* Prime multiplier for variety */
}

/* ========== 10-OPERAND FUNCTION ========== */
#ifdef __AVX512F__
#include <immintrin.h>
/* AVX-512 intrinsic that can expand to many operands */
static long func_10_operands(__m512i a, __m512i b, __m512i c, __m512i d,
                             __m512i e, __m512i f, __m512i g, __m512i h,
                             __mmask16 mask, void *ptr) {
    /* Complex store operation that may require many operands */
    __m512i result = _mm512_mask_add_epi32(a, mask, b, c);
    result = _mm512_mask_add_epi32(result, mask, d, e);
    result = _mm512_mask_add_epi32(result, mask, f, g);
    result = _mm512_mask_add_epi32(result, mask, result, h);
    
    /* Store with mask - may expand to many operands */
    _mm512_mask_storeu_epi32(ptr, mask, result);
    
    /* Use result to prevent optimization */
    long sum = 0;
    int *p = (int*)&result;
    for (int i = 0; i < 16; i++) {
        sum += p[i];
    }
    return sum;
}
#elif defined(__ARM_FEATURE_SVE)
#include <arm_sve.h>
/* ARM SVE intrinsic with many operands */
static long func_10_operands(svint32_t a, svint32_t b, svint32_t c, svint32_t d,
                             svint32_t e, svint32_t f, svint32_t g, svint32_t h,
                             svbool_t pg, void *ptr) {
    /* Complex SVE operation */
    svint32_t result = svadd_m(pg, a, b);
    result = svadd_m(pg, result, c);
    result = svadd_m(pg, result, d);
    result = svadd_m(pg, result, e);
    result = svadd_m(pg, result, f);
    result = svadd_m(pg, result, g);
    result = svadd_m(pg, result, h);
    
    /* Store operation */
    svst1(pg, (int32_t*)ptr, result);
    
    /* Extract and sum elements */
    int32_t temp[svcntw()];
    svst1(pg, temp, result);
    
    long sum = 0;
    for (int i = 0; i < svcntw(); i++) {
        sum += temp[i];
    }
    return sum;
}
#else
/* Generic fallback using inline assembly with 10 operands */
static long func_10_operands(long a, long b, long c, long d, long e,
                             long f, long g, long h, long i, long j) {
    long result1, result2, result3;
    
    /* Extended inline assembly with 10 explicit operands */
    asm volatile (
        /* Dummy multi-operand operation */
        "mov %0, %1\n\t"
        "add %0, %0, %2\n\t"
        "add %0, %0, %3\n\t"
        "add %0, %0, %4\n\t"
        "add %0, %0, %5\n\t"
        "add %0, %0, %6\n\t"
        "add %0, %0, %7\n\t"
        "add %0, %0, %8\n\t"
        "add %0, %0, %9"
        : "=r" (result1)
        : "r" (a), "r" (b), "r" (c), "r" (d), 
          "r" (e), "r" (f), "r" (g), "r" (h), "r" (i)
        : "cc"
    );
    
    /* Use all inputs to prevent optimization */
    result2 = j + a + b + c + d;
    result3 = e + f + g + h + i;
    
    return result1 + result2 + result3;
}
#endif

/* ========== 11-OPERAND FUNCTION ========== */
#ifdef __AVX512F__
#include <immintrin.h>
static long func_11_operands(__m512i a, __m512i b, __m512i c, __m512i d,
                             __m512i e, __m512i f, __m512i g, __m512i h,
                             __m512i i, __mmask16 mask, void *ptr) {
    /* Even more complex operation chain */
    __m512i temp1 = _mm512_mask_add_epi32(a, mask, b, c);
    __m512i temp2 = _mm512_mask_add_epi32(d, mask, e, f);
    __m512i temp3 = _mm512_mask_add_epi32(g, mask, h, i);
    
    __m512i result = _mm512_mask_add_epi32(temp1, mask, temp2, temp3);
    
    /* Compress store operation - can require many operands */
    _mm512_mask_compressstoreu_epi32(ptr, mask, result);
    
    /* Use result */
    long sum = 0;
    int *p = (int*)&result;
    for (int j = 0; j < 16; j++) {
        sum += p[j];
    }
    return sum;
}
#elif defined(__ARM_FEATURE_SVE)
#include <arm_sve.h>
static long func_11_operands(svint32_t a, svint32_t b, svint32_t c, svint32_t d,
                             svint32_t e, svint32_t f, svint32_t g, svint32_t h,
                             svint32_t i, svbool_t pg, void *ptr) {
    /* SVE scatter store with many operands */
    svuint64_t indices = svdup_u64(0);
    svint32_t result = svadd_m(pg, a, b);
    result = svadd_m(pg, result, c);
    result = svadd_m(pg, result, d);
    result = svadd_m(pg, result, e);
    result = svadd_m(pg, result, f);
    result = svadd_m(pg, result, g);
    result = svadd_m(pg, result, h);
    result = svadd_m(pg, result, i);
    
    /* Complex store operation */
    svst1_scatter_u64index_s32(pg, (int32_t*)ptr, indices, result);
    
    /* Extract and sum */
    int32_t temp[svcntw()];
    svst1(pg, temp, result);
    
    long sum = 0;
    for (int j = 0; j < svcntw(); j++) {
        sum += temp[j];
    }
    return sum;
}
#else
/* Generic fallback with 11 operands */
static long func_11_operands(long a, long b, long c, long d, long e,
                             long f, long g, long h, long i, long j, long k) {
    long result1, result2, result3;
    
    /* Inline assembly with 11 explicit operands */
    asm volatile (
        /* Complex dummy operation using all 11 inputs */
        "mov %0, %1\n\t"
        "add %0, %0, %2\n\t"
        "add %0, %0, %3\n\t"
        "add %0, %0, %4\n\t"
        "add %0, %0, %5\n\t"
        "add %0, %0, %6\n\t"
        "add %0, %0, %7\n\t"
        "add %0, %0, %8\n\t"
        "add %0, %0, %9\n\t"
        "add %0, %0, %10"
        : "=r" (result1)
        : "r" (a), "r" (b), "r" (c), "r" (d), "r" (e),
          "r" (f), "r" (g), "r" (h), "r" (i), "r" (j)
        : "cc"
    );
    
    /* Use all inputs including the 11th */
    result2 = k + a + b + c + d + e;
    result3 = f + g + h + i + j;
    
    return result1 + result2 + result3;
}
#endif

/* ========== MAIN FUNCTION ========== */
int main(int argc, char **argv) {
    /* Initialize many variables with non-constant values */
    long vars[20];
    for (int idx = 0; idx < 20; idx++) {
        vars[idx] = get_value_from_argv(argc, argv, idx);
    }
    
    long total = 0;
    
    /* Call 10-operand function */
    total += func_10_operands(vars[0], vars[1], vars[2], vars[3], vars[4],
                              vars[5], vars[6], vars[7], vars[8], vars[9]);
    
    /* Call 11-operand function */
    total += func_11_operands(vars[10], vars[11], vars[12], vars[13], vars[14],
                              vars[15], vars[16], vars[17], vars[18], vars[19], 
                              vars[0]); /* Reuse first var as 11th operand */
    
    /* Use result to prevent optimization */
    printf("Result: %ld\n", total);
    
    /* Additional volatile store to ensure operations aren't optimized away */
    volatile long check = total;
    (void)check;
    
    return (int)(total % 256);
}
