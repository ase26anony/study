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

/* Complex shuffle with many operands */
NOOPT v4si test_10_operands(v4si a, v4si b, v4si c, v4si d, 
                           v4si mask1, v4si mask2, v4si mask3) {
    volatile v4si temp1, temp2, temp3;
    
    /* Force multiple operations that might expand to many operands */
    
    /* 1. Shuffle with variable mask - can expand to many operands */
    v4si shuffled = __builtin_shuffle(a, b, mask1);
    BARRIER();
    
    /* 2. Another shuffle with different mask */
    v4si shuffled2 = __builtin_shuffle(c, d, mask2);
    BARRIER();
    
    /* 3. Blend operation using conditional operator on vectors */
    /* This generates VEC_COND_EXPR which may need many operands */
    v4si cmp_result = (shuffled > shuffled2) ? shuffled : shuffled2;
    BARRIER();
    
    /* 4. Arithmetic combination */
    v4si arith = (shuffled * shuffled2) + (shuffled / (shuffled2 + 1));
    BARRIER();
    
    /* 5. Another conditional with arithmetic */
    v4si final = (cmp_result > arith) ? 
                 (shuffled * 2 - shuffled2) : 
                 (shuffled2 * 3 + shuffled);
    BARRIER();
    
    /* Store to volatile to force all operations */
    temp1 = shuffled;
    temp2 = shuffled2;
    temp3 = final;
    
    return final + temp1 + temp2;
}

/* Test with floating point vectors */
NOOPT v4sf test_11_operands_fp(v4sf a, v4sf b, v4sf c, v4sf d,
                              v4sf e, v4sf f, v4si mask) {
    volatile v4sf temp1, temp2, temp3, temp4;
    
    /* Complex chain of FP operations */
    
    /* 1. Multiple shuffles */
    v4sf s1 = __builtin_shuffle(a, b, mask);
    BARRIER();
    
    v4sf s2 = __builtin_shuffle(c, d, mask);
    BARRIER();
    
    v4sf s3 = __builtin_shuffle(e, f, mask);
    BARRIER();
    
    /* 2. Arithmetic combinations */
    v4sf m1 = s1 * s2 + s3;
    BARRIER();
    
    v4sf m2 = s2 / (s1 + 1.0f) - s3;
    BARRIER();
    
    /* 3. Conditional with FP comparison */
    v4sf cmp = (s1 > s2) ? m1 : m2;
    BARRIER();
    
    /* 4. Blend-like operation using conditional */
    v4sf blended = (cmp > 0.0f) ? (s1 + s2) : (s2 - s1);
    BARRIER();
    
    /* 5. Final combination */
    v4sf result = blended * m1 + m2 / (cmp + 0.5f);
    BARRIER();
    
    /* Force memory operations */
    temp1 = s1;
    temp2 = s2;
    temp3 = m1;
    temp4 = result;
    
    return result + temp1 + temp2 + temp3;
}

#ifdef __AVX__
/* AVX version with larger vectors */
NOOPT v8si test_avx_10_operands(v8si a, v8si b, v8si c, v8si mask) {
    volatile v8si temp1, temp2;
    
    /* Complex AVX operations */
    
    /* 1. Shuffle 256-bit vectors */
    v8si shuffled = __builtin_shuffle(a, b, mask);
    BARRIER();
    
    /* 2. Arithmetic with many operands */
    v8si arith1 = shuffled * a + b;
    BARRIER();
    
    v8si arith2 = shuffled / (a + 1) - b;
    BARRIER();
    
    /* 3. Conditional on 256-bit vectors */
    v8si cond = (shuffled > arith1) ? arith1 : arith2;
    BARRIER();
    
    /* 4. Blend operation */
    v8si blended = (cond > 0) ? (shuffled + a) : (shuffled - a);
    BARRIER();
    
    /* 5. Final combination */
    v8si result = blended * cond + arith1 - arith2;
    BARRIER();
    
    temp1 = shuffled;
    temp2 = result;
    
    return result + temp1;
}

NOOPT v4df test_avx_11_operands(v4df a, v4df b, v4df c, v4df d,
                               v8si mask) {
    volatile v4df temp1, temp2, temp3;
    
    /* Complex double-precision AVX operations */
    
    /* Convert mask for double shuffle */
    v4di mask_d = __builtin_convertvector(mask, v4di);
    
    /* 1. Shuffle doubles */
    v4df s1 = __builtin_shuffle(a, b, mask_d);
    BARRIER();
    
    v4df s2 = __builtin_shuffle(c, d, mask_d);
    BARRIER();
    
    /* 2. Arithmetic */
    v4df m1 = s1 * s2 + a;
    BARRIER();
    
    v4df m2 = s2 / (s1 + 1.0) - b;
    BARRIER();
    
    /* 3. Conditional with FP */
    v4df cmp = (s1 > s2) ? m1 : m2;
    BARRIER();
    
    /* 4. Blend */
    v4df blended = (cmp > 0.0) ? (s1 + s2) : (s1 - s2);
    BARRIER();
    
    /* 5. More arithmetic */
    v4df result = blended * cmp + m1 / (m2 + 0.5);
    BARRIER();
    
    temp1 = s1;
    temp2 = s2;
    temp3 = result;
    
    return result + temp1 + temp2;
}
#endif

/* Main test driver */
int main(void) {
    int i;
    int checksum = 0;
    
    /* Initialize test vectors */
    v4si a_int = {1, 2, 3, 4};
    v4si b_int = {5, 6, 7, 8};
    v4si c_int = {9, 10, 11, 12};
    v4si d_int = {13, 14, 15, 16};
    v4si mask1 = {3, 2, 1, 0};
    v4si mask2 = {1, 0, 3, 2};
    v4si mask3 = {2, 3, 0, 1};
    
    v4sf a_float = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf b_float = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf c_float = {9.0f, 10.0f, 11.0f, 12.0f};
    v4sf d_float = {13.0f, 14.0f, 15.0f, 16.0f};
    v4sf e_float = {17.0f, 18.0f, 19.0f, 20.0f};
    v4sf f_float = {21.0f, 22.0f, 23.0f, 24.0f};
    
    /* Test integer vector operations */
    printf("Testing 10-operand integer vector expansion...\n");
    v4si int_result = test_10_operands(a_int, b_int, c_int, d_int, 
                                      mask1, mask2, mask3);
    
    /* Extract and sum results */
    int* int_ptr = (int*)&int_result;
    for (i = 0; i < 4; i++) {
        checksum += int_ptr[i];
    }
    
    /* Test floating point vector operations */
    printf("Testing 11-operand floating point vector expansion...\n");
    v4sf float_result = test_11_operands_fp(a_float, b_float, c_float, d_float,
                                           e_float, f_float, mask1);
    
    /* Extract and sum FP results */
    float* float_ptr = (float*)&float_result;
    for (i = 0; i < 4; i++) {
        checksum += (int)float_ptr[i];
    }
    
#ifdef __AVX__
    /* Test AVX operations if available */
    printf("Testing AVX 10/11-operand expansions...\n");
    
    v8si avx_int_a = {1, 2, 3, 4, 5, 6, 7, 8};
    v8si avx_int_b = {9, 10, 11, 12, 13, 14, 15, 16};
    v8si avx_int_c = {17, 18, 19, 20, 21, 22, 23, 24};
    v8si avx_mask = {7, 6, 5, 4, 3, 2, 1, 0};
    
    v4df avx_double_a = {1.0, 2.0, 3.0, 4.0};
    v4df avx_double_b = {5.0, 6.0, 7.0, 8.0};
    v4df avx_double_c = {9.0, 10.0, 11.0, 12.0};
    v4df avx_double_d = {13.0, 14.0, 15.0, 16.0};
    
    v8si avx_int_result = test_avx_10_operands(avx_int_a, avx_int_b, 
                                              avx_int_c, avx_mask);
    
    v4df avx_double_result = test_avx_11_operands(avx_double_a, avx_double_b,
                                                 avx_double_c, avx_double_d,
                                                 avx_mask);
    
    /* Add AVX results to checksum */
    int* avx_int_ptr = (int*)&avx_int_result;
    for (i = 0; i < 8; i++) {
        checksum += avx_int_ptr[i];
    }
    
    double* avx_double_ptr = (double*)&avx_double_result;
    for (i = 0; i < 4; i++) {
        checksum += (int)avx_double_ptr[i];
    }
#endif
    
    printf("Final checksum: %d\n", checksum);
    
    /* Return based on checksum to ensure execution */
    return (checksum > 0) ? 0 : 1;
}
