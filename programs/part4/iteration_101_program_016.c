/* Test program to cover 10/11 operand expansion in optabs.cc */
#include <stdio.h>
#include <string.h>
#include <x86intrin.h>

/* GCC vector types */
typedef int v4si __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* AVX types if available */
#ifdef __AVX__
typedef int v8si __attribute__((vector_size(32)));
typedef float v8sf __attribute__((vector_size(32)));
typedef double v4df __attribute__((vector_size(32)));
#endif

/* AVX-512 types if available */
#ifdef __AVX512F__
typedef int v16si __attribute__((vector_size(64)));
typedef float v16sf __attribute__((vector_size(64)));
typedef double v8df __attribute__((vector_size(64)));
#endif

/* Force no inlining to prevent optimization */
__attribute__((noinline, noipa))
v4si test_10_operands(v4si a, v4si b, v4si c, v4si d, v4si mask) {
    volatile v4si temp1, temp2, temp3;
    
    /* Complex shuffle with runtime mask - may expand to many operands */
    v4si shuffled = __builtin_shuffle(a, b, mask);
    temp1 = shuffled;
    
    /* Vector conditional with arithmetic - generates VEC_COND_EXPR */
    v4si cmp_result = (a > b) ? (c * d) : (c + d);
    temp2 = cmp_result;
    
    /* Blend operation using conditional */
    v4si blend_mask = (mask != (v4si){0, 0, 0, 0});
    v4si blended = blend_mask ? shuffled : cmp_result;
    
    /* More arithmetic to use all inputs */
    v4si result = blended + a - b + c * d;
    
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    return result;
}

__attribute__((noinline, noipa))
v4sf test_11_operands(v4sf a, v4sf b, v4sf c, v4sf d, v4sf e, v4si mask) {
    volatile v4sf temp1, temp2, temp3, temp4;
    
    /* Multiple vector operations chained together */
    v4sf t1 = a + b;
    temp1 = t1;
    
    v4sf t2 = c * d;
    temp2 = t2;
    
    /* Conditional with multiple operations */
    v4sf t3 = (a > b) ? (t1 * t2) : (t1 / t2);
    temp3 = t3;
    
    /* Shuffle with float to int conversion */
    v4si int_vec = __builtin_convertvector(t3, v4si);
    v4si shuffled_int = __builtin_shuffle(int_vec, __builtin_convertvector(e, v4si), mask);
    
    /* Convert back and blend */
    v4sf shuffled = __builtin_convertvector(shuffled_int, v4sf);
    v4sf result = (shuffled > e) ? shuffled : e;
    
    /* Use all inputs in final computation */
    result = result + a - b + c * d + e;
    
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    return result;
}

#ifdef __AVX__
__attribute__((noinline, noipa))
v8si test_avx_many_operands(v8si a, v8si b, v8si c, v8si d, v8si mask) {
    volatile v8si temp1, temp2;
    
    /* AVX shuffle with 256-bit vectors */
    v8si shuffled = __builtin_shuffle(a, b, mask);
    temp1 = shuffled;
    
    /* Complex conditional with arithmetic */
    v8si cmp = (a > b);
    v8si true_val = c * d + shuffled;
    v8si false_val = c + d - shuffled;
    v8si result = cmp ? true_val : false_val;
    
    /* Additional operations to increase operand count */
    result = result + __builtin_shuffle(result, mask);
    
    asm volatile("" ::: "memory");
    
    return result;
}
#endif

#ifdef __AVX512F__
__attribute__((noinline, noipa))
v16si test_avx512_many_operands(v16si a, v16si b, v16si c, v16si d, v16si mask) {
    volatile v16si temp1;
    
    /* AVX-512 operations with masking */
    v16si shuffled = __builtin_shuffle(a, b, mask);
    
    /* Complex masked operation */
    v16si mask_cond = (mask > (v16si){0});
    v16si result = mask_cond ? (c * d + shuffled) : (c + d - shuffled);
    
    /* Additional shuffle */
    result = __builtin_shuffle(result, mask);
    
    asm volatile("" ::: "memory");
    
    return result;
}
#endif

/* Main test driver */
int main() {
    /* Initialize test vectors */
    v4si v4a = {1, 2, 3, 4};
    v4si v4b = {5, 6, 7, 8};
    v4si v4c = {9, 10, 11, 12};
    v4si v4d = {13, 14, 15, 16};
    v4si v4mask = {3, 2, 1, 0};
    
    v4sf v4fa = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf v4fb = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf v4fc = {9.0f, 10.0f, 11.0f, 12.0f};
    v4sf v4fd = {13.0f, 14.0f, 15.0f, 16.0f};
    v4sf v4fe = {17.0f, 18.0f, 19.0f, 20.0f};
    
    printf("Testing 10/11 operand expansion patterns...\n");
    
    /* Test 10-operand path */
    v4si result1 = test_10_operands(v4a, v4b, v4c, v4d, v4mask);
    
    /* Test 11-operand path */
    v4sf result2 = test_11_operands(v4fa, v4fb, v4fc, v4fd, v4fe, v4mask);
    
#ifdef __AVX__
    v8si v8a = {1, 2, 3, 4, 5, 6, 7, 8};
    v8si v8b = {9, 10, 11, 12, 13, 14, 15, 16};
    v8si v8c = {17, 18, 19, 20, 21, 22, 23, 24};
    v8si v8d = {25, 26, 27, 28, 29, 30, 31, 32};
    v8si v8mask = {7, 6, 5, 4, 3, 2, 1, 0};
    
    v8si result3 = test_avx_many_operands(v8a, v8b, v8c, v8d, v8mask);
#endif
    
#ifdef __AVX512F__
    v16si v16a = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
    v16si v16b = {17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32};
    v16si v16c = {33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48};
    v16si v16d = {49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64};
    v16si v16mask = {15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0};
    
    v16si result4 = test_avx512_many_operands(v16a, v16b, v16c, v16d, v16mask);
#endif
    
    /* Compute checksums to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < 4; i++) {
        checksum += result1[i];
        checksum += (int)result2[i];
    }
    
#ifdef __AVX__
    for (int i = 0; i < 8; i++) {
        checksum += result3[i];
    }
#endif
    
#ifdef __AVX512F__
    for (int i = 0; i < 16; i++) {
        checksum += result4[i];
    }
#endif
    
    printf("Checksum: %d\n", checksum);
    
    /* Return based on checksum to ensure execution */
    return (checksum != 0) ? 0 : 1;
}
