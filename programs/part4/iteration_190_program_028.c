/* This program aims to trigger GCC's RTL expansion for instructions with
   10 and 11 operands, covering specific cases in optabs.cc */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Simple PRNG to generate non-constant values */
static uint32_t prng_state = 123456789;
static uint32_t prng() {
    prng_state = prng_state * 1103515245 + 12345;
    return prng_state;
}

/* Function for 10 operands */
int func_10_operands(uint64_t a0, uint64_t a1, uint64_t a2, uint64_t a3,
                     uint64_t a4, uint64_t a5, uint64_t a6, uint64_t a7,
                     uint64_t a8, uint64_t a9) {
    int result = 0;
    
#if defined(__AVX512F__) && defined(__AVX512VL__)
    /* AVX-512 intrinsic that may expand to many operands */
    #include <immintrin.h>
    __m512i v0 = _mm512_set_epi64(a9, a8, a7, a6, a5, a4, a3, a2);
    __m512i v1 = _mm512_set_epi64(a1, a0, a9, a8, a7, a6, a5, a4);
    __mmask8 mask = (a0 & 0xFF) | ((a1 & 0xFF) << 8);
    
    /* Complex operation that might require many operands during expansion */
    __m512i res = _mm512_mask_add_epi64(v0, mask, v0, v1);
    
    /* Extract and combine results */
    uint64_t res_arr[8];
    _mm512_storeu_si512(res_arr, res);
    for (int i = 0; i < 8; i++) {
        result += (int)(res_arr[i] & 0xFFFFFFFF);
    }
    
#elif defined(__ARM_FEATURE_SVE)
    /* ARM SVE intrinsic - placeholder for high operand count */
    /* Note: Actual SVE intrinsics would require proper vector types */
    result = (int)(a0 + a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9);
    
#else
    /* Generic inline assembly with 10 operands */
    uint64_t out0, out1, out2, out3;
    
    asm volatile (
        /* Dummy operations using all 10 input operands */
        "mov %0, %4\n\t"
        "add %0, %0, %5\n\t"
        "add %0, %0, %6\n\t"
        "mov %1, %7\n\t"
        "add %1, %1, %8\n\t"
        "add %1, %1, %9\n\t"
        "mov %2, %10\n\t"
        "add %2, %2, %11\n\t"
        "add %2, %2, %12\n\t"
        "mov %3, %13\n\t"
        "add %3, %3, %14\n\t"
        "add %3, %3, %15\n\t"
        : "=r"(out0), "=r"(out1), "=r"(out2), "=r"(out3)
        : "r"(a0), "r"(a1), "r"(a2), "r"(a3), "r"(a4), 
          "r"(a5), "r"(a6), "r"(a7), "r"(a8), "r"(a9)
        : "cc"
    );
    
    result = (int)(out0 + out1 + out2 + out3);
#endif
    
    return result;
}

/* Function for 11 operands */
int func_11_operands(uint64_t a0, uint64_t a1, uint64_t a2, uint64_t a3,
                     uint64_t a4, uint64_t a5, uint64_t a6, uint64_t a7,
                     uint64_t a8, uint64_t a9, uint64_t a10) {
    int result = 0;
    
#if defined(__AVX512F__) && defined(__AVX512VL__)
    /* Another AVX-512 operation with different operand arrangement */
    #include <immintrin.h>
    __m512i v0 = _mm512_set_epi64(a10, a9, a8, a7, a6, a5, a4, a3);
    __m512i v1 = _mm512_set_epi64(a2, a1, a0, a10, a9, a8, a7, a6);
    __m512i v2 = _mm512_set_epi64(a5, a4, a3, a2, a1, a0, a10, a9);
    __mmask8 mask1 = (a0 & 0xFF) | ((a1 & 0xFF) << 8);
    __mmask8 mask2 = (a2 & 0xFF) | ((a3 & 0xFF) << 8);
    
    /* Complex masked blend operation */
    __m512i blended = _mm512_mask_blend_epi64(mask1, v0, v1);
    __m512i res = _mm512_mask_add_epi64(v2, mask2, blended, v2);
    
    uint64_t res_arr[8];
    _mm512_storeu_si512(res_arr, res);
    for (int i = 0; i < 8; i++) {
        result += (int)(res_arr[i] & 0xFFFFFFFF);
    }
    
#elif defined(__ARM_FEATURE_SVE)
    /* ARM SVE placeholder */
    result = (int)(a0 + a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10);
    
#else
    /* Generic inline assembly with 11 operands */
    uint64_t out0, out1, out2, out3, out4;
    
    asm volatile (
        /* Dummy operations using all 11 input operands */
        "mov %0, %5\n\t"
        "add %0, %0, %6\n\t"
        "add %0, %0, %7\n\t"
        "mov %1, %8\n\t"
        "add %1, %1, %9\n\t"
        "add %1, %1, %10\n\t"
        "mov %2, %11\n\t"
        "add %2, %2, %12\n\t"
        "add %2, %2, %13\n\t"
        "mov %3, %14\n\t"
        "add %3, %3, %15\n\t"
        "add %3, %3, %16\n\t"
        "mov %4, %17\n\t"
        "add %4, %4, %18\n\t"
        "add %4, %4, %19\n\t"
        : "=r"(out0), "=r"(out1), "=r"(out2), "=r"(out3), "=r"(out4)
        : "r"(a0), "r"(a1), "r"(a2), "r"(a3), "r"(a4), 
          "r"(a5), "r"(a6), "r"(a7), "r"(a8), "r"(a9),
          "r"(a10)
        : "cc"
    );
    
    result = (int)(out0 + out1 + out2 + out3 + out4);
#endif
    
    return result;
}

int main(int argc, char *argv[]) {
    /* Initialize 12 variables with non-constant values */
    uint64_t vars[12];
    
    /* Use argv for some values, PRNG for others to ensure variability */
    for (int i = 0; i < 12; i++) {
        if (i < argc && argv[i] != NULL) {
            vars[i] = (uint64_t)argv[i][0] + i * 256;
        } else {
            vars[i] = prng() + i * 1024;
        }
    }
    
    /* Call both functions with overlapping but different operand sets */
    int res1 = func_10_operands(vars[0], vars[1], vars[2], vars[3],
                                vars[4], vars[5], vars[6], vars[7],
                                vars[8], vars[9]);
    
    int res2 = func_11_operands(vars[1], vars[2], vars[3], vars[4],
                                vars[5], vars[6], vars[7], vars[8],
                                vars[9], vars[10], vars[11]);
    
    /* Combine results in a non-trivial way */
    int final_result = res1 * 3 + res2 * 7;
    
    printf("Result: %d (from %d and %d)\n", final_result, res1, res2);
    
    /* Use result to affect return code (prevents dead code elimination) */
    return (final_result & 255) == 0 ? 0 : 1;
}
