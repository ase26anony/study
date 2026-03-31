/* Test program to trigger 10/11 operand expansion in optabs.cc */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Prevent optimization from eliminating critical operations */
#define NOINLINE __attribute__((noinline, noipa))

/* Vector types for various architectures */
#ifdef __x86_64__
#include <x86intrin.h>
#include <immintrin.h>
#elif defined(__aarch64__)
#include <arm_neon.h>
#endif

/* Complex multi-operand operation using inline assembly */
NOINLINE static uint64_t multi_operand_asm(uint64_t a, uint64_t b, uint64_t c,
                                          uint64_t d, uint64_t e, uint64_t f,
                                          uint64_t g, uint64_t h, uint64_t i,
                                          uint64_t j) {
    uint64_t result;
    
#ifdef __x86_64__
    /* x86_64 inline assembly with many operands */
    asm volatile (
        "mov %[a], %%rax\n\t"
        "add %[b], %%rax\n\t"
        "adc %[c], %%rax\n\t"
        "adc %[d], %%rax\n\t"
        "adc %[e], %%rax\n\t"
        "adc %[f], %%rax\n\t"
        "adc %[g], %%rax\n\t"
        "adc %[h], %%rax\n\t"
        "adc %[i], %%rax\n\t"
        "adc %[j], %%rax\n\t"
        "mov %%rax, %[result]"
        : [result] "=r" (result)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j)
        : "rax", "cc"
    );
#elif defined(__aarch64__)
    /* ARM64 inline assembly with many operands */
    asm volatile (
        "add %x[result], %x[a], %x[b]\n\t"
        "add %x[result], %x[result], %x[c]\n\t"
        "add %x[result], %x[result], %x[d]\n\t"
        "add %x[result], %x[result], %x[e]\n\t"
        "add %x[result], %x[result], %x[f]\n\t"
        "add %x[result], %x[result], %x[g]\n\t"
        "add %x[result], %x[result], %x[h]\n\t"
        "add %x[result], %x[result], %x[i]\n\t"
        "add %x[result], %x[result], %x[j]"
        : [result] "=r" (result)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j)
    );
#else
    /* Generic fallback - still creates many operands */
    result = a + b + c + d + e + f + g + h + i + j;
#endif
    
    return result;
}

/* Multi-precision multiplication that may expand to many operands */
NOINLINE static uint64_t multi_precision_mul(uint32_t a, uint32_t b, uint32_t c,
                                            uint32_t d, uint32_t e, uint32_t f) {
    /* This complex expression may trigger multi-operand expansion */
    uint64_t result = (uint64_t)a * b;
    result += (uint64_t)c * d;
    result += (uint64_t)e * f;
    
    /* Additional operations to increase operand count */
    result = (result << 5) | (result >> 59);  /* rotate */
    result ^= (result >> 32);
    result *= 0x9e3779b97f4a7c15ULL;  /* golden ratio */
    
    return result;
}

/* Vector operations that may expand to many operands */
NOINLINE static void vector_operations(uint64_t *result) {
    /* Use GCC vector extensions */
    typedef uint64_t v2u64 __attribute__((vector_size(16)));
    typedef uint32_t v4u32 __attribute__((vector_size(16)));
    
    v4u32 va = {1, 2, 3, 4};
    v4u32 vb = {5, 6, 7, 8};
    v4u32 vc = {9, 10, 11, 12};
    v4u32 vd = {13, 14, 15, 16};
    
    /* Complex vector expression that may expand to many operands */
    v4u32 vr = va * vb + vc * vd;
    vr = (vr << 2) | (vr >> 30);  /* element-wise rotate */
    
    /* Convert to 64-bit and store */
    v2u64 vr64 = {vr[0] + vr[1], vr[2] + vr[3]};
    result[0] = vr64[0];
    result[1] = vr64[1];
}

/* Function that combines multiple expansion strategies */
NOINLINE static uint64_t combined_expansion(uint64_t a, uint64_t b, uint64_t c,
                                           uint64_t d, uint64_t e, uint64_t f,
                                           uint64_t g, uint64_t h, uint64_t i,
                                           uint64_t j, int use_asm) {
    uint64_t result;
    
    if (use_asm) {
        /* Path 1: Use inline assembly with many operands */
        result = multi_operand_asm(a, b, c, d, e, f, g, h, i, j);
    } else {
        /* Path 2: Use complex arithmetic that may expand to many operands */
        uint32_t a32 = a & 0xFFFFFFFF;
        uint32_t b32 = (a >> 32) & 0xFFFFFFFF;
        uint32_t c32 = b & 0xFFFFFFFF;
        uint32_t d32 = (b >> 32) & 0xFFFFFFFF;
        uint32_t e32 = c & 0xFFFFFFFF;
        uint32_t f32 = (c >> 32) & 0xFFFFFFFF;
        
        result = multi_precision_mul(a32, b32, c32, d32, e32, f32);
        
        /* Mix in remaining operands */
        result ^= d ^ e ^ f ^ g ^ h ^ i ^ j;
    }
    
    /* Additional operations to prevent dead code elimination */
    result = (result * 1103515245 + 12345) & 0x7FFFFFFF;
    
    return result;
}

int main(int argc, char *argv[]) {
    uint64_t operands[10];
    uint64_t final_result = 0;
    int use_asm_path = 0;
    
    /* Use command line arguments to influence code path */
    if (argc > 1) {
        use_asm_path = atoi(argv[1]) & 1;
    }
    
    /* Initialize operands with varying values */
    for (int i = 0; i < 10; i++) {
        operands[i] = (uint64_t)i * 0x123456789ABCDEFULL + argc;
    }
    
    /* Call the combined expansion function multiple times */
    for (int iteration = 0; iteration < (argc > 2 ? atoi(argv[2]) : 1); iteration++) {
        /* Modify operands slightly each iteration */
        for (int i = 0; i < 10; i++) {
            operands[i] ^= (iteration << i);
        }
        
        /* This call may trigger 10-operand expansion */
        uint64_t res1 = combined_expansion(
            operands[0], operands[1], operands[2], operands[3], operands[4],
            operands[5], operands[6], operands[7], operands[8], operands[9],
            use_asm_path
        );
        
        /* Also test vector operations */
        uint64_t vec_res[2];
        vector_operations(vec_res);
        
        /* Combine results */
        final_result ^= res1 ^ vec_res[0] ^ vec_res[1];
        
        /* Additional complex computation that might trigger expansion */
        uint64_t temp = operands[0];
        for (int i = 1; i < 10; i++) {
            /* Complex expression that may expand to many operands */
            temp = (temp * 0x5DEECE66DULL + 0xB) & ((1ULL << 48) - 1);
            temp ^= operands[i];
            temp = (temp >> 16) | (temp << 48);  /* rotate */
        }
        final_result += temp;
    }
    
    /* Print result to prevent optimization */
    printf("Result: 0x%016llX\n", (unsigned long long)final_result);
    
    return (int)(final_result & 0x7FFFFFFF);
}
