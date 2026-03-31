/* test_optabs_coverage.c - Cover 10/11 operand expansion in optabs.cc */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Simple PRNG to generate non-constant values */
static unsigned int prng_state = 123456789;

static unsigned int prng(void) {
    prng_state = prng_state * 1103515245 + 12345;
    return prng_state;
}

/* Function to use 10 operands */
int func_10_operands(int a0, int a1, int a2, int a3, int a4,
                     int a5, int a6, int a7, int a8, int a9) {
    int result = 0;
    
#if defined(__AVX512F__) && defined(__AVX512VL__)
    /* AVX-512 intrinsic that can expand to many operands */
    /* Using gather instruction with mask, index, scale, etc. */
    __m512i src = _mm512_set_epi32(a9, a8, a7, a6, a5, a4, a3, a2, a1, a0,
                                   0, 0, 0, 0, 0, 0);
    __m512i index = _mm512_set_epi32(1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
                                     11, 12, 13, 14, 15, 16);
    __mmask16 mask = 0xAAAA;  /* 1010101010101010 */
    int scale = 4;
    __m512i vresult;
    
    /* This gather operation involves many internal operands */
    vresult = _mm512_mask_i32gather_epi32(_mm512_setzero_si512(),
                                          mask, index, 
                                          (const void *)&a0, scale);
    
    /* Extract and combine results */
    result = _mm512_reduce_add_epi32(vresult);
    
#elif defined(__ARM_FEATURE_SVE)
    /* ARM SVE scatter store with predicate, base, data */
    /* Note: SVE requires variable-length vectors */
    svbool_t pg = svwhilelt_b32(0, 16);
    svint32_t data = svdup_s32(a0);
    svuint32_t bases = svdup_u32(0);
    
    /* Complex scatter operation with many operands */
    svst1_scatter_u32base_s32(pg, (int32_t *)&a0, bases, data);
    
    result = a0;  /* Use the modified value */
    
#else
    /* Generic inline assembly with 10 operands */
    /* Forces expansion to handle many rtx operands */
    asm volatile (
        "# 10-operand dummy operation\n\t"
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
#endif
    
    return result + a9;  /* Ensure all 10 inputs are used */
}

/* Function to use 11 operands */
int func_11_operands(int a0, int a1, int a2, int a3, int a4, int a5,
                     int a6, int a7, int a8, int a9, int a10) {
    int result = 0;
    
#if defined(__AVX512F__) && defined(__AVX512VL__) && defined(__AVX512BW__)
    /* AVX-512 masked compress store with many operands */
    __m512i src = _mm512_set_epi32(a10, a9, a8, a7, a6, a5, a4, a3, a2, a1, a0,
                                   0, 0, 0, 0, 0);
    __mmask16 mask = 0x5555;  /* 0101010101010101 */
    char buffer[64] __attribute__((aligned(64)));
    
    /* This operation involves mask, address, source - expands to many operands */
    _mm512_mask_compressstoreu_epi32(buffer, mask, src);
    
    /* Use the stored data */
    result = *(int *)buffer;
    
#elif defined(__ARM_FEATURE_SVE)
    /* ARM SVE gather with predicate, base, offset, data */
    svbool_t pg = svwhilelt_b32(0, 16);
    svint32_t data = svdup_s32(a0);
    svuint32_t bases = svdup_u32(0);
    svint32_t offsets = svdup_s32(1);
    
    /* Complex gather with predicate */
    data = svld1_gather_u32offset_s32(pg, (const int32_t *)&a0, bases);
    
    result = svaddv_s32(pg, data);
    
#else
    /* Generic inline assembly with 11 operands */
    int temp1, temp2;
    
    asm volatile (
        "# 11-operand dummy operation\n\t"
        "mov %0, %1\n\t"
        "add %0, %0, %2\n\t"
        "add %0, %0, %3\n\t"
        "add %0, %0, %4\n\t"
        "mov %11, %5\n\t"
        "add %11, %11, %6\n\t"
        "add %0, %0, %7\n\t"
        "add %0, %0, %8\n\t"
        "add %0, %0, %9\n\t"
        "add %0, %0, %10\n\t"
        "add %0, %0, %11"
        : "=r" (result), "=&r" (temp1), "=&r" (temp2)
        : "r" (a0), "r" (a1), "r" (a2), "r" (a3), "r" (a4),
          "r" (a5), "r" (a6), "r" (a7), "r" (a8)
        : "cc"
    );
    
    /* Use the 11th operand separately to ensure it's not optimized out */
    result += a9 + a10;
#endif
    
    return result;
}

/* Helper to initialize vector types if needed */
#ifdef __AVX512F__
#include <immintrin.h>
#endif

#ifdef __ARM_FEATURE_SVE
#include <arm_sve.h>
#endif

int main(int argc, char *argv[]) {
    int vals[20];
    int i, result10, result11, final_result = 0;
    
    /* Initialize with non-constant values using argv and PRNG */
    for (i = 0; i < 20; i++) {
        if (i < argc && argv[i]) {
            vals[i] = argv[i][0] + i;  /* Use argv content */
        } else {
            vals[i] = prng() & 0xFF;   /* Use PRNG for remaining */
        }
    }
    
    /* Call 10-operand function */
    result10 = func_10_operands(vals[0], vals[1], vals[2], vals[3], vals[4],
                                vals[5], vals[6], vals[7], vals[8], vals[9]);
    
    /* Call 11-operand function */
    result11 = func_11_operands(vals[10], vals[11], vals[12], vals[13], vals[14],
                                vals[15], vals[16], vals[17], vals[18], vals[19],
                                result10);  /* Mix results */
    
    /* Combine results to prevent optimization */
    final_result = result10 + result11;
    
    /* Print result to ensure side effect */
    printf("Final result: %d\n", final_result);
    
    /* Also use all values in a way compiler can't optimize away */
    for (i = 0; i < 20; i++) {
        final_result ^= vals[i];
    }
    
    return final_result & 0xFF;  /* Return non-constant value */
}
