#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Simple PRNG to generate non-constant values */
static uint64_t simple_rand(uint64_t seed) {
    return seed * 1103515245 + 12345;
}

/* Function to trigger 10-operand expansion */
int func_10_operands(uint64_t a0, uint64_t a1, uint64_t a2, uint64_t a3,
                     uint64_t a4, uint64_t a5, uint64_t a6, uint64_t a7,
                     uint64_t a8, uint64_t a9) {
    int result = 0;
    
#if defined(__AVX512F__) && defined(__AVX512VL__)
    /* AVX-512 intrinsic that can expand to many operands */
    #include <immintrin.h>
    __m512i v0 = _mm512_set_epi64(a9, a8, a7, a6, a5, a4, a3, a2);
    __m512i v1 = _mm512_set_epi64(a1, a0, a9, a8, a7, a6, a5, a4);
    __mmask8 mask = (__mmask8)((a0 ^ a1 ^ a2 ^ a3) & 0xFF);
    char* addr = (char*)(uintptr_t)a0;
    
    /* _mm512_mask_compressstoreu_epi32 expands to many operands */
    _mm512_mask_compressstoreu_epi32(addr, mask, v0);
    
    /* Use result to prevent optimization */
    result = _mm512_reduce_add_epi64(v0) + _mm512_reduce_add_epi64(v1);
    
#elif defined(__ARM_FEATURE_SVE)
    /* ARM SVE intrinsic with many operands */
    #include <arm_sve.h>
    svuint64_t data = svdup_u64(a0);
    svuint64_t bases = svdup_u64(a1);
    svbool_t pred = svwhilelt_b64(0, 8);
    
    /* Complex scatter operation with multiple operands */
    svstnt1_scatter_u64base_u64(pred, (uint64_t*)a2, bases, data);
    
    result = (int)(a0 + a1 + a2);
    
#else
    /* Generic inline assembly with 10 explicit operands */
    /* This should trigger the 10-operand expansion path */
    uint64_t out0, out1, out2, out3;
    
    asm volatile (
        "/* 10-operand dummy assembly */\n\t"
        "mov %0, %4\n\t"
        "add %0, %5\n\t"
        "mov %1, %6\n\t"
        "add %1, %7\n\t"
        "mov %2, %8\n\t"
        "add %2, %9\n\t"
        "mov %3, %10\n\t"
        "add %3, %4\n\t"
        : "=r"(out0), "=r"(out1), "=r"(out2), "=r"(out3)
        : "r"(a0), "r"(a1), "r"(a2), "r"(a3), "r"(a4), 
          "r"(a5), "r"(a6)
        : "cc"
    );
    
    result = (int)(out0 + out1 + out2 + out3 + a7 + a8 + a9);
#endif
    
    return result;
}

/* Function to trigger 11-operand expansion */
int func_11_operands(uint64_t a0, uint64_t a1, uint64_t a2, uint64_t a3,
                     uint64_t a4, uint64_t a5, uint64_t a6, uint64_t a7,
                     uint64_t a8, uint64_t a9, uint64_t a10) {
    int result = 0;
    
#if defined(__AVX512F__) && defined(__AVX512VBMI2__)
    /* AVX-512 VBMI2 has instructions with many operands */
    #include <immintrin.h>
    __m512i v0 = _mm512_set_epi64(a10, a9, a8, a7, a6, a5, a4, a3);
    __m512i v1 = _mm512_set_epi64(a2, a1, a0, a10, a9, a8, a7, a6);
    __m512i v2 = _mm512_set_epi64(a5, a4, a3, a2, a1, a0, a10, a9);
    __mmask16 mask1 = (__mmask16)((a0 ^ a1 ^ a2) & 0xFFFF);
    __mmask16 mask2 = (__mmask16)((a3 ^ a4 ^ a5) & 0xFFFF);
    
    /* Complex permute operation - expands to many operands */
    __m512i res = _mm512_mask2_permutex2var_epi64(v0, v1, mask1, v2);
    res = _mm512_maskz_compress_epi64(mask2, res);
    
    result = _mm512_reduce_add_epi64(res);
    
#elif defined(__ARM_FEATURE_SVE2)
    /* ARM SVE2 scatter with 3 vectors and predicate */
    #include <arm_sve.h>
    svuint64_t data1 = svdup_u64(a0);
    svuint64_t data2 = svdup_u64(a1);
    svuint64_t bases1 = svdup_u64(a2);
    svuint64_t bases2 = svdup_u64(a3);
    svbool_t pred1 = svwhilelt_b64(0, 4);
    svbool_t pred2 = svwhilelt_b64(4, 8);
    
    /* Multiple scatter operations combined */
    svstnt1_scatter_u64base_u64(pred1, (uint64_t*)a4, bases1, data1);
    svstnt1_scatter_u64base_u64(pred2, (uint64_t*)a5, bases2, data2);
    
    result = (int)(a0 + a1 + a2 + a3 + a4 + a5);
    
#else
    /* Generic inline assembly with 11 explicit operands */
    uint64_t out0, out1, out2, out3, out4;
    
    asm volatile (
        "/* 11-operand dummy assembly */\n\t"
        "mov %0, %5\n\t"
        "add %0, %6\n\t"
        "mov %1, %7\n\t"
        "add %1, %8\n\t"
        "mov %2, %9\n\t"
        "add %2, %10\n\t"
        "mov %3, %11\n\t"
        "add %3, %5\n\t"
        "mov %4, %6\n\t"
        "add %4, %7\n\t"
        : "=r"(out0), "=r"(out1), "=r"(out2), "=r"(out3), "=r"(out4)
        : "r"(a0), "r"(a1), "r"(a2), "r"(a3), "r"(a4), 
          "r"(a5), "r"(a6), "r"(a7)
        : "cc"
    );
    
    result = (int)(out0 + out1 + out2 + out3 + out4 + a8 + a9 + a10);
#endif
    
    return result;
}

int main(int argc, char *argv[]) {
    uint64_t vals[12];
    uint64_t seed = 0x12345678;
    
    /* Initialize with non-constant values */
    for (int i = 0; i < 12; i++) {
        if (argc > i + 1) {
            vals[i] = strtoull(argv[i + 1], NULL, 0);
        } else {
            seed = simple_rand(seed);
            vals[i] = seed ^ (i * 0x987654321);
        }
    }
    
    /* Call both functions to trigger different expansion paths */
    int res1 = func_10_operands(vals[0], vals[1], vals[2], vals[3],
                                vals[4], vals[5], vals[6], vals[7],
                                vals[8], vals[9]);
    
    int res2 = func_11_operands(vals[0], vals[1], vals[2], vals[3],
                                vals[4], vals[5], vals[6], vals[7],
                                vals[8], vals[9], vals[10]);
    
    /* Combine results to prevent dead code elimination */
    int final_result = res1 + res2;
    
    printf("Result: %d (0x%x)\n", final_result, final_result);
    
    return final_result != 0 ? 0 : 1;
}
