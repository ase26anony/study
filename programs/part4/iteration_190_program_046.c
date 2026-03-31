/* Test program for covering 10/11 operand expansion in optabs.cc */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Architecture-specific headers */
#ifdef __AVX512F__
#include <immintrin.h>
#endif

#ifdef __ARM_FEATURE_SVE
#include <arm_sve.h>
#endif

/* Simple PRNG for generating non-constant values */
static uint64_t simple_rand(uint64_t seed) {
    return seed * 6364136223846793005ULL + 1442695040888963407ULL;
}

/* Function for 10 operands */
int func_10_operands(uint64_t a0, uint64_t a1, uint64_t a2, uint64_t a3,
                     uint64_t a4, uint64_t a5, uint64_t a6, uint64_t a7,
                     uint64_t a8, uint64_t a9) {
    int result = 0;
    
#if defined(__AVX512F__)
    /* AVX-512 intrinsic that may expand to many operands */
    __m512i v0 = _mm512_set_epi64(a9, a8, a7, a6, a5, a4, a3, a2);
    __m512i v1 = _mm512_set_epi64(a1, a0, a9, a8, a7, a6, a5, a4);
    __mmask8 mask = (__mmask8)((a0 ^ a1 ^ a2 ^ a3) & 0xFF);
    
    /* Complex operation with many internal operands */
    __m512i temp = _mm512_mask_add_epi64(v0, mask, v0, v1);
    
    /* Extract and combine results */
    result = _mm512_reduce_add_epi64(temp) & 0x7FFFFFFF;
    
#elif defined(__ARM_FEATURE_SVE)
    /* ARM SVE - using inline asm as fallback since specific 10-op intrinsics are rare */
    uint64_t out0, out1, out2, out3;
    asm volatile (
        "add %[o0], %[a0], %[a1]\n\t"
        "add %[o1], %[a2], %[a3]\n\t"
        "add %[o2], %[a4], %[a5]\n\t"
        "add %[o3], %[a6], %[a7]\n\t"
        : [o0] "=r" (out0), [o1] "=r" (out1), [o2] "=r" (out2), [o3] "=r" (out3)
        : [a0] "r" (a0), [a1] "r" (a1), [a2] "r" (a2), [a3] "r" (a3),
          [a4] "r" (a4), [a5] "r" (a5), [a6] "r" (a6), [a7] "r" (a7),
          [a8] "r" (a8), [a9] "r" (a9)
        : "cc"
    );
    result = (out0 + out1 + out2 + out3 + a8 + a9) & 0x7FFFFFFF;
    
#else
    /* Generic inline assembly with 10 explicit operands */
    uint64_t temp1, temp2, temp3, temp4;
    asm volatile (
        "/* 10-operand dummy operation */\n\t"
        "mov %[t1], %[a0]\n\t"
        "add %[t1], %[t1], %[a1]\n\t"
        "mov %[t2], %[a2]\n\t"
        "add %[t2], %[t2], %[a3]\n\t"
        "mov %[t3], %[a4]\n\t"
        "add %[t3], %[t3], %[a5]\n\t"
        "mov %[t4], %[a6]\n\t"
        "add %[t4], %[t4], %[a7]\n\t"
        "add %[t1], %[t1], %[t2]\n\t"
        "add %[t3], %[t3], %[t4]\n\t"
        "add %[t1], %[t1], %[t3]\n\t"
        "add %[t1], %[t1], %[a8]\n\t"
        "add %[t1], %[t1], %[a9]"
        : [t1] "=&r" (temp1), [t2] "=&r" (temp2), [t3] "=&r" (temp3), [t4] "=&r" (temp4)
        : [a0] "r" (a0), [a1] "r" (a1), [a2] "r" (a2), [a3] "r" (a3),
          [a4] "r" (a4), [a5] "r" (a5), [a6] "r" (a6), [a7] "r" (a7),
          [a8] "r" (a8), [a9] "r" (a9)
        : "cc"
    );
    result = temp1 & 0x7FFFFFFF;
#endif
    
    return result;
}

/* Function for 11 operands */
int func_11_operands(uint64_t a0, uint64_t a1, uint64_t a2, uint64_t a3,
                     uint64_t a4, uint64_t a5, uint64_t a6, uint64_t a7,
                     uint64_t a8, uint64_t a9, uint64_t a10) {
    int result = 0;
    
#if defined(__AVX512F__)
    /* Another AVX-512 operation with different operands */
    __m512i v0 = _mm512_set_epi64(a10, a9, a8, a7, a6, a5, a4, a3);
    __m512i v1 = _mm512_set_epi64(a2, a1, a0, a10, a9, a8, a7, a6);
    __m512i v2 = _mm512_set_epi64(a5, a4, a3, a2, a1, a0, a10, a9);
    __mmask8 mask1 = (__mmask8)((a0 ^ a2 ^ a4 ^ a6) & 0xFF);
    __mmask8 mask2 = (__mmask8)((a1 ^ a3 ^ a5 ^ a7) & 0xFF);
    
    /* Complex sequence that may require many operands during expansion */
    __m512i temp1 = _mm512_mask_add_epi64(v0, mask1, v0, v1);
    __m512i temp2 = _mm512_mask_sub_epi64(v1, mask2, v1, v2);
    __m512i final = _mm512_add_epi64(temp1, temp2);
    
    result = _mm512_reduce_add_epi64(final) & 0x7FFFFFFF;
    
#elif defined(__ARM_FEATURE_SVE)
    /* ARM SVE with 11 operands */
    uint64_t out0, out1, out2, out3;
    asm volatile (
        "add %[o0], %[a0], %[a10]\n\t"
        "add %[o1], %[a1], %[a9]\n\t"
        "add %[o2], %[a2], %[a8]\n\t"
        "add %[o3], %[a3], %[a7]\n\t"
        "add %[o0], %[o0], %[o1]\n\t"
        "add %[o2], %[o2], %[o3]\n\t"
        "add %[o0], %[o0], %[o2]\n\t"
        "add %[o0], %[o0], %[a4]\n\t"
        "add %[o0], %[o0], %[a5]\n\t"
        "add %[o0], %[o0], %[a6]"
        : [o0] "=&r" (out0), [o1] "=&r" (out1), [o2] "=&r" (out2), [o3] "=&r" (out3)
        : [a0] "r" (a0), [a1] "r" (a1), [a2] "r" (a2), [a3] "r" (a3),
          [a4] "r" (a4), [a5] "r" (a5), [a6] "r" (a6), [a7] "r" (a7),
          [a8] "r" (a8), [a9] "r" (a9), [a10] "r" (a10)
        : "cc"
    );
    result = out0 & 0x7FFFFFFF;
    
#else
    /* Generic inline assembly with 11 explicit operands */
    uint64_t temp1, temp2, temp3, temp4;
    asm volatile (
        "/* 11-operand dummy operation */\n\t"
        "mov %[t1], %[a0]\n\t"
        "add %[t1], %[t1], %[a1]\n\t"
        "mov %[t2], %[a2]\n\t"
        "add %[t2], %[t2], %[a3]\n\t"
        "mov %[t3], %[a4]\n\t"
        "add %[t3], %[t3], %[a5]\n\t"
        "mov %[t4], %[a6]\n\t"
        "add %[t4], %[t4], %[a7]\n\t"
        "add %[t1], %[t1], %[t2]\n\t"
        "add %[t3], %[t3], %[t4]\n\t"
        "add %[t1], %[t1], %[t3]\n\t"
        "add %[t1], %[t1], %[a8]\n\t"
        "add %[t1], %[t1], %[a9]\n\t"
        "add %[t1], %[t1], %[a10]"
        : [t1] "=&r" (temp1), [t2] "=&r" (temp2), [t3] "=&r" (temp3), [t4] "=&r" (temp4)
        : [a0] "r" (a0), [a1] "r" (a1), [a2] "r" (a2), [a3] "r" (a3),
          [a4] "r" (a4), [a5] "r" (a5), [a6] "r" (a6), [a7] "r" (a7),
          [a8] "r" (a8), [a9] "r" (a9), [a10] "r" (a10)
        : "cc"
    );
    result = temp1 & 0x7FFFFFFF;
#endif
    
    return result;
}

int main(int argc, char *argv[]) {
    uint64_t vals[12];
    uint64_t seed = 0x123456789ABCDEFULL;
    
    /* Initialize with non-constant values using PRNG and argv */
    for (int i = 0; i < 12; i++) {
        seed = simple_rand(seed);
        vals[i] = seed ^ (uint64_t)(argv[0] ? argv[0][i % argc] : 0);
        vals[i] ^= (uint64_t)(i * 7919);  /* Prime multiplier for variation */
    }
    
    /* Call both functions with different operand counts */
    int res10 = func_10_operands(vals[0], vals[1], vals[2], vals[3],
                                 vals[4], vals[5], vals[6], vals[7],
                                 vals[8], vals[9]);
    
    int res11 = func_11_operands(vals[1], vals[2], vals[3], vals[4],
                                 vals[5], vals[6], vals[7], vals[8],
                                 vals[9], vals[10], vals[11]);
    
    /* Combine results to prevent optimization */
    int final_result = res10 + res11;
    
    printf("Result: %d (from 10-op: %d, 11-op: %d)\n", 
           final_result, res10, res11);
    
    return final_result != 0 ? 0 : 1;
}
