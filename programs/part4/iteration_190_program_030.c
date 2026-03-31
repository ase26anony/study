/* Test program to cover 10/11 operand expansion in optabs.cc */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Simple PRNG to prevent constant propagation */
static unsigned int seed = 12345;
static unsigned int prng(void) {
    seed = seed * 1103515245 + 12345;
    return seed;
}

/* Function using 10 operands */
int func_10_operands(int a, int b, int c, int d, int e,
                     int f, int g, int h, int i, int j) {
    int result = 0;
    
#if defined(__AVX512F__) && defined(__AVX512VL__)
    /* AVX-512 intrinsic with many operands */
    #include <immintrin.h>
    __m512i v1 = _mm512_set_epi32(a, b, c, d, e, f, g, h, i, j, 0, 0, 0, 0, 0, 0);
    __m512i v2 = _mm512_set_epi32(j, i, h, g, f, e, d, c, b, a, 0, 0, 0, 0, 0, 0);
    __mmask16 mask = 0x00FF;
    
    /* _mm512_mask_compressstoreu_epi32 expands to many operands */
    volatile char buffer[64] __attribute__((aligned(64)));
    _mm512_mask_compressstoreu_epi32(buffer, mask, v1);
    
    /* Use result to prevent optimization */
    __m512i loaded = _mm512_load_epi32(buffer);
    result = _mm512_reduce_add_epi32(loaded);
    
#elif defined(__ARM_FEATURE_SVE)
    /* ARM SVE intrinsic - may expand to many operands */
    #include <arm_sve.h>
    svbool_t pg = svwhilelt_b32(0, 16);
    svint32_t data = svdup_s32(a);
    
    /* Complex SVE operation with multiple operands */
    svint32_t result_vec = svmla_s32_z(pg, data, svdup_s32(b), svdup_s32(c));
    result = svaddv_s32(pg, result_vec);
    
#else
    /* Generic inline assembly with 10 operands */
    int temp1, temp2, temp3, temp4, temp5;
    
    /* Extended asm with 10 operands (5 inputs, 5 outputs) */
    asm volatile (
        "/* 10-operand dummy operation */\n\t"
        "mov %0, %5\n\t"
        "add %0, %0, %6\n\t"
        "mov %1, %7\n\t"
        "sub %1, %1, %8\n\t"
        "mov %2, %9\n\t"
        "and %2, %2, %10\n\t"
        "mov %3, %5\n\t"
        "or %3, %3, %7\n\t"
        "mov %4, %6\n\t"
        "xor %4, %4, %8"
        : "=r"(temp1), "=r"(temp2), "=r"(temp3), "=r"(temp4), "=r"(temp5)
        : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e), "r"(f)
        : "cc"
    );
    
    result = temp1 + temp2 + temp3 + temp4 + temp5 + g + h + i + j;
#endif
    
    return result;
}

/* Function using 11 operands */
int func_11_operands(int a, int b, int c, int d, int e, int f,
                     int g, int h, int i, int j, int k) {
    int result = 0;
    
#if defined(__AVX512F__) && defined(__AVX512BW__)
    /* Another AVX-512 intrinsic that may use many operands */
    #include <immintrin.h>
    __m512i v1 = _mm512_set_epi32(a, b, c, d, e, f, g, h, i, j, k, 0, 0, 0, 0, 0);
    __m512i v2 = _mm512_set_epi32(k, j, i, h, g, f, e, d, c, b, a, 0, 0, 0, 0, 0);
    __mmask32 mask = 0x0000FFFF;
    
    /* Complex permute operation */
    __m512i permuted = _mm512_mask_permutexvar_epi16(v1, mask, v2, v1);
    
    /* Reduce to scalar */
    result = _mm512_reduce_add_epi32(permuted);
    
#elif defined(__ARM_FEATURE_SVE)
    /* ARM SVE with 11 operands worth of parameters */
    #include <arm_sve.h>
    svbool_t pg = svwhilelt_b32(0, 16);
    svint32_t vals[11];
    
    /* Create multiple vector values */
    vals[0] = svdup_s32(a);
    vals[1] = svdup_s32(b);
    vals[2] = svdup_s32(c);
    vals[3] = svdup_s32(d);
    vals[4] = svdup_s32(e);
    vals[5] = svdup_s32(f);
    vals[6] = svdup_s32(g);
    vals[7] = svdup_s32(h);
    vals[8] = svdup_s32(i);
    vals[9] = svdup_s32(j);
    vals[10] = svdup_s32(k);
    
    /* Chain operations to use all values */
    svint32_t accum = svadd_z(pg, vals[0], vals[1]);
    for (int idx = 2; idx < 11; idx++) {
        accum = svmla_z(pg, accum, vals[idx], svdup_s32(idx));
    }
    
    result = svaddv_s32(pg, accum);
    
#else
    /* Generic inline assembly with 11 operands */
    int temp1, temp2, temp3, temp4, temp5, temp6;
    
    /* Extended asm with 11 operands (6 outputs, 5 inputs) */
    asm volatile (
        "/* 11-operand dummy operation */\n\t"
        "mov %0, %6\n\t"
        "add %0, %0, %7\n\t"
        "mov %1, %8\n\t"
        "sub %1, %1, %9\n\t"
        "mov %2, %10\n\t"
        "and %2, %2, %11\n\t"
        "mov %3, %6\n\t"
        "or %3, %3, %8\n\t"
        "mov %4, %7\n\t"
        "xor %4, %4, %9\n\t"
        "mov %5, %10\n\t"
        "not %5"
        : "=r"(temp1), "=r"(temp2), "=r"(temp3), "=r"(temp4), 
          "=r"(temp5), "=r"(temp6)
        : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e), "r"(f)
        : "cc"
    );
    
    result = temp1 + temp2 + temp3 + temp4 + temp5 + temp6 + g + h + i + j + k;
#endif
    
    return result;
}

int main(int argc, char *argv[]) {
    /* Initialize 11 variables with non-constant values */
    int vars[11];
    
    /* Use argv for some variability, PRNG for the rest */
    for (int i = 0; i < 11; i++) {
        if (i < argc && argv[i] != NULL) {
            vars[i] = argv[i][0] + i;  /* Simple hash from argv */
        } else {
            vars[i] = prng() % 1000;
        }
    }
    
    /* Call both functions to trigger different expansion paths */
    int result1 = func_10_operands(vars[0], vars[1], vars[2], vars[3], vars[4],
                                   vars[5], vars[6], vars[7], vars[8], vars[9]);
    
    int result2 = func_11_operands(vars[0], vars[1], vars[2], vars[3], vars[4],
                                   vars[5], vars[6], vars[7], vars[8], vars[9], vars[10]);
    
    /* Combine results to prevent dead code elimination */
    int final_result = result1 + result2;
    
    printf("Result: %d\n", final_result);
    
    /* Additional test with different values */
    for (int i = 0; i < 11; i++) {
        vars[i] = prng() % 1000;
    }
    
    result1 = func_10_operands(vars[1], vars[2], vars[3], vars[4], vars[5],
                               vars[6], vars[7], vars[8], vars[9], vars[10]);
    
    result2 = func_11_operands(vars[10], vars[9], vars[8], vars[7], vars[6],
                               vars[5], vars[4], vars[3], vars[2], vars[1], vars[0]);
    
    final_result += result1 * 3 + result2;
    
    printf("Final result: %d\n", final_result);
    
    return final_result != 0 ? 0 : 1;
}
