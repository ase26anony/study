/* Test program to cover 10/11-operand expansion in optabs.cc */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Simple PRNG to generate non-constant values */
static uint64_t simple_rand(uint64_t seed) {
    return seed * 6364136223846793005ULL + 1442695040888963407ULL;
}

/* Function using 10 operands */
int func_10_operands(uint64_t a0, uint64_t a1, uint64_t a2, uint64_t a3,
                     uint64_t a4, uint64_t a5, uint64_t a6, uint64_t a7,
                     uint64_t a8, uint64_t a9) {
    int result = 0;
    
#if defined(__AVX512F__) && defined(__AVX512VL__)
    /* AVX-512 intrinsic with many operands */
    #include <immintrin.h>
    __m512i v0 = _mm512_set_epi64(a9, a8, a7, a6, a5, a4, a3, a2);
    __m512i v1 = _mm512_set_epi64(a1, a0, a9, a8, a7, a6, a5, a4);
    __mmask8 mask = (__mmask8)((a0 ^ a1 ^ a2) & 0xFF);
    char* addr = (char*)(uintptr_t)a3;
    
    /* _mm512_mask_compressstoreu_epi32 uses mask, address, and data */
    /* When expanded, this requires multiple operands */
    _mm512_mask_compressstoreu_epi32(addr, mask, v0);
    
    /* Use the result */
    __m512i v2 = _mm512_loadu_si512((__m512i*)addr);
    result = _mm512_reduce_add_epi64(v2) + _mm512_reduce_add_epi64(v1);
    
#elif defined(__ARM_FEATURE_SVE)
    /* ARM SVE intrinsic with many operands */
    #include <arm_sve.h>
    svuint64_t data = svdup_u64(a0);
    svuint64_t bases = svdup_u64(a1);
    svbool_t pred = svwhilelt_b64(a2, a3);
    
    /* svstnt1_scatter_u64base_u64 has predicate, bases, data */
    /* Expansion requires multiple operand handling */
    svstnt1_scatter_u64base_u64(pred, bases, data);
    
    /* Use the result */
    result = (int)(a0 + a1 + a2 + a3);
    
#else
    /* Generic inline assembly with 10 operands */
    /* Forces expansion to handle many operands */
    uint64_t out0, out1, out2;
    
    asm volatile (
        /* Dummy operations using all 10 input operands */
        "mov %0, %1\n\t"
        "add %0, %0, %2\n\t"
        "add %0, %0, %3\n\t"
        "add %0, %0, %4\n\t"
        "add %0, %0, %5\n\t"
        "add %0, %0, %6\n\t"
        "add %0, %0, %7\n\t"
        "add %0, %0, %8\n\t"
        "add %0, %0, %9\n\t"
        "mov %10, %0\n\t"
        "mov %11, %0"
        : "=&r"(out0), "=&r"(out1), "=&r"(out2)
        : "r"(a0), "r"(a1), "r"(a2), "r"(a3), 
          "r"(a4), "r"(a5), "r"(a6), "r"(a7),
          "r"(a8), "r"(a9)
        : "cc"
    );
    
    result = (int)(out0 + out1 + out2 + a0 + a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9);
#endif
    
    return result;
}

/* Function using 11 operands */
int func_11_operands(uint64_t a0, uint64_t a1, uint64_t a2, uint64_t a3,
                     uint64_t a4, uint64_t a5, uint64_t a6, uint64_t a7,
                     uint64_t a8, uint64_t a9, uint64_t a10) {
    int result = 0;
    
#if defined(__AVX512F__) && defined(__AVX512VBMI2__)
    /* Another AVX-512 intrinsic that may use many operands */
    #include <immintrin.h>
    __m512i v0 = _mm512_set_epi64(a10, a9, a8, a7, a6, a5, a4, a3);
    __m512i v1 = _mm512_set_epi64(a2, a1, a0, a10, a9, a8, a7, a6);
    __m512i v2 = _mm512_set_epi64(a5, a4, a3, a2, a1, a0, a10, a9);
    __mmask16 mask1 = (__mmask16)((a0 ^ a1 ^ a2 ^ a3) & 0xFFFF);
    __mmask16 mask2 = (__mmask16)((a4 ^ a5 ^ a6 ^ a7) & 0xFFFF);
    
    /* _mm512_mask_shrdv_epi64 uses dest, mask, a, b, count */
    /* When expanded with all immediates/materialized, needs many operands */
    __m512i res = _mm512_mask_shrdv_epi64(v0, mask1, v1, v2, _mm512_set1_epi64(a8));
    res = _mm512_mask_shrdv_epi64(res, mask2, v2, v0, _mm512_set1_epi64(a9));
    
    result = _mm512_reduce_add_epi64(res) + (int)a10;
    
#elif defined(__ARM_FEATURE_SVE2)
    /* ARM SVE2 intrinsic with many operands */
    #include <arm_sve.h>
    svuint64_t data1 = svdup_u64(a0);
    svuint64_t data2 = svdup_u64(a1);
    svuint64_t data3 = svdup_u64(a2);
    svbool_t pred1 = svwhilelt_b64(a3, a4);
    svbool_t pred2 = svwhilelt_b64(a5, a6);
    
    /* Complex SVE2 operation using multiple predicates and data */
    svuint64_t res = svadd_u64_z(pred1, data1, data2);
    res = svmla_u64_z(pred2, res, data3, svdup_u64(a7));
    
    /* Use the result */
    result = (int)(a0 + a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10);
    
#else
    /* Generic inline assembly with 11 operands */
    uint64_t out0, out1, out2, out3;
    
    asm volatile (
        /* Dummy operations using all 11 input operands */
        "mov %0, %1\n\t"
        "add %0, %0, %2\n\t"
        "add %0, %0, %3\n\t"
        "add %0, %0, %4\n\t"
        "add %0, %0, %5\n\t"
        "add %0, %0, %6\n\t"
        "add %0, %0, %7\n\t"
        "add %0, %0, %8\n\t"
        "add %0, %0, %9\n\t"
        "add %0, %0, %10\n\t"
        "mov %11, %0\n\t"
        "mov %12, %0\n\t"
        "mov %13, %0"
        : "=&r"(out0), "=&r"(out1), "=&r"(out2), "=&r"(out3)
        : "r"(a0), "r"(a1), "r"(a2), "r"(a3), 
          "r"(a4), "r"(a5), "r"(a6), "r"(a7),
          "r"(a8), "r"(a9), "r"(a10)
        : "cc"
    );
    
    result = (int)(out0 + out1 + out2 + out3 + a0 + a1 + a2 + a3 + a4 + 
                   a5 + a6 + a7 + a8 + a9 + a10);
#endif
    
    return result;
}

int main(int argc, char *argv[]) {
    /* Generate non-constant values using argv and PRNG */
    uint64_t seed = (uintptr_t)argv;
    uint64_t vals[12];
    
    for (int i = 0; i < 12; i++) {
        seed = simple_rand(seed);
        /* Mix in argv for more variability */
        if (i < argc) {
            seed ^= (uintptr_t)argv[i];
        }
        vals[i] = seed;
    }
    
    /* Call 10-operand function */
    int res10 = func_10_operands(vals[0], vals[1], vals[2], vals[3],
                                 vals[4], vals[5], vals[6], vals[7],
                                 vals[8], vals[9]);
    
    /* Call 11-operand function */
    int res11 = func_11_operands(vals[0], vals[1], vals[2], vals[3],
                                 vals[4], vals[5], vals[6], vals[7],
                                 vals[8], vals[9], vals[10]);
    
    /* Combine results to prevent optimization */
    int final_result = res10 + res11 + (int)vals[11];
    
    printf("Result: %d\n", final_result);
    
    /* Use result in a conditional to prevent dead code elimination */
    if (final_result != 0 || argc > 1) {
        return final_result & 0xFF;
    }
    
    return 0;
}
