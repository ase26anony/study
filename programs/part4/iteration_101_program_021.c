/* Test program to cover 10/11 operand expansion in optabs.cc */
#include <stdio.h>
#include <string.h>
#include <stdint.h>

/* Use GCC vector extensions for portability */
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
typedef int v16si __attribute__((vector_size(64)));
typedef float v16sf __attribute__((vector_size(64)));

/* Compiler barrier to prevent optimization */
#define COMPILER_BARRIER() asm volatile("" ::: "memory")

/* Noinline function to prevent optimization across calls */
__attribute__((noinline, target("avx2")))
v8si test_10_operand_expansion(v8si a, v8si b, v8si c, v8si d, 
                               v8si e, v8si f, v8si g, v8si h) {
    volatile v8si v1, v2, v3, v4, v5, v6, v7, v8;
    
    /* Complex shuffle operation - may require many operands */
    v8si shuffle_mask = {7, 6, 5, 4, 3, 2, 1, 0};
    v8si shuffled1 = __builtin_shuffle(a, b, shuffle_mask);
    v8si shuffled2 = __builtin_shuffle(c, d, shuffle_mask);
    
    COMPILER_BARRIER();
    
    /* Vector conditional expression with comparison */
    v8si cmp_mask = a > b;
    v8si cond_result = cmp_mask ? (shuffled1 * c) : (shuffled2 / (d + 1));
    
    COMPILER_BARRIER();
    
    /* Chain of arithmetic operations */
    v8si temp1 = a * b + c;
    v8si temp2 = d - e * f;
    v8si temp3 = g / (h + 1);
    
    /* Complex blend-like operation using conditional */
    v8si blend_mask = (temp1 > temp2);
    v8si blended = blend_mask ? temp1 : temp2;
    
    /* Combine everything */
    v8si result = cond_result + blended * temp3 - shuffled1 + shuffled2;
    
    /* Force memory operations */
    v1 = a;
    v2 = b;
    v3 = result;
    v4 = shuffled1;
    v5 = shuffled2;
    v6 = cond_result;
    v7 = blended;
    v8 = temp3;
    
    COMPILER_BARRIER();
    
    return result;
}

/* Test AVX-512 patterns which may require more operands */
#ifdef __AVX512F__
__attribute__((noinline, target("avx512f")))
v16si test_avx512_11_operand(v16si a, v16si b, v16si c, v16si d,
                            v16si e, v16si f, v16si g, v16si h) {
    volatile v16si v1, v2, v3;
    
    /* Create a complex mask for conditional operations */
    v16si mask1 = a > b;
    v16si mask2 = c < d;
    
    /* Complex conditional with multiple operations */
    v16si temp1 = a * b + c * d;
    v16si temp2 = e * f - g * h;
    
    /* Nested conditional expression */
    v16si result = mask1 ? (mask2 ? temp1 : temp2) : (temp1 - temp2);
    
    /* Shuffle with runtime mask */
    int shuffle_indices[16];
    for (int i = 0; i < 16; i++) {
        shuffle_indices[i] = (i * 3) % 16;
    }
    v16si shuffle_vec = __builtin_shufflevector(a, b, 
        shuffle_indices[0], shuffle_indices[1], shuffle_indices[2],
        shuffle_indices[3], shuffle_indices[4], shuffle_indices[5],
        shuffle_indices[6], shuffle_indices[7], shuffle_indices[8],
        shuffle_indices[9], shuffle_indices[10], shuffle_indices[11],
        shuffle_indices[12], shuffle_indices[13], shuffle_indices[14],
        shuffle_indices[15]);
    
    result = result + shuffle_vec;
    
    /* Force memory operations */
    v1 = a;
    v2 = b;
    v3 = result;
    
    COMPILER_BARRIER();
    
    return result;
}
#endif

/* Test with floating point vectors */
__attribute__((noinline, target("avx")))
v4df test_fp_10_operand(v4df a, v4df b, v4df c, v4df d,
                        v4df e, v4df f, v4df g, v4df h) {
    volatile v4df v1, v2, v3;
    
    /* Complex FP arithmetic chain */
    v4df temp1 = a * b + c;
    v4df temp2 = d - e * f;
    v4df temp3 = g / (h + 1.0);
    
    /* FP comparison and conditional */
    v4df cmp = a > b;
    v4df result = cmp ? (temp1 * temp2) : (temp3 / temp1);
    
    /* Additional operations to increase operand count */
    result = result + __builtin_ia32_roundpd256(result, 0);  /* Round to nearest */
    
    /* Blend operation */
    v4df blend_mask = {1.0, -1.0, 1.0, -1.0};
    v4df blended = __builtin_ia32_blendvpd256(result, temp1, blend_mask);
    
    result = result * blended - temp2 + temp3;
    
    /* Force memory operations */
    v1 = a;
    v2 = b;
    v3 = result;
    
    COMPILER_BARRIER();
    
    return result;
}

int main() {
    /* Initialize test vectors with pattern values */
    v8si a = {1, 2, 3, 4, 5, 6, 7, 8};
    v8si b = {8, 7, 6, 5, 4, 3, 2, 1};
    v8si c = {2, 3, 4, 5, 6, 7, 8, 9};
    v8si d = {9, 8, 7, 6, 5, 4, 3, 2};
    v8si e = {3, 4, 5, 6, 7, 8, 9, 10};
    v8si f = {10, 9, 8, 7, 6, 5, 4, 3};
    v8si g = {4, 5, 6, 7, 8, 9, 10, 11};
    v8si h = {11, 10, 9, 8, 7, 6, 5, 4};
    
    /* Test 10-operand expansion */
    v8si result1 = test_10_operand_expansion(a, b, c, d, e, f, g, h);
    
    /* Test FP 10-operand expansion */
    v4df fa = {1.0, 2.0, 3.0, 4.0};
    v4df fb = {4.0, 3.0, 2.0, 1.0};
    v4df fc = {2.0, 3.0, 4.0, 5.0};
    v4df fd = {5.0, 4.0, 3.0, 2.0};
    v4df fe = {3.0, 4.0, 5.0, 6.0};
    v4df ff = {6.0, 5.0, 4.0, 3.0};
    v4df fg = {4.0, 5.0, 6.0, 7.0};
    v4df fh = {7.0, 6.0, 5.0, 4.0};
    
    v4df result2 = test_fp_10_operand(fa, fb, fc, fd, fe, ff, fg, fh);
    
    #ifdef __AVX512F__
    /* Test AVX-512 for 11-operand expansion */
    v16si a512 = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
    v16si b512 = {16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1};
    v16si c512 = {2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17};
    v16si d512 = {17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2};
    v16si e512 = {3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18};
    v16si f512 = {18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3};
    v16si g512 = {4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19};
    v16si h512 = {19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4};
    
    v16si result3 = test_avx512_11_operand(a512, b512, c512, d512, 
                                          e512, f512, g512, h512);
    #endif
    
    /* Compute checksums to prevent dead code elimination */
    int checksum1 = 0;
    for (int i = 0; i < 8; i++) {
        checksum1 += result1[i];
    }
    
    double checksum2 = 0.0;
    for (int i = 0; i < 4; i++) {
        checksum2 += result2[i];
    }
    
    #ifdef __AVX512F__
    int checksum3 = 0;
    for (int i = 0; i < 16; i++) {
        checksum3 += result3[i];
    }
    #endif
    
    /* Print results to ensure execution */
    printf("Checksum1: %d\n", checksum1);
    printf("Checksum2: %f\n", checksum2);
    #ifdef __AVX512F__
    printf("Checksum3: %d\n", checksum3);
    #endif
    
    return 0;
}
