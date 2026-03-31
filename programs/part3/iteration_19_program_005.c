/* Test program to trigger 10/11 operand RTL expansion in optabs.cc */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Prevent optimization from eliminating our test cases */
#define NOINLINE __attribute__((noinline, noipa))

/* Vector types for potential multi-operand expansions */
typedef int v4si __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Complex arithmetic that might expand to many operands */
NOINLINE uint64_t multi_operand_arith(uint64_t a, uint64_t b, uint64_t c,
                                      uint64_t d, uint64_t e, uint64_t f,
                                      uint64_t g, uint64_t h, uint64_t i,
                                      uint64_t j) {
    /* Complex expression that might require many temporary operands */
    uint64_t t1 = (a * b) >> 32;
    uint64_t t2 = (c * d) >> 32;
    uint64_t t3 = (e * f) >> 32;
    uint64_t t4 = (g * h) >> 32;
    uint64_t t5 = (i * j) >> 32;
    
    /* Mix them in a way that prevents optimization */
    return ((t1 + t2) * (t3 + t4)) ^ t5;
}

/* Vector operations that might expand to many operands */
NOINLINE v4si vector_multi_op(v4si a, v4si b, v4si c, v4si d,
                              v4si e, v4si f, v4si g, v4si h) {
    /* Complex vector expression */
    v4si t1 = a * b + c;
    v4si t2 = d * e + f;
    v4si t3 = g * h + a;
    v4si t4 = b * c + d;
    
    return (t1 * t2) + (t3 * t4);
}

/* Target-specific inline assembly with many operands */
NOINLINE uint64_t target_specific_multi_operand(uint64_t a, uint64_t b,
                                                uint64_t c, uint64_t d,
                                                uint64_t e, uint64_t f,
                                                uint64_t g, uint64_t h,
                                                uint64_t i, uint64_t j) {
    uint64_t result1, result2;
    
#ifdef __x86_64__
    /* x86_64 inline assembly with many operands */
    asm volatile (
        /* Complex sequence that uses many registers */
        "movq %[a], %%rax\n\t"
        "mulq %[b]\n\t"
        "movq %%rax, %[r1]\n\t"
        "movq %[c], %%rax\n\t"
        "mulq %[d]\n\t"
        "addq %[r1], %%rax\n\t"
        "movq %%rax, %[r1]\n\t"
        "movq %[e], %%rax\n\t"
        "mulq %[f]\n\t"
        "movq %[g], %%rbx\n\t"
        "imulq %[h], %%rbx\n\t"
        "addq %%rbx, %%rax\n\t"
        "movq %[i], %%rbx\n\t"
        "imulq %[j], %%rbx\n\t"
        "addq %%rbx, %%rax\n\t"
        "xorq %[r1], %%rax\n\t"
        "movq %%rax, %[r2]"
        : [r1] "=&r" (result1), [r2] "=&r" (result2)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j)
        : "rax", "rbx", "rdx", "cc"
    );
#elif defined(__aarch64__)
    /* ARM64 inline assembly with many operands */
    asm volatile (
        "mul %x[r1], %x[a], %x[b]\n\t"
        "mul %x[tmp], %x[c], %x[d]\n\t"
        "add %x[r1], %x[r1], %x[tmp]\n\t"
        "mul %x[tmp], %x[e], %x[f]\n\t"
        "add %x[r1], %x[r1], %x[tmp]\n\t"
        "mul %x[tmp], %x[g], %x[h]\n\t"
        "add %x[r1], %x[r1], %x[tmp]\n\t"
        "mul %x[tmp], %x[i], %x[j]\n\t"
        "add %x[r2], %x[r1], %x[tmp]"
        : [r1] "=&r" (result1), [r2] "=&r" (result2), [tmp] "=&r" (result1)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j)
        : "cc"
    );
#elif defined(__powerpc64__)
    /* PowerPC inline assembly with many operands */
    asm volatile (
        "mulld %[r1], %[a], %[b]\n\t"
        "mulld %[tmp], %[c], %[d]\n\t"
        "add %[r1], %[r1], %[tmp]\n\t"
        "mulld %[tmp], %[e], %[f]\n\t"
        "add %[r1], %[r1], %[tmp]\n\t"
        "mulld %[tmp], %[g], %[h]\n\t"
        "add %[r1], %[r1], %[tmp]\n\t"
        "mulld %[tmp], %[i], %[j]\n\t"
        "add %[r2], %[r1], %[tmp]"
        : [r1] "=&r" (result1), [r2] "=&r" (result2), [tmp] "=&r" (result1)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j)
        : "cc"
    );
#else
    /* Generic fallback */
    result1 = a * b + c * d + e * f + g * h;
    result2 = result1 + i * j;
#endif
    
    return result1 ^ result2;
}

/* Function that uses compiler builtins for complex operations */
NOINLINE v4sf builtin_multi_op(v4sf a, v4sf b, v4sf c, v4sf d,
                               v4sf e, v4sf f, v4sf g, v4sf h) {
    /* Chain of operations that might expand to many operands */
    v4sf t1 = __builtin_ia32_addps(a, b);
    v4sf t2 = __builtin_ia32_mulps(c, d);
    v4sf t3 = __builtin_ia32_subps(e, f);
    v4sf t4 = __builtin_ia32_divps(g, h);
    
    v4sf t5 = __builtin_ia32_addps(t1, t2);
    v4sf t6 = __builtin_ia32_subps(t3, t4);
    
    return __builtin_ia32_mulps(t5, t6);
}

/* Multi-precision arithmetic that might need many operands */
NOINLINE __int128 multi_precision_op(uint64_t a, uint64_t b, uint64_t c,
                                     uint64_t d, uint64_t e, uint64_t f,
                                     uint64_t g, uint64_t h) {
    /* 128-bit arithmetic using 64-bit parts */
    __int128 val1 = (__int128)a * b;
    __int128 val2 = (__int128)c * d;
    __int128 val3 = (__int128)e * f;
    __int128 val4 = (__int128)g * h;
    
    /* Complex combination */
    __int128 result = (val1 + val2) * (val3 - val4);
    result = result >> 32;
    result = result * (val1 - val3);
    
    return result;
}

/* Main test driver */
int main(int argc, char *argv[]) {
    uint64_t result = 0;
    
    /* Initialize test values based on argc to get some variation */
    uint64_t vals[10];
    for (int i = 0; i < 10; i++) {
        vals[i] = (uint64_t)(argc + i) * 123456789;
    }
    
    /* Test different code paths based on command line */
    if (argc > 1) {
        /* Path 1: Complex arithmetic with many operands */
        for (int i = 0; i < (argc % 10); i++) {
            result += multi_operand_arith(vals[0], vals[1], vals[2], vals[3],
                                         vals[4], vals[5], vals[6], vals[7],
                                         vals[8], vals[9]);
        }
    } else {
        /* Path 2: Target-specific multi-operand assembly */
        for (int i = 0; i < 5; i++) {
            result += target_specific_multi_operand(vals[0], vals[1], vals[2],
                                                   vals[3], vals[4], vals[5],
                                                   vals[6], vals[7], vals[8],
                                                   vals[9]);
        }
    }
    
    /* Test vector operations */
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v4si vec_c = {9, 10, 11, 12};
    v4si vec_d = {13, 14, 15, 16};
    v4si vec_e = {17, 18, 19, 20};
    v4si vec_f = {21, 22, 23, 24};
    v4si vec_g = {25, 26, 27, 28};
    v4si vec_h = {29, 30, 31, 32};
    
    v4si vec_result = vector_multi_op(vec_a, vec_b, vec_c, vec_d,
                                      vec_e, vec_f, vec_g, vec_h);
    
    /* Use the vector result to prevent dead code elimination */
    for (int i = 0; i < 4; i++) {
        result += vec_result[i];
    }
    
    /* Test multi-precision arithmetic */
    __int128 big_result = multi_precision_op(vals[0], vals[1], vals[2], vals[3],
                                            vals[4], vals[5], vals[6], vals[7]);
    result += (uint64_t)big_result + (uint64_t)(big_result >> 64);
    
    printf("Result: %lu\n", (unsigned long)result);
    
    return 0;
}
