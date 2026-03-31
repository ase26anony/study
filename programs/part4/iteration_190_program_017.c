/* Compile with: gcc -O1 -march=native -fdump-rtl-expand -o test_optabs test_optabs.c */
/* For x86 AVX-512: gcc -O1 -mavx512f -mavx512vl -fdump-rtl-expand -o test_optabs test_optabs.c */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Simple PRNG to prevent constant propagation */
static uint64_t simple_rand(uint64_t *seed) {
    *seed = *seed * 1103515245 + 12345;
    return *seed;
}

/* Function to use 10 operands */
int func_10_operands(uint64_t a, uint64_t b, uint64_t c, uint64_t d, uint64_t e,
                     uint64_t f, uint64_t g, uint64_t h, uint64_t i, uint64_t j) {
    int result = 0;
    
    /* Architecture-specific paths for high operand count operations */
    
#ifdef __AVX512F__
    /* AVX-512 compress store with mask - can involve many operands when expanded */
    /* Note: Actual intrinsic may not have exactly 10 operands, but expansion might */
    /* We'll use inline assembly as fallback for precise control */
    __m512i vec1, vec2, vec3, vec4;
    __mmask16 mask;
    void *addr;
    
    /* Initialize with our parameters to prevent optimization */
    vec1 = _mm512_set_epi64(j, i, h, g, f, e, d, c);
    vec2 = _mm512_set_epi64(a, b, c, d, e, f, g, h);
    mask = (__mmask16)((a ^ b ^ c ^ d) & 0xFFFF);
    
    /* Complex operation that might expand to many operands */
    /* _mm512_mask_compressstoreu_epi64(addr, mask, vec1); */
    
    /* Instead, use inline assembly with exactly 10 operands */
    __asm__ volatile (
        "/* 10-operand dummy operation %0 %1 %2 %3 %4 %5 %6 %7 %8 %9 */"
        : "=r"(result)
        : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e), 
          "r"(f), "r"(g), "r"(h), "r"(i), "r"(j)
        : "memory"
    );
    
#elif defined(__ARM_FEATURE_SVE)
    /* ARM SVE scatter store with predicate - can have many operands */
    /* Use inline assembly for precise operand count */
    __asm__ volatile (
        "/* SVE 10-operand operation %0 %1 %2 %3 %4 %5 %6 %7 %8 %9 */"
        : "=r"(result)
        : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e),
          "r"(f), "r"(g), "r"(h), "r"(i), "r"(j)
        : "memory"
    );
    
#else
    /* Generic fallback: inline assembly with 10 operands */
    /* This should trigger the 10-operand expansion path */
    __asm__ volatile (
        "/* Generic 10-operand expansion test */\n"
        "add %0, %1, %2\n"
        "add %0, %0, %3\n"
        "add %0, %0, %4\n"
        "add %0, %0, %5\n"
        "add %0, %0, %6\n"
        "add %0, %0, %7\n"
        "add %0, %0, %8\n"
        "add %0, %0, %9"
        : "=r"(result)
        : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e),
          "r"(f), "r"(g), "r"(h), "r"(i), "r"(j)
    );
#endif
    
    /* Use result in computation to prevent dead code elimination */
    return result + (a ^ b ^ c ^ d ^ e ^ f ^ g ^ h ^ i ^ j);
}

/* Function to use 11 operands */
int func_11_operands(uint64_t a, uint64_t b, uint64_t c, uint64_t d, uint64_t e,
                     uint64_t f, uint64_t g, uint64_t h, uint64_t i, uint64_t j,
                     uint64_t k) {
    int result = 0;
    
    /* Architecture-specific paths for 11 operands */
    
#ifdef __AVX512F__
    /* More complex AVX-512 operation with mask, multiple vectors, and address */
    /* Use inline assembly for exact operand count */
    __asm__ volatile (
        "/* 11-operand AVX-512-like operation */\n"
        "mov %0, %1\n"
        "add %0, %0, %2\n"
        "add %0, %0, %3\n"
        "add %0, %0, %4\n"
        "add %0, %0, %5\n"
        "add %0, %0, %6\n"
        "add %0, %0, %7\n"
        "add %0, %0, %8\n"
        "add %0, %0, %9\n"
        "add %0, %0, %10"
        : "=r"(result)
        : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e),
          "r"(f), "r"(g), "r"(h), "r"(i), "r"(j),
          "r"(k)
    );
    
#elif defined(__ARM_FEATURE_SVE)
    /* ARM SVE with predicate, base, offset, and data */
    __asm__ volatile (
        "/* SVE 11-operand scatter-gather */"
        : "=r"(result)
        : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e),
          "r"(f), "r"(g), "r"(h), "r"(i), "r"(j),
          "r"(k)
        : "memory"
    );
    
#else
    /* Generic fallback: inline assembly with 11 operands */
    /* This should trigger the 11-operand expansion path in optabs.cc */
    __asm__ volatile (
        "/* Generic 11-operand expansion test */\n"
        "mov %0, %1\n"
        "add %0, %0, %2\n"
        "add %0, %0, %3\n"
        "add %0, %0, %4\n"
        "add %0, %0, %5\n"
        "add %0, %0, %6\n"
        "add %0, %0, %7\n"
        "add %0, %0, %8\n"
        "add %0, %0, %9\n"
        "add %0, %0, %10"
        : "=r"(result)
        : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e),
          "r"(f), "r"(g), "r"(h), "r"(i), "r"(j),
          "r"(k)
    );
#endif
    
    /* Use result in computation */
    return result + (a ^ b ^ c ^ d ^ e ^ f ^ g ^ h ^ i ^ j ^ k);
}

int main(int argc, char *argv[]) {
    uint64_t seed = 0x123456789ABCDEF0ULL;
    uint64_t vars[11];
    int result1, result2, final_result;
    
    /* Initialize 11 variables with non-constant values */
    for (int i = 0; i < 11; i++) {
        /* Use argv if available, otherwise PRNG */
        if (argc > i + 1) {
            vars[i] = (uint64_t)atoi(argv[i + 1]);
        } else {
            vars[i] = simple_rand(&seed);
        }
    }
    
    /* Call function with 10 operands */
    result1 = func_10_operands(vars[0], vars[1], vars[2], vars[3], vars[4],
                               vars[5], vars[6], vars[7], vars[8], vars[9]);
    
    /* Call function with 11 operands */
    result2 = func_11_operands(vars[0], vars[1], vars[2], vars[3], vars[4],
                               vars[5], vars[6], vars[7], vars[8], vars[9],
                               vars[10]);
    
    /* Combine results to prevent optimization */
    final_result = result1 + result2;
    
    printf("Result: %d\n", final_result);
    
    /* Check RTL dump for coverage verification */
    /* The -fdump-rtl-expand flag will create a .expand file showing the expansion */
    
    return final_result != 0 ? 0 : 1;
}
