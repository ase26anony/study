/* Test program to cover 10/11 operand expansion in optabs.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <immintrin.h>

/* Prevent optimization */
#define NOOPT __attribute__((noinline, noclone))
#define BARRIER() asm volatile("" ::: "memory")

/* Vector types using GCC extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));
typedef double v4df __attribute__((vector_size(32)));
typedef long long v2di __attribute__((vector_size(16)));
typedef float v8sf __attribute__((vector_size(32)));
typedef int v8si __attribute__((vector_size(32)));

/* Complex operation that may require many operands */
NOOPT v4si test_10_operands(v4si a, v4si b, v4si c, v4si d, v4si mask) {
    volatile v4si temp1, temp2, temp3;
    
    /* Shuffle with runtime mask - may expand to many operands */
    v4si shuffle1 = __builtin_shuffle(a, b, mask);
    temp1 = shuffle1;
    BARRIER();
    
    /* Vector conditional with comparison - generates VEC_COND_EXPR */
    v4si cmp = a > b;
    v4si cond_result = cmp ? (a * b + c) : (d - a);
    temp2 = cond_result;
    BARRIER();
    
    /* Complex blend-like operation */
    v4si blend = __builtin_shufflevector(a, b, 0, 5, 2, 7);
    v4si blend2 = __builtin_shufflevector(c, d, 1, 4, 3, 6);
    v4si final_blend = (mask > (v4si){0, 1, 2, 3}) ? blend : blend2;
    
    /* Combine everything */
    return shuffle1 + cond_result + final_blend;
}

/* Test with AVX types for more operands */
NOOPT v4df test_11_operands_avx(v4df a, v4df b, v4df c, v4df d, v4df e, v4df mask) {
    volatile v4df temp1, temp2;
    
    /* Complex conditional with multiple operations */
    v4df cmp = a > b;
    v4df true_val = a * b + c * d;
    v4df false_val = e / (a + (v4df){1.0, 2.0, 3.0, 4.0});
    v4df cond_result = cmp ? true_val : false_val;
    temp1 = cond_result;
    BARRIER();
    
    /* Shuffle with mask */
    v4df shuffle1 = __builtin_shuffle(a, b, (v4si){3, 2, 1, 0});
    v4df shuffle2 = __builtin_shuffle(c, d, (v4si){1, 0, 3, 2});
    
    /* Blend based on comparison */
    v4df blend = (mask > (v4df){0.5, 1.5, 2.5, 3.5}) ? shuffle1 : shuffle2;
    
    /* Another conditional */
    v4df final = (cond_result > blend) ? 
                 cond_result * blend + e : 
                 cond_result / blend - e;
    
    temp2 = final;
    BARRIER();
    
    return final;
}

/* Use intrinsics for x86-specific many-operand instructions */
#ifdef __AVX2__
NOOPT __m256i test_intrinsic_many_ops(__m256i a, __m256i b, __m256i c, 
                                      __m256i d, __m256i mask) {
    volatile __m256i temp;
    
    /* AVX2 blendv may use many operands */
    __m256i blend1 = _mm256_blendv_epi8(a, b, mask);
    __m256i blend2 = _mm256_blendv_epi8(c, d, mask);
    
    /* Shuffle */
    __m256i shuf = _mm256_shuffle_epi32(blend1, _MM_SHUFFLE(3, 2, 1, 0));
    
    /* Conditional move */
    __m256i cmp = _mm256_cmpgt_epi32(a, b);
    __m256i sel = _mm256_blendv_epi8(shuf, blend2, cmp);
    
    temp = sel;
    BARRIER();
    
    /* More operations to increase operand count */
    __m256i add1 = _mm256_add_epi32(sel, a);
    __m256i mul1 = _mm256_mullo_epi32(add1, b);
    __m256i sub1 = _mm256_sub_epi32(mul1, c);
    
    return _mm256_add_epi32(sub1, d);
}
#endif

/* Main test function that combines everything */
NOOPT v8si comprehensive_test(v8si a, v8si b, v8si c, v8si d, v8si e) {
    volatile v8si temp[5];
    
    /* Split 256-bit vectors into 128-bit for more operations */
    v4si a_low = __builtin_convertvector((v2di){a[0], a[1]}, v4si);
    v4si a_high = __builtin_convertvector((v2di){a[2], a[3]}, v4si);
    v4si b_low = __builtin_convertvector((v2di){b[0], b[1]}, v4si);
    v4si b_high = __builtin_convertvector((v2di){b[2], b[3]}, v4si);
    
    /* Create a mask */
    v4si mask = (v4si){3, 2, 1, 0};
    
    /* Test 10 operand path */
    v4si res1 = test_10_operands(a_low, b_low, a_high, b_high, mask);
    temp[0] = __builtin_convertvector(res1, v8si);
    BARRIER();
    
    /* More operations */
    v8si add1 = a + b;
    v8si mul1 = add1 * c;
    v8si sub1 = mul1 - d;
    v8si blend1 = __builtin_shufflevector(a, b, 0, 9, 2, 11, 4, 13, 6, 15);
    
    temp[1] = blend1;
    BARRIER();
    
    /* Complex conditional on 256-bit vectors */
    v8si cmp = a > b;
    v8si true_val = (a * b) + (c * d);
    v8si false_val = e / (a + (v8si){1, 2, 3, 4, 5, 6, 7, 8});
    v8si cond_result = cmp ? true_val : false_val;
    
    temp[2] = cond_result;
    BARRIER();
    
    /* Combine results */
    v8si final = blend1 + cond_result + sub1;
    
    /* Convert to different type and back */
    v8sf float_vec = __builtin_convertvector(final, v8sf);
    v8si int_vec = __builtin_convertvector(float_vec, v8si);
    
    temp[3] = int_vec;
    BARRIER();
    
    return final + int_vec;
}

int main() {
    /* Initialize vectors with pattern values */
    v4si v4a = {1, 2, 3, 4};
    v4si v4b = {5, 6, 7, 8};
    v4si v4c = {9, 10, 11, 12};
    v4si v4d = {13, 14, 15, 16};
    v4si v4mask = {3, 2, 1, 0};
    
    v4df v4da = {1.0, 2.0, 3.0, 4.0};
    v4df v4db = {5.0, 6.0, 7.0, 8.0};
    v4df v4dc = {9.0, 10.0, 11.0, 12.0};
    v4df v4dd = {13.0, 14.0, 15.0, 16.0};
    v4df v4de = {17.0, 18.0, 19.0, 20.0};
    v4df v4dmask = {0.5, 1.5, 2.5, 3.5};
    
    v8si v8a = {1, 2, 3, 4, 5, 6, 7, 8};
    v8si v8b = {9, 10, 11, 12, 13, 14, 15, 16};
    v8si v8c = {17, 18, 19, 20, 21, 22, 23, 24};
    v8si v8d = {25, 26, 27, 28, 29, 30, 31, 32};
    v8si v8e = {33, 34, 35, 36, 37, 38, 39, 40};
    
    /* Call test functions to trigger expansion */
    v4si res1 = test_10_operands(v4a, v4b, v4c, v4d, v4mask);
    v4df res2 = test_11_operands_avx(v4da, v4db, v4dc, v4dd, v4de, v4dmask);
    v8si res3 = comprehensive_test(v8a, v8b, v8c, v8d, v8e);
    
#ifdef __AVX2__
    __m256i avx_a = _mm256_set_epi32(1, 2, 3, 4, 5, 6, 7, 8);
    __m256i avx_b = _mm256_set_epi32(9, 10, 11, 12, 13, 14, 15, 16);
    __m256i avx_c = _mm256_set_epi32(17, 18, 19, 20, 21, 22, 23, 24);
    __m256i avx_d = _mm256_set_epi32(25, 26, 27, 28, 29, 30, 31, 32);
    __m256i avx_mask = _mm256_set_epi32(0xFF, 0, 0xFF, 0, 0xFF, 0, 0xFF, 0);
    __m256i res4 = test_intrinsic_many_ops(avx_a, avx_b, avx_c, avx_d, avx_mask);
#endif
    
    /* Compute checksums to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < 4; i++) {
        checksum += res1[i];
        checksum += (int)res2[i];
    }
    
    for (int i = 0; i < 8; i++) {
        checksum += res3[i];
    }
    
#ifdef __AVX2__
    int avx_data[8];
    _mm256_storeu_si256((__m256i*)avx_data, res4);
    for (int i = 0; i < 8; i++) {
        checksum += avx_data[i];
    }
#endif
    
    printf("Checksum: %d\n", checksum);
    
    /* Return based on checksum to ensure execution */
    return (checksum > 1000) ? 0 : 1;
}
