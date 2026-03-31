/* Test program for GCC optabs.cc 10/11 operand expansion coverage */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Simple PRNG to generate non-constant values */
static uint64_t simple_rand(uint64_t seed) {
    return seed * 6364136223846793005ULL + 1442695040888963407ULL;
}

/* Function using 10 operands */
int func_10_operands(uint64_t a0, uint64_t a1, uint64_t a2, uint64_t a3,
                     uint64_t a4, uint64_t a5, uint64_t a6, uint64_t a7,
                     uint64_t a8, uint64_t a9) {
    int result = 0;
    
    /* Architecture-specific 10-operand paths */
#if defined(__AVX512F__) && defined(__AVX512VL__)
    /* AVX-512 mask compress store with many operands */
    #include <immintrin.h>
    __m512i data = _mm512_set_epi64(a9, a8, a7, a6, a5, a4, a3, a2);
    __mmask8 mask = (__mmask8)((a0 & 0xFF) | ((a1 & 0xFF) << 8));
    char buffer[64] __attribute__((aligned(64)));
    
    /* This intrinsic expands to many operands during RTL generation */
    _mm512_mask_compressstoreu_epi64(buffer, mask, data);
    
    /* Use the result */
    for (int i = 0; i < 8; i++) {
        result += ((uint64_t*)buffer)[i];
    }
    
#elif defined(__ARM_FEATURE_SVE)
    /* ARM SVE scatter store with predicate and base addressing */
    #include <arm_sve.h>
    svuint64_t data = svdup_u64(a0);
    svuint64_t bases = svdup_u64(a1);
    svbool_t pred = svwhilelt_b64(a2, a3);
    
    /* Complex SVE operation that may expand to many operands */
    svstnt1_scatter_u64base_u64(pred, bases, data);
    
    result = (int)(a0 + a1 + a2 + a3);
    
#else
    /* Generic fallback: inline assembly with 10 explicit operands */
    uint64_t out0, out1, out2, out3, out4;
    
    asm volatile (
        /* Dummy multi-operand template */
        "mov %0, %5\n\t"
        "add %0, %6\n\t"
        "mov %1, %7\n\t"
        "add %1, %8\n\t"
        "mov %2, %9\n\t"
        "add %2, %10\n\t"
        "mov %3, %11\n\t"
        "add %3, %12\n\t"
        "mov %4, %13\n\t"
        "add %4, %14"
        : "=r"(out0), "=r"(out1), "=r"(out2), "=r"(out3), "=r"(out4)
        : "r"(a0), "r"(a1), "r"(a2), "r"(a3), "r"(a4),
          "r"(a5), "r"(a6), "r"(a7), "r"(a8), "r"(a9)
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
    
    /* Architecture-specific 11-operand paths */
#if defined(__AVX512F__) && defined(__AVX512VBMI2__)
    /* AVX-512 compress/expand with mask and multiple data sources */
    #include <immintrin.h>
    __m512i data1 = _mm512_set_epi64(a10, a9, a8, a7, a6, a5, a4, a3);
    __m512i data2 = _mm512_set_epi64(a2, a1, a0, a10, a9, a8, a7, a6);
    __mmask16 mask = (__mmask16)((a0 & 0xFFFF) | ((a1 & 0xFFFF) << 16));
    
    /* Complex operation that may need many operands */
    __m512i compressed = _mm512_mask_compress_epi64(data1, mask, data2);
    
    /* Extract and sum elements */
    uint64_t temp[8];
    _mm512_storeu_epi64(temp, compressed);
    for (int i = 0; i < 8; i++) {
        result += (int)temp[i];
    }
    
#elif defined(__ARM_FEATURE_SVE2)
    /* ARM SVE2 complex gather with predicate and multiple offsets */
    #include <arm_sve.h>
    svuint64_t data = svdup_u64(a0);
    svuint64_t bases = svdup_u64(a1);
    svuint64_t offsets = svdup_u64(a2);
    svbool_t pred = svwhilelt_b64(a3, a4);
    
    /* SVE2 gather with predicate - may expand to many operands */
    svuint64_t gathered = svldnt1_gather_u64offset_u64(pred, bases, offsets);
    
    result = (int)(a0 + a1 + a2 + a3 + a4);
    
#else
    /* Generic fallback: inline assembly with 11 explicit operands */
    uint64_t out0, out1, out2, out3, out4, out5;
    
    asm volatile (
        /* Dummy multi-operand template */
        "mov %0, %6\n\t"
        "add %0, %7\n\t"
        "mov %1, %8\n\t"
        "add %1, %9\n\t"
        "mov %2, %10\n\t"
        "add %2, %11\n\t"
        "mov %3, %12\n\t"
        "add %3, %13\n\t"
        "mov %4, %14\n\t"
        "add %4, %15\n\t"
        "mov %5, %16\n\t"
        "add %5, %17"
        : "=r"(out0), "=r"(out1), "=r"(out2), "=r"(out3), 
          "=r"(out4), "=r"(out5)
        : "r"(a0), "r"(a1), "r"(a2), "r"(a3), "r"(a4),
          "r"(a5), "r"(a6), "r"(a7), "r"(a8), "r"(a9),
          "r"(a10), "r"(a0)  /* Reuse a0 as last operand */
        : "cc"
    );
    
    result = (int)(out0 + out1 + out2 + out3 + out4 + out5);
#endif
    
    return result;
}

int main(int argc, char *argv[]) {
    uint64_t vals[12];
    uint64_t seed = 0x123456789ABCDEFULL;
    
    /* Initialize with non-constant values */
    for (int i = 0; i < 12; i++) {
        seed = simple_rand(seed);
        /* Mix in argv for additional variability */
        if (i < argc) {
            seed ^= (uint64_t)argv[i];
        }
        vals[i] = seed;
    }
    
    /* Call both functions with many operands */
    int result1 = func_10_operands(vals[0], vals[1], vals[2], vals[3],
                                   vals[4], vals[5], vals[6], vals[7],
                                   vals[8], vals[9]);
    
    int result2 = func_11_operands(vals[0], vals[1], vals[2], vals[3],
                                   vals[4], vals[5], vals[6], vals[7],
                                   vals[8], vals[9], vals[10]);
    
    /* Combine results to prevent optimization */
    int final_result = result1 + result2;
    
    /* Use the result */
    printf("Result: %d\n", final_result);
    
    /* Additional computation to ensure all values are used */
    uint64_t checksum = 0;
    for (int i = 0; i < 12; i++) {
        checksum ^= vals[i];
    }
    printf("Checksum: %016llx\n", (unsigned long long)checksum);
    
    return final_result != 0 ? 0 : 1;
}
