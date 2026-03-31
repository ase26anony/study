/* Test program to trigger 10/11 operand RTL expansion in optabs.cc */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Prevent optimization and ensure expansion happens locally */
#define NOOPT __attribute__((noinline, noipa, optimize("O0")))

/* Vector types for potential multi-operand expansions */
typedef int v4si __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Complex arithmetic that might expand to many operands */
NOOPT uint64_t multi_operand_arithmetic(uint64_t a, uint64_t b, uint64_t c,
                                        uint64_t d, uint64_t e, uint64_t f,
                                        uint64_t g, uint64_t h, uint64_t i) {
    /* Complex expression that might expand to many operations */
    uint64_t t1 = (a * b) >> 32;
    uint64_t t2 = (c * d) >> 32;
    uint64_t t3 = (e * f) >> 32;
    uint64_t t4 = (g * h) >> 32;
    
    /* Multi-step computation */
    uint64_t result = t1 + t2;
    result = (result * t3) >> 32;
    result = (result + t4) * i;
    result = (result >> 32) + (a & b & c & d & e & f & g & h & i);
    
    return result;
}

/* Function using inline assembly with many operands */
NOOPT uint64_t inline_asm_multi_operand(uint64_t a, uint64_t b, uint64_t c,
                                        uint64_t d, uint64_t e, uint64_t f,
                                        uint64_t g, uint64_t h, uint64_t i,
                                        uint64_t j) {
    uint64_t result1, result2;
    
#ifdef __x86_64__
    /* x86_64 inline assembly with many operands */
    asm volatile (
        "movq %[a], %%rax\n\t"
        "addq %[b], %%rax\n\t"
        "adcq %[c], %%rax\n\t"
        "adcq %[d], %%rax\n\t"
        "adcq %[e], %%rax\n\t"
        "adcq %[f], %%rax\n\t"
        "adcq %[g], %%rax\n\t"
        "adcq %[h], %%rax\n\t"
        "adcq %[i], %%rax\n\t"
        "movq %%rax, %[r1]\n\t"
        "imulq %[j], %[r1]\n\t"
        "movq %[r1], %[r2]"
        : [r1] "=&r" (result1), [r2] "=r" (result2)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j)
        : "rax", "cc"
    );
#elif defined(__aarch64__)
    /* ARM64 inline assembly with many operands */
    asm volatile (
        "add %x[r1], %x[a], %x[b]\n\t"
        "add %x[r1], %x[r1], %x[c]\n\t"
        "add %x[r1], %x[r1], %x[d]\n\t"
        "add %x[r1], %x[r1], %x[e]\n\t"
        "add %x[r1], %x[r1], %x[f]\n\t"
        "add %x[r1], %x[r1], %x[g]\n\t"
        "add %x[r1], %x[r1], %x[h]\n\t"
        "add %x[r1], %x[r1], %x[i]\n\t"
        "mul %x[r2], %x[r1], %x[j]"
        : [r1] "=&r" (result1), [r2] "=r" (result2)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j)
    );
#else
    /* Generic fallback */
    result1 = a + b + c + d + e + f + g + h + i;
    result2 = result1 * j;
#endif
    
    return result1 + result2;
}

/* Vector operations that might expand to many operands */
NOOPT v4si vector_multi_operand(v4si a, v4si b, v4si c, v4si d) {
    /* Complex vector expression */
    v4si t1 = a * b + c;
    v4si t2 = b * c + d;
    v4si t3 = c * d + a;
    v4si t4 = d * a + b;
    
    /* Multi-step vector computation */
    v4si result = (t1 & t2) | (t3 ^ t4);
    result = result + (a >> 2) + (b >> 3) + (c >> 4) + (d >> 5);
    
    /* Element-wise operations */
    result[0] = (result[0] * a[0] + b[0] * c[0]) >> 4;
    result[1] = (result[1] * a[1] + b[1] * c[1]) >> 4;
    result[2] = (result[2] * a[2] + b[2] * c[2]) >> 4;
    result[3] = (result[3] * a[3] + b[3] * c[3]) >> 4;
    
    return result;
}

/* Mixed operations to increase chances of hitting 10/11 operands */
NOOPT uint64_t mixed_operations(uint64_t a, uint64_t b, uint64_t c,
                                uint64_t d, uint64_t e, uint64_t f) {
    /* Create many intermediate values */
    uint64_t t1 = (a * b) >> 32;
    uint64_t t2 = (c * d) >> 32;
    uint64_t t3 = (e * f) >> 32;
    uint64_t t4 = (a + b + c + d + e + f) & 0xFFFFFFFF;
    uint64_t t5 = (a ^ b ^ c ^ d ^ e ^ f);
    uint64_t t6 = (a & b & c & d & e & f);
    uint64_t t7 = (a | b | c | d | e | f);
    uint64_t t8 = (a << 5) | (b << 4) | (c << 3) | (d << 2) | (e << 1) | f;
    
    /* Complex computation with many operands */
    uint64_t result = t1 + t2;
    result = (result * t3 + t4) >> 16;
    result = (result ^ t5) & t6;
    result = result | t7;
    result = result + t8;
    result = (result * a + b * c + d * e + f * t1) >> 8;
    
    return result;
}

/* Test function that combines all approaches */
NOOPT uint64_t test_combined(uint64_t seed) {
    uint64_t a = seed + 1;
    uint64_t b = seed + 2;
    uint64_t c = seed + 3;
    uint64_t d = seed + 4;
    uint64_t e = seed + 5;
    uint64_t f = seed + 6;
    uint64_t g = seed + 7;
    uint64_t h = seed + 8;
    uint64_t i = seed + 9;
    uint64_t j = seed + 10;
    
    uint64_t result = 0;
    
    /* Try different code paths based on seed */
    if (seed & 1) {
        result += multi_operand_arithmetic(a, b, c, d, e, f, g, h, i);
    }
    
    if (seed & 2) {
        result += inline_asm_multi_operand(a, b, c, d, e, f, g, h, i, j);
    }
    
    if (seed & 4) {
        /* Use vector operations */
        v4si va = {a, b, c, d};
        v4si vb = {b, c, d, e};
        v4si vc = {c, d, e, f};
        v4si vd = {d, e, f, g};
        v4si vresult = vector_multi_operand(va, vb, vc, vd);
        result += vresult[0] + vresult[1] + vresult[2] + vresult[3];
    }
    
    if (seed & 8) {
        result += mixed_operations(a, b, c, d, e, f);
    }
    
    return result;
}

int main(int argc, char *argv[]) {
    uint64_t total = 0;
    
    /* Use command line arguments to vary execution */
    int iterations = (argc > 1) ? atoi(argv[1]) : 100;
    int start = (argc > 2) ? atoi(argv[2]) : 0;
    
    /* Loop to increase coverage chances */
    for (int n = start; n < start + iterations; n++) {
        /* Call with different seeds to trigger different expansions */
        uint64_t result = test_combined(n);
        
        /* Use result to prevent dead code elimination */
        total += result;
        
        /* Occasionally print to prevent optimization */
        if (n % 1000 == 0) {
            printf("Progress: n=%d, result=%lu, total=%lu\n", 
                   n, (unsigned long)result, (unsigned long)total);
        }
    }
    
    printf("Final total: %lu\n", (unsigned long)total);
    return (total > 0) ? 0 : 1;
}
