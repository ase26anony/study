/* gcov-optabs-test.c - Test program for covering 10/11 operand expansion in optabs.cc */

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

/* Helper to convert command line arguments to integers */
static int arg_to_int(const char *arg, int default_val) {
    if (arg && *arg) {
        return atoi(arg);
    }
    return default_val;
}

/* Function designed to trigger 10-operand expansion */
int func_10_operands(int a0, int a1, int a2, int a3, int a4,
                     int a5, int a6, int a7, int a8, int a9) {
    int result = 0;
    
#if defined(__AVX512F__) && defined(__AVX512VL__)
    /* AVX-512 intrinsic that may expand to many operands */
    #include <immintrin.h>
    __m512i v0 = _mm512_set1_epi32(a0);
    __m512i v1 = _mm512_set1_epi32(a1);
    __m512i v2 = _mm512_set1_epi32(a2);
    __m512i v3 = _mm512_set1_epi32(a3);
    __m512i v4 = _mm512_set1_epi32(a4);
    __m512i v5 = _mm512_set1_epi32(a5);
    __m512i v6 = _mm512_set1_epi32(a6);
    __m512i v7 = _mm512_set1_epi32(a7);
    __m512i v8 = _mm512_set1_epi32(a8);
    __m512i v9 = _mm512_set1_epi32(a9);
    
    /* Complex operation with many operands */
    __m512i sum = _mm512_add_epi32(v0, v1);
    sum = _mm512_add_epi32(sum, v2);
    sum = _mm512_add_epi32(sum, v3);
    sum = _mm512_add_epi32(sum, v4);
    sum = _mm512_add_epi32(sum, v5);
    sum = _mm512_add_epi32(sum, v6);
    sum = _mm512_add_epi32(sum, v7);
    sum = _mm512_add_epi32(sum, v8);
    sum = _mm512_add_epi32(sum, v9);
    
    /* Reduce to scalar */
    result = _mm512_reduce_add_epi32(sum);
    
#elif defined(__ARM_FEATURE_SVE)
    /* ARM SVE may have complex scatter/gather operations */
    #include <arm_sve.h>
    /* Create predicate from inputs */
    svbool_t pg = svwhilelt_b32(0, 16);
    
    /* Use inline assembly as fallback for SVE */
    asm volatile (
        "/* SVE 10-operand operation */\n\t"
        : "=r" (result)
        : "r" (a0), "r" (a1), "r" (a2), "r" (a3), "r" (a4),
          "r" (a5), "r" (a6), "r" (a7), "r" (a8), "r" (a9)
        : "memory"
    );
    
#else
    /* Generic inline assembly with 10 explicit operands */
    /* This should trigger the 10-operand expansion path */
    asm volatile (
        "/* 10-operand dummy operation */\n\t"
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
        : "r" (a0), "r" (a1), "r" (a2), "r" (a3), "r" (a4),
          "r" (a5), "r" (a6), "r" (a7), "r" (a8), "r" (a9)
    );
#endif
    
    /* Ensure result is used */
    return result ^ 0x55AA55AA;
}

/* Function designed to trigger 11-operand expansion */
int func_11_operands(int a0, int a1, int a2, int a3, int a4, int a5,
                     int a6, int a7, int a8, int a9, int a10) {
    int result = 0;
    
#if defined(__AVX512F__) && defined(__AVX512BW__)
    /* Another AVX-512 intrinsic that might use many operands */
    #include <immintrin.h>
    __mmask64 mask = (a0 & 0xFF) | ((a1 & 0xFF) << 8) |
                     ((a2 & 0xFF) << 16) | ((a3 & 0xFF) << 24);
    
    /* Use inline assembly to ensure 11 operands */
    asm volatile (
        "/* AVX-512 11-operand operation */\n\t"
        : "=r" (result)
        : "r" (a0), "r" (a1), "r" (a2), "r" (a3), "r" (a4),
          "r" (a5), "r" (a6), "r" (a7), "r" (a8), "r" (a9),
          "r" (a10), "k" (mask)
        : "memory"
    );
    
#elif defined(__powerpc__) || defined(__PPC__)
    /* PowerPC Altivec/VSX may have multi-operand instructions */
    asm volatile (
        "/* PowerPC 11-operand operation */\n\t"
        : "=r" (result)
        : "r" (a0), "r" (a1), "r" (a2), "r" (a3), "r" (a4),
          "r" (a5), "r" (a6), "r" (a7), "r" (a8), "r" (a9),
          "r" (a10)
        : "memory"
    );
    
#else
    /* Generic inline assembly with 11 explicit operands */
    /* This should trigger the 11-operand expansion path */
    asm volatile (
        "/* 11-operand dummy operation */\n\t"
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
        "add %0, %0, %11"
        : "=r" (result)
        : "r" (a0), "r" (a1), "r" (a2), "r" (a3), "r" (a4),
          "r" (a5), "r" (a6), "r" (a7), "r" (a8), "r" (a9),
          "r" (a10)
    );
#endif
    
    /* Ensure result is used */
    return result ^ 0xAA55AA55;
}

int main(int argc, char *argv[]) {
    int vals[12];
    int i;
    
    /* Initialize values using PRNG and command line arguments */
    for (i = 0; i < 12; i++) {
        if (i < argc - 1) {
            vals[i] = arg_to_int(argv[i + 1], 0);
        } else {
            vals[i] = prng_next() % 1000;
        }
    }
    
    /* Call 10-operand function */
    int result10 = func_10_operands(vals[0], vals[1], vals[2], vals[3], vals[4],
                                    vals[5], vals[6], vals[7], vals[8], vals[9]);
    
    /* Call 11-operand function */
    int result11 = func_11_operands(vals[0], vals[1], vals[2], vals[3], vals[4],
                                    vals[5], vals[6], vals[7], vals[8], vals[9],
                                    vals[10]);
    
    /* Combine results to prevent optimization */
    int final_result = result10 + result11 + vals[11];
    
    printf("Result: %d (10-op: %d, 11-op: %d)\n", 
           final_result, result10, result11);
    
    return final_result != 0 ? 0 : 1;
}
