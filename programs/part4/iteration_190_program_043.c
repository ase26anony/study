/* test_optabs_coverage.c - Cover 10/11 operand expansion in optabs.cc */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

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
    /* AVX-512 mask compress store - can involve many operands when expanded */
    /* Using _mm512_mask_compressstoreu_epi32 which takes mask, address, and data */
    /* The expansion may generate many operand RTXs */
    {
        __m512i data = _mm512_set_epi32(a9, a8, a7, a6, a5, a4, a3, a2, a1, a0, 
                                        a0, a1, a2, a3, a4, a5);
        __mmask16 mask = (a0 & 0xFFFF);
        int32_t dest[16] __attribute__((aligned(64)));
        
        _mm512_mask_compressstoreu_epi32(dest, mask, data);
        
        /* Use the result to prevent optimization */
        for (int i = 0; i < 16; i++) {
            result += dest[i];
        }
    }
#elif defined(__ARM_FEATURE_SVE)
    /* ARM SVE scatter store with multiple operands */
    /* Note: Actual SVE intrinsics may vary - this is a conceptual example */
    {
        /* This would typically involve predicate, base addresses, and data */
        /* The expansion often creates many operand RTXs */
        svbool_t pg = svwhilelt_b32(0, 16);
        svuint32_t bases = svld1_u32(pg, (uint32_t[]){a0, a1, a2, a3, a4, a5, a6, a7, a8, a9});
        svuint32_t data = svadd_u32_z(pg, bases, svdup_u32(a0));
        
        uint32_t dest[16];
        svst1_u32(pg, dest, data);
        
        for (int i = 0; i < 16; i++) {
            result += dest[i];
        }
    }
#else
    /* Generic fallback: inline assembly with 10 explicit operands */
    /* This forces the compiler to handle 10 operand RTXs */
    asm volatile (
        /* Dummy operation that uses all 10 input registers */
        "add %[r0], %[r0], %[r1]\n\t"
        "add %[r0], %[r0], %[r2]\n\t"
        "add %[r0], %[r0], %[r3]\n\t"
        "add %[r0], %[r0], %[r4]\n\t"
        "add %[r0], %[r0], %[r5]\n\t"
        "add %[r0], %[r0], %[r6]\n\t"
        "add %[r0], %[r0], %[r7]\n\t"
        "add %[r0], %[r0], %[r8]\n\t"
        "add %[r0], %[r0], %[r9]"
        : [r0] "+r" (a0)
        : [r1] "r" (a1), [r2] "r" (a2), [r3] "r" (a3),
          [r4] "r" (a4), [r5] "r" (a5), [r6] "r" (a6),
          [r7] "r" (a7), [r8] "r" (a8), [r9] "r" (a9)
        : "cc"
    );
    result = a0;
#endif
    
    return result;
}

/* Function to use 11 operands */
int func_11_operands(int a0, int a1, int a2, int a3, int a4, int a5,
                     int a6, int a7, int a8, int a9, int a10) {
    int result = 0;
    
#if defined(__AVX512F__) && defined(__AVX512VL__) && defined(__AVX512BW__)
    /* AVX-512 masked gather with multiple operands */
    /* _mm512_mask_i32gather_epi32 has mask, index scale, base, and src */
    {
        __m512i index = _mm512_set_epi32(a10, a9, a8, a7, a6, a5, a4, a3, a2, a1, a0,
                                         a0, a1, a2, a3, a4);
        __mmask16 mask = (a0 & 0xFFFF) | ((a10 & 0xFFFF) << 8);
        const int* base = (const int*)&a0;
        __m512i src = _mm512_set1_epi32(a10);
        
        __m512i gathered = _mm512_mask_i32gather_epi32(src, mask, index, base, 4);
        
        /* Sum elements */
        result = _mm512_reduce_add_epi32(gathered);
    }
#elif defined(__ARM_FEATURE_SVE)
    /* ARM SVE gather with predicate, bases, and offsets */
    {
        svbool_t pg = svwhilelt_b32(0, 16);
        svuint32_t bases = svld1_u32(pg, (uint32_t[]){a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10});
        svuint32_t offsets = svmul_u32_z(pg, bases, svdup_u32(a10));
        
        uint32_t dest[16];
        /* Simulated gather operation */
        for (int i = 0; i < 16; i++) {
            dest[i] = (uint32_t)(a0 + i * a10);
        }
        
        for (int i = 0; i < 16; i++) {
            result += dest[i];
        }
    }
#else
    /* Generic fallback: inline assembly with 11 explicit operands */
    /* This forces the compiler to handle 11 operand RTXs */
    asm volatile (
        /* Dummy operation that uses all 11 input registers */
        "add %[r0], %[r0], %[r1]\n\t"
        "add %[r0], %[r0], %[r2]\n\t"
        "add %[r0], %[r0], %[r3]\n\t"
        "add %[r0], %[r0], %[r4]\n\t"
        "add %[r0], %[r0], %[r5]\n\t"
        "add %[r0], %[r0], %[r6]\n\t"
        "add %[r0], %[r0], %[r7]\n\t"
        "add %[r0], %[r0], %[r8]\n\t"
        "add %[r0], %[r0], %[r9]\n\t"
        "add %[r0], %[r0], %[r10]"
        : [r0] "+r" (a0)
        : [r1] "r" (a1), [r2] "r" (a2), [r3] "r" (a3),
          [r4] "r" (a4), [r5] "r" (a5), [r6] "r" (a6),
          [r7] "r" (a7), [r8] "r" (a8), [r9] "r" (a9),
          [r10] "r" (a10)
        : "cc"
    );
    result = a0;
#endif
    
    return result;
}

int main(int argc, char *argv[]) {
    /* Generate 11 non-constant values */
    int vals[11];
    
    /* Use argv for some values, PRNG for others to ensure non-constness */
    for (int i = 0; i < 11; i++) {
        if (i < argc && i < 11) {
            vals[i] = (int)(uintptr_t)argv[i];
        } else {
            vals[i] = prng();
        }
    }
    
    /* Call both functions to trigger both code paths */
    int result1 = func_10_operands(vals[0], vals[1], vals[2], vals[3], vals[4],
                                   vals[5], vals[6], vals[7], vals[8], vals[9]);
    
    int result2 = func_11_operands(vals[0], vals[1], vals[2], vals[3], vals[4],
                                   vals[5], vals[6], vals[7], vals[8], vals[9], vals[10]);
    
    /* Combine results to prevent dead code elimination */
    int final_result = result1 + result2;
    
    printf("Result: %d\n", final_result);
    
    return final_result != 0 ? 0 : 1;
}
