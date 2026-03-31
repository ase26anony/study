/* Test program to trigger 10/11-operand RTL expansion in GCC's optabs.cc */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Prevent optimization from removing our test cases */
#define NOOPT __attribute__((noinline, noipa, optimize("O0")))

/* Vector types for potential multi-operand expansion */
typedef int v4si __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Complex arithmetic that might expand to many operands */
NOOPT uint64_t multi_operand_arithmetic(uint64_t a, uint64_t b, uint64_t c,
                                        uint64_t d, uint64_t e, uint64_t f,
                                        uint64_t g, uint64_t h, uint64_t i,
                                        uint64_t j) {
    /* Complex expression that GCC might break into many operations */
    uint64_t result = 0;
    
    /* Multiple operations that could combine into a single multi-operand pattern */
    result = a * b + c * d + e * f + g * h + i * j;
    result = result * a + result * b + result * c;
    result = (result << 5) | (result >> 59); /* Rotate */
    
    return result;
}

/* Multi-precision arithmetic - often expands to many RTL operands */
NOOPT void wide_multiply(uint64_t a, uint64_t b, uint64_t *hi, uint64_t *lo) {
    /* 128-bit multiply using 64-bit parts */
    uint64_t a_lo = a & 0xFFFFFFFF;
    uint64_t a_hi = a >> 32;
    uint64_t b_lo = b & 0xFFFFFFFF;
    uint64_t b_hi = b >> 32;
    
    uint64_t p0 = a_lo * b_lo;
    uint64_t p1 = a_lo * b_hi;
    uint64_t p2 = a_hi * b_lo;
    uint64_t p3 = a_hi * b_hi;
    
    uint64_t carry = (p0 >> 32) + (p1 & 0xFFFFFFFF) + (p2 & 0xFFFFFFFF);
    *lo = (p0 & 0xFFFFFFFF) | (carry << 32);
    *hi = (p1 >> 32) + (p2 >> 32) + p3 + (carry >> 32);
}

/* Target-specific multi-operand operations */
#ifdef __x86_64__
#include <x86intrin.h>

NOOPT uint64_t x86_multi_operand_test(uint64_t a, uint64_t b, uint64_t c,
                                      uint64_t d, uint64_t e, uint64_t f,
                                      uint64_t g, uint64_t h, uint64_t i,
                                      uint64_t j) {
    uint64_t result = 0;
    
    /* Extended inline assembly with many operands */
    asm volatile (
        /* Complex operation using many registers */
        "mov %[a], %%rax\n\t"
        "mov %[b], %%rbx\n\t"
        "mov %[c], %%rcx\n\t"
        "mov %[d], %%rdx\n\t"
        "mov %[e], %%rsi\n\t"
        "mov %[f], %%rdi\n\t"
        "mov %[g], %%r8\n\t"
        "mov %[h], %%r9\n\t"
        "mov %[i], %%r10\n\t"
        "mov %[j], %%r11\n\t"
        /* Do some computation with all registers */
        "add %%rbx, %%rax\n\t"
        "add %%rcx, %%rax\n\t"
        "add %%rdx, %%rax\n\t"
        "add %%rsi, %%rax\n\t"
        "add %%rdi, %%rax\n\t"
        "add %%r8, %%rax\n\t"
        "add %%r9, %%rax\n\t"
        "add %%r10, %%rax\n\t"
        "add %%r11, %%rax\n\t"
        "mov %%rax, %[result]\n\t"
        : [result] "=r" (result)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j)
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11",
          "cc", "memory"
    );
    
    return result;
}

/* AVX-512 operations can have many operands with masks */
NOOPT v4sf x86_vector_test(v4sf a, v4sf b, v4sf c, v4sf d, v4sf e) {
    v4sf result;
    
    /* Fused multiply-add chain - might expand to many operands */
    result = a * b + c;
    result = result * d + e;
    result = result * a + b;
    result = result * c + d;
    
    return result;
}

#elif defined(__aarch64__)
#include <arm_neon.h>

NOOPT uint64_t arm_multi_operand_test(uint64_t a, uint64_t b, uint64_t c,
                                      uint64_t d, uint64_t e, uint64_t f,
                                      uint64_t g, uint64_t h, uint64_t i,
                                      uint64_t j) {
    uint64_t result = 0;
    
    /* ARM inline assembly with many operands */
    asm volatile (
        /* Load all values into registers */
        "mov x0, %[a]\n\t"
        "mov x1, %[b]\n\t"
        "mov x2, %[c]\n\t"
        "mov x3, %[d]\n\t"
        "mov x4, %[e]\n\t"
        "mov x5, %[f]\n\t"
        "mov x6, %[g]\n\t"
        "mov x7, %[h]\n\t"
        "mov x8, %[i]\n\t"
        "mov x9, %[j]\n\t"
        /* Complex computation using all registers */
        "add x0, x0, x1\n\t"
        "add x0, x0, x2\n\t"
        "add x0, x0, x3\n\t"
        "add x0, x0, x4\n\t"
        "add x0, x0, x5\n\t"
        "add x0, x0, x6\n\t"
        "add x0, x0, x7\n\t"
        "add x0, x0, x8\n\t"
        "add x0, x0, x9\n\t"
        "mov %[result], x0\n\t"
        : [result] "=r" (result)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j)
        : "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7", "x8", "x9",
          "cc", "memory"
    );
    
    return result;
}

#elif defined(__powerpc64__) || defined(__PPC64__)

NOOPT uint64_t ppc_multi_operand_test(uint64_t a, uint64_t b, uint64_t c,
                                      uint64_t d, uint64_t e, uint64_t f,
                                      uint64_t g, uint64_t h, uint64_t i,
                                      uint64_t j) {
    uint64_t result = 0;
    
    /* PowerPC inline assembly with many operands */
    asm volatile (
        /* Load operands */
        "mr 3, %[a]\n\t"
        "mr 4, %[b]\n\t"
        "mr 5, %[c]\n\t"
        "mr 6, %[d]\n\t"
        "mr 7, %[e]\n\t"
        "mr 8, %[f]\n\t"
        "mr 9, %[g]\n\t"
        "mr 10, %[h]\n\t"
        "mr 11, %[i]\n\t"
        "mr 12, %[j]\n\t"
        /* Compute with all registers */
        "add 3, 3, 4\n\t"
        "add 3, 3, 5\n\t"
        "add 3, 3, 6\n\t"
        "add 3, 3, 7\n\t"
        "add 3, 3, 8\n\t"
        "add 3, 3, 9\n\t"
        "add 3, 3, 10\n\t"
        "add 3, 3, 11\n\t"
        "add 3, 3, 12\n\t"
        "mr %[result], 3\n\t"
        : [result] "=r" (result)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j)
        : "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11", "r12",
          "cc", "memory"
    );
    
    return result;
}

#else
/* Generic fallback */
NOOPT uint64_t generic_multi_operand_test(uint64_t a, uint64_t b, uint64_t c,
                                         uint64_t d, uint64_t e, uint64_t f,
                                         uint64_t g, uint64_t h, uint64_t i,
                                         uint64_t j) {
    /* Complex expression that might trigger multi-operand expansion */
    uint64_t t1 = (a * b) >> 32;
    uint64_t t2 = (c * d) >> 32;
    uint64_t t3 = (e * f) >> 32;
    uint64_t t4 = (g * h) >> 32;
    uint64_t t5 = (i * j) >> 32;
    
    return t1 + t2 + t3 + t4 + t5 +
           (a & b) + (c & d) + (e & f) + (g & h) + (i & j);
}
#endif

/* Vector operations that might expand to many operands */
NOOPT v4si vector_multi_operand(v4si a, v4si b, v4si c, v4si d, v4si e) {
    v4si result;
    
    /* Complex vector expression */
    result = a * b + c;
    result = result * d + e;
    result = result * a + b;
    result = result * c + d;
    result = result * e + a;
    
    /* Element-wise operations */
    result += (a >> 2) & b;
    result |= (c << 3) ^ d;
    result &= ~e;
    
    return result;
}

/* Test function that combines multiple approaches */
NOOPT uint64_t combined_test(uint64_t a, uint64_t b, uint64_t c,
                            uint64_t d, uint64_t e, uint64_t f,
                            uint64_t g, uint64_t h, uint64_t i,
                            uint64_t j, int mode) {
    uint64_t result = 0;
    
    if (mode == 0) {
        result = multi_operand_arithmetic(a, b, c, d, e, f, g, h, i, j);
    } else if (mode == 1) {
        uint64_t hi, lo;
        wide_multiply(a + b, c + d, &hi, &lo);
        result = hi + lo;
    }
#ifdef __x86_64__
    else if (mode == 2) {
        result = x86_multi_operand_test(a, b, c, d, e, f, g, h, i, j);
    }
#elif defined(__aarch64__)
    else if (mode == 2) {
        result = arm_multi_operand_test(a, b, c, d, e, f, g, h, i, j);
    }
#elif defined(__powerpc64__) || defined(__PPC64__)
    else if (mode == 2) {
        result = ppc_multi_operand_test(a, b, c, d, e, f, g, h, i, j);
    }
#endif
    else {
        /* Generic path */
        result = a + b + c + d + e + f + g + h + i + j;
    }
    
    return result;
}

int main(int argc, char *argv[]) {
    uint64_t result = 0;
    
    /* Initialize test values */
    uint64_t vals[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    /* Test different code paths based on command line */
    int mode = 0;
    if (argc > 1) {
        mode = atoi(argv[1]) % 4;
    }
    
    /* Loop to increase coverage opportunities */
    for (int iter = 0; iter < (argc > 2 ? atoi(argv[2]) : 1); iter++) {
        /* Modify values slightly each iteration */
        for (int i = 0; i < 10; i++) {
            vals[i] = (vals[i] * 1103515245 + 12345) & 0x7FFFFFFF;
        }
        
        /* Call the multi-operand function */
        result += combined_test(vals[0], vals[1], vals[2], vals[3], vals[4],
                               vals[5], vals[6], vals[7], vals[8], vals[9],
                               mode);
        
        /* Also test vector operations */
        v4si vec_a = {vals[0], vals[1], vals[2], vals[3]};
        v4si vec_b = {vals[4], vals[5], vals[6], vals[7]};
        v4si vec_c = {vals[8], vals[9], vals[0], vals[1]};
        v4si vec_d = {vals[2], vals[3], vals[4], vals[5]};
        v4si vec_e = {vals[6], vals[7], vals[8], vals[9]};
        
        v4si vec_result = vector_multi_operand(vec_a, vec_b, vec_c, vec_d, vec_e);
        
        /* Use the vector result to prevent dead code elimination */
        for (int i = 0; i < 4; i++) {
            result += ((uint64_t*)&vec_result)[i];
        }
    }
    
    printf("Result: %lu\n", (unsigned long)result);
    return 0;
}
