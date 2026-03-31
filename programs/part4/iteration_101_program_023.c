/* Test program to cover 10/11 operand expansion in optabs.cc */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Compiler barriers to prevent optimization */
#define COMPILER_BARRIER() asm volatile("" ::: "memory")

/* Vector types using GCC extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Larger vector types for AVX */
#ifdef __AVX__
typedef int v8si __attribute__((vector_size(32)));
typedef float v8sf __attribute__((vector_size(32)));
typedef double v4df __attribute__((vector_size(32)));
#endif

#ifdef __AVX512F__
typedef int v16si __attribute__((vector_size(64)));
typedef float v16sf __attribute__((vector_size(64)));
typedef double v8df __attribute__((vector_size(64)));
#endif

/* Test function with many vector operations - marked noinline to prevent optimization */
__attribute__((noinline, noipa))
v4si test_10_operand_expansion(v4si a, v4si b, v4si c, v4si d, v4si mask) {
    volatile v4si temp1, temp2, temp3;
    
    /* Complex shuffle operation - may expand to many operands */
    v4si shuffle1 = __builtin_shuffle(a, b, (v4si){3, 2, 1, 0});
    COMPILER_BARRIER();
    
    /* Vector conditional with comparison - generates VEC_COND_EXPR */
    v4si cmp = a > b;
    v4si cond_result = cmp ? (a * b + c) : (d - a);
    COMPILER_BARRIER();
    
    /* Another shuffle with dynamic mask */
    v4si shuffle2 = __builtin_shuffle(cond_result, shuffle1, mask);
    COMPILER_BARRIER();
    
    /* Blend operation using conditional */
    v4si blend = (mask > (v4si){1, 2, 3, 4}) ? shuffle1 : shuffle2;
    COMPILER_BARRIER();
    
    /* Store to volatile to force memory ops */
    temp1 = blend;
    temp2 = cond_result;
    temp3 = shuffle2;
    
    /* Complex expression that might need many temporaries */
    v4si result = (temp1 * temp2) + (temp3 << 2) - (a & b) | (c ^ d);
    
    return result;
}

#ifdef __AVX__
__attribute__((noinline, noipa))
v8si test_11_operand_expansion(v8si a, v8si b, v8si c, v8si d, v8si e, v8si mask) {
    volatile v8si temp1, temp2, temp3, temp4;
    
    /* Multiple shuffles with large vectors */
    v8si shuffle1 = __builtin_shufflevector(a, b, 0, 1, 2, 3, 4, 5, 6, 7);
    v8si shuffle2 = __builtin_shufflevector(c, d, 7, 6, 5, 4, 3, 2, 1, 0);
    COMPILER_BARRIER();
    
    /* Complex conditional with vector comparison */
    v8si cmp = a > b;
    v8si cond1 = cmp ? (shuffle1 * shuffle2) : (a + b);
    COMPILER_BARRIER();
    
    /* Another conditional */
    v8si cmp2 = c < d;
    v8si cond2 = cmp2 ? (cond1 >> 1) : (cond1 << 1);
    COMPILER_BARRIER();
    
    /* Blend with mask */
    v8si blend = __builtin_shufflevector(cond1, cond2, 
        0, 9, 2, 11, 4, 13, 6, 15);
    COMPILER_BARRIER();
    
    /* Store to volatile variables */
    temp1 = blend;
    temp2 = cond1;
    temp3 = cond2;
    temp4 = shuffle1;
    
    /* Very complex expression that might need 11 operands */
    v8si result = (temp1 & temp2) | (temp3 ^ temp4) + 
                  (a * b) - (c / (d + (v8si){1})) +
                  (e << (mask & (v8si){3}));
    
    return result;
}
#endif

/* Test with floating point vectors */
__attribute__((noinline, noipa))
v4sf test_float_expansion(v4sf a, v4sf b, v4sf c, v4sf d, v4si mask) {
    volatile v4sf temp1, temp2;
    
    /* Convert mask to float for shuffle */
    v4sf mask_f = __builtin_convertvector(mask, v4sf);
    COMPILER_BARRIER();
    
    /* Shuffle with float vectors */
    v4sf shuffle = __builtin_shuffle(a, b, (v4si){2, 3, 0, 1});
    COMPILER_BARRIER();
    
    /* Conditional with float comparison */
    v4sf cmp = a > b;
    v4sf cond = cmp ? (a * b + c) : (d - a);
    COMPILER_BARRIER();
    
    /* Blend operation */
    v4sf blend = __builtin_shuffle(shuffle, cond, (v4si){0, 5, 2, 7});
    
    /* Store to volatile */
    temp1 = blend;
    temp2 = cond;
    
    /* Complex floating point expression */
    v4sf result = temp1 * temp2 + a / (b + (v4sf){1.0f}) - c * d;
    
    return result;
}

int main() {
    /* Initialize test vectors */
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = {9, 10, 11, 12};
    v4si d = {13, 14, 15, 16};
    v4si mask = {0, 3, 1, 2};
    
    v4sf fa = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf fb = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf fc = {9.0f, 10.0f, 11.0f, 12.0f};
    v4sf fd = {13.0f, 14.0f, 15.0f, 16.0f};
    
    printf("Testing 10/11 operand expansion coverage...\n");
    
    /* Call test functions */
    v4si result1 = test_10_operand_expansion(a, b, c, d, mask);
    v4sf result2 = test_float_expansion(fa, fb, fc, fd, mask);
    
    /* Compute checksums to prevent dead code elimination */
    int checksum1 = 0;
    float checksum2 = 0.0f;
    
    for (int i = 0; i < 4; i++) {
        checksum1 += result1[i];
        checksum2 += result2[i];
    }
    
    printf("Integer checksum: %d\n", checksum1);
    printf("Float checksum: %f\n", checksum2);
    
#ifdef __AVX__
    /* Test with AVX vectors if available */
    v8si avx_a = {1, 2, 3, 4, 5, 6, 7, 8};
    v8si avx_b = {9, 10, 11, 12, 13, 14, 15, 16};
    v8si avx_c = {17, 18, 19, 20, 21, 22, 23, 24};
    v8si avx_d = {25, 26, 27, 28, 29, 30, 31, 32};
    v8si avx_e = {33, 34, 35, 36, 37, 38, 39, 40};
    v8si avx_mask = {0, 7, 1, 6, 2, 5, 3, 4};
    
    v8si result3 = test_11_operand_expansion(avx_a, avx_b, avx_c, avx_d, avx_e, avx_mask);
    
    int checksum3 = 0;
    for (int i = 0; i < 8; i++) {
        checksum3 += result3[i];
    }
    printf("AVX integer checksum: %d\n", checksum3);
#endif
    
    /* Return based on checksums to ensure execution */
    return (checksum1 != 0 && checksum2 != 0.0f) ? 0 : 1;
}
