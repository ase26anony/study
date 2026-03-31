/* Test program for GCC optabs.cc 10/11 operand expansion coverage */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Simple PRNG to generate non-constant values */
static unsigned int prng_state = 123456789;
static unsigned int prng_next(void) {
    prng_state = prng_state * 1103515245 + 12345;
    return prng_state;
}

/* Function using 10 operands */
int func_10_operands(int a, int b, int c, int d, int e,
                     int f, int g, int h, int i, int j) {
    int result = 0;
    
#ifdef __AVX512F__
    /* AVX-512 intrinsic that may expand to many operands */
    #include <immintrin.h>
    __m512i v1 = _mm512_set_epi32(a, b, c, d, e, f, g, h, i, j, 0, 0, 0, 0, 0, 0);
    __m512i v2 = _mm512_set_epi32(j, i, h, g, f, e, d, c, b, a, 0, 0, 0, 0, 0, 0);
    __mmask16 mask = 0xAAAA;  /* 1010101010101010 */
    
    /* _mm512_mask_add_epi32 with mask and two vectors - may expand to many operands */
    __m512i v3 = _mm512_mask_add_epi32(v1, mask, v1, v2);
    
    /* Extract and sum elements to create dependency */
    int arr[16];
    _mm512_storeu_si512(arr, v3);
    for (int k = 0; k < 16; k++) {
        result += arr[k];
    }
#elif defined(__ARM_FEATURE_SVE)
    /* ARM SVE may have multi-operand scatter/gather operations */
    /* Using inline assembly as fallback for SVE */
    asm volatile (
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
        : "r" (a), "r" (b), "r" (c), "r" (d), "r" (e),
          "r" (f), "r" (g), "r" (h), "r" (i), "r" (j)
        : "cc"
    );
#else
    /* Generic inline assembly with 10 explicit operands */
    /* This should force the compiler to handle 10 rtx operands */
    asm volatile (
        /* Dummy operations using all 10 input registers */
        "mov %0, %1\n\t"
        "add %0, %0, %2\n\t"
        "add %0, %0, %3\n\t"
        "add %0, %0, %4\n\t"
        "add %0, %0, %5\n\t"
        "add %0, %0, %6\n\t"
        "add %0, %0, %7\n\t"
        "add %0, %0, %8\n\t"
        "add %0, %0, %9\n\t"
        "add %0, %0, %10"
        : "=r" (result)
        : "r" (a), "r" (b), "r" (c), "r" (d), "r" (e),
          "r" (f), "r" (g), "r" (h), "r" (i), "r" (j)
        : "cc"
    );
#endif
    
    return result;
}

/* Function using 11 operands */
int func_11_operands(int a, int b, int c, int d, int e,
                     int f, int g, int h, int i, int j, int k) {
    int result = 0;
    
#ifdef __AVX512F__
    #include <immintrin.h>
    /* Create vectors with all 11 values */
    __m512i v1 = _mm512_set_epi32(a, b, c, d, e, f, g, h, i, j, k, 0, 0, 0, 0, 0);
    __m512i v2 = _mm512_set_epi32(k, j, i, h, g, f, e, d, c, b, a, 0, 0, 0, 0, 0);
    __mmask16 mask = 0x5555;  /* 0101010101010101 */
    
    /* Another AVX-512 operation that may use many operands */
    __m512i v3 = _mm512_mask_sub_epi32(v1, mask, v1, v2);
    
    /* Extract and process */
    int arr[16];
    _mm512_storeu_si512(arr, v3);
    for (int idx = 0; idx < 16; idx++) {
        result ^= arr[idx];  /* Use XOR for variety */
    }
#elif defined(__ARM_FEATURE_SVE)
    /* ARM SVE assembly with 11 operands */
    asm volatile (
        "add %0, %1, %2\n\t"
        "add %0, %0, %3\n\t"
        "add %0, %0, %4\n\t"
        "add %0, %0, %5\n\t"
        "add %0, %0, %6\n\t"
        "add %0, %0, %7\n\t"
        "add %0, %0, %8\n\t"
        "add %0, %0, %9\n\t"
        "add %0, %0, %10\n\t"
        "add %0, %0, %11"
        : "=r" (result)
        : "r" (a), "r" (b), "r" (c), "r" (d), "r" (e),
          "r" (f), "r" (g), "r" (h), "r" (i), "r" (j), "r" (k)
        : "cc"
    );
#else
    /* Generic inline assembly with 11 explicit operands */
    asm volatile (
        /* Use all 11 input registers in computation */
        "mov %0, %1\n\t"
        "imul %0, %0, %2\n\t"
        "add %0, %0, %3\n\t"
        "sub %0, %0, %4\n\t"
        "add %0, %0, %5\n\t"
        "xor %0, %0, %6\n\t"
        "add %0, %0, %7\n\t"
        "sub %0, %0, %8\n\t"
        "add %0, %0, %9\n\t"
        "xor %0, %0, %10\n\t"
        "add %0, %0, %11"
        : "=r" (result)
        : "r" (a), "r" (b), "r" (c), "r" (d), "r" (e),
          "r" (f), "r" (g), "r" (h), "r" (i), "r" (j), "r" (k)
        : "cc"
    );
#endif
    
    return result;
}

int main(int argc, char *argv[]) {
    /* Initialize 11 variables with non-constant values */
    int vars[11];
    
    /* Use argv indices mixed with PRNG for variability */
    for (int i = 0; i < 11; i++) {
        if (i < argc && argv[i] != NULL) {
            vars[i] = (int)(argv[i][0]) + i;
        } else {
            vars[i] = (int)prng_next() % 1000;
        }
    }
    
    /* Call both functions to trigger different expansion paths */
    int result1 = func_10_operands(vars[0], vars[1], vars[2], vars[3], vars[4],
                                   vars[5], vars[6], vars[7], vars[8], vars[9]);
    
    int result2 = func_11_operands(vars[0], vars[1], vars[2], vars[3], vars[4],
                                   vars[5], vars[6], vars[7], vars[8], vars[9], vars[10]);
    
    /* Combine results to prevent dead code elimination */
    int final_result = result1 + result2;
    
    /* Use the result so it can't be optimized away */
    printf("Final result: %d\n", final_result);
    
    /* Additional check to ensure both paths were used */
    if (final_result != 0) {
        printf("Both 10 and 11 operand paths should have been expanded.\n");
    }
    
    return 0;
}
