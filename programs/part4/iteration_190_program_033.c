/* Test program for GCC optabs.cc 10/11 operand expansion coverage */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Helper to generate pseudo-random values from argv */
static uint64_t mix(uint64_t x) {
    x ^= x >> 33;
    x *= 0xff51afd7ed558ccdULL;
    x ^= x >> 33;
    x *= 0xc4ceb9fe1a85ec53ULL;
    x ^= x >> 33;
    return x;
}

/* Function for 10 operands */
#ifdef __AVX512F__
#include <immintrin.h>
int func_10_operands(__m512i a, __m512i b, __m512i c, __m512i d,
                     __m512i e, __m512i f, __m512i g, __m512i h,
                     __mmask16 mask, void* addr) {
    /* AVX-512 masked store with many operands */
    _mm512_mask_storeu_epi32(addr, mask, _mm512_add_epi32(a, b));
    
    /* Complex chain of operations to use all operands */
    __m512i t1 = _mm512_add_epi32(c, d);
    __m512i t2 = _mm512_sub_epi32(e, f);
    __m512i t3 = _mm512_mullo_epi32(g, h);
    __m512i result = _mm512_add_epi32(t1, _mm512_add_epi32(t2, t3));
    
    /* Reduce to scalar */
    return _mm512_reduce_add_epi32(result);
}
#elif defined(__ARM_FEATURE_SVE)
#include <arm_sve.h>
int func_10_operands(svint32_t a, svint32_t b, svint32_t c, svint32_t d,
                     svint32_t e, svint32_t f, svint32_t g, svint32_t h,
                     svbool_t mask, int64_t* addr) {
    /* SVE scatter store with multiple operands */
    svint64_t indices = svdup_s64(0);
    svst1_scatter_s64index_s32(mask, addr, indices, a);
    
    /* Use all vector operands */
    svint32_t t1 = svadd_s32_z(mask, a, b);
    svint32_t t2 = svsub_s32_z(mask, c, d);
    svint32_t t3 = svmul_s32_z(mask, e, f);
    svint32_t t4 = svadd_s32_z(mask, g, h);
    svint32_t result = svadd_s32_z(mask, t1, svadd_s32_z(mask, t2, 
                                  svadd_s32_z(mask, t3, t4)));
    
    /* Reduce to scalar */
    return svaddv_s32(mask, result);
}
#else
/* Generic fallback using inline assembly with 10 operands */
int func_10_operands(int a, int b, int c, int d, int e,
                     int f, int g, int h, int i, int j) {
    int result1, result2, result3;
    
    /* Extended inline assembly with 10 explicit operands */
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
        "add %[r1], %[r1], %[r3]"
        : [r1] "=r" (result1), [r2] "=r" (result2), [r3] "=r" (result3)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j)
        : "cc"
    );
    
    return result1 + result2 + result3;
}
#endif

/* Function for 11 operands */
#ifdef __AVX512F__
int func_11_operands(__m512i a, __m512i b, __m512i c, __m512i d,
                     __m512i e, __m512i f, __m512i g, __m512i h,
                     __m512i i, __mmask16 mask, void* addr) {
    /* Complex AVX-512 operation using all 11 operands */
    __m512i t1 = _mm512_add_epi32(a, b);
    __m512i t2 = _mm512_sub_epi32(c, d);
    __m512i t3 = _mm512_mullo_epi32(e, f);
    __m512i t4 = _mm512_and_si512(g, h);
    __m512i t5 = _mm512_or_si512(i, t1);
    
    /* Masked blend operation */
    __m512i result = _mm512_mask_blend_epi32(mask, t2, t3);
    result = _mm512_add_epi32(result, t4);
    result = _mm512_add_epi32(result, t5);
    
    /* Store and reduce */
    _mm512_mask_storeu_epi32(addr, mask, result);
    return _mm512_reduce_add_epi32(result);
}
#elif defined(__ARM_FEATURE_SVE)
int func_11_operands(svint32_t a, svint32_t b, svint32_t c, svint32_t d,
                     svint32_t e, svint32_t f, svint32_t g, svint32_t h,
                     svint32_t i, svbool_t mask, int64_t* addr) {
    /* SVE operation with 11 operands */
    svint32_t t1 = svadd_s32_z(mask, a, b);
    svint32_t t2 = svsub_s32_z(mask, c, d);
    svint32_t t3 = svmul_s32_z(mask, e, f);
    svint32_t t4 = svand_s32_z(mask, g, h);
    svint32_t t5 = svorr_s32_z(mask, i, t1);
    
    /* Complex reduction */
    svint32_t result = svadd_s32_z(mask, t2, t3);
    result = svadd_s32_z(mask, result, t4);
    result = svadd_s32_z(mask, result, t5);
    
    /* Scatter store */
    svint64_t indices = svdup_s64(0);
    svst1_scatter_s64index_s32(mask, addr, indices, result);
    
    return svaddv_s32(mask, result);
}
#else
/* Generic fallback with 11 operands */
int func_11_operands(int a, int b, int c, int d, int e,
                     int f, int g, int h, int i, int j, int k) {
    int result1, result2, result3, result4;
    
    /* Extended inline assembly with 11 explicit operands */
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
        "add %[r1], %[r1], %[r4]"
        : [r1] "=r" (result1), [r2] "=r" (result2),
          [r3] "=r" (result3), [r4] "=r" (result4)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j), [k] "r" (k)
        : "cc"
    );
    
    return result1 + result2 + result3 + result4;
}
#endif

int main(int argc, char *argv[]) {
    /* Initialize variables from argv to prevent constant propagation */
    uint64_t seed = 0;
    for (int i = 0; i < argc; i++) {
        seed = mix(seed + (uintptr_t)argv[i]);
    }
    
    /* Generate 11 different values */
    int vals[11];
    for (int i = 0; i < 11; i++) {
        seed = mix(seed + i);
        vals[i] = (int)(seed & 0x7FFFFFFF);
    }
    
    /* Call both functions */
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
    
    return (final_result > 0) ? 0 : 1;
}
