/* Test program to cover 10/11-operand expansion in optabs.cc */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Simple PRNG to generate non-constant values */
static uint64_t simple_rand(uint64_t seed) {
    return seed * 6364136223846793005ULL + 1442695040888963407ULL;
}

/* Function using 10 operands */
int func_10_operands(uint64_t a0, uint64_t a1, uint64_t a2, uint64_t a3,
                     uint64_t a4, uint64_t a5, uint64_t a6, uint64_t a7,
                     uint64_t a8, uint64_t a9) {
    int result = 0;
    
    /* Architecture-specific implementations */
#if defined(__AVX512F__) && defined(__AVX512VL__)
    /* AVX-512 mask compress store with many operands when expanded */
    /* This intrinsic expands to multiple instructions with many operands */
    #include <immintrin.h>
    {
        volatile uint32_t mem[16] = {0};
        __mmask16 mask = (__mmask16)(a0 & 0xFFFF);
        __m512i data = _mm512_set_epi32(
            a1, a2, a3, a4, a5, a6, a7, a8,
            a9, a0, a1, a2, a3, a4, a5, a6
        );
        _mm512_mask_compressstoreu_epi32(mem, mask, data);
        result = mem[0] + mem[15];
    }
#elif defined(__ARM_FEATURE_SVE)
    /* ARM SVE scatter store with predicate and multiple registers */
    /* Note: Actual intrinsic names may vary */
    #include <arm_sve.h>
    {
        volatile uint64_t mem[16] = {0};
        svbool_t pg = svwhilelt_b64(a0, a0 + 16);
        svuint64_t bases = svdup_u64(a1);
        svuint64_t data = svdup_u64(a2);
        /* This would typically expand to many operands */
        asm volatile("" : : "r"(pg), "r"(bases), "r"(data), "m"(mem));
        result = mem[0];
    }
#else
    /* Generic fallback: inline assembly with 10 explicit operands */
    /* This forces the compiler to handle 10 rtx operands */
    uint64_t out0, out1, out2, out3, out4;
    asm volatile(
        "# 10-operand dummy operation\n\t"
        "mov %0, %5\n\t"
        "add %0, %6\n\t"
        "mov %1, %7\n\t"
        "add %1, %8\n\t"
        "mov %2, %9\n\t"
        "add %2, %10\n\t"
        "mov %3, %5\n\t"
        "sub %3, %6\n\t"
        "mov %4, %7\n\t"
        "xor %4, %8"
        : "=&r"(out0), "=&r"(out1), "=&r"(out2), "=&r"(out3), "=&r"(out4)
        : "r"(a0), "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(a5)
        : "cc"
    );
    result = (int)(out0 + out1 + out2 + out3 + out4);
#endif
    
    return result;
}

/* Function using 11 operands */
int func_11_operands(uint64_t a0, uint64_t a1, uint64_t a2, uint64_t a3,
                     uint64_t a4, uint64_t a5, uint64_t a6, uint64_t a7,
                     uint64_t a8, uint64_t a9, uint64_t a10) {
    int result = 0;
    
#if defined(__AVX512F__) && defined(__AVX512VBMI2__)
    /* AVX-512 VPCOMPRESSB with mask and multiple registers */
    #include <immintrin.h>
    {
        volatile char mem[64] = {0};
        __mmask64 mask = (__mmask64)(a0 & 0xFFFFFFFF);
        __m512i data = _mm512_set_epi8(
            a1 & 0xFF, a2 & 0xFF, a3 & 0xFF, a4 & 0xFF,
            a5 & 0xFF, a6 & 0xFF, a7 & 0xFF, a8 & 0xFF,
            a9 & 0xFF, a10 & 0xFF, a0 & 0xFF, a1 & 0xFF,
            a2 & 0xFF, a3 & 0xFF, a4 & 0xFF, a5 & 0xFF,
            a6 & 0xFF, a7 & 0xFF, a8 & 0xFF, a9 & 0xFF,
            a10 & 0xFF, a0 & 0xFF, a1 & 0xFF, a2 & 0xFF,
            a3 & 0xFF, a4 & 0xFF, a5 & 0xFF, a6 & 0xFF,
            a7 & 0xFF, a8 & 0xFF, a9 & 0xFF, a10 & 0xFF,
            a0 & 0xFF, a1 & 0xFF, a2 & 0xFF, a3 & 0xFF,
            a4 & 0xFF, a5 & 0xFF, a6 & 0xFF, a7 & 0xFF,
            a8 & 0xFF, a9 & 0xFF, a10 & 0xFF, a0 & 0xFF,
            a1 & 0xFF, a2 & 0xFF, a3 & 0xFF, a4 & 0xFF,
            a5 & 0xFF, a6 & 0xFF, a7 & 0xFF, a8 & 0xFF,
            a9 & 0xFF, a10 & 0xFF, a0 & 0xFF, a1 & 0xFF,
            a2 & 0xFF, a3 & 0xFF, a4 & 0xFF, a5 & 0xFF
        );
        _mm512_mask_compressstoreu_epi8(mem, mask, data);
        result = mem[0] + mem[63];
    }
#elif defined(__ARM_FEATURE_SVE2)
    /* ARM SVE2 complex gather load with multiple predicates */
    #include <arm_sve.h>
    {
        volatile uint64_t src[16] = {0};
        svbool_t pg1 = svwhilelt_b64(a0, a0 + 8);
        svbool_t pg2 = svwhilelt_b64(a1, a1 + 8);
        svuint64_t offsets = svdup_u64(a2);
        /* Complex operation that expands to many operands */
        asm volatile("" : : "r"(pg1), "r"(pg2), "r"(offsets), "m"(src));
        result = src[0];
    }
#else
    /* Generic fallback: inline assembly with 11 explicit operands */
    uint64_t out0, out1, out2, out3, out4, out5;
    asm volatile(
        "# 11-operand dummy operation\n\t"
        "mov %0, %6\n\t"
        "add %0, %7\n\t"
        "mov %1, %8\n\t"
        "add %1, %9\n\t"
        "mov %2, %10\n\t"
        "add %2, %11\n\t"
        "mov %3, %6\n\t"
        "sub %3, %7\n\t"
        "mov %4, %8\n\t"
        "xor %4, %9\n\t"
        "mov %5, %10\n\t"
        "or %5, %11"
        : "=&r"(out0), "=&r"(out1), "=&r"(out2), 
          "=&r"(out3), "=&r"(out4), "=&r"(out5)
        : "r"(a0), "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(a5)
        : "cc"
    );
    result = (int)(out0 + out1 + out2 + out3 + out4 + out5);
#endif
    
    return result;
}

int main(int argc, char *argv[]) {
    uint64_t vals[11];
    uint64_t seed = 0x123456789ABCDEF0ULL;
    
    /* Initialize with non-constant values */
    for (int i = 0; i < 11; i++) {
        if (argc > i + 1) {
            vals[i] = strtoull(argv[i + 1], NULL, 0);
        } else {
            seed = simple_rand(seed + i);
            vals[i] = seed;
        }
    }
    
    /* Call both functions with dependency chain */
    int res1 = func_10_operands(
        vals[0], vals[1], vals[2], vals[3], vals[4],
        vals[5], vals[6], vals[7], vals[8], vals[9]
    );
    
    int res2 = func_11_operands(
        vals[0], vals[1], vals[2], vals[3], vals[4],
        vals[5], vals[6], vals[7], vals[8], vals[9], vals[10]
    );
    
    /* Combine results to prevent elimination */
    int final_result = res1 + res2;
    
    /* Use result in non-eliminable way */
    printf("Result: %d (0x%x)\n", final_result, final_result);
    
    return final_result != 0 ? 0 : 1;
}
