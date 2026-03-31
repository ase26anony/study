/* This program aims to trigger GCC's RTL expansion for instructions with
   10 and 11 operands, covering specific cases in optabs.cc */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Simple PRNG to generate non-constant values */
static uint64_t simple_rand(uint64_t seed) {
    return seed * 6364136223846793005ULL + 1442695040888963407ULL;
}

/* Function to trigger 10-operand expansion */
int func_10_operands(uint64_t a0, uint64_t a1, uint64_t a2, uint64_t a3,
                     uint64_t a4, uint64_t a5, uint64_t a6, uint64_t a7,
                     uint64_t a8, uint64_t a9) {
    uint64_t result = 0;
    
#if defined(__AVX512F__) && defined(__AVX512VL__)
    /* AVX-512 intrinsic that may expand to many operands */
    #include <immintrin.h>
    __m512i v1 = _mm512_set_epi64(a9, a8, a7, a6, a5, a4, a3, a2);
    __m512i v2 = _mm512_set_epi64(a8, a7, a6, a5, a4, a3, a2, a1);
    __mmask8 mask = (__mmask8)((a0 & 0xFF) | 0x1);
    
    /* Complex operation that might require many operands during expansion */
    __m512i res = _mm512_mask_add_epi64(v1, mask, v1, v2);
    
    /* Extract and combine results */
    result = _mm512_extract_epi64(res, 0) +
             _mm512_extract_epi64(res, 1) +
             _mm512_extract_epi64(res, 2);
             
#elif defined(__ARM_FEATURE_SVE)
    /* ARM SVE intrinsic - adjust based on actual SVE availability */
    /* Note: Actual SVE intrinsics may vary */
    #include <arm_sve.h>
    /* Using inline assembly as fallback for SVE */
    __asm__ volatile (
        "add %0, %1, %2\n\t"
        "add %0, %0, %3\n\t"
        "add %0, %0, %4\n\t"
        "add %0, %0, %5\n\t"
        "add %0, %0, %6\n\t"
        "add %0, %0, %7\n\t"
        "add %0, %0, %8\n\t"
        "add %0, %0, %9"
        : "=r" (result)
        : "r" (a0), "r" (a1), "r" (a2), "r" (a3),
          "r" (a4), "r" (a5), "r" (a6), "r" (a7), "r" (a8)
        : "cc"
    );
    
#else
    /* Generic inline assembly with 10 operands */
    /* This should force the compiler to handle 10 operands during expansion */
    __asm__ volatile (
        /* Dummy operations using all 10 input registers */
        "mov %[out], %[in0]\n\t"
        "add %[out], %[out], %[in1]\n\t"
        "add %[out], %[out], %[in2]\n\t"
        "add %[out], %[out], %[in3]\n\t"
        "add %[out], %[out], %[in4]\n\t"
        "add %[out], %[out], %[in5]\n\t"
        "add %[out], %[out], %[in6]\n\t"
        "add %[out], %[out], %[in7]\n\t"
        "add %[out], %[out], %[in8]\n\t"
        "add %[out], %[out], %[in9]"
        : [out] "=r" (result)
        : [in0] "r" (a0), [in1] "r" (a1), [in2] "r" (a2),
          [in3] "r" (a3), [in4] "r" (a4), [in5] "r" (a5),
          [in6] "r" (a6), [in7] "r" (a7), [in8] "r" (a8),
          [in9] "r" (a9)
        : "cc"
    );
#endif
    
    return (int)(result % 1000);
}

/* Function to trigger 11-operand expansion */
int func_11_operands(uint64_t a0, uint64_t a1, uint64_t a2, uint64_t a3,
                     uint64_t a4, uint64_t a5, uint64_t a6, uint64_t a7,
                     uint64_t a8, uint64_t a9, uint64_t a10) {
    uint64_t result = 0;
    
#if defined(__AVX512F__) && defined(__AVX512BW__)
    /* Another AVX-512 intrinsic that might use many operands */
    #include <immintrin.h>
    __m512i v1 = _mm512_set_epi64(a10, a9, a8, a7, a6, a5, a4, a3);
    __m512i v2 = _mm512_set_epi64(a9, a8, a7, a6, a5, a4, a3, a2);
    __m512i v3 = _mm512_set_epi64(a8, a7, a6, a5, a4, a3, a2, a1);
    __mmask16 mask = (__mmask16)((a0 & 0xFFFF) | 0x1);
    
    /* Complex operation with blending */
    __m512i temp = _mm512_mask_add_epi64(v1, mask, v2, v3);
    __m512i res = _mm512_mask_blend_epi64(mask >> 1, v1, temp);
    
    /* Extract results */
    result = _mm512_extract_epi64(res, 0) +
             _mm512_extract_epi64(res, 1) +
             _mm512_extract_epi64(res, 2) +
             _mm512_extract_epi64(res, 3);
             
#elif defined(__ARM_FEATURE_SVE)
    /* ARM SVE with 11 operands */
    __asm__ volatile (
        "add %0, %1, %2\n\t"
        "add %0, %0, %3\n\t"
        "add %0, %0, %4\n\t"
        "add %0, %0, %5\n\t"
        "add %0, %0, %6\n\t"
        "add %0, %0, %7\n\t"
        "add %0, %0, %8\n\t"
        "add %0, %0, %9\n\t"
        "add %0, %0, %10"
        : "=r" (result)
        : "r" (a0), "r" (a1), "r" (a2), "r" (a3),
          "r" (a4), "r" (a5), "r" (a6), "r" (a7),
          "r" (a8), "r" (a9), "r" (a10)
        : "cc"
    );
    
#else
    /* Generic inline assembly with 11 operands */
    __asm__ volatile (
        /* Dummy operations using all 11 input registers */
        "mov %[out], %[in0]\n\t"
        "add %[out], %[out], %[in1]\n\t"
        "add %[out], %[out], %[in2]\n\t"
        "add %[out], %[out], %[in3]\n\t"
        "add %[out], %[out], %[in4]\n\t"
        "add %[out], %[out], %[in5]\n\t"
        "add %[out], %[out], %[in6]\n\t"
        "add %[out], %[out], %[in7]\n\t"
        "add %[out], %[out], %[in8]\n\t"
        "add %[out], %[out], %[in9]\n\t"
        "add %[out], %[out], %[in10]"
        : [out] "=r" (result)
        : [in0] "r" (a0), [in1] "r" (a1), [in2] "r" (a2),
          [in3] "r" (a3), [in4] "r" (a4), [in5] "r" (a5),
          [in6] "r" (a6), [in7] "r" (a7), [in8] "r" (a8),
          [in9] "r" (a9), [in10] "r" (a10)
        : "cc"
    );
#endif
    
    return (int)(result % 1000);
}

int main(int argc, char *argv[]) {
    uint64_t values[12];
    uint64_t seed = 0x123456789ABCDEF0ULL;
    
    /* Initialize values with non-constant data */
    for (int i = 0; i < 12; i++) {
        if (argc > i + 1) {
            values[i] = strtoull(argv[i + 1], NULL, 0);
        } else {
            seed = simple_rand(seed + i);
            values[i] = seed;
        }
    }
    
    /* Call both functions to trigger different expansion paths */
    int result1 = func_10_operands(values[0], values[1], values[2], values[3],
                                   values[4], values[5], values[6], values[7],
                                   values[8], values[9]);
    
    int result2 = func_11_operands(values[0], values[1], values[2], values[3],
                                   values[4], values[5], values[6], values[7],
                                   values[8], values[9], values[10]);
    
    /* Combine results to prevent optimization */
    int final_result = result1 + result2;
    
    /* Use the result to prevent dead code elimination */
    printf("Result: %d (from %d + %d)\n", final_result, result1, result2);
    
    return final_result != 0 ? 0 : 1;
}
