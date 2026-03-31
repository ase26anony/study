/* Test program to cover 10/11 operand expansion in optabs.cc */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Simple PRNG to generate non-constant values */
static uint64_t simple_rand(uint64_t seed) {
    return seed * 6364136223846793005ULL + 1442695040888963407ULL;
}

/* Function to use 10 operands */
int func_10_operands(uint64_t a0, uint64_t a1, uint64_t a2, uint64_t a3,
                     uint64_t a4, uint64_t a5, uint64_t a6, uint64_t a7,
                     uint64_t a8, uint64_t a9) {
    int result = 0;
    
    /* Architecture-specific implementations */
#if defined(__AVX512F__) && defined(__AVX512VL__)
    /* AVX-512 mask compress store intrinsic - expands to many operands */
    #include <immintrin.h>
    {
        volatile uint32_t mem[16] = {0};
        __mmask16 mask = (__mmask16)((a0 ^ a1) & 0xFFFF);
        __m512i data = _mm512_set_epi32(
            a2 & 0xFFFFFFFF, a3 & 0xFFFFFFFF, a4 & 0xFFFFFFFF, a5 & 0xFFFFFFFF,
            a6 & 0xFFFFFFFF, a7 & 0xFFFFFFFF, a8 & 0xFFFFFFFF, a9 & 0xFFFFFFFF,
            (a2>>32) & 0xFFFFFFFF, (a3>>32) & 0xFFFFFFFF, (a4>>32) & 0xFFFFFFFF,
            (a5>>32) & 0xFFFFFFFF, (a6>>32) & 0xFFFFFFFF, (a7>>32) & 0xFFFFFFFF,
            (a8>>32) & 0xFFFFFFFF, (a9>>32) & 0xFFFFFFFF);
        
        _mm512_mask_compressstoreu_epi32(mem, mask, data);
        
        /* Use result to prevent optimization */
        for (int i = 0; i < 16; i++) {
            result += mem[i];
        }
    }
#elif defined(__ARM_FEATURE_SVE)
    /* ARM SVE scatter store - may expand to many operands */
    #include <arm_sve.h>
    {
        volatile uint64_t mem[16] = {0};
        svbool_t pg = svwhilelt_b64(0, 16);
        svuint64_t bases = svdup_u64(0);
        svuint64_t data = svdup_u64(a0);
        
        /* Complex SVE operation with many operands */
        svst1_scatter_u64base_u64(pg, mem, bases, data);
        
        result = (int)(a0 + a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9);
    }
#else
    /* Generic fallback: inline assembly with 10 explicit operands */
    uint64_t out0, out1, out2, out3, out4;
    
    asm volatile (
        /* Dummy operation that uses all 10 input operands */
        "mov %0, %5\n\t"
        "add %0, %0, %6\n\t"
        "mov %1, %7\n\t"
        "add %1, %1, %8\n\t"
        "mov %2, %9\n\t"
        "add %2, %2, %10\n\t"
        "mov %3, %11\n\t"
        "add %3, %3, %12\n\t"
        "mov %4, %13\n\t"
        "add %4, %4, %14\n\t"
        : "=r"(out0), "=r"(out1), "=r"(out2), "=r"(out3), "=r"(out4)
        : "r"(a0), "r"(a1), "r"(a2), "r"(a3), "r"(a4),
          "r"(a5), "r"(a6), "r"(a7), "r"(a8), "r"(a9)
        : "cc"
    );
    
    /* Combine results to prevent dead code elimination */
    result = (int)(out0 + out1 + out2 + out3 + out4);
#endif
    
    return result;
}

/* Function to use 11 operands */
int func_11_operands(uint64_t a0, uint64_t a1, uint64_t a2, uint64_t a3,
                     uint64_t a4, uint64_t a5, uint64_t a6, uint64_t a7,
                     uint64_t a8, uint64_t a9, uint64_t a10) {
    int result = 0;
    
#if defined(__AVX512F__) && defined(__AVX512VBMI2__)
    /* AVX-512 VPCOMPRESS with mask - may use many operands */
    #include <immintrin.h>
    {
        __mmask16 mask = (__mmask16)((a0 ^ a1) & 0xFFFF);
        __m512i src = _mm512_set_epi64(a2, a3, a4, a5, a6, a7, a8, a9);
        __m512i dst = _mm512_setzero_si512();
        
        dst = _mm512_mask_compress_epi64(dst, mask, src);
        
        /* Extract and sum elements */
        uint64_t temp[8];
        _mm512_storeu_si512(temp, dst);
        for (int i = 0; i < 8; i++) {
            result += (int)(temp[i] & 0xFFFFFFFF);
        }
    }
#elif defined(__ARM_FEATURE_SVE)
    /* ARM SVE with 11 operands */
    #include <arm_sve.h>
    {
        svuint64_t x = svdup_u64(a0);
        svuint64_t y = svdup_u64(a1);
        svuint64_t z = svdup_u64(a2);
        
        /* Complex SVE operation chain */
        svuint64_t r = svadd_u64_x(svptrue_b64(), x, y);
        r = svadd_u64_x(svptrue_b64(), r, z);
        
        uint64_t temp;
        svst1(svptrue_b64(), &temp, r);
        result = (int)temp;
    }
#else
    /* Generic fallback: inline assembly with 11 explicit operands */
    uint64_t out0, out1, out2, out3, out4, out5;
    
    asm volatile (
        /* Dummy operations using all 11 input operands */
        "mov %0, %6\n\t"
        "add %0, %0, %7\n\t"
        "mov %1, %8\n\t"
        "add %1, %1, %9\n\t"
        "mov %2, %10\n\t"
        "add %2, %2, %11\n\t"
        "mov %3, %12\n\t"
        "add %3, %3, %13\n\t"
        "mov %4, %14\n\t"
        "add %4, %4, %15\n\t"
        "mov %5, %16\n\t"
        "add %5, %5, %6\n\t"  /* Reuse a0 */
        : "=r"(out0), "=r"(out1), "=r"(out2), "=r"(out3), 
          "=r"(out4), "=r"(out5)
        : "r"(a0), "r"(a1), "r"(a2), "r"(a3), "r"(a4),
          "r"(a5), "r"(a6), "r"(a7), "r"(a8), "r"(a9),
          "r"(a10)
        : "cc"
    );
    
    /* Combine results */
    result = (int)(out0 + out1 + out2 + out3 + out4 + out5);
#endif
    
    return result + (int)a10;  /* Ensure a10 is used */
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
    
    /* Call both functions with many operands */
    int res1 = func_10_operands(vals[0], vals[1], vals[2], vals[3],
                                vals[4], vals[5], vals[6], vals[7],
                                vals[8], vals[9]);
    
    int res2 = func_11_operands(vals[0], vals[1], vals[2], vals[3],
                                vals[4], vals[5], vals[6], vals[7],
                                vals[8], vals[9], vals[10]);
    
    /* Combine and print results to prevent optimization */
    int final_result = res1 + res2;
    printf("Result: %d (0x%x)\n", final_result, final_result);
    
    return final_result != 0 ? 0 : 1;
}
