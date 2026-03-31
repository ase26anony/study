/* Test program for GCC optabs.cc 10/11 operand expansion coverage */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Simple PRNG to generate non-constant values */
static uint64_t prng_state = 123456789;

static uint64_t simple_rand(void) {
    prng_state = prng_state * 1103515245 + 12345;
    return prng_state;
}

/* Function for 10 operands */
#ifdef __AVX512F__
#include <immintrin.h>
int func_10_operands(__m512i a, __m512i b, __m512i c, __m512i d, 
                     __m512i e, __m512i f, __m512i g, __m512i h,
                     __mmask16 mask, void* ptr) {
    /* AVX-512 masked store with many operands */
    _mm512_mask_storeu_epi32(ptr, mask, a);
    
    /* Use all operands in computation to prevent optimization */
    __m512i t1 = _mm512_add_epi32(a, b);
    __m512i t2 = _mm512_add_epi32(c, d);
    __m512i t3 = _mm512_add_epi32(e, f);
    __m512i t4 = _mm512_add_epi32(g, h);
    
    __m512i sum = _mm512_add_epi32(t1, t2);
    sum = _mm512_add_epi32(sum, t3);
    sum = _mm512_add_epi32(sum, t4);
    
    /* Extract and return a scalar result */
    return _mm512_reduce_add_epi32(sum);
}
#elif defined(__ARM_FEATURE_SVE)
#include <arm_sve.h>
int func_10_operands(svint32_t a, svint32_t b, svint32_t c, svint32_t d,
                     svint32_t e, svint32_t f, svint32_t g, svint32_t h,
                     svbool_t mask, int32_t* ptr) {
    /* SVE scatter store with predicate */
    svint32_t indices = svindex_s32(0, 1);
    svst1_scatter_s32index_s32(mask, ptr, indices, a);
    
    /* Use all operands */
    svint32_t t1 = svadd_s32_z(mask, a, b);
    svint32_t t2 = svadd_s32_z(mask, c, d);
    svint32_t t3 = svadd_s32_z(mask, e, f);
    svint32_t t4 = svadd_s32_z(mask, g, h);
    
    svint32_t sum = svadd_s32_z(mask, t1, t2);
    sum = svadd_s32_z(mask, sum, t3);
    sum = svadd_s32_z(mask, sum, t4);
    
    /* Horizontal reduction */
    return svaddv_s32(mask, sum);
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
                     __m512i i, __mmask16 mask, void* ptr) {
    /* Complex AVX-512 operation using all 11 parameters */
    __m512i blended = _mm512_mask_blend_epi32(mask, a, b);
    _mm512_mask_storeu_epi32(ptr, mask, blended);
    
    /* Chain computations with all operands */
    __m512i t1 = _mm512_add_epi32(c, d);
    __m512i t2 = _mm512_add_epi32(e, f);
    __m512i t3 = _mm512_add_epi32(g, h);
    __m512i t4 = _mm512_add_epi32(i, blended);
    
    __m512i sum = _mm512_add_epi32(t1, t2);
    sum = _mm512_add_epi32(sum, t3);
    sum = _mm512_add_epi32(sum, t4);
    
    return _mm512_reduce_add_epi32(sum);
}
#elif defined(__ARM_FEATURE_SVE)
int func_11_operands(svint32_t a, svint32_t b, svint32_t c, svint32_t d,
                     svint32_t e, svint32_t f, svint32_t g, svint32_t h,
                     svint32_t i, svbool_t mask, int32_t* ptr) {
    /* SVE operation with 11 operands */
    svint32_t blended = svsel_s32(mask, a, b);
    svint32_t indices = svindex_s32(0, 1);
    svst1_scatter_s32index_s32(mask, ptr, indices, blended);
    
    /* Use all operands */
    svint32_t t1 = svadd_s32_z(mask, c, d);
    svint32_t t2 = svadd_s32_z(mask, e, f);
    svint32_t t3 = svadd_s32_z(mask, g, h);
    svint32_t t4 = svadd_s32_z(mask, i, blended);
    
    svint32_t sum = svadd_s32_z(mask, t1, t2);
    sum = svadd_s32_z(mask, sum, t3);
    sum = svadd_s32_z(mask, sum, t4);
    
    return svaddv_s32(mask, sum);
}
#else
/* Generic fallback with 11 operands */
int func_11_operands(int a, int b, int c, int d, int e,
                     int f, int g, int h, int i, int j, int k) {
    int result;
    
    /* Extended inline assembly with 11 input operands */
    __asm__ volatile (
        "/* 11-operand dummy operation */\n\t"
        "add %[a], %[b]\n\t"
        "add %[c], %[d]\n\t"
        "add %[e], %[f]\n\t"
        "add %[g], %[h]\n\t"
        "add %[i], %[j]\n\t"
        "add %[k], %[a]\n\t"
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
    
    /* Use argv indices and PRNG for variability */
    for (int idx = 0; idx < 20; idx++) {
        if (argc > idx + 1) {
            vals[idx] = atoi(argv[idx + 1]);
        } else {
            vals[idx] = (int)(simple_rand() % 1000);
        }
    }
    
    /* Call both functions with appropriate number of arguments */
    int result1 = func_10_operands(vals[0], vals[1], vals[2], vals[3],
                                   vals[4], vals[5], vals[6], vals[7],
                                   vals[8], vals[9]);
    
    int result2 = func_11_operands(vals[10], vals[11], vals[12], vals[13],
                                   vals[14], vals[15], vals[16], vals[17],
                                   vals[18], vals[19], vals[0]);
    
    /* Combine results to prevent dead code elimination */
    int final_result = result1 + result2;
    
    printf("Result: %d\n", final_result);
    
    return final_result != 0 ? 0 : 1;
}
