/* Test program to cover 10/11 operand expansion in optabs.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __SSE2__
#include <emmintrin.h>
#endif
#ifdef __AVX__
#include <immintrin.h>
#endif

/* Define large vector types */
typedef int v8si __attribute__((vector_size(32)));
typedef float v8sf __attribute__((vector_size(32)));
typedef double v4df __attribute__((vector_size(32)));
typedef long long v4di __attribute__((vector_size(32)));

/* Test function with many vector operations - marked noinline to prevent optimization */
__attribute__((noinline, target("avx2")))
v8si test_many_operands(v8si a, v8si b, v8si c, v8si d, v8si e, v8si mask_vec) {
    volatile v8si temp1, temp2, temp3, temp4;
    v8si result, shuffle_result, blend_result, cond_result;
    
    /* Compiler barrier to prevent reordering */
    asm volatile("" ::: "memory");
    
    /* Complex shuffle operation - may require many operands */
    /* Using non-constant mask to force runtime shuffle */
    shuffle_result = __builtin_shuffle(a, b, mask_vec);
    temp1 = shuffle_result;  /* volatile store to force operation */
    
    /* Another shuffle with different vectors */
    v8si mask2 = {7, 6, 5, 4, 3, 2, 1, 0};
    v8si shuffle2 = __builtin_shuffle(c, d, mask2);
    temp2 = shuffle2;
    
    /* Complex arithmetic chain - each operation may expand to multiple insns */
    v8si arith1 = a * b + c;
    v8si arith2 = d - e * a;
    v8si arith3 = arith1 / (arith2 + 1);
    temp3 = arith3;
    
    /* Vector conditional expression - generates VEC_COND_EXPR */
    v8si cmp_mask = a > b;
    cond_result = cmp_mask ? (c * d) : (e + a);
    temp4 = cond_result;
    
    /* Blend operation using conditional */
    v8si blend_mask = b > c;
    v8si blended = blend_mask ? arith1 : arith2;
    
    /* Combine all results */
    result = shuffle_result + shuffle2 + arith3 + cond_result + blended;
    
    /* Final compiler barrier */
    asm volatile("" ::: "memory");
    
    return result;
}

/* Second test function targeting 11 operands with AVX-512 style operations */
#ifdef __AVX512F__
typedef int v16si __attribute__((vector_size(64)));
typedef float v16sf __attribute__((vector_size(64)));

__attribute__((noinline, target("avx512f")))
v16si test_11_operands(v16si a, v16si b, v16si c, v16si d, 
                       v16si e, v16si f, v16si mask) {
    /* Complex operation chain that might require 11 operands */
    v16si temp1, temp2, temp3;
    
    /* Multiple shuffles with large vectors */
    v16si shuffle1 = __builtin_shufflevector(a, b, 
        0, 16, 1, 17, 2, 18, 3, 19, 4, 20, 5, 21, 6, 22, 7, 23);
    v16si shuffle2 = __builtin_shufflevector(c, d,
        15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0);
    
    /* Conditional blend with mask */
    v16si cmp = a > b;
    v16si blended = cmp ? shuffle1 : shuffle2;
    
    /* More arithmetic */
    v16si arith1 = a * b + c * d;
    v16si arith2 = e * f - a;
    v16si arith3 = arith1 / (arith2 + 1);
    
    /* Combine with mask */
    v16si final = blended + arith3;
    
    /* Store to volatile to force computation */
    volatile v16si vol_store = final;
    
    return final;
}
#endif

/* Test with floating point vectors */
__attribute__((noinline, target("avx")))
v4df test_fp_many_operands(v4df a, v4df b, v4df c, v4df d, v4di mask) {
    volatile v4df temp1, temp2;
    v4df result;
    
    /* Shuffle double precision vectors */
    v4df shuffle1 = __builtin_shuffle(a, b, mask);
    temp1 = shuffle1;
    
    /* Conditional expression with FP */
    v4df cmp_mask = a > b;
    v4df cond_result = cmp_mask ? (c * d) : (c + d);
    temp2 = cond_result;
    
    /* Complex FP arithmetic */
    v4df arith1 = a * b + c / d;
    v4df arith2 = __builtin_shuffle(arith1, cond_result, mask);
    
    result = shuffle1 + cond_result + arith2;
    
    return result;
}

int main() {
    int checksum = 0;
    
    /* Initialize 8-element integer vectors */
    v8si a = {1, 2, 3, 4, 5, 6, 7, 8};
    v8si b = {8, 7, 6, 5, 4, 3, 2, 1};
    v8si c = {2, 4, 6, 8, 10, 12, 14, 16};
    v8si d = {1, 3, 5, 7, 9, 11, 13, 15};
    v8si e = {5, 5, 5, 5, 5, 5, 5, 5};
    v8si mask = {0, 8, 1, 9, 2, 10, 3, 11};  /* Interleaving shuffle mask */
    
    /* Call test function */
    v8si result = test_many_operands(a, b, c, d, e, mask);
    
    /* Compute checksum to prevent dead code elimination */
    for (int i = 0; i < 8; i++) {
        checksum += result[i];
    }
    
    /* Test with floating point if AVX is available */
    #ifdef __AVX__
    v4df fa = {1.0, 2.0, 3.0, 4.0};
    v4df fb = {4.0, 3.0, 2.0, 1.0};
    v4df fc = {1.5, 2.5, 3.5, 4.5};
    v4df fd = {0.5, 1.5, 2.5, 3.5};
    v4di fmask = {0, 4, 1, 5};
    
    v4df fp_result = test_fp_many_operands(fa, fb, fc, fd, fmask);
    
    /* Add FP checksum (convert to int) */
    for (int i = 0; i < 4; i++) {
        checksum += (int)fp_result[i];
    }
    #endif
    
    /* Test AVX-512 path if available */
    #ifdef __AVX512F__
    v16si a16 = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
    v16si b16 = {16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1};
    v16si c16 = {2,4,6,8,10,12,14,16,18,20,22,24,26,28,30,32};
    v16si d16 = {1,3,5,7,9,11,13,15,17,19,21,23,25,27,29,31};
    v16si e16 = {5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5};
    v16si f16 = {3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3};
    v16si mask16 = {0,16,1,17,2,18,3,19,4,20,5,21,6,22,7,23};
    
    v16si result16 = test_11_operands(a16, b16, c16, d16, e16, f16, mask16);
    
    for (int i = 0; i < 16; i++) {
        checksum += result16[i];
    }
    #endif
    
    printf("Checksum: %d\n", checksum);
    
    /* Return based on checksum to ensure execution */
    return (checksum > 1000) ? 0 : 1;
}
