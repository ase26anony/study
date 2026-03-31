/* Test program to cover 10/11 operand expansion in optabs.cc */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Simple PRNG to generate non-constant values */
static unsigned int seed = 12345;
static unsigned int prng(void) {
    seed = seed * 1103515245 + 12345;
    return seed;
}

/* Function to trigger 10-operand expansion */
int func_10_operands(uint64_t a0, uint64_t a1, uint64_t a2, uint64_t a3,
                     uint64_t a4, uint64_t a5, uint64_t a6, uint64_t a7,
                     uint64_t a8, uint64_t a9) {
    int result = 0;
    
#if defined(__AVX512F__) && defined(__AVX512VL__)
    /* AVX-512 intrinsic that can expand to many operands */
    /* Using masked gather operation which has many parameters */
    __m512i index = _mm512_set_epi64(a9, a8, a7, a6, a5, a4, a3, a2);
    __m512i src = _mm512_set_epi64(a1, a0, a3, a2, a5, a4, a7, a6);
    __mmask8 mask = (__mmask8)((a0 ^ a1 ^ a2 ^ a3 ^ a4) & 0xFF);
    void* base = (void*)(uintptr_t)a0;
    
    /* This intrinsic expands to many operands during RTL generation */
    __m512i gathered = _mm512_mask_i64gather_epi64(src, mask, index, base, 1);
    
    /* Use the result to prevent optimization */
    long long* ptr = (long long*)&gathered;
    for (int i = 0; i < 8; i++) {
        result += (int)(ptr[i] & 0xFFFFFFFF);
    }
    
#elif defined(__ARM_FEATURE_SVE)
    /* ARM SVE scatter store with predicate - many operands */
    /* Note: Actual SVE intrinsics require specific types */
    svbool_t pg = svwhilelt_b8(0, 8);
    svuint64_t data = svdup_u64(a0);
    svuint64_t bases = svdup_u64(a1);
    
    /* Complex SVE operation that may expand to many operands */
    /* This is a placeholder - actual SVE code would be more complex */
    svst1_scatter_u64base_u64(pg, bases, data);
    
    result = (int)(a0 + a1 + a2 + a3 + a4);
    
#else
    /* Generic inline assembly with 10 operands */
    /* Forces the compiler to handle 10 rtx operands */
    uint64_t out0, out1, out2;
    
    __asm__ volatile (
        /* Dummy operations that use all 10 input operands */
        "mov %0, %3\n\t"
        "add %0, %0, %4\n\t"
        "add %0, %0, %5\n\t"
        "mov %1, %6\n\t"
        "add %1, %1, %7\n\t"
        "mov %2, %8\n\t"
        "add %2, %2, %9\n\t"
        "add %0, %0, %1\n\t"
        "add %0, %0, %2\n\t"
        : "=r" (out0), "=r" (out1), "=r" (out2)
        : "r" (a0), "r" (a1), "r" (a2), "r" (a3), 
          "r" (a4), "r" (a5), "r" (a6), "r" (a7)
        : "cc"
    );
    
    /* Use all results to prevent dead code elimination */
    result = (int)((out0 + out1 + out2 + a8 + a9) & 0x7FFFFFFF);
#endif
    
    return result;
}

/* Function to trigger 11-operand expansion */
int func_11_operands(uint64_t a0, uint64_t a1, uint64_t a2, uint64_t a3,
                     uint64_t a4, uint64_t a5, uint64_t a6, uint64_t a7,
                     uint64_t a8, uint64_t a9, uint64_t a10) {
    int result = 0;
    
#if defined(__AVX512F__) && defined(__AVX512VL__) && defined(__AVX512BW__)
    /* AVX-512 masked compress store - can use many operands */
    char buffer[64] __attribute__((aligned(64)));
    __m512i data = _mm512_set_epi64(a10, a9, a8, a7, a6, a5, a4, a3);
    __mmask8 mask = (__mmask8)((a0 ^ a1 ^ a2) & 0xFF);
    
    /* This intrinsic needs address, mask, and data - expands to many operands */
    _mm512_mask_compressstoreu_epi64(buffer, mask, data);
    
    /* Use the stored data */
    uint64_t* ptr = (uint64_t*)buffer;
    for (int i = 0; i < 8; i++) {
        result += (int)(ptr[i] & 0xFFFF);
    }
    result += (int)(a0 + a1 + a2);
    
#elif defined(__ARM_FEATURE_SVE)
    /* ARM SVE gather with predicate and offset - many operands */
    svbool_t pg = svwhilelt_b8(0, 8);
    svuint64_t offsets = svdup_u64(a1);
    svuint64_t data = svdup_u64(a2);
    
    /* Complex SVE gather operation */
    /* Placeholder for actual SVE code */
    svuint64_t gathered = svld1_gather_u64offset_u64(pg, (const uint64_t*)(uintptr_t)a0, offsets);
    
    result = (int)(a0 + a1 + a2 + a3 + a4 + a5);
    
#else
    /* Generic inline assembly with 11 operands */
    uint64_t out0, out1, out2, out3;
    
    __asm__ volatile (
        /* Use all 11 input operands in various combinations */
        "mov %0, %4\n\t"
        "add %0, %0, %5\n\t"
        "mov %1, %6\n\t"
        "add %1, %1, %7\n\t"
        "mov %2, %8\n\t"
        "add %2, %2, %9\n\t"
        "mov %3, %10\n\t"
        "add %3, %3, %11\n\t"
        "add %0, %0, %1\n\t"
        "add %2, %2, %3\n\t"
        "add %0, %0, %2\n\t"
        : "=r" (out0), "=r" (out1), "=r" (out2), "=r" (out3)
        : "r" (a0), "r" (a1), "r" (a2), "r" (a3),
          "r" (a4), "r" (a5), "r" (a6), "r" (a7),
          "r" (a8), "r" (a9)
        : "cc"
    );
    
    /* Combine results with the 11th operand */
    result = (int)((out0 + out1 + out2 + out3 + a10) & 0x7FFFFFFF);
#endif
    
    return result;
}

int main(int argc, char *argv[]) {
    /* Generate 12 non-constant values using argv and PRNG */
    uint64_t vals[12];
    
    /* Use argv indices for some variability */
    for (int i = 0; i < 12; i++) {
        if (i < argc) {
            vals[i] = (uint64_t)(argv[i][0] * (i + 1));
        } else {
            vals[i] = prng();
        }
        /* Mix in some arithmetic to prevent constant folding */
        vals[i] = vals[i] ^ (prng() << 16);
    }
    
    /* Call both functions to trigger different expansion paths */
    int result1 = func_10_operands(vals[0], vals[1], vals[2], vals[3],
                                   vals[4], vals[5], vals[6], vals[7],
                                   vals[8], vals[9]);
    
    int result2 = func_11_operands(vals[0], vals[1], vals[2], vals[3],
                                   vals[4], vals[5], vals[6], vals[7],
                                   vals[8], vals[9], vals[10]);
    
    /* Use results in a non-removable way */
    int final_result = result1 + result2 + (int)vals[11];
    
    printf("Result: %d\n", final_result);
    
    /* Return value depends on computation */
    return (final_result > 1000) ? 0 : 1;
}
