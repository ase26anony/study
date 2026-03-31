/* Test program to cover 10/11 operand expansion in optabs.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Use GCC vector extensions for portable vector operations */
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
__attribute__((noinline, noipa))
v8si test_many_operands(v8si a, v8si b, v8si c, v8si d, v8si e, v8si mask) {
    volatile v8si temp1, temp2, temp3, temp4;
    v8si result;
    
    /* Compiler barrier to prevent reordering */
    asm volatile("" ::: "memory");
    
    /* Complex shuffle operation - may require many operands */
    /* __builtin_shuffle with variable mask forces runtime shuffling */
    v8si shuffled = __builtin_shuffle(a, b, mask);
    temp1 = shuffled;  /* volatile store forces operation */
    
    /* Vector conditional expression with comparison */
    /* This generates VEC_COND_EXPR which may expand to many operands */
    v8si cmp_result = (mask > (v8si){0,1,2,3,4,5,6,7}) 
                     ? (a * b + c) 
                     : (d - e * a);
    temp2 = cmp_result;
    
    /* Another barrier */
    asm volatile("" ::: "memory");
    
    /* Chain of operations that may require temporary registers */
    /* Each operation potentially adds operands */
    v8si chain1 = a + b;
    v8si chain2 = chain1 * c;
    v8si chain3 = chain2 - d;
    v8si chain4 = chain3 / (e + (v8si){1,1,1,1,1,1,1,1});
    
    /* Complex blend-like operation using conditional */
    v8si blend = (mask & 1) ? chain4 : shuffled;
    temp3 = blend;
    
    /* Final combination with many operands */
    result = blend + cmp_result + chain4;
    temp4 = result;
    
    /* Final barrier */
    asm volatile("" ::: "memory");
    
    return result;
}

/* Another test function focusing on floating-point vectors */
__attribute__((noinline, noipa))
v4df test_fp_many_operands(v4df a, v4df b, v4df c, v4df d, v4di mask) {
    volatile v4df temp1, temp2;
    v4df result;
    
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /* Complex FP conditional with comparison */
    v4df cmp = (v4df)(mask > (v4di){0,1,2,3});
    v4df true_val = a * b + c;
    v4df false_val = d / (a + 1.0);
    
    /* This conditional may expand to many operands */
    v4df cond_result = cmp ? true_val : false_val;
    temp1 = cond_result;
    
    /* Shuffle with conversion - may require extra operands */
    v4di int_vec = (v4di){0, 1, 2, 3};
    v4df converted = (v4df)int_vec;
    
    /* Complex expression with many terms */
    result = cond_result * converted + a - b / c;
    temp2 = result;
    
    asm volatile("" ::: "memory");
    
    return result;
}

/* Test with AVX-512 style 512-bit vectors if available */
#ifdef __AVX512F__
typedef int v16si __attribute__((vector_size(64)));
typedef float v16sf __attribute__((vector_size(64)));

__attribute__((noinline, noipa))
v16si test_avx512_many_operands(v16si a, v16si b, v16si c, v16si mask) {
    volatile v16si temp;
    
    /* Very complex shuffle with mask - likely needs many operands */
    v16si shuffled1 = __builtin_shuffle(a, b, mask);
    v16si shuffled2 = __builtin_shuffle(b, c, mask + 1);
    
    /* Nested conditional */
    v16si cond1 = (mask > 0) ? (a * b) : (c + 1);
    v16si cond2 = (mask < 10) ? (b - a) : (c * 2);
    
    /* Combine with many operations */
    v16si result = shuffled1 + shuffled2 * cond1 - cond2 / (a + 1);
    temp = result;
    
    asm volatile("" ::: "memory");
    
    return result;
}
#endif

int main() {
    /* Initialize vectors with pattern values */
    v8si a = {1, 2, 3, 4, 5, 6, 7, 8};
    v8si b = {8, 7, 6, 5, 4, 3, 2, 1};
    v8si c = {2, 4, 6, 8, 10, 12, 14, 16};
    v8si d = {16, 14, 12, 10, 8, 6, 4, 2};
    v8si e = {3, 6, 9, 12, 15, 18, 21, 24};
    v8si mask = {0, 2, 4, 6, 1, 3, 5, 7};  /* Shuffle mask */
    
    /* Call test function */
    v8si result = test_many_operands(a, b, c, d, e, mask);
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < 8; i++) {
        checksum += result[i];
    }
    
    /* Test floating-point version */
    v4df fa = {1.0, 2.0, 3.0, 4.0};
    v4df fb = {2.0, 3.0, 4.0, 5.0};
    v4df fc = {0.5, 1.5, 2.5, 3.5};
    v4df fd = {4.0, 3.0, 2.0, 1.0};
    v4di fmask = {0, 1, 0, 1};
    
    v4df fresult = test_fp_many_operands(fa, fb, fc, fd, fmask);
    
    double fchecksum = 0;
    for (int i = 0; i < 4; i++) {
        fchecksum += fresult[i];
    }
    
    /* Test AVX-512 version if available */
#ifdef __AVX512F__
    v16si a512 = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
    v16si b512 = {16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1};
    v16si c512 = {2,4,6,8,10,12,14,16,18,20,22,24,26,28,30,32};
    v16si mask512 = {0,2,4,6,8,10,12,14,1,3,5,7,9,11,13,15};
    
    v16si result512 = test_avx512_many_operands(a512, b512, c512, mask512);
    
    int checksum512 = 0;
    for (int i = 0; i < 16; i++) {
        checksum512 += result512[i];
    }
    
    printf("Checksums: %d (SSE), %.2f (FP), %d (AVX512)\n", 
           checksum, fchecksum, checksum512);
    
    /* Return based on checksums to ensure execution */
    return (checksum + (int)fchecksum + checksum512) != 0 ? 0 : 1;
#else
    printf("Checksums: %d (SSE), %.2f (FP)\n", checksum, fchecksum);
    
    /* Return based on checksums to ensure execution */
    return (checksum + (int)fchecksum) != 0 ? 0 : 1;
#endif
}
