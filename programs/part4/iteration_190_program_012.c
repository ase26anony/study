/* Test case for GCC optabs.cc 10/11 operand expansion coverage */
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
    /* AVX-512 mask register - acts as 11th operand internally */
    __mmask16 mask = 0xAAAA;
    
    /* Complex operation with many operands - may expand to 10+ operands */
    __m512i temp1 = _mm512_add_epi32(a, b);
    __m512i temp2 = _mm512_add_epi32(c, d);
    __m512i temp3 = _mm512_add_epi32(e, f);
    __m512i temp4 = _mm512_add_epi32(g, h);
    
    /* Blend operation with mask - adds operand count */
    __m512i blended = _mm512_mask_blend_epi32(mask, temp1, temp2);
    
    /* Another operation combining results */
    __m512i result = _mm512_add_epi32(_mm512_add_epi32(temp3, temp4), blended);
    
    /* Use all input parameters to prevent optimization */
    result = _mm512_add_epi32(result, i);
    result = _mm512_add_epi32(result, j);
    
    /* Reduce to scalar for return */
    return _mm512_reduce_add_epi32(result);
}
#elif defined(__ARM_FEATURE_SVE)
#include <arm_sve.h>
int func_10_operands(svint32_t a, svint32_t b, svint32_t c, svint32_t d,
                     svint32_t e, svint32_t f, svint32_t g, svint32_t h,
                     svint32_t i, svint32_t j) {
    /* SVE predicate - adds to operand count */
    svbool_t pg = svptrue_b32();
    
    /* Complex SVE operation sequence */
    svint32_t temp1 = svadd_x(pg, a, b);
    svint32_t temp2 = svadd_x(pg, c, d);
    svint32_t temp3 = svadd_x(pg, e, f);
    svint32_t temp4 = svadd_x(pg, g, h);
    
    /* Combine operations */
    svint32_t combined = svadd_x(pg, temp1, temp2);
    svint32_t result = svadd_x(pg, svadd_x(pg, temp3, temp4), combined);
    
    /* Use remaining parameters */
    result = svadd_x(pg, result, i);
    result = svadd_x(pg, result, j);
    
    /* Extract first element */
    return svlastb(svptrue_b32(), result);
}
#else
/* Generic fallback using inline assembly with 10 operands */
int func_10_operands(int a, int b, int c, int d, int e,
                     int f, int g, int h, int i, int j) {
    int result;
    
    /* Extended inline assembly with 10 input operands and 1 output */
    __asm__ volatile (
        "/* 10-operand dummy operation */\n\t"
        "add %[a], %[b]\n\t"
        "add %[c], %[d]\n\t"
        "add %[e], %[f]\n\t"
        "add %[g], %[h]\n\t"
        "add %[i], %[j]\n\t"
        "mov %[res], %[a]"
        : [res] "=r" (result)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j)
        : "cc"
    );
    
    return result;
}
#endif

/* Function for 11 operands */
#ifdef __AVX512F__
int func_11_operands(__m512i a, __m512i b, __m512i c, __m512i d,
                     __m512i e, __m512i f, __m512i g, __m512i h,
                     __m512i i, __m512i j, __m512i k) {
    /* Multiple mask registers for complex operation */
    __mmask16 mask1 = 0x5555;
    __mmask16 mask2 = 0xAAAA;
    __mmask16 mask3 = 0xFFFF;
    
    /* Complex sequence with many operands */
    __m512i temp1 = _mm512_maskz_add_epi32(mask1, a, b);
    __m512i temp2 = _mm512_maskz_add_epi32(mask2, c, d);
    __m512i temp3 = _mm512_mask_add_epi32(e, mask3, f, g);
    
    /* More operations using all parameters */
    __m512i temp4 = _mm512_add_epi32(h, i);
    __m512i temp5 = _mm512_add_epi32(j, k);
    
    /* Combine everything */
    __m512i combined = _mm512_add_epi32(temp1, temp2);
    combined = _mm512_add_epi32(combined, temp3);
    combined = _mm512_add_epi32(combined, temp4);
    combined = _mm512_add_epi32(combined, temp5);
    
    /* Reduce to scalar */
    return _mm512_reduce_add_epi32(combined);
}
#elif defined(__ARM_FEATURE_SVE)
int func_11_operands(svint32_t a, svint32_t b, svint32_t c, svint32_t d,
                     svint32_t e, svint32_t f, svint32_t g, svint32_t h,
                     svint32_t i, svint32_t j, svint32_t k) {
    /* Multiple predicates */
    svbool_t pg_all = svptrue_b32();
    svbool_t pg_even = svptrue_pat_b32(SV_VL2);
    svbool_t pg_odd = svptrue_pat_b32(SV_VL3);
    
    /* Complex SVE operation chain */
    svint32_t temp1 = svadd_x(pg_all, a, b);
    svint32_t temp2 = svadd_x(pg_even, c, d);
    svint32_t temp3 = svadd_x(pg_odd, e, f);
    svint32_t temp4 = svadd_x(pg_all, g, h);
    svint32_t temp5 = svadd_x(pg_all, i, j);
    
    /* Use all parameters including k */
    svint32_t combined = svadd_x(pg_all, temp1, temp2);
    combined = svadd_x(pg_all, combined, temp3);
    combined = svadd_x(pg_all, combined, temp4);
    combined = svadd_x(pg_all, combined, temp5);
    combined = svadd_x(pg_all, combined, k);
    
    return svlastb(pg_all, combined);
}
#else
/* Generic fallback with 11 operands */
int func_11_operands(int a, int b, int c, int d, int e,
                     int f, int g, int h, int i, int j, int k) {
    int result;
    
    /* Inline assembly with 11 input operands */
    __asm__ volatile (
        "/* 11-operand dummy operation */\n\t"
        "add %[a], %[b]\n\t"
        "add %[c], %[d]\n\t"
        "add %[e], %[f]\n\t"
        "add %[g], %[h]\n\t"
        "add %[i], %[j]\n\t"
        "add %0, %[k]\n\t"
        "mov %[res], %[a]"
        : [res] "=r" (result)
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
    int vals[20];
    
    /* Use argv for some variability, PRNG for the rest */
    for (int idx = 0; idx < 20; idx++) {
        if (idx < argc && idx < 10) {
            vals[idx] = (argv[idx][0] * 31 + idx) & 0xFF;
        } else {
            vals[idx] = (int)(prng_next() & 0xFF);
        }
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
    
    /* Use the result */
    printf("Result: %d\n", final_result);
    
    return final_result != 0 ? 0 : 1;
}
