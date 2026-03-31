/* Test case to cover 10 and 11 operand expansion in optabs.cc */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Simple PRNG to generate non-constant values */
static uint64_t simple_rand(uint64_t seed) {
    return seed * 1103515245 + 12345;
}

/* Function to trigger 10-operand expansion */
int func_10_operands(uint64_t a0, uint64_t a1, uint64_t a2, uint64_t a3,
                     uint64_t a4, uint64_t a5, uint64_t a6, uint64_t a7,
                     uint64_t a8, uint64_t a9) {
    int result = 0;
    
#if defined(__AVX512F__) && defined(__AVX512VL__)
    /* AVX-512 intrinsic that may expand to many operands */
    #include <immintrin.h>
    __m512i v0 = _mm512_set_epi64(a9, a8, a7, a6, a5, a4, a3, a2);
    __m512i v1 = _mm512_set_epi64(a1, a0, a9, a8, a7, a6, a5, a4);
    __mmask8 mask = (__mmask8)((a0 ^ a1 ^ a2 ^ a3) & 0xFF);
    
    /* Complex AVX-512 operation with many operands */
    __m512i res = _mm512_mask_add_epi64(v0, mask, v0, v1);
    
    /* Extract and combine results */
    uint64_t temp[8];
    _mm512_storeu_si512(temp, res);
    for (int i = 0; i < 8; i++) {
        result += (int)(temp[i] & 0xFFFFFFFF);
    }
    
#elif defined(__ARM_FEATURE_SVE)
    /* ARM SVE intrinsic - may require many operands during expansion */
    #include <arm_sve.h>
    svuint64_t sv0 = svdup_u64(a0);
    svuint64_t sv1 = svdup_u64(a1);
    svuint64_t sv2 = svdup_u64(a2);
    svuint64_t sv3 = svdup_u64(a3);
    svuint64_t sv4 = svdup_u64(a4);
    svuint64_t sv5 = svdup_u64(a5);
    svuint64_t sv6 = svdup_u64(a6);
    svuint64_t sv7 = svdup_u64(a7);
    svuint64_t sv8 = svdup_u64(a8);
    svuint64_t sv9 = svdup_u64(a9);
    
    /* Complex SVE operation chain */
    svuint64_t sum0 = svadd_u64_x(svptrue_b64(), sv0, sv1);
    svuint64_t sum1 = svadd_u64_x(svptrue_b64(), sv2, sv3);
    svuint64_t sum2 = svadd_u64_x(svptrue_b64(), sv4, sv5);
    svuint64_t sum3 = svadd_u64_x(svptrue_b64(), sv6, sv7);
    svuint64_t sum4 = svadd_u64_x(svptrue_b64(), sv8, sv9);
    
    svuint64_t final = svadd_u64_x(svptrue_b64(), 
                      svadd_u64_x(svptrue_b64(), sum0, sum1),
                      svadd_u64_x(svptrue_b64(), sum2,
                      svadd_u64_x(svptrue_b64(), sum3, sum4)));
    
    /* Extract result */
    uint64_t temp;
    svst1_u64(svptrue_b64(), &temp, final);
    result = (int)(temp & 0xFFFFFFFF);
    
#else
    /* Fallback: Extended inline assembly with 10 operands */
    uint64_t out0, out1, out2, out3, out4;
    
    __asm__ volatile (
        /* Complex multi-operand operation */
        "mov %0, %5\n\t"
        "add %0, %0, %6\n\t"
        "mov %1, %7\n\t"
        "add %1, %1, %8\n\t"
        "mov %2, %9\n\t"
        "add %2, %2, %10\n\t"
        "mov %3, %11\n\t"
        "add %3, %3, %12\n\t"
        "mov %4, %13\n\t"
        "add %4, %4, %14\n\t"
        : "=r"(out0), "=r"(out1), "=r"(out2), "=r"(out3), "=r"(out4)
        : "r"(a0), "r"(a1), "r"(a2), "r"(a3), "r"(a4), 
          "r"(a5), "r"(a6), "r"(a7), "r"(a8), "r"(a9)
        : "cc"
    );
    
    result = (int)((out0 + out1 + out2 + out3 + out4) & 0xFFFFFFFF);
#endif
    
    return result;
}

/* Function to trigger 11-operand expansion */
int func_11_operands(uint64_t a0, uint64_t a1, uint64_t a2, uint64_t a3,
                     uint64_t a4, uint64_t a5, uint64_t a6, uint64_t a7,
                     uint64_t a8, uint64_t a9, uint64_t a10) {
    int result = 0;
    
#if defined(__AVX512F__) && defined(__AVX512BW__) && defined(__AVX512VL__)
    /* Another AVX-512 intrinsic that may use many operands */
    #include <immintrin.h>
    __m512i v0 = _mm512_set_epi64(a10, a9, a8, a7, a6, a5, a4, a3);
    __m512i v1 = _mm512_set_epi64(a2, a1, a0, a10, a9, a8, a7, a6);
    __m512i v2 = _mm512_set_epi64(a5, a4, a3, a2, a1, a0, a10, a9);
    __mmask16 mask16 = (__mmask16)((a0 ^ a1 ^ a2 ^ a3 ^ a4) & 0xFFFF);
    
    /* Complex operation with blending */
    __m512i blended = _mm512_mask_blend_epi64(mask16, v0, v1);
    __m512i res = _mm512_add_epi64(blended, v2);
    
    /* Extract and combine results */
    uint64_t temp[8];
    _mm512_storeu_si512(temp, res);
    for (int i = 0; i < 8; i++) {
        result += (int)(temp[i] & 0xFFFFFFFF);
    }
    
#elif defined(__ARM_FEATURE_SVE2)
    /* ARM SVE2 intrinsic with predicate operations */
    #include <arm_sve.h>
    svuint64_t sv0 = svdup_u64(a0);
    svuint64_t sv1 = svdup_u64(a1);
    svuint64_t sv2 = svdup_u64(a2);
    svuint64_t sv3 = svdup_u64(a3);
    svuint64_t sv4 = svdup_u64(a4);
    svuint64_t sv5 = svdup_u64(a5);
    svuint64_t sv6 = svdup_u64(a6);
    svuint64_t sv7 = svdup_u64(a7);
    svuint64_t sv8 = svdup_u64(a8);
    svuint64_t sv9 = svdup_u64(a9);
    svuint64_t sv10 = svdup_u64(a10);
    
    /* Create predicate from comparison */
    svbool_t pg = svcmpgt_u64(svptrue_b64(), sv0, sv5);
    
    /* Predicated operations with many operands */
    svuint64_t sel0 = svsel_u64(pg, sv1, sv2);
    svuint64_t sel1 = svsel_u64(pg, sv3, sv4);
    svuint64_t sel2 = svsel_u64(pg, sv6, sv7);
    svuint64_t sel3 = svsel_u64(pg, sv8, sv9);
    
    svuint64_t sum0 = svadd_u64_x(pg, sel0, sel1);
    svuint64_t sum1 = svadd_u64_x(pg, sel2, sel3);
    svuint64_t sum2 = svadd_u64_x(pg, sum0, sum1);
    svuint64_t final = svadd_u64_x(pg, sum2, sv10);
    
    /* Extract result */
    uint64_t temp;
    svst1_u64(pg, &temp, final);
    result = (int)(temp & 0xFFFFFFFF);
    
#else
    /* Fallback: Extended inline assembly with 11 operands */
    uint64_t out0, out1, out2, out3, out4, out5;
    
    __asm__ volatile (
        /* Complex operation with 11 input operands */
        "mov %0, %6\n\t"
        "add %0, %0, %7\n\t"
        "mov %1, %8\n\t"
        "add %1, %1, %9\n\t"
        "mov %2, %10\n\t"
        "add %2, %2, %11\n\t"
        "mov %3, %12\n\t"
        "add %3, %3, %13\n\t"
        "mov %4, %14\n\t"
        "add %4, %4, %15\n\t"
        "mov %5, %16\n\t"
        "add %5, %5, %6\n\t"  /* Reuse a0 */
        : "=r"(out0), "=r"(out1), "=r"(out2), "=r"(out3), 
          "=r"(out4), "=r"(out5)
        : "r"(a0), "r"(a1), "r"(a2), "r"(a3), "r"(a4), 
          "r"(a5), "r"(a6), "r"(a7), "r"(a8), "r"(a9),
          "r"(a10)
        : "cc"
    );
    
    result = (int)((out0 + out1 + out2 + out3 + out4 + out5) & 0xFFFFFFFF);
#endif
    
    return result;
}

int main(int argc, char *argv[]) {
    /* Initialize 11 variables with non-constant values */
    uint64_t vars[11];
    uint64_t seed = 123456789;
    
    for (int i = 0; i < 11; i++) {
        if (argc > i + 1) {
            vars[i] = strtoull(argv[i + 1], NULL, 0);
        } else {
            seed = simple_rand(seed);
            vars[i] = seed;
        }
    }
    
    /* Call both functions to trigger different expansion paths */
    int res1 = func_10_operands(vars[0], vars[1], vars[2], vars[3],
                                vars[4], vars[5], vars[6], vars[7],
                                vars[8], vars[9]);
    
    int res2 = func_11_operands(vars[0], vars[1], vars[2], vars[3],
                                vars[4], vars[5], vars[6], vars[7],
                                vars[8], vars[9], vars[10]);
    
    /* Combine results to prevent optimization */
    int final_result = res1 + res2;
    printf("Result: %d\n", final_result);
    
    return final_result != 0 ? 0 : 1;
}
