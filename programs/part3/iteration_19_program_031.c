/* Test program to trigger 10/11-operand RTL expansion in GCC's optabs.cc */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Prevent optimization from eliminating critical code */
#define NOINLINE __attribute__((noinline, noipa))

/* Vector types for potential multi-operand expansions */
typedef int v4si __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Complex arithmetic that might expand to many operands */
NOINLINE uint64_t complex_mul_highpart(uint64_t a, uint64_t b, uint64_t c, 
                                       uint64_t d, uint64_t e, uint64_t f,
                                       uint64_t g, uint64_t h, uint64_t i) {
    /* Multi-precision arithmetic that may need many temporary operands */
    uint64_t t1 = a * b;
    uint64_t t2 = c * d;
    uint64_t t3 = e * f;
    uint64_t t4 = g * h;
    
    /* Complex expression that might expand to many RTL operands */
    uint64_t result = ((t1 >> 32) * (t2 >> 32)) + 
                     ((t3 >> 32) * (t4 >> 32)) +
                     ((a >> 32) * (b >> 32)) +
                     ((c >> 32) * (d >> 32)) +
                     ((e >> 32) * (f >> 32)) +
                     ((g >> 32) * (h >> 32)) + i;
    
    return result;
}

/* Function using inline assembly with many operands */
NOINLINE uint64_t multi_operand_asm(uint64_t a, uint64_t b, uint64_t c,
                                    uint64_t d, uint64_t e, uint64_t f,
                                    uint64_t g, uint64_t h, uint64_t i,
                                    uint64_t j) {
    uint64_t result1, result2;
    
#ifdef __x86_64__
    /* x86-64 inline assembly that might expand to many operands */
    asm volatile (
        /* Complex sequence that might require many temporaries */
        "movq %[a], %%rax\n\t"
        "mulq %[b]\n\t"
        "movq %%rax, %[r1]\n\t"
        "movq %%rdx, %[r2]\n\t"
        "movq %[c], %%rax\n\t"
        "mulq %[d]\n\t"
        "addq %%rax, %[r1]\n\t"
        "adcq %%rdx, %[r2]\n\t"
        "movq %[e], %%rax\n\t"
        "mulq %[f]\n\t"
        "addq %%rax, %[r1]\n\t"
        "adcq %%rdx, %[r2]\n\t"
        "movq %[g], %%rax\n\t"
        "mulq %[h]\n\t"
        "addq %%rax, %[r1]\n\t"
        "adcq %%rdx, %[r2]\n\t"
        "addq %[i], %[r1]\n\t"
        "adcq $0, %[r2]\n\t"
        "addq %[j], %[r1]\n\t"
        "adcq $0, %[r2]\n\t"
        : [r1] "=&r" (result1), [r2] "=&r" (result2)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j)
        : "rax", "rdx", "cc"
    );
#elif defined(__aarch64__)
    /* ARM64 inline assembly */
    asm volatile (
        "mul %x[r1], %x[a], %x[b]\n\t"
        "mul %x[r2], %x[c], %x[d]\n\t"
        "add %x[r1], %x[r1], %x[r2]\n\t"
        "mul %x[r2], %x[e], %x[f]\n\t"
        "add %x[r1], %x[r1], %x[r2]\n\t"
        "mul %x[r2], %x[g], %x[h]\n\t"
        "add %x[r1], %x[r1], %x[r2]\n\t"
        "add %x[r1], %x[r1], %x[i]\n\t"
        "add %x[r1], %x[r1], %x[j]\n\t"
        : [r1] "=&r" (result1), [r2] "=&r" (result2)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j)
        : "cc"
    );
#else
    /* Generic fallback */
    result1 = a * b + c * d + e * f + g * h + i + j;
    result2 = 0;
#endif
    
    return result1 + result2;
}

/* Vector operations that might expand to many operands */
NOINLINE v4si vector_permute(v4si a, v4si b, v4si c, v4si d,
                             v4si mask1, v4si mask2) {
    v4si result;
    
    /* Complex vector permutation that might need many temporaries */
    v4si t1 = __builtin_shufflevector(a, b, 0, 2, 4, 6);
    v4si t2 = __builtin_shufflevector(c, d, 1, 3, 5, 7);
    v4si t3 = __builtin_shufflevector(a, c, 2, 3, 6, 7);
    v4si t4 = __builtin_shufflevector(b, d, 0, 1, 4, 5);
    
    /* Mix with masks */
    result = (t1 & mask1) | (t2 & ~mask1);
    result = result + (t3 & mask2) + (t4 & ~mask2);
    
    /* Additional operations to increase operand count */
    result = result * a + result * b - result * c + result * d;
    
    return result;
}

/* Function that combines multiple expansion strategies */
NOINLINE uint64_t combined_expansion(uint64_t a, uint64_t b, uint64_t c,
                                     uint64_t d, uint64_t e, uint64_t f,
                                     uint64_t g, uint64_t h, uint64_t i,
                                     uint64_t j, int mode) {
    uint64_t result = 0;
    
    if (mode == 0) {
        /* Use complex arithmetic */
        result = complex_mul_highpart(a, b, c, d, e, f, g, h, i);
    } else if (mode == 1) {
        /* Use inline assembly */
        result = multi_operand_asm(a, b, c, d, e, f, g, h, i, j);
    } else if (mode == 2) {
        /* Mixed operations */
        v4si va = {a, b, c, d};
        v4si vb = {e, f, g, h};
        v4si vc = {i, j, a, b};
        v4si vd = {c, d, e, f};
        v4si mask1 = {0xFFFFFFFF, 0, 0xFFFFFFFF, 0};
        v4si mask2 = {0, 0xFFFFFFFF, 0, 0xFFFFFFFF};
        
        v4si vresult = vector_permute(va, vb, vc, vd, mask1, mask2);
        result = vresult[0] + vresult[1] + vresult[2] + vresult[3];
    } else {
        /* Chain of operations that might expand to many operands */
        result = (((((((((a * b) + c) * d) + e) * f) + g) * h) + i) * j);
    }
    
    return result;
}

/* Main function with different code paths */
int main(int argc, char *argv[]) {
    uint64_t results[4] = {0};
    
    /* Initialize operands with varying values */
    uint64_t operands[10];
    for (int i = 0; i < 10; i++) {
        operands[i] = (uint64_t)(argc + i) * 0x123456789ABCDEFULL;
    }
    
    /* Call functions with different modes based on argc */
    for (int mode = 0; mode < 4; mode++) {
        if (argc > mode) {
            /* Force different expansion paths */
            results[mode] = combined_expansion(
                operands[0], operands[1], operands[2], operands[3],
                operands[4], operands[5], operands[6], operands[7],
                operands[8], operands[9], mode);
            
            /* Add some computation to prevent dead code elimination */
            results[mode] ^= (results[mode] >> 32);
            results[mode] *= 0x5DEECE66DULL;
        }
    }
    
    /* Combine results and print to ensure they're used */
    uint64_t final_result = 0;
    for (int i = 0; i < 4; i++) {
        final_result += results[i];
    }
    
    printf("Result: 0x%016llX\n", (unsigned long long)final_result);
    
    /* Additional loop with varying inputs to increase coverage */
    if (argc > 1) {
        uint64_t sum = 0;
        for (int iter = 0; iter < 100; iter++) {
            for (int i = 0; i < 10; i++) {
                operands[i] = (operands[i] * 0x5DEECE66DULL + 0xB) & 0xFFFFFFFFFFFFULL;
            }
            
            sum += combined_expansion(
                operands[0], operands[1], operands[2], operands[3],
                operands[4], operands[5], operands[6], operands[7],
                operands[8], operands[9], iter % 4);
        }
        printf("Iterative sum: 0x%016llX\n", (unsigned long long)sum);
    }
    
    return 0;
}
