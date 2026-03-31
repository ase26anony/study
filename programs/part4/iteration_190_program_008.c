/* Test program for GCC optabs.cc 10/11 operand expansion coverage */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Simple PRNG to generate non-constant values */
static uint32_t prng_state = 123456789;
static uint32_t prng() {
    prng_state = prng_state * 1103515245 + 12345;
    return prng_state;
}

/* Function using 10 operands */
#ifdef __AVX512F__
#include <immintrin.h>
int func_10_operands(__m512i a, __m512i b, __m512i c, __m512i d,
                     __m512i e, __m512i f, __m512i g, __m512i h,
                     __mmask16 mask, void* addr) {
    /* AVX-512 masked store with many operands */
    _mm512_mask_compressstoreu_epi32(addr, mask, a);
    
    /* Use all operands in computation to prevent optimization */
    __m512i t1 = _mm512_add_epi32(a, b);
    __m512i t2 = _mm512_add_epi32(c, d);
    __m512i t3 = _mm512_add_epi32(e, f);
    __m512i t4 = _mm512_add_epi32(g, h);
    
    __m512i sum = _mm512_add_epi32(_mm512_add_epi32(t1, t2),
                                   _mm512_add_epi32(t3, t4));
    
    /* Extract and return a scalar result */
    return _mm512_reduce_add_epi32(sum) + (int)mask;
}
#elif defined(__ARM_FEATURE_SVE)
#include <arm_sve.h>
int func_10_operands(svint32_t a, svint32_t b, svint32_t c, svint32_t d,
                     svint32_t e, svint32_t f, svint32_t g, svint32_t h,
                     svbool_t mask, uint64_t* addr) {
    /* SVE scatter store with multiple operands */
    svint32_t data = svadd_s32_z(mask, a, b);
    svst1_scatter_u64base_s32(mask, addr, data);
    
    /* Use all operands */
    svint32_t t1 = svadd_s32_z(mask, a, b);
    svint32_t t2 = svadd_s32_z(mask, c, d);
    svint32_t t3 = svadd_s32_z(mask, e, f);
    svint32_t t4 = svadd_s32_z(mask, g, h);
    
    svint32_t sum = svadd_s32_z(mask, svadd_s32_z(mask, t1, t2),
                                svadd_s32_z(mask, t3, t4));
    
    /* Extract result */
    return svaddv_s32(mask, sum) + svcntp_b32(mask, mask);
}
#else
/* Generic fallback using inline assembly with 10 operands */
int func_10_operands(int a, int b, int c, int d, int e,
                     int f, int g, int h, int i, int j) {
    int result;
    
    /* Extended inline assembly with 10 input operands */
    asm volatile (
        /* Dummy operation that uses all 10 inputs */
        "addl %[a], %[b]\n\t"
        "addl %[c], %[d]\n\t"
        "addl %[e], %[f]\n\t"
        "addl %[g], %[h]\n\t"
        "addl %[i], %[j]\n\t"
        "movl %[b], %[result]"
        : [result] "=r" (result)
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
                     __m512i i, __mmask16 mask, void* addr) {
    /* Complex AVX-512 operation using 11 operands */
    __m512i blended = _mm512_mask_blend_epi32(mask, a, b);
    __m512i added = _mm512_add_epi32(blended, c);
    
    /* Use all 11 operands in various combinations */
    __m512i t1 = _mm512_add_epi32(d, e);
    __m512i t2 = _mm512_add_epi32(f, g);
    __m512i t3 = _mm512_add_epi32(h, i);
    
    /* Store using address operand */
    _mm512_storeu_si512(addr, added);
    
    __m512i sum = _mm512_add_epi32(_mm512_add_epi32(t1, t2), t3);
    sum = _mm512_add_epi32(sum, blended);
    
    return _mm512_reduce_add_epi32(sum) + (int)mask;
}
#elif defined(__ARM_FEATURE_SVE)
int func_11_operands(svint32_t a, svint32_t b, svint32_t c, svint32_t d,
                     svint32_t e, svint32_t f, svint32_t g, svint32_t h,
                     svint32_t i, svbool_t mask, uint64_t* addr) {
    /* SVE operation with 11 operands */
    svint32_t sel = svsel_s32(mask, a, b);
    svint32_t added = svadd_s32_z(mask, sel, c);
    
    svst1_s32(mask, (int32_t*)addr, added);
    
    svint32_t t1 = svadd_s32_z(mask, d, e);
    svint32_t t2 = svadd_s32_z(mask, f, g);
    svint32_t t3 = svadd_s32_z(mask, h, i);
    
    svint32_t sum = svadd_s32_z(mask, svadd_s32_z(mask, t1, t2), t3);
    sum = svadd_s32_z(mask, sum, sel);
    
    return svaddv_s32(mask, sum) + svcntp_b32(mask, mask);
}
#else
/* Generic fallback with 11 operands */
int func_11_operands(int a, int b, int c, int d, int e,
                     int f, int g, int h, int i, int j, int k) {
    int result;
    
    /* Extended inline assembly with 11 input operands */
    asm volatile (
        "movl %[a], %%eax\n\t"
        "addl %[b], %%eax\n\t"
        "addl %[c], %%eax\n\t"
        "addl %[d], %%eax\n\t"
        "addl %[e], %%eax\n\t"
        "addl %[f], %%eax\n\t"
        "addl %[g], %%eax\n\t"
        "addl %[h], %%eax\n\t"
        "addl %[i], %%eax\n\t"
        "addl %[j], %%eax\n\t"
        "addl %[k], %%eax\n\t"
        "movl %%eax, %[result]"
        : [result] "=r" (result)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j), [k] "r" (k)
        : "eax", "cc"
    );
    
    return result;
}
#endif

int main(int argc, char *argv[]) {
    /* Initialize variables with non-constant values */
    int vals[20];
    
    /* Use argv indices and PRNG for variability */
    for (int i = 0; i < 20; i++) {
        vals[i] = prng() + (argc > 1 ? argv[1][i % (argc-1)] : 0);
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
    
    /* Print result to ensure computation isn't optimized away */
    printf("Result: %d\n", final_result);
    
    return final_result != 0 ? 0 : 1;
}
