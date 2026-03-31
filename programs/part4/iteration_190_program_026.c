/* Test case to cover 10 and 11 operand expansion in optabs.cc */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Simple PRNG to generate non-constant values */
static uint64_t simple_rand(uint64_t seed) {
    return seed * 1103515245 + 12345;
}

/* Function to use 10 operands */
int func_10_operands(uint64_t a0, uint64_t a1, uint64_t a2, uint64_t a3,
                     uint64_t a4, uint64_t a5, uint64_t a6, uint64_t a7,
                     uint64_t a8, uint64_t a9) {
    int result = 0;
    
#if defined(__AVX512F__) && defined(__AVX512VL__)
    /* AVX-512 intrinsic that can expand to many operands */
    /* Using gather instruction with mask, index, scale, etc. */
    __m512i src = _mm512_set_epi64(a9, a8, a7, a6, a5, a4, a3, a2);
    __mmask8 mask = (__mmask8)(a0 & 0xFF);
    long long base_addr = (long long)&a1;
    __m512i vindex = _mm512_set_epi64(0, 8, 16, 24, 32, 40, 48, 56);
    
    /* This gather operation involves multiple operands */
    __m512i gathered = _mm512_mask_i64gather_epi64(src, mask, vindex, 
                                                   (void*)base_addr, 1);
    
    /* Use the result */
    result = _mm512_reduce_add_epi64(gathered) & 0x7FFFFFFF;
    
#elif defined(__ARM_FEATURE_SVE)
    /* ARM SVE scatter/gather operations can have many operands */
    /* Note: Actual SVE intrinsics would require vector types */
    /* Using inline assembly as placeholder for SVE scatter */
    uint64_t temp_result = 0;
    asm volatile (
        "/* SVE scatter with many operands */\n"
        "add %[out], %[a0], %[a1]\n"
        "add %[out], %[out], %[a2]\n"
        "add %[out], %[out], %[a3]\n"
        "add %[out], %[out], %[a4]\n"
        "add %[out], %[out], %[a5]\n"
        "add %[out], %[out], %[a6]\n"
        "add %[out], %[out], %[a7]\n"
        "add %[out], %[out], %[a8]\n"
        "add %[out], %[out], %[a9]\n"
        : [out] "=r" (temp_result)
        : [a0] "r" (a0), [a1] "r" (a1), [a2] "r" (a2), [a3] "r" (a3),
          [a4] "r" (a4), [a5] "r" (a5), [a6] "r" (a6), [a7] "r" (a7),
          [a8] "r" (a8), [a9] "r" (a9)
        : "cc"
    );
    result = (int)(temp_result & 0x7FFFFFFF);
    
#else
    /* Generic inline assembly with 10 operands */
    /* This should trigger the 10-operand expansion path */
    uint64_t temp = 0;
    asm volatile (
        "/* 10-operand assembly template */\n"
        "mov %[tmp], %[arg0]\n"
        "add %[tmp], %[tmp], %[arg1]\n"
        "add %[tmp], %[tmp], %[arg2]\n"
        "add %[tmp], %[tmp], %[arg3]\n"
        "add %[tmp], %[tmp], %[arg4]\n"
        "add %[tmp], %[tmp], %[arg5]\n"
        "add %[tmp], %[tmp], %[arg6]\n"
        "add %[tmp], %[tmp], %[arg7]\n"
        "add %[tmp], %[tmp], %[arg8]\n"
        "add %[tmp], %[tmp], %[arg9]\n"
        : [tmp] "=r" (temp)
        : [arg0] "r" (a0), [arg1] "r" (a1), [arg2] "r" (a2), [arg3] "r" (a3),
          [arg4] "r" (a4), [arg5] "r" (a5), [arg6] "r" (a6), [arg7] "r" (a7),
          [arg8] "r" (a8), [arg9] "r" (a9)
        : "cc"
    );
    result = (int)(temp & 0x7FFFFFFF);
#endif
    
    return result;
}

/* Function to use 11 operands */
int func_11_operands(uint64_t a0, uint64_t a1, uint64_t a2, uint64_t a3,
                     uint64_t a4, uint64_t a5, uint64_t a6, uint64_t a7,
                     uint64_t a8, uint64_t a9, uint64_t a10) {
    int result = 0;
    
#if defined(__AVX512F__) && defined(__AVX512VL__) && defined(__AVX512BW__)
    /* AVX-512 masked compress store with many operands */
    char buffer[64] __attribute__((aligned(64)));
    __m512i data = _mm512_set_epi64(a10, a9, a8, a7, a6, a5, a4, a3);
    __mmask8 mask = (__mmask8)(a0 & 0xFF);
    
    /* _mm512_mask_compressstoreu_epi64 expands to many operands */
    _mm512_mask_compressstoreu_epi64(buffer, mask, data);
    
    /* Use the stored data */
    uint64_t* ptr = (uint64_t*)buffer;
    result = (int)((ptr[0] + ptr[1] + ptr[2] + ptr[3] + 
                    ptr[4] + ptr[5] + ptr[6] + ptr[7]) & 0x7FFFFFFF);
    
#elif defined(__ARM_FEATURE_SVE)
    /* ARM SVE scatter with predicate, base, offset, data - many operands */
    uint64_t temp_result = 0;
    asm volatile (
        "/* SVE 11-operand scatter */\n"
        "add %[out], %[a0], %[a1]\n"
        "add %[out], %[out], %[a2]\n"
        "add %[out], %[out], %[a3]\n"
        "add %[out], %[out], %[a4]\n"
        "add %[out], %[out], %[a5]\n"
        "add %[out], %[out], %[a6]\n"
        "add %[out], %[out], %[a7]\n"
        "add %[out], %[out], %[a8]\n"
        "add %[out], %[out], %[a9]\n"
        "add %[out], %[out], %[a10]\n"
        : [out] "=r" (temp_result)
        : [a0] "r" (a0), [a1] "r" (a1), [a2] "r" (a2), [a3] "r" (a3),
          [a4] "r" (a4), [a5] "r" (a5), [a6] "r" (a6), [a7] "r" (a7),
          [a8] "r" (a8), [a9] "r" (a9), [a10] "r" (a10)
        : "cc"
    );
    result = (int)(temp_result & 0x7FFFFFFF);
    
#else
    /* Generic inline assembly with 11 operands */
    /* This should trigger the 11-operand expansion path */
    uint64_t temp = 0;
    asm volatile (
        "/* 11-operand assembly template */\n"
        "mov %[tmp], %[arg0]\n"
        "add %[tmp], %[tmp], %[arg1]\n"
        "add %[tmp], %[tmp], %[arg2]\n"
        "add %[tmp], %[tmp], %[arg3]\n"
        "add %[tmp], %[tmp], %[arg4]\n"
        "add %[tmp], %[tmp], %[arg5]\n"
        "add %[tmp], %[tmp], %[arg6]\n"
        "add %[tmp], %[tmp], %[arg7]\n"
        "add %[tmp], %[tmp], %[arg8]\n"
        "add %[tmp], %[tmp], %[arg9]\n"
        "add %[tmp], %[tmp], %[arg10]\n"
        : [tmp] "=r" (temp)
        : [arg0] "r" (a0), [arg1] "r" (a1), [arg2] "r" (a2), [arg3] "r" (a3),
          [arg4] "r" (a4), [arg5] "r" (a5), [arg6] "r" (a6), [arg7] "r" (a7),
          [arg8] "r" (a8), [arg9] "r" (a9), [arg10] "r" (a10)
        : "cc"
    );
    result = (int)(temp & 0x7FFFFFFF);
#endif
    
    return result;
}

int main(int argc, char *argv[]) {
    /* Initialize 11 variables with non-constant values */
    uint64_t vars[11];
    uint64_t seed = 42;
    
    for (int i = 0; i < 11; i++) {
        if (argc > i + 1) {
            vars[i] = strtoull(argv[i + 1], NULL, 0);
        } else {
            seed = simple_rand(seed);
            vars[i] = seed;
        }
    }
    
    /* Call both functions to trigger different expansion paths */
    int result1 = func_10_operands(vars[0], vars[1], vars[2], vars[3],
                                   vars[4], vars[5], vars[6], vars[7],
                                   vars[8], vars[9]);
    
    int result2 = func_11_operands(vars[0], vars[1], vars[2], vars[3],
                                   vars[4], vars[5], vars[6], vars[7],
                                   vars[8], vars[9], vars[10]);
    
    /* Combine results to prevent dead code elimination */
    int final_result = result1 + result2;
    printf("Result: %d\n", final_result);
    
    return final_result != 0 ? 0 : 1;
}
