/* Test program to cover 10/11 operand expansion in optabs.cc */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Simple PRNG to generate non-constant values */
static uint32_t prng_state = 123456789;
static uint32_t prng() {
    prng_state = prng_state * 1103515245 + 12345;
    return prng_state;
}

/* Function to use 10 operands */
int func_10_operands(uint64_t a0, uint64_t a1, uint64_t a2, uint64_t a3,
                     uint64_t a4, uint64_t a5, uint64_t a6, uint64_t a7,
                     uint64_t a8, uint64_t a9) {
    uint64_t result = 0;
    
    /* Architecture-specific high operand count operations */
#if defined(__AVX512F__) && defined(__AVX512VL__)
    /* AVX-512 mask compress store with many operands when expanded */
    #include <immintrin.h>
    __mmask8 mask = (__mmask8)((a0 ^ a1) & 0xFF);
    int64_t data[8] = {a0, a1, a2, a3, a4, a5, a6, a7};
    __m512i vec = _mm512_loadu_si512(data);
    volatile int64_t* addr = (int64_t*)(uintptr_t)a8;
    
    /* This intrinsic expands to many operands in RTL */
    _mm512_mask_compressstoreu_epi64(addr, mask, vec);
    
    /* Use the result */
    for (int i = 0; i < 8; i++) {
        result += ((int64_t*)addr)[i];
    }
    
#elif defined(__ARM_FEATURE_SVE)
    /* ARM SVE scatter store with predicate and base addressing */
    #include <arm_sve.h>
    svbool_t pg = svwhilelt_b64(a0, a1);
    uint64_t bases[4] = {a2, a3, a4, a5};
    uint64_t data[4] = {a6, a7, a8, a9};
    
    /* Complex SVE operation that may expand to many operands */
    svst1_scatter_u64base_u64(pg, svld1_u64(pg, bases), svld1_u64(pg, data));
    
    /* Compute result from data */
    for (int i = 0; i < 4; i++) {
        result += data[i];
    }
    
#else
    /* Generic fallback: inline assembly with 10 explicit operands */
    /* This forces the compiler to handle 10 rtx operands during expansion */
    uint64_t tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8, tmp9;
    
    asm volatile (
        /* Dummy multi-operand operation template */
        "mov %0, %1\n\t"
        "add %0, %2\n\t"
        "add %0, %3\n\t"
        "add %0, %4\n\t"
        "add %0, %5\n\t"
        "add %0, %6\n\t"
        "add %0, %7\n\t"
        "add %0, %8\n\t"
        "add %0, %9"
        : "=r" (tmp0)
        : "r" (a0), "r" (a1), "r" (a2), "r" (a3),
          "r" (a4), "r" (a5), "r" (a6), "r" (a7), "r" (a8)
        : "cc"
    );
    
    /* Use all operands in computation */
    result = a0 + a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + tmp0;
#endif
    
    return (int)(result & 0x7FFFFFFF);
}

/* Function to use 11 operands */
int func_11_operands(uint64_t a0, uint64_t a1, uint64_t a2, uint64_t a3,
                     uint64_t a4, uint64_t a5, uint64_t a6, uint64_t a7,
                     uint64_t a8, uint64_t a9, uint64_t a10) {
    uint64_t result = 0;
    
    /* Architecture-specific 11-operand operations */
#if defined(__AVX512F__) && defined(__AVX512BW__) && defined(__AVX512VL__)
    /* AVX-512 masked gather with many operands */
    #include <immintrin.h>
    __mmask8 mask = (__mmask8)((a0 ^ a1 ^ a2) & 0xFF);
    int64_t idx[8] = {a3, a4, a5, a6, a7, a8, a9, a10};
    const int64_t* base = (const int64_t*)(uintptr_t)a0;
    __m512i vindex = _mm512_loadu_si512(idx);
    __m512i src = _mm512_set1_epi64(a1);
    int scale = 1;
    
    /* This gather operation expands to many operands */
    __m512i gathered = _mm512_mask_i64gather_epi64(src, mask, vindex, base, scale);
    
    /* Sum elements */
    result = _mm512_reduce_add_epi64(gathered);
    
#elif defined(__ARM_FEATURE_SVE)
    /* ARM SVE gather with predicate and offset */
    #include <arm_sve.h>
    svbool_t pg = svwhilelt_b64(a0, a1);
    uint64_t bases[4] = {a2, a3, a4, a5};
    int64_t offsets[4] = {a6, a7, a8, a9};
    
    /* Complex SVE gather that may expand to many operands */
    svuint64_t data = svld1_gather_u64offset_u64(pg, 
                       svld1_u64(pg, bases),
                       svld1_s64(pg, offsets));
    
    /* Extract and sum */
    uint64_t extracted[4];
    svst1_u64(pg, extracted, data);
    for (int i = 0; i < 4; i++) {
        result += extracted[i];
    }
    
#else
    /* Generic fallback: inline assembly with 11 explicit operands */
    uint64_t tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8, tmp9, tmp10;
    
    asm volatile (
        /* Dummy 11-operand operation */
        "mov %0, %1\n\t"
        "add %0, %2\n\t"
        "add %0, %3\n\t"
        "add %0, %4\n\t"
        "add %0, %5\n\t"
        "add %0, %6\n\t"
        "add %0, %7\n\t"
        "add %0, %8\n\t"
        "add %0, %9\n\t"
        "add %0, %10"
        : "=r" (tmp0)
        : "r" (a0), "r" (a1), "r" (a2), "r" (a3),
          "r" (a4), "r" (a5), "r" (a6), "r" (a7),
          "r" (a8), "r" (a9), "r" (a10)
        : "cc"
    );
    
    /* Use all operands */
    result = a0 + a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10 + tmp0;
#endif
    
    return (int)(result & 0x7FFFFFFF);
}

int main(int argc, char *argv[]) {
    /* Generate 12 non-constant values using PRNG and argv */
    uint64_t vals[12];
    
    for (int i = 0; i < 12; i++) {
        /* Mix PRNG with argv for non-constness */
        vals[i] = prng();
        if (i < argc) {
            vals[i] ^= (uint64_t)argv[i];
        }
    }
    
    /* Call 10-operand function */
    int res10 = func_10_operands(vals[0], vals[1], vals[2], vals[3],
                                 vals[4], vals[5], vals[6], vals[7],
                                 vals[8], vals[9]);
    
    /* Call 11-operand function */
    int res11 = func_11_operands(vals[0], vals[1], vals[2], vals[3],
                                 vals[4], vals[5], vals[6], vals[7],
                                 vals[8], vals[9], vals[10]);
    
    /* Combine results to prevent elimination */
    int final_result = res10 + res11;
    
    /* Use result to prevent dead code elimination */
    if (final_result > 1000000) {
        printf("Result: %d\n", final_result % 1000);
    } else {
        printf("Result: %d\n", final_result);
    }
    
    return 0;
}
