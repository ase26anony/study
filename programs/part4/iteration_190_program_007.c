/* Test program to cover 10/11 operand expansion in optabs.cc */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Simple PRNG to generate non-constant values */
static uint64_t seed = 123456789;

static uint64_t rand_u64(void) {
    seed = seed * 6364136223846793005ULL + 1442695040888963407ULL;
    return seed;
}

/* Architecture-specific includes */
#ifdef __AVX512F__
#include <immintrin.h>
#endif

#ifdef __ARM_FEATURE_SVE
#include <arm_sve.h>
#endif

/* Function for 10 operands */
int func_10_operands(uint64_t a0, uint64_t a1, uint64_t a2, uint64_t a3,
                     uint64_t a4, uint64_t a5, uint64_t a6, uint64_t a7,
                     uint64_t a8, uint64_t a9) {
    int result = 0;
    
#if defined(__AVX512F__)
    /* AVX-512 mask compress store - can involve many operands when expanded */
    __m512i src = _mm512_set_epi64(a9, a8, a7, a6, a5, a4, a3, a2);
    __mmask8 mask = (__mmask8)((a0 & 0xFF) | ((a1 & 0xFF) << 8));
    char buffer[64] __attribute__((aligned(64)));
    
    /* This intrinsic expands to multiple instructions with many operands */
    _mm512_mask_compressstoreu_epi64(buffer, mask, src);
    
    /* Use the result */
    for (int i = 0; i < 8; i++) {
        result += ((uint64_t*)buffer)[i] & 0xFF;
    }
    
#elif defined(__ARM_FEATURE_SVE)
    /* ARM SVE scatter store with predicate - many operands */
    svuint64_t data = svdup_u64(a0);
    svuint64_t bases = svdup_u64(a1);
    svbool_t pred = svwhilelt_b64(0, 8);
    
    /* Complex SVE operation that may expand to many operands */
    uint64_t buffer[8];
    svst1_scatter_u64base_u64(pred, buffer, bases, data);
    
    /* Use the result */
    for (int i = 0; i < 8; i++) {
        result += buffer[i] & 0xFF;
    }
    
#else
    /* Generic fallback: inline assembly with 10 operands */
    uint64_t r0, r1, r2, r3, r4, r5, r6, r7, r8, r9;
    
    /* Extended asm with 10 input operands and 10 output operands */
    asm volatile (
        /* Dummy operations that use all operands */
        "mov %[out0], %[in0] \n\t"
        "add %[out0], %[in1] \n\t"
        "mov %[out1], %[in2] \n\t"
        "add %[out1], %[in3] \n\t"
        "mov %[out2], %[in4] \n\t"
        "add %[out2], %[in5] \n\t"
        "mov %[out3], %[in6] \n\t"
        "add %[out3], %[in7] \n\t"
        "mov %[out4], %[in8] \n\t"
        "add %[out4], %[in9] \n\t"
        /* More dummy operations to ensure all outputs are used */
        "mov %[out5], %[out0] \n\t"
        "add %[out5], %[out1] \n\t"
        "mov %[out6], %[out2] \n\t"
        "add %[out6], %[out3] \n\t"
        "mov %[out7], %[out4] \n\t"
        "mov %[out8], %[out5] \n\t"
        "add %[out8], %[out6] \n\t"
        "mov %[out9], %[out7] \n\t"
        "add %[out9], %[out8] \n\t"
        : [out0] "=&r" (r0), [out1] "=&r" (r1), [out2] "=&r" (r2),
          [out3] "=&r" (r3), [out4] "=&r" (r4), [out5] "=&r" (r5),
          [out6] "=&r" (r6), [out7] "=&r" (r7), [out8] "=&r" (r8),
          [out9] "=&r" (r9)
        : [in0] "r" (a0), [in1] "r" (a1), [in2] "r" (a2), [in3] "r" (a3),
          [in4] "r" (a4), [in5] "r" (a5), [in6] "r" (a6), [in7] "r" (a7),
          [in8] "r" (a8), [in9] "r" (a9)
        : "cc"
    );
    
    /* Combine results to prevent optimization */
    result = (int)((r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9) & 0xFFFF);
#endif
    
    return result;
}

/* Function for 11 operands */
int func_11_operands(uint64_t a0, uint64_t a1, uint64_t a2, uint64_t a3,
                     uint64_t a4, uint64_t a5, uint64_t a6, uint64_t a7,
                     uint64_t a8, uint64_t a9, uint64_t a10) {
    int result = 0;
    
#if defined(__AVX512F__)
    /* Another AVX-512 operation with mask and multiple data sources */
    __m512i src1 = _mm512_set_epi64(a10, a9, a8, a7, a6, a5, a4, a3);
    __m512i src2 = _mm512_set_epi64(a2, a1, a0, a10, a9, a8, a7, a6);
    __mmask8 mask = (__mmask8)((a0 & 0xFF) | ((a1 & 0xFF) << 8));
    
    /* Blend operation with mask - expands to multiple instructions */
    __m512i blended = _mm512_mask_blend_epi64(mask, src1, src2);
    
    /* Extract and use result */
    uint64_t temp[8];
    _mm512_storeu_si512(temp, blended);
    for (int i = 0; i < 8; i++) {
        result += temp[i] & 0xFF;
    }
    
#elif defined(__ARM_FEATURE_SVE)
    /* SVE operation with multiple predicates and data */
    svuint64_t data1 = svdup_u64(a0);
    svuint64_t data2 = svdup_u64(a1);
    svuint64_t bases = svdup_u64(a2);
    svbool_t pred1 = svwhilelt_b64(0, 4);
    svbool_t pred2 = svwhilelt_b64(4, 8);
    
    /* Complex scatter with multiple predicates */
    uint64_t buffer[8];
    svst1_scatter_u64base_u64(pred1, buffer, bases, data1);
    svst1_scatter_u64base_u64(pred2, &buffer[4], bases, data2);
    
    for (int i = 0; i < 8; i++) {
        result += buffer[i] & 0xFF;
    }
    
#else
    /* Generic fallback: inline assembly with 11 operands */
    uint64_t r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10;
    
    asm volatile (
        /* Use all 11 input operands in various combinations */
        "mov %[out0], %[in0] \n\t"
        "add %[out0], %[in1] \n\t"
        "mov %[out1], %[in2] \n\t"
        "add %[out1], %[in3] \n\t"
        "mov %[out2], %[in4] \n\t"
        "add %[out2], %[in5] \n\t"
        "mov %[out3], %[in6] \n\t"
        "add %[out3], %[in7] \n\t"
        "mov %[out4], %[in8] \n\t"
        "add %[out4], %[in9] \n\t"
        "mov %[out5], %[in10] \n\t"
        /* Combine results */
        "add %[out5], %[out0] \n\t"
        "mov %[out6], %[out1] \n\t"
        "add %[out6], %[out2] \n\t"
        "mov %[out7], %[out3] \n\t"
        "add %[out7], %[out4] \n\t"
        "mov %[out8], %[out5] \n\t"
        "add %[out8], %[out6] \n\t"
        "mov %[out9], %[out7] \n\t"
        "add %[out9], %[out8] \n\t"
        "mov %[out10], %[out9] \n\t"
        "add %[out10], %[in0] \n\t"
        : [out0] "=&r" (r0), [out1] "=&r" (r1), [out2] "=&r" (r2),
          [out3] "=&r" (r3), [out4] "=&r" (r4), [out5] "=&r" (r5),
          [out6] "=&r" (r6), [out7] "=&r" (r7), [out8] "=&r" (r8),
          [out9] "=&r" (r9), [out10] "=&r" (r10)
        : [in0] "r" (a0), [in1] "r" (a1), [in2] "r" (a2), [in3] "r" (a3),
          [in4] "r" (a4), [in5] "r" (a5), [in6] "r" (a6), [in7] "r" (a7),
          [in8] "r" (a8), [in9] "r" (a9), [in10] "r" (a10)
        : "cc"
    );
    
    /* Combine all results */
    result = (int)((r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + r10) & 0xFFFF);
#endif
    
    return result;
}

int main(int argc, char *argv[]) {
    uint64_t args[11];
    int i;
    
    /* Initialize arguments with non-constant values */
    for (i = 0; i < 11; i++) {
        if (argc > i + 1) {
            args[i] = strtoull(argv[i + 1], NULL, 0);
        } else {
            args[i] = rand_u64();
        }
    }
    
    /* Call both functions with the appropriate number of arguments */
    int result1 = func_10_operands(args[0], args[1], args[2], args[3],
                                   args[4], args[5], args[6], args[7],
                                   args[8], args[9]);
    
    int result2 = func_11_operands(args[0], args[1], args[2], args[3],
                                   args[4], args[5], args[6], args[7],
                                   args[8], args[9], args[10]);
    
    /* Combine and print results to prevent optimization */
    int final_result = result1 + result2;
    printf("Result: %d (0x%x)\n", final_result, final_result);
    
    return final_result != 0 ? 0 : 1;
}
