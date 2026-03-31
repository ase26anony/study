/* Test case to cover 10 and 11 operand expansion in optabs.cc */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Simple PRNG to generate non-constant values */
static unsigned int seed = 12345;
static unsigned int prng(void) {
    seed = seed * 1103515245 + 12345;
    return seed;
}

/* Function to use 10 operands */
int func_10_operands(int a, int b, int c, int d, int e,
                     int f, int g, int h, int i, int j) {
    int result = 0;
    
#if defined(__AVX512F__) && defined(__AVX512VL__)
    /* AVX-512 intrinsic that can expand to many operands */
    #include <immintrin.h>
    __m512i v1 = _mm512_set_epi32(j, i, h, g, f, e, d, c, b, a, 0, 0, 0, 0, 0, 0);
    __m512i v2 = _mm512_set_epi32(a, b, c, d, e, f, g, h, i, j, 0, 0, 0, 0, 0, 0);
    __mmask16 mask = 0x00FF;  /* Use lower 8 elements */
    
    /* _mm512_mask_add_epi32 expands to many operands during RTL generation */
    __m512i v3 = _mm512_mask_add_epi32(v1, mask, v1, v2);
    
    /* Extract and sum results to prevent optimization */
    int arr[16];
    _mm512_storeu_si512(arr, v3);
    for (int k = 0; k < 8; k++) {
        result += arr[k];
    }
    
#elif defined(__ARM_FEATURE_SVE)
    /* ARM SVE intrinsic - adjust based on available SVE width */
    #include <arm_sve.h>
    svint32_t vec1 = svdup_s32(a);
    svint32_t vec2 = svdup_s32(b);
    svint32_t vec3 = svdup_s32(c);
    svint32_t vec4 = svdup_s32(d);
    svint32_t vec5 = svdup_s32(e);
    svint32_t vec6 = svdup_s32(f);
    svint32_t vec7 = svdup_s32(g);
    svint32_t vec8 = svdup_s32(h);
    svint32_t vec9 = svdup_s32(i);
    svint32_t vec10 = svdup_s32(j);
    
    /* Complex SVE operation that may expand to many operands */
    svint32_t sum = svadd_s32_z(svptrue_b32(), vec1, vec2);
    sum = svadd_s32_z(svptrue_b32(), sum, vec3);
    sum = svadd_s32_z(svptrue_b32(), sum, vec4);
    sum = svadd_s32_z(svptrue_b32(), sum, vec5);
    sum = svadd_s32_z(svptrue_b32(), sum, vec6);
    sum = svadd_s32_z(svptrue_b32(), sum, vec7);
    sum = svadd_s32_z(svptrue_b32(), sum, vec8);
    sum = svadd_s32_z(svptrue_b32(), sum, vec9);
    sum = svadd_s32_z(svptrue_b32(), sum, vec10);
    
    result = svaddv_s32(svptrue_b32(), sum);
    
#else
    /* Generic inline assembly with 10 operands */
    int temp1, temp2, temp3, temp4, temp5;
    
    /* Extended asm with 10 input/output operands */
    asm volatile (
        "mov %0, %5\n\t"
        "add %0, %0, %6\n\t"
        "add %0, %0, %7\n\t"
        "add %0, %0, %8\n\t"
        "add %0, %0, %9\n\t"
        "mov %1, %0\n\t"
        "add %1, %1, %10\n\t"
        "add %1, %1, %11\n\t"
        "add %1, %1, %12\n\t"
        "add %1, %1, %13\n\t"
        "mov %2, %1\n\t"
        "add %2, %2, %14\n\t"
        : "=&r" (temp1), "=&r" (temp2), "=&r" (temp3),
          "=&r" (temp4), "=&r" (temp5)
        : "r" (a), "r" (b), "r" (c), "r" (d), "r" (e),
          "r" (f), "r" (g), "r" (h), "r" (i), "r" (j)
        : "cc"
    );
    
    result = temp1 + temp2 + temp3 + temp4 + temp5;
#endif
    
    return result;
}

/* Function to use 11 operands */
int func_11_operands(int a, int b, int c, int d, int e, int f,
                     int g, int h, int i, int j, int k) {
    int result = 0;
    
#if defined(__AVX512F__) && defined(__AVX512VL__)
    /* AVX-512 intrinsic with mask operations */
    #include <immintrin.h>
    __m512i v1 = _mm512_set_epi32(k, j, i, h, g, f, e, d, c, b, a, 0, 0, 0, 0, 0);
    __m512i v2 = _mm512_set_epi32(a, b, c, d, e, f, g, h, i, j, k, 0, 0, 0, 0, 0);
    __mmask16 mask = 0x07FF;  /* Use lower 11 elements */
    
    /* Complex masked operation */
    __m512i v3 = _mm512_mask_mullo_epi32(v1, mask, v1, v2);
    
    /* Extract and process results */
    int arr[16];
    _mm512_storeu_si512(arr, v3);
    for (int idx = 0; idx < 11; idx++) {
        result += arr[idx];
    }
    
#elif defined(__ARM_FEATURE_SVE)
    /* ARM SVE with predicate operations */
    #include <arm_sve.h>
    svint32_t vec1 = svdup_s32(a);
    svint32_t vec2 = svdup_s32(b);
    svint32_t vec3 = svdup_s32(c);
    svint32_t vec4 = svdup_s32(d);
    svint32_t vec5 = svdup_s32(e);
    svint32_t vec6 = svdup_s32(f);
    svint32_t vec7 = svdup_s32(g);
    svint32_t vec8 = svdup_s32(h);
    svint32_t vec9 = svdup_s32(i);
    svint32_t vec10 = svdup_s32(j);
    svint32_t vec11 = svdup_s32(k);
    
    /* Create predicate for first 11 lanes (assuming SVE vector length > 11) */
    svbool_t pg = svwhilelt_b32(0, 11);
    
    /* Multiple operations under predicate control */
    svint32_t prod = svmul_s32_z(pg, vec1, vec2);
    prod = svmla_s32_z(pg, prod, vec3, vec4);
    prod = svmla_s32_z(pg, prod, vec5, vec6);
    prod = svmla_s32_z(pg, prod, vec7, vec8);
    prod = svmla_s32_z(pg, prod, vec9, vec10);
    prod = svadd_s32_z(pg, prod, vec11);
    
    result = svaddv_s32(pg, prod);
    
#else
    /* Generic inline assembly with 11 operands */
    int temp1, temp2, temp3, temp4, temp5, temp6;
    
    asm volatile (
        "mov %0, %6\n\t"
        "imul %0, %0, %7\n\t"
        "add %0, %0, %8\n\t"
        "add %0, %0, %9\n\t"
        "add %0, %0, %10\n\t"
        "mov %1, %0\n\t"
        "add %1, %1, %11\n\t"
        "add %1, %1, %12\n\t"
        "add %1, %1, %13\n\t"
        "add %1, %1, %14\n\t"
        "mov %2, %1\n\t"
        "add %2, %2, %15\n\t"
        "mov %3, %2\n\t"
        "add %3, %3, %16\n\t"
        : "=&r" (temp1), "=&r" (temp2), "=&r" (temp3),
          "=&r" (temp4), "=&r" (temp5), "=&r" (temp6)
        : "r" (a), "r" (b), "r" (c), "r" (d), "r" (e),
          "r" (f), "r" (g), "r" (h), "r" (i), "r" (j), "r" (k)
        : "cc"
    );
    
    result = temp1 + temp2 + temp3 + temp4 + temp5 + temp6;
#endif
    
    return result;
}

int main(int argc, char *argv[]) {
    /* Generate 11 non-constant values */
    int vals[11];
    
    /* Use argv for some values, PRNG for others to ensure variability */
    for (int i = 0; i < 11; i++) {
        if (i < argc - 1 && i < 11) {
            vals[i] = atoi(argv[i + 1]);
        } else {
            vals[i] = prng() % 100;
        }
        /* Ensure non-zero to avoid optimization */
        if (vals[i] == 0) vals[i] = 1;
    }
    
    /* Call both functions with overlapping but different operand counts */
    int result1 = func_10_operands(vals[0], vals[1], vals[2], vals[3], vals[4],
                                   vals[5], vals[6], vals[7], vals[8], vals[9]);
    
    int result2 = func_11_operands(vals[0], vals[1], vals[2], vals[3], vals[4],
                                   vals[5], vals[6], vals[7], vals[8], vals[9], vals[10]);
    
    /* Combine results in a non-trivial way to prevent dead code elimination */
    int final_result = result1 * 3 + result2 * 7;
    
    printf("Result: %d\n", final_result);
    
    /* Use result to affect return code (prevents optimization) */
    return (final_result > 0) ? 0 : 1;
}
