/* Test program to cover 10/11 operand expansion in optabs.cc */
#include <stdio.h>
#include <string.h>
#include <stdint.h>

/* Prevent optimization */
#define NOINLINE __attribute__((noinline, noclone))
#define BARRIER() asm volatile("" ::: "memory")

/* Vector types using GCC extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

#ifdef __AVX__
typedef int v8si __attribute__((vector_size(32)));
typedef float v8sf __attribute__((vector_size(32)));
typedef double v4df __attribute__((vector_size(32)));
#endif

#ifdef __AVX512F__
typedef int v16si __attribute__((vector_size(64)));
typedef float v16sf __attribute__((vector_size(64)));
#endif

/* Complex shuffle with many operands */
NOINLINE v4si test_10_operands(v4si a, v4si b, v4si c, v4si d, 
                               v4si mask1, v4si mask2) {
    volatile v4si temp1, temp2, temp3;
    
    /* Chain operations to force many operands */
    /* 1. Shuffle with variable mask - may expand to many operands */
    v4si shuffled = __builtin_shuffle(a, b, mask1);
    BARRIER();
    
    /* 2. Conditional expression with comparison */
    v4si cmp = a > b;
    v4si cond_result = cmp ? (c * d) : (a + b);
    BARRIER();
    
    /* 3. Another shuffle with different mask */
    v4si shuffled2 = __builtin_shuffle(c, d, mask2);
    BARRIER();
    
    /* 4. Blend operation using conditional */
    v4si blend_mask = mask1 & mask2;
    v4si blended = blend_mask ? shuffled : shuffled2;
    BARRIER();
    
    /* 5. Complex arithmetic chain */
    v4si result = (a * b) + (c * d) - (shuffled * shuffled2) / (cond_result + 1);
    BARRIER();
    
    /* Force memory operations */
    temp1 = result;
    temp2 = blended;
    temp3 = cond_result;
    
    return temp1 + temp2 + temp3;
}

#ifdef __AVX__
/* Test with AVX vectors - more likely to need many operands */
NOINLINE v8si test_avx_10_operands(v8si a, v8si b, v8si c, v8si d,
                                   v8si mask1, v8si mask2, v8si mask3) {
    volatile v8si temp1, temp2, temp3, temp4;
    
    /* Multiple shuffles with large vectors */
    v8si shuffle1 = __builtin_shufflevector(a, b, 
        0, 8, 1, 9, 2, 10, 3, 11);
    BARRIER();
    
    v8si shuffle2 = __builtin_shufflevector(c, d,
        4, 12, 5, 13, 6, 14, 7, 15);
    BARRIER();
    
    /* Vector comparison with many operands */
    v8si cmp1 = a > b;
    v8si cmp2 = c > d;
    v8si cmp_combined = cmp1 & cmp2;
    BARRIER();
    
    /* Complex conditional with arithmetic */
    v8si cond_val = cmp_combined ? 
        (a * b + c * d) : 
        (a / (b + 1) - c / (d + 1));
    BARRIER();
    
    /* Blend with multiple masks */
    v8si mask_combined = mask1 | mask2 | mask3;
    v8si blended = mask_combined ? shuffle1 : shuffle2;
    BARRIER();
    
    /* Final computation with many operands */
    v8si result = (shuffle1 * shuffle2) + 
                  (cond_val * blended) - 
                  (a + b + c + d) / 4;
    BARRIER();
    
    /* Force to memory */
    temp1 = result;
    temp2 = blended;
    temp3 = cond_val;
    temp4 = shuffle1;
    
    return temp1 + temp2 + temp3 + temp4;
}
#endif

#ifdef __AVX512F__
/* AVX-512 with 16-element vectors - high chance of many operands */
NOINLINE v16si test_avx512_11_operands(v16si a, v16si b, v16si c, 
                                       v16si d, v16si e, v16si mask) {
    volatile v16si temps[6];
    
    /* Multiple operations that might need 11 operands */
    v16si shuffle1 = __builtin_shufflevector(a, b,
        0, 16, 1, 17, 2, 18, 3, 19,
        4, 20, 5, 21, 6, 22, 7, 23);
    BARRIER();
    
    v16si shuffle2 = __builtin_shufflevector(c, d,
        8, 24, 9, 25, 10, 26, 11, 27,
        12, 28, 13, 29, 14, 30, 15, 31);
    BARRIER();
    
    /* Multiple comparisons */
    v16si cmp1 = a > e;
    v16si cmp2 = b > c;
    v16si cmp3 = d > a;
    BARRIER();
    
    /* Complex conditional with many operands */
    v16si cond_mask = cmp1 & cmp2 | cmp3;
    v16si cond_result = cond_mask ? 
        (a * b * c) : 
        (d + e + shuffle1);
    BARRIER();
    
    /* Blend with mask */
    v16si blended = mask ? shuffle1 : shuffle2;
    BARRIER();
    
    /* Arithmetic with many inputs */
    v16si result = (a + b) * (c - d) + 
                   (e * shuffle1) / (shuffle2 + 1) - 
                   blended + cond_result;
    BARRIER();
    
    /* Store to volatile arrays */
    temps[0] = result;
    temps[1] = blended;
    temps[2] = cond_result;
    temps[3] = shuffle1;
    temps[4] = shuffle2;
    temps[5] = cmp1;
    
    /* Combine all results */
    v16si final = temps[0];
    for (int i = 1; i < 6; i++) {
        final = final + temps[i];
    }
    
    return final;
}
#endif

/* Main test driver */
int main() {
    int checksum = 0;
    
    /* Initialize SSE vectors */
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = {9, 10, 11, 12};
    v4si d = {13, 14, 15, 16};
    v4si mask1 = {0, 2, 1, 3};
    v4si mask2 = {3, 1, 2, 0};
    
    /* Test 10-operand path */
    v4si result1 = test_10_operands(a, b, c, d, mask1, mask2);
    
    /* Extract and sum elements */
    int *r1 = (int*)&result1;
    for (int i = 0; i < 4; i++) {
        checksum += r1[i];
    }
    
#ifdef __AVX__
    /* Initialize AVX vectors */
    v8si avx_a = {1, 2, 3, 4, 5, 6, 7, 8};
    v8si avx_b = {9, 10, 11, 12, 13, 14, 15, 16};
    v8si avx_c = {17, 18, 19, 20, 21, 22, 23, 24};
    v8si avx_d = {25, 26, 27, 28, 29, 30, 31, 32};
    v8si avx_mask1 = {0, 7, 1, 6, 2, 5, 3, 4};
    v8si avx_mask2 = {4, 3, 5, 2, 6, 1, 7, 0};
    v8si avx_mask3 = {1, 0, 3, 2, 5, 4, 7, 6};
    
    /* Test AVX path */
    v8si result2 = test_avx_10_operands(avx_a, avx_b, avx_c, avx_d, 
                                        avx_mask1, avx_mask2, avx_mask3);
    
    int *r2 = (int*)&result2;
    for (int i = 0; i < 8; i++) {
        checksum += r2[i];
    }
#endif

#ifdef __AVX512F__
    /* Initialize AVX-512 vectors */
    v16si avx512_a = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
    v16si avx512_b = {17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32};
    v16si avx512_c = {33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48};
    v16si avx512_d = {49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64};
    v16si avx512_e = {65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80};
    v16si avx512_mask = {0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1};
    
    /* Test 11-operand path */
    v16si result3 = test_avx512_11_operands(avx512_a, avx512_b, avx512_c,
                                           avx512_d, avx512_e, avx512_mask);
    
    int *r3 = (int*)&result3;
    for (int i = 0; i < 16; i++) {
        checksum += r3[i];
    }
#endif
    
    printf("Checksum: %d\n", checksum);
    
    /* Return non-zero if checksum is 0 (unlikely) */
    return checksum == 0 ? 1 : 0;
}
