/* Test program to cover 10/11 operand expansion in optabs.cc */
#include <stdio.h>
#include <string.h>
#include <x86intrin.h>

/* Define large vector types */
typedef int v8si __attribute__((vector_size(32)));
typedef long long v4di __attribute__((vector_size(32)));
typedef float v8sf __attribute__((vector_size(32)));
typedef double v4df __attribute__((vector_size(32)));

/* Prevent optimization */
#define KEEP(expr) do { \
    volatile __typeof__(expr) _tmp = (expr); \
    asm volatile("" : : "r"(&_tmp) : "memory"); \
} while(0)

/* Compiler barrier */
#define BARRIER() asm volatile("" : : : "memory")

/* Noinline test function with many vector operations */
__attribute__((noinline, target("avx2")))
v8si test_10_11_operands(v8si a, v8si b, v8si c, v8si d, 
                         v8si e, v8si f, v8si g, v8si h) {
    volatile v8si v1, v2, v3, v4, v5, v6, v7, v8;
    
    /* Complex shuffle operation - may require many operands */
    v8si shuffle_mask = {7, 6, 5, 4, 3, 2, 1, 0};
    v1 = __builtin_shuffle(a, b, shuffle_mask);
    BARRIER();
    
    /* Vector conditional expression with comparison */
    v8si cmp_mask = a > b;
    v2 = cmp_mask ? (c * d + e) : (f - g * h);
    BARRIER();
    
    /* Chain of operations that may expand to many operands */
    v3 = a + b * c - d / (e + 1) + f % (g + 2) * h;
    BARRIER();
    
    /* Another shuffle with different pattern */
    v8si shuffle_mask2 = {0, 2, 4, 6, 1, 3, 5, 7};
    v4 = __builtin_shuffle(v1, v2, shuffle_mask2);
    BARRIER();
    
    /* Blend operation using conditional */
    v8si blend_mask = v3 > v4;
    v5 = blend_mask ? v1 : v2;
    v6 = blend_mask ? v3 : v4;
    BARRIER();
    
    /* Final complex expression combining everything */
    v7 = (v5 + v6) * (v1 - v2) / (v3 + v4) % (a + b);
    BARRIER();
    
    /* Use volatile stores to force operations */
    v8 = v7;
    KEEP(v8);
    
    return v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8;
}

/* Test with AVX-512 types for even more operands */
#ifdef __AVX512F__
typedef int v16si __attribute__((vector_size(64)));
typedef double v8df __attribute__((vector_size(64)));

__attribute__((noinline, target("avx512f")))
v16si test_avx512_operands(v16si a, v16si b, v16si c, v16si d) {
    /* Complex AVX-512 operation that may need many operands */
    v16si mask = a > b;
    v16si res = mask ? (c * d + a) : (b - c * d);
    
    /* Shuffle with 16-element mask */
    v16si shuffle_mask = {15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0};
    v16si shuffled = __builtin_shuffle(res, shuffle_mask);
    
    /* Multiple operations chained */
    v16si final = shuffled + (a & b) | (c ^ d) * (a | b);
    
    volatile v16si keeper = final;
    asm volatile("" : : "r"(&keeper) : "memory");
    
    return final;
}
#endif

/* Test with floating point vectors */
__attribute__((noinline, target("avx")))
v4df test_fp_operands(v4df a, v4df b, v4df c, v4df d,
                      v4df e, v4df f, v4df g, v4df h) {
    /* FP comparison and conditional */
    v4df cmp = a > b;
    v4df res1 = cmp ? (c * d) : (e / f);
    BARRIER();
    
    /* Complex FP expression */
    v4df res2 = (a + b) * (c - d) / (e + f) - (g * h);
    BARRIER();
    
    /* Blend based on comparison */
    v4df blend_mask = res1 > res2;
    v4df final = blend_mask ? res1 : res2;
    
    /* Force use of all operands in final computation */
    final = final + a + b + c + d + e + f + g + h;
    
    volatile v4df keeper = final;
    asm volatile("" : : "r"(&keeper) : "memory");
    
    return final;
}

int main() {
    /* Initialize test vectors with pattern */
    v8si a = {1, 2, 3, 4, 5, 6, 7, 8};
    v8si b = {8, 7, 6, 5, 4, 3, 2, 1};
    v8si c = {2, 3, 4, 5, 6, 7, 8, 9};
    v8si d = {9, 8, 7, 6, 5, 4, 3, 2};
    v8si e = {3, 4, 5, 6, 7, 8, 9, 10};
    v8si f = {10, 9, 8, 7, 6, 5, 4, 3};
    v8si g = {4, 5, 6, 7, 8, 9, 10, 11};
    v8si h = {11, 10, 9, 8, 7, 6, 5, 4};
    
    /* Call test function */
    v8si result = test_10_11_operands(a, b, c, d, e, f, g, h);
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < 8; i++) {
        checksum += result[i];
    }
    
    /* Test FP vectors */
    v4df fa = {1.0, 2.0, 3.0, 4.0};
    v4df fb = {4.0, 3.0, 2.0, 1.0};
    v4df fc = {1.5, 2.5, 3.5, 4.5};
    v4df fd = {4.5, 3.5, 2.5, 1.5};
    v4df fe = {0.1, 0.2, 0.3, 0.4};
    v4df ff = {0.4, 0.3, 0.2, 0.1};
    v4df fg = {1.1, 2.2, 3.3, 4.4};
    v4df fh = {4.4, 3.3, 2.2, 1.1};
    
    v4df fp_result = test_fp_operands(fa, fb, fc, fd, fe, ff, fg, fh);
    
    /* Add FP checksum */
    for (int i = 0; i < 4; i++) {
        checksum += (int)fp_result[i];
    }
    
#ifdef __AVX512F__
    /* Test AVX-512 if available */
    v16si avx512_a = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
    v16si avx512_b = {16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1};
    v16si avx512_c = {2,4,6,8,10,12,14,16,18,20,22,24,26,28,30,32};
    v16si avx512_d = {32,30,28,26,24,22,20,18,16,14,12,10,8,6,4,2};
    
    v16si avx512_result = test_avx512_operands(avx512_a, avx512_b, 
                                               avx512_c, avx512_d);
    
    for (int i = 0; i < 16; i++) {
        checksum += avx512_result[i];
    }
#endif
    
    printf("Checksum: %d\n", checksum);
    
    /* Return based on checksum to ensure execution */
    return checksum != 0 ? 0 : 1;
}
