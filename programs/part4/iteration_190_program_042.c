/* test_optabs_coverage.c
 * This program is designed to trigger the 10 and 11 operand expansion
 * paths in GCC's optabs.cc (lines 8254-8263).
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Simple PRNG to generate non-constant values */
static unsigned int prng_state = 123456789;

static unsigned int prng(void) {
    prng_state = prng_state * 1103515245 + 12345;
    return prng_state;
}

/* Helper to initialize values from argv or PRNG */
static unsigned long long init_value(int idx, char *argv[]) {
    if (argv && idx > 0) {
        return (unsigned long long)(argv[idx] ? atoi(argv[idx]) : 0);
    }
    return prng();
}

/* ====== 10-OPERAND FUNCTION ====== */
unsigned long long func_10_operands(
    unsigned long long a0, unsigned long long a1,
    unsigned long long a2, unsigned long long a3,
    unsigned long long a4, unsigned long long a5,
    unsigned long long a6, unsigned long long a7,
    unsigned long long a8, unsigned long long a9)
{
    unsigned long long result = 0;
    
#if defined(__AVX512F__) && defined(__AVX512VL__)
    /* x86 AVX-512: Use _mm512_mask_compressstoreu_epi32 which expands
     * to many operands when dealing with mask, address, and data */
    #include <immintrin.h>
    
    /* Create volatile variables to prevent optimization */
    volatile unsigned int* addr = (unsigned int*)(uintptr_t)a0;
    __mmask16 mask = (__mmask16)a1;
    __m512i data = _mm512_set_epi64(a9, a8, a7, a6, a5, a4, a3, a2);
    
    /* This intrinsic expands to multiple operands */
    _mm512_mask_compressstoreu_epi32((void*)addr, mask, data);
    
    /* Use the result */
    result = (unsigned long long)mask + a0 + a9;
    
#elif defined(__ARM_FEATURE_SVE)
    /* ARM SVE: Use scatter store with predicate and multiple registers */
    #include <arm_sve.h>
    
    svbool_t pg = svwhilelt_b64(a0, a1);
    svuint64_t bases = svdup_u64(a2);
    svuint64_t data = svdup_u64(a3);
    
    /* Complex SVE operation that may expand to many operands */
    svstnt1_scatter_u64base_u64(pg, (uint64_t*)(uintptr_t)a4, bases, data);
    
    result = a0 + a1 + a2 + a3 + a4;
    
#else
    /* Generic fallback: Extended inline assembly with 10 operands */
    unsigned long long out0, out1, out2;
    
    __asm__ volatile (
        /* Dummy operations that use all 10 input operands */
        "add %[o0], %[a0], %[a1]\n\t"
        "add %[o1], %[a2], %[a3]\n\t"
        "add %[o2], %[a4], %[a5]\n\t"
        "add %[o0], %[o0], %[a6]\n\t"
        "add %[o1], %[o1], %[a7]\n\t"
        "add %[o2], %[o2], %[a8]\n\t"
        "add %[o0], %[o0], %[a9]\n\t"
        : [o0] "=&r" (out0), [o1] "=&r" (out1), [o2] "=&r" (out2)
        : [a0] "r" (a0), [a1] "r" (a1), [a2] "r" (a2),
          [a3] "r" (a3), [a4] "r" (a4), [a5] "r" (a5),
          [a6] "r" (a6), [a7] "r" (a7), [a8] "r" (a8),
          [a9] "r" (a9)
        : "cc"
    );
    
    result = out0 + out1 + out2;
#endif
    
    return result;
}

/* ====== 11-OPERAND FUNCTION ====== */
unsigned long long func_11_operands(
    unsigned long long a0, unsigned long long a1,
    unsigned long long a2, unsigned long long a3,
    unsigned long long a4, unsigned long long a5,
    unsigned long long a6, unsigned long long a7,
    unsigned long long a8, unsigned long long a9,
    unsigned long long a10)
{
    unsigned long long result = 0;
    
#if defined(__AVX512F__) && defined(__AVX512VBMI2__)
    /* x86 AVX-512 VBMI2: Use compress/expand with mask and multiple data */
    #include <immintrin.h>
    
    __mmask64 mask = (__mmask64)(a0 | (a1 << 8) | (a2 << 16) | (a3 << 24));
    __m512i src = _mm512_set_epi64(a10, a9, a8, a7, a6, a5, a4, a3);
    __m512i dst = _mm512_set_epi64(a2, a1, a0, a9, a8, a7, a6, a5);
    
    /* This should expand to many operands */
    dst = _mm512_mask_compress_epi8(dst, mask, src);
    
    /* Extract and combine results */
    result = _mm512_extract_epi64(dst, 0) +
             _mm512_extract_epi64(dst, 1) +
             a10;
    
#elif defined(__ARM_FEATURE_SVE2)
    /* ARM SVE2: Complex gather load with multiple predicates */
    #include <arm_sve.h>
    
    svbool_t pg0 = svwhilelt_b32(a0, a1);
    svbool_t pg1 = svwhilelt_b32(a2, a3);
    svuint32_t bases = svdup_u32(a4);
    
    svuint32_t data0 = svld1_gather_u32base_u32(pg0, bases);
    svuint32_t data1 = svld1_gather_u32base_u32(pg1, bases);
    
    svuint32_t combined = svadd_u32_z(svptrue_b32(), data0, data1);
    uint32_t extracted = svlastb_u32(svptrue_b32(), combined);
    
    result = extracted + a5 + a6 + a7 + a8 + a9 + a10;
    
#else
    /* Generic fallback: Extended inline assembly with 11 operands */
    unsigned long long out0, out1, out2, out3;
    
    __asm__ volatile (
        /* Use all 11 input operands in various combinations */
        "mov %[o0], %[a0]\n\t"
        "add %[o0], %[o0], %[a1]\n\t"
        "mov %[o1], %[a2]\n\t"
        "add %[o1], %[o1], %[a3]\n\t"
        "mov %[o2], %[a4]\n\t"
        "add %[o2], %[o2], %[a5]\n\t"
        "mov %[o3], %[a6]\n\t"
        "add %[o3], %[o3], %[a7]\n\t"
        "add %[o0], %[o0], %[a8]\n\t"
        "add %[o1], %[o1], %[a9]\n\t"
        "add %[o2], %[o2], %[a10]\n\t"
        "add %[o3], %[o3], %[a0]\n\t"
        : [o0] "=&r" (out0), [o1] "=&r" (out1),
          [o2] "=&r" (out2), [o3] "=&r" (out3)
        : [a0] "r" (a0), [a1] "r" (a1), [a2] "r" (a2),
          [a3] "r" (a3), [a4] "r" (a4), [a5] "r" (a5),
          [a6] "r" (a6), [a7] "r" (a7), [a8] "r" (a8),
          [a9] "r" (a9), [a10] "r" (a10)
        : "cc"
    );
    
    result = out0 + out1 + out2 + out3;
#endif
    
    return result;
}

/* ====== MAIN FUNCTION ====== */
int main(int argc, char *argv[]) {
    unsigned long long vals[12];
    unsigned long long result1, result2, final_result;
    
    /* Initialize 12 values from argv or PRNG */
    for (int i = 0; i < 12; i++) {
        vals[i] = init_value(i, argv);
    }
    
    /* Call 10-operand function with first 10 values */
    result1 = func_10_operands(
        vals[0], vals[1], vals[2], vals[3], vals[4],
        vals[5], vals[6], vals[7], vals[8], vals[9]
    );
    
    /* Call 11-operand function with first 11 values */
    result2 = func_11_operands(
        vals[0], vals[1], vals[2], vals[3], vals[4],
        vals[5], vals[6], vals[7], vals[8], vals[9],
        vals[10]
    );
    
    /* Combine results to prevent optimization */
    final_result = result1 + result2 + vals[11];
    
    /* Print result to ensure side effects */
    printf("Final result: %llu\n", final_result);
    
    return (int)(final_result & 0x7FFFFFFF);
}
