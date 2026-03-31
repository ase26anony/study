/* test_many_operands.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <immintrin.h>

/* Use GCC vector extensions for maximum flexibility */
typedef int v4si __attribute__((vector_size(16)));
typedef int v8si __attribute__((vector_size(32)));
typedef int v16si __attribute__((vector_size(64)));
typedef double v2df __attribute__((vector_size(16)));
typedef double v4df __attribute__((vector_size(32)));
typedef double v8df __attribute__((vector_size(64)));
typedef float v4sf __attribute__((vector_size(16)));
typedef float v8sf __attribute__((vector_size(32)));
typedef float v16sf __attribute__((vector_size(64)));

/* Force no inlining to keep operations separate */
__attribute__((noinline, noipa))
v8si test_10_operands(v8si a, v8si b, v8si c, v8si d, v8si e) {
    volatile v8si temp1, temp2, temp3;
    
    /* Complex shuffle operation that may require many operands */
    v8si shuffle_mask = {7, 6, 5, 4, 3, 2, 1, 0};
    v8si shuffled = __builtin_shuffle(a, b, shuffle_mask);
    
    /* Store to volatile to force memory operations */
    temp1 = shuffled;
    
    /* Vector conditional expression with comparison */
    v8si mask = a > b;
    v8si cond_result = mask ? (c * d) : (d + e);
    
    temp2 = cond_result;
    
    /* Chain of operations that might expand to many operands */
    v8si complex_op = (a + b) * (c - d) / (e + 1);
    
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /* Another complex operation with blend-like behavior */
    v8si blend_mask = (a > c) | (b < d);
    v8si blended = blend_mask ? complex_op : cond_result;
    
    temp3 = blended;
    
    /* Final checksum combining all results */
    return shuffled + cond_result + blended + temp1 + temp2 + temp3;
}

__attribute__((noinline, noipa))
v4df test_11_operands(v4df a, v4df b, v4df c, v4df d, v4df e, v4df f) {
    volatile v4df temp1, temp2, temp3, temp4;
    
    /* Complex vector operation chain */
    v4df t1 = a * b + c;
    v4df t2 = d / e - f;
    
    temp1 = t1;
    temp2 = t2;
    
    /* Vector comparison and conditional */
    v4df mask = (a > b) & (c < d);
    v4df cond_val = mask ? t1 : t2;
    
    /* Shuffle with non-constant pattern */
    int shuffle_pattern[4] = {3, 2, 1, 0};
    v4df shuffled;
    for (int i = 0; i < 4; i++) {
        shuffled[i] = cond_val[shuffle_pattern[i]];
    }
    
    temp3 = shuffled;
    
    /* Another barrier */
    asm volatile("" ::: "memory");
    
    /* Complex expression that might require many operands */
    v4df complex_expr = (a + b) * (c - d) / (e + f) + (a * c) - (b * d);
    
    /* Blend operation simulated with conditional */
    v4df blend_mask = (a > 0.5) | (b < -0.5);
    v4df blended = blend_mask ? complex_expr : shuffled;
    
    temp4 = blended;
    
    /* Combine everything - this might generate many operands */
    return t1 + t2 + cond_val + shuffled + complex_expr + blended + 
           temp1 + temp2 + temp3 + temp4;
}

/* Test AVX-512 types for even more operands */
#ifdef __AVX512F__
__attribute__((noinline, noipa))
v16si test_avx512_operands(v16si a, v16si b, v16si c, v16si d) {
    volatile v16si temp1, temp2;
    
    /* Very large shuffle */
    v16si shuffle_mask = {15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0};
    v16si shuffled = __builtin_shuffle(a, b, shuffle_mask);
    
    /* Complex conditional with mask */
    v16si mask = (a > b) & (c < d);
    v16si result = mask ? (shuffled * c) : (shuffled + d);
    
    temp1 = result;
    
    /* Another complex operation */
    v16si final = result + a - b * c / (d + 1);
    
    temp2 = final;
    
    return final + temp1 + temp2 + shuffled;
}
#endif

int main() {
    /* Initialize test vectors */
    v8si v8a = {1, 2, 3, 4, 5, 6, 7, 8};
    v8si v8b = {8, 7, 6, 5, 4, 3, 2, 1};
    v8si v8c = {2, 4, 6, 8, 10, 12, 14, 16};
    v8si v8d = {1, 3, 5, 7, 9, 11, 13, 15};
    v8si v8e = {0, 1, 0, 1, 0, 1, 0, 1};
    
    v4df v4a = {1.0, 2.0, 3.0, 4.0};
    v4df v4b = {4.0, 3.0, 2.0, 1.0};
    v4df v4c = {0.5, 1.5, 2.5, 3.5};
    v4df v4d = {1.1, 2.2, 3.3, 4.4};
    v4df v4e = {0.1, 0.2, 0.3, 0.4};
    v4df v4f = {1.0, 2.0, 3.0, 4.0};
    
    /* Call test functions */
    v8si result8 = test_10_operands(v8a, v8b, v8c, v8d, v8e);
    v4df result4 = test_11_operands(v4a, v4b, v4c, v4d, v4e, v4f);
    
#ifdef __AVX512F__
    v16si v16a = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
    v16si v16b = {16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1};
    v16si v16c = {2,4,6,8,10,12,14,16,18,20,22,24,26,28,30,32};
    v16si v16d = {1,3,5,7,9,11,13,15,17,19,21,23,25,27,29,31};
    v16si result16 = test_avx512_operands(v16a, v16b, v16c, v16d);
#endif
    
    /* Compute checksums to prevent dead code elimination */
    int sum8 = 0;
    for (int i = 0; i < 8; i++) {
        sum8 += result8[i];
    }
    
    double sum4 = 0.0;
    for (int i = 0; i < 4; i++) {
        sum4 += result4[i];
    }
    
#ifdef __AVX512F__
    int sum16 = 0;
    for (int i = 0; i < 16; i++) {
        sum16 += result16[i];
    }
    printf("Checksums: int8=%d, double4=%f, int16=%d\n", sum8, sum4, sum16);
#else
    printf("Checksums: int8=%d, double4=%f\n", sum8, sum4);
#endif
    
    return (sum8 != 0 && sum4 != 0.0) ? 0 : 1;
}
