/* Test program to cover 10/11 operand expansion in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <x86intrin.h>

/* Define large vector types */
typedef int v8si __attribute__((vector_size(32)));
typedef float v8sf __attribute__((vector_size(32)));
typedef double v4df __attribute__((vector_size(32)));
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Force no optimization on specific operations */
#define KEEP(expr) do { \
    volatile __typeof__(expr) _tmp = (expr); \
    asm volatile("" : : "r"(&_tmp) : "memory"); \
} while(0)

/* Compiler barrier */
#define BARRIER() asm volatile("" : : : "memory")

/* Noinline to prevent optimization across function boundaries */
__attribute__((noinline, target("avx2")))
v8si test_10_operand_expansion(v8si a, v8si b, v8si c, v8si d, 
                               v8si e, v8si f, v8si mask) {
    volatile v8si v1, v2, v3, v4, v5;
    
    /* Complex shuffle operation that may require many operands */
    v8si shuffle1 = __builtin_shuffle(a, b, 
        (v8si){0, 9, 2, 11, 4, 13, 6, 15});
    KEEP(shuffle1);
    BARRIER();
    
    /* Another shuffle with different pattern */
    v8si shuffle2 = __builtin_shuffle(c, d,
        (v8si){15, 14, 13, 12, 7, 6, 5, 4});
    KEEP(shuffle2);
    BARRIER();
    
    /* Vector conditional with comparison - may generate VEC_COND_EXPR */
    v8si cmp = (mask > (v8si){0, 1, 2, 3, 4, 5, 6, 7}) ? shuffle1 : shuffle2;
    KEEP(cmp);
    BARRIER();
    
    /* Complex arithmetic chain */
    v8si t1 = a * b + c;
    v8si t2 = d - e * f;
    v8si t3 = t1 / (t2 + (v8si){1, 1, 1, 1, 1, 1, 1, 1});
    KEEP(t3);
    BARRIER();
    
    /* Blend operation using conditional */
    v8si blend = (cmp > t3) ? (a & b) | c : (d ^ e) & f;
    KEEP(blend);
    BARRIER();
    
    /* Final combination */
    v8si result = blend + shuffle1 - shuffle2 * t3;
    return result;
}

__attribute__((noinline, target("avx2")))
v4df test_11_operand_expansion(v4df a, v4df b, v4df c, v4df d,
                               v4df e, v4df f, v4df g) {
    volatile v4df v1, v2, v3;
    
    /* Vector comparison generating mask */
    v4df cmp_mask = (a > b) ? (v4df){1.0, 1.0, 1.0, 1.0} 
                            : (v4df){0.0, 0.0, 0.0, 0.0};
    KEEP(cmp_mask);
    BARRIER();
    
    /* Complex conditional with arithmetic */
    v4df cond1 = (c > d) ? a * b : c / d;
    v4df cond2 = (e > f) ? b + c : d - e;
    KEEP(cond1);
    KEEP(cond2);
    BARRIER();
    
    /* Shuffle with conversion-like pattern */
    v4df shuffle1 = __builtin_shuffle(a, b, (v4si){0, 1, 4, 5});
    v4df shuffle2 = __builtin_shuffle(c, d, (v4si){2, 3, 6, 7});
    KEEP(shuffle1);
    KEEP(shuffle2);
    BARRIER();
    
    /* Nested conditional expression */
    v4df nested = (cmp_mask > (v4df){0.5, 0.5, 0.5, 0.5}) 
                  ? (cond1 * cond2) 
                  : (shuffle1 + shuffle2);
    KEEP(nested);
    BARRIER();
    
    /* Final computation with many operands */
    v4df result = nested * a + b / c - d * e + f / g;
    return result;
}

/* Test with AVX-512 if available */
#ifdef __AVX512F__
typedef int v16si __attribute__((vector_size(64)));
typedef float v16sf __attribute__((vector_size(64)));

__attribute__((noinline, target("avx512f")))
v16si test_avx512_many_operands(v16si a, v16si b, v16si c, v16si d,
                                v16si e, v16si f, v16si mask) {
    /* AVX-512 operations with masking can generate many operands */
    v16si masked_add = __builtin_ia32_paddd512_mask(a, b, c, 
        (__mmask16)0xAAAA, 0);
    v16si masked_blend = __builtin_ia32_blendmd_512_mask(a, b, c,
        (__mmask16)0x5555);
    
    /* Complex shuffle */
    v16si shuffle = __builtin_shuffle(masked_add, masked_blend,
        (v16si){0, 17, 2, 19, 4, 21, 6, 23, 
                8, 25, 10, 27, 12, 29, 14, 31});
    
    /* Conditional with mask */
    v16si result = (mask > shuffle) ? a & b : c | d;
    return result;
}
#endif

int main() {
    /* Initialize vectors with pattern values */
    v8si a8 = {1, 2, 3, 4, 5, 6, 7, 8};
    v8si b8 = {8, 7, 6, 5, 4, 3, 2, 1};
    v8si c8 = {2, 4, 6, 8, 10, 12, 14, 16};
    v8si d8 = {16, 14, 12, 10, 8, 6, 4, 2};
    v8si e8 = {3, 6, 9, 12, 15, 18, 21, 24};
    v8si f8 = {24, 21, 18, 15, 12, 9, 6, 3};
    v8si mask8 = {0, 1, 0, 1, 0, 1, 0, 1};
    
    v4df ad = {1.0, 2.0, 3.0, 4.0};
    v4df bd = {4.0, 3.0, 2.0, 1.0};
    v4df cd = {1.5, 2.5, 3.5, 4.5};
    v4df dd = {4.5, 3.5, 2.5, 1.5};
    v4df ed = {0.5, 1.5, 2.5, 3.5};
    v4df fd = {3.5, 2.5, 1.5, 0.5};
    v4df gd = {2.0, 2.0, 2.0, 2.0};
    
    /* Call test functions */
    v8si res8 = test_10_operand_expansion(a8, b8, c8, d8, e8, f8, mask8);
    v4df resd = test_11_operand_expansion(ad, bd, cd, dd, ed, fd, gd);
    
    /* Compute checksums to prevent dead code elimination */
    int sum8 = 0;
    double sumd = 0.0;
    
    for (int i = 0; i < 8; i++) {
        sum8 += res8[i];
    }
    
    for (int i = 0; i < 4; i++) {
        sumd += resd[i];
    }
    
    /* Use results to affect program output */
    printf("Integer checksum: %d\n", sum8);
    printf("Double checksum: %f\n", sumd);
    
    /* Return based on checksums to ensure execution */
    return (sum8 > 0 && sumd > 0.0) ? 0 : 1;
}
