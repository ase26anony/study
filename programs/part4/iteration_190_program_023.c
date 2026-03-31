/* Test program for GCC optabs.cc 10/11-operand expansion coverage */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Simple PRNG to generate non-constant values */
static uint64_t simple_rand(uint64_t seed) {
    return seed * 6364136223846793005ULL + 1442695040888963407ULL;
}

/* Function for 10-operand expansion */
#ifdef __AVX512F__
#include <immintrin.h>
int func_10_operands(__m512i a, __m512i b, __m512i c, __m512i d,
                     __m512i e, __m512i f, __m512i g, __m512i h,
                     __m512i i, __m512i j) {
    /* AVX-512 mask register operation with many operands */
    __mmask16 mask = 0xAAAA;
    __m512i result = _mm512_mask_add_epi32(a, mask, b, c);
    result = _mm512_mask_add_epi32(result, mask, d, e);
    result = _mm512_mask_add_epi32(result, mask, f, g);
    result = _mm512_mask_add_epi32(result, mask, h, i);
    result = _mm512_mask_add_epi32(result, mask, result, j);
    
    /* Force use of result */
    int sum = 0;
    int* res_ptr = (int*)&result;
    for (int k = 0; k < 16; k++) {
        sum += res_ptr[k];
    }
    return sum;
}
#elif defined(__ARM_FEATURE_SVE)
#include <arm_sve.h>
int func_10_operands(svint32_t a, svint32_t b, svint32_t c, svint32_t d,
                     svint32_t e, svint32_t f, svint32_t g, svint32_t h,
                     svint32_t i, svint32_t j) {
    /* SVE predicate with multiple operations */
    svbool_t pg = svptrue_b32();
    svint32_t result = svadd_m(pg, a, b);
    result = svadd_m(pg, result, c);
    result = svadd_m(pg, result, d);
    result = svadd_m(pg, result, e);
    result = svadd_m(pg, result, f);
    result = svadd_m(pg, result, g);
    result = svadd_m(pg, result, h);
    result = svadd_m(pg, result, i);
    result = svadd_m(pg, result, j);
    
    /* Extract and sum elements */
    int32_t temp[16];
    svst1(pg, temp, result);
    int sum = 0;
    for (int k = 0; k < 16; k++) {
        sum += temp[k];
    }
    return sum;
}
#else
/* Generic fallback using inline assembly with 10 operands */
int func_10_operands(int a, int b, int c, int d, int e,
                     int f, int g, int h, int i, int j) {
    int result1, result2, result3;
    
    /* Extended asm with 10 input operands and 3 outputs */
    asm volatile (
        "mov %0, %3\n\t"
        "add %0, %0, %4\n\t"
        "add %0, %0, %5\n\t"
        "mov %1, %6\n\t"
        "add %1, %1, %7\n\t"
        "add %1, %1, %8\n\t"
        "mov %2, %9\n\t"
        "add %2, %2, %10\n\t"
        "add %0, %0, %1\n\t"
        "add %0, %0, %2"
        : "=r"(result1), "=r"(result2), "=r"(result3)
        : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e),
          "r"(f), "r"(g), "r"(h), "r"(i), "r"(j)
        : "cc"
    );
    
    return result1 + result2 + result3;
}
#endif

/* Function for 11-operand expansion */
#ifdef __AVX512F__
int func_11_operands(__m512i a, __m512i b, __m512i c, __m512i d,
                     __m512i e, __m512i f, __m512i g, __m512i h,
                     __m512i i, __m512i j, __m512i k) {
    /* Complex AVX-512 operation chain with 11 operands */
    __mmask16 mask1 = 0x5555;
    __mmask16 mask2 = 0xAAAA;
    __m512i temp1 = _mm512_mask_add_epi32(a, mask1, b, c);
    __m512i temp2 = _mm512_mask_sub_epi32(d, mask2, e, f);
    __m512i temp3 = _mm512_mask_mul_epi32(g, mask1, h, i);
    
    /* Combine with remaining operands */
    __m512i result = _mm512_add_epi32(temp1, temp2);
    result = _mm512_add_epi32(result, temp3);
    result = _mm512_add_epi32(result, j);
    result = _mm512_add_epi32(result, k);
    
    /* Force use of result */
    int sum = 0;
    int* res_ptr = (int*)&result;
    for (int idx = 0; idx < 16; idx++) {
        sum += res_ptr[idx];
    }
    return sum;
}
#elif defined(__ARM_FEATURE_SVE)
int func_11_operands(svint32_t a, svint32_t b, svint32_t c, svint32_t d,
                     svint32_t e, svint32_t f, svint32_t g, svint32_t h,
                     svint32_t i, svint32_t j, svint32_t k) {
    /* SVE operation with 11 vector operands */
    svbool_t pg = svptrue_b32();
    svint32_t temp1 = svadd_m(pg, a, b);
    svint32_t temp2 = svsub_m(pg, c, d);
    svint32_t temp3 = svmul_m(pg, e, f);
    
    svint32_t result = svadd_m(pg, temp1, temp2);
    result = svadd_m(pg, result, temp3);
    result = svadd_m(pg, result, g);
    result = svadd_m(pg, result, h);
    result = svadd_m(pg, result, i);
    result = svadd_m(pg, result, j);
    result = svadd_m(pg, result, k);
    
    /* Extract and sum */
    int32_t temp[16];
    svst1(pg, temp, result);
    int sum = 0;
    for (int idx = 0; idx < 16; idx++) {
        sum += temp[idx];
    }
    return sum;
}
#else
/* Generic fallback with 11 operands using inline assembly */
int func_11_operands(int a, int b, int c, int d, int e, int f,
                     int g, int h, int i, int j, int k) {
    int r1, r2, r3, r4;
    
    /* Extended asm with 11 input operands */
    asm volatile (
        "mov %0, %4\n\t"
        "add %0, %0, %5\n\t"
        "mov %1, %6\n\t"
        "add %1, %1, %7\n\t"
        "mov %2, %8\n\t"
        "add %2, %2, %9\n\t"
        "mov %3, %10\n\t"
        "add %3, %3, %11\n\t"
        "add %0, %0, %1\n\t"
        "add %2, %2, %3\n\t"
        "add %0, %0, %2\n\t"
        "add %0, %0, %12\n\t"
        "add %0, %0, %13"
        : "=r"(r1), "=r"(r2), "=r"(r3), "=r"(r4)
        : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e),
          "r"(f), "r"(g), "r"(h), "r"(i), "r"(j), "r"(k)
        : "cc"
    );
    
    return r1 + r2 + r3 + r4;
}
#endif

int main(int argc, char *argv[]) {
    uint64_t seed = 42;
    int result = 0;
    
    /* Generate non-constant values to prevent optimization */
    int vals[20];
    for (int i = 0; i < 20; i++) {
        seed = simple_rand(seed);
        vals[i] = (int)(seed ^ (seed >> 32)) & 0xFFF;
        if (argc > 1) {
            vals[i] ^= argv[1][0]; /* Mix in argv to prevent constant folding */
        }
    }
    
    /* Test 10-operand path */
    result += func_10_operands(vals[0], vals[1], vals[2], vals[3], vals[4],
                               vals[5], vals[6], vals[7], vals[8], vals[9]);
    
    /* Test 11-operand path */
    result += func_11_operands(vals[10], vals[11], vals[12], vals[13], vals[14],
                               vals[15], vals[16], vals[17], vals[18], vals[19],
                               vals[0]); /* Reuse first value for 11th operand */
    
    printf("Result: %d\n", result);
    return result != 0 ? 0 : 1;
}
