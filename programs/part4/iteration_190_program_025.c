/* test_optabs_coverage.c
 * Generates RTL expansions with 10 and 11 operands to cover optabs.cc lines
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Simple PRNG to generate non-constant values */
static unsigned int seed = 12345;
static unsigned int prng(void) {
    seed = seed * 1103515245 + 12345;
    return seed;
}

/* Function to trigger 10-operand expansion */
int func_10_operands(int a, int b, int c, int d, int e,
                     int f, int g, int h, int i, int j) {
    int result = 0;
    
#if defined(__AVX512F__) && defined(__AVX512VL__)
    /* AVX-512 mask compress store with many operands when expanded */
    #include <immintrin.h>
    __mmask16 mask = (__mmask16)(a & 0xFFFF);
    int data[16] = {a, b, c, d, e, f, g, h, i, j, a, b, c, d, e, f};
    __m512i vec = _mm512_loadu_si512((const __m512i*)data);
    _mm512_mask_compressstoreu_epi32((void*)data, mask, vec);
    result = data[0] + data[15];
    
#elif defined(__ARM_FEATURE_SVE)
    /* ARM SVE scatter store with predicate and multiple operands */
    #include <arm_sve.h>
    svbool_t pg = svwhilelt_b32(0, 16);
    uint64_t bases[16] = {a, b, c, d, e, f, g, h, i, j, a, b, c, d, e, f};
    uint64_t data[16] = {j, i, h, g, f, e, d, c, b, a, j, i, h, g, f, e};
    svuint64_t vec_bases = svld1_u64(pg, bases);
    svuint64_t vec_data = svld1_u64(pg, data);
    svst1_scatter_u64base_u64(pg, vec_bases, vec_data);
    result = (int)bases[0] + (int)data[15];
    
#elif defined(__PPC64__) || defined(__powerpc64__)
    /* PowerPC vector permute with many operands */
    #include <altivec.h>
    vector signed int v1 = {a, b, c, d};
    vector signed int v2 = {e, f, g, h};
    vector signed int v3 = {i, j, a, b};
    vector signed int v4 = {c, d, e, f};
    vector unsigned char perm = {0,1,2,3, 4,5,6,7, 8,9,10,11, 12,13,14,15};
    vector signed int res1 = vec_perm(v1, v2, perm);
    vector signed int res2 = vec_perm(v3, v4, perm);
    result = res1[0] + res2[3];
    
#else
    /* Generic inline assembly with 10 operands */
    int out1, out2;
    asm volatile (
        /* Dummy operation that uses all 10 inputs and produces 2 outputs */
        "add %0, %2, %3\n\t"
        "add %0, %0, %4\n\t"
        "add %0, %0, %5\n\t"
        "add %0, %0, %6\n\t"
        "add %0, %0, %7\n\t"
        "add %0, %0, %8\n\t"
        "add %0, %0, %9\n\t"
        "add %0, %0, %10\n\t"
        "add %0, %0, %11\n\t"
        "mul %1, %2, %3\n\t"
        "add %1, %1, %4\n\t"
        : "=&r" (out1), "=&r" (out2)
        : "r" (a), "r" (b), "r" (c), "r" (d), "r" (e),
          "r" (f), "r" (g), "r" (h), "r" (i), "r" (j)
        : "cc"
    );
    result = out1 + out2;
#endif
    
    return result;
}

/* Function to trigger 11-operand expansion */
int func_11_operands(int a, int b, int c, int d, int e,
                     int f, int g, int h, int i, int j, int k) {
    int result = 0;
    
#if defined(__AVX512F__) && defined(__AVX512VL__) && defined(__AVX512BW__)
    /* AVX-512 masked gather with multiple operands */
    #include <immintrin.h>
    __mmask16 mask = (__mmask16)((a ^ b) & 0xFFFF);
    int base = a;
    long long indices[8] = {b, c, d, e, f, g, h, i};
    int scale = j;
    __m512i vindex = _mm512_loadu_si512((const __m512i*)indices);
    __m512i gathered = _mm512_mask_i64gather_epi32(
        _mm512_setzero_si512(), mask, vindex, 
        (const void*)&base, scale);
    result = _mm512_reduce_add_epi32(gathered) + k;
    
#elif defined(__ARM_FEATURE_SVE)
    /* ARM SVE gather load with predicate and multiple operands */
    #include <arm_sve.h>
    svbool_t pg = svwhilelt_b32(0, 16);
    uint64_t bases[16] = {a, b, c, d, e, f, g, h, i, j, k, a, b, c, d, e};
    svuint64_t vec_bases = svld1_u64(pg, bases);
    svuint64_t gathered = svld1_gather_u64base_u64(pg, vec_bases);
    uint64_t temp[16];
    svst1_u64(pg, temp, gathered);
    result = (int)temp[0] + (int)temp[15];
    
#elif defined(__PPC64__) || defined(__powerpc64__)
    /* PowerPC vector multiply-sum with many operands */
    #include <altivec.h>
    vector signed short v1 = {a, b, c, d, e, f, g, h};
    vector signed short v2 = {i, j, k, a, b, c, d, e};
    vector signed int v3 = {f, g, h, i};
    vector signed int v4 = {j, k, a, b};
    vector signed int res = vec_msum(v1, v2, v3);
    res = vec_add(res, v4);
    result = res[0] + res[1] + res[2] + res[3];
    
#else
    /* Generic inline assembly with 11 operands */
    int out1, out2, out3;
    asm volatile (
        /* Complex dummy operations using all 11 inputs */
        "mov %0, %3\n\t"
        "add %0, %0, %4\n\t"
        "sub %0, %0, %5\n\t"
        "xor %0, %0, %6\n\t"
        "and %0, %0, %7\n\t"
        "or %0, %0, %8\n\t"
        "shl %0, %0, %9\n\t"
        "shr %0, %0, %10\n\t"
        "mov %1, %11\n\t"
        "imul %1, %1, %12\n\t"
        "mov %2, %13\n\t"
        "add %2, %2, %14\n\t"
        : "=&r" (out1), "=&r" (out2), "=&r" (out3)
        : "r" (a), "r" (b), "r" (c), "r" (d), "r" (e),
          "r" (f), "r" (g), "r" (h), "r" (i), "r" (j), "r" (k)
        : "cc"
    );
    result = out1 + out2 + out3;
#endif
    
    return result;
}

int main(int argc, char *argv[]) {
    /* Generate 12 non-constant values using argv and PRNG */
    int vals[12];
    
    /* Use argv indices if available, otherwise use PRNG */
    for (int i = 0; i < 12; i++) {
        if (i + 1 < argc) {
            vals[i] = atoi(argv[i + 1]);
        } else {
            vals[i] = prng() % 1000;
        }
    }
    
    /* Call both functions to trigger different expansion paths */
    int res10 = func_10_operands(vals[0], vals[1], vals[2], vals[3], vals[4],
                                 vals[5], vals[6], vals[7], vals[8], vals[9]);
    
    int res11 = func_11_operands(vals[0], vals[1], vals[2], vals[3], vals[4],
                                 vals[5], vals[6], vals[7], vals[8], vals[9],
                                 vals[10]);
    
    /* Combine results to prevent optimization */
    int final_result = res10 + res11 + vals[11];
    
    printf("Result: %d\n", final_result);
    
    /* Use result in a conditional to prevent dead code elimination */
    if (final_result > 1000000) {
        printf("Large result detected\n");
    }
    
    return final_result & 0xFF;
}
