/* 
 * Program to trigger x86 condition code mnemonics in i386.cc
 * Compile with: gcc -O3 -ffast-math -march=x86-64 -fdump-rtl-final -o test_conds test_conds.c
 */

#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <xmmintrin.h>
#include <immintrin.h>

/* Vector types for triggering vector comparisons */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Test function 1: Direct unordered comparisons using math.h macros */
__attribute__((noinline))
static int test_unordered_comparisons(float a, float b, double c, double d) {
    int sum = 0;
    
    /* These should generate UNORDERED/ORDERED condition codes */
    sum += isunordered(a, b) ? 1 : 0;
    sum += isordered(a, b) ? 2 : 0;
    
    /* UNEQ: unordered or equal */
    sum += !islessgreater(c, d) ? 4 : 0;  /* Equivalent to UNEQ */
    
    /* UNGE: unordered or greater-or-equal (not less) */
    sum += !isless(c, d) ? 8 : 0;  /* Should generate nlt (UNGE) */
    
    /* UNGT: unordered or greater (not less-or-equal) */
    sum += !islessequal(c, d) ? 16 : 0;  /* Should generate nle (UNGT) */
    
    /* UNLE: unordered or less-or-equal */
    sum += islessequal(c, d) ? 32 : 0;  /* Should generate ule (UNLE) */
    
    /* UNLT: unordered or less */
    sum += isless(c, d) ? 64 : 0;  /* Should generate ult (UNLT) */
    
    /* LTGT: less or greater (unordered) */
    sum += islessgreater(c, d) ? 128 : 0;  /* Should generate une (LTGT) */
    
    return sum;
}

/* Test function 2: Mixed comparisons in conditional expressions */
__attribute__((noinline))
static int test_mixed_conditionals(float a, float b, float c, float d, float e, float f) {
    int sum = 0;
    
    /* Complex conditional with different comparison types */
    for (int i = 0; i < 3; i++) {
        /* This complex expression should generate multiple condition codes */
        int cond = ((a < b) ? (c != d) : (e >= f)) ? 1 : 0;
        sum += cond;
        
        /* Another mixed expression */
        cond = ((a == b) ? (c > d) : (e <= f)) ? 2 : 0;
        sum += cond;
        
        /* Ternary with unordered check */
        cond = ((a != a) ? (b < c) : (d > e)) ? 4 : 0;  /* a != a checks for NaN */
        sum += cond;
    }
    
    return sum;
}

/* Test function 3: Vectorized comparisons using GCC vector extensions */
__attribute__((noinline))
static int test_vector_comparisons(v4sf v1, v4sf v2, v2df d1, v2df d2) {
    int sum = 0;
    
    /* Vector comparisons that should generate condition codes */
    v4sf cmp_result;
    
    /* Unordered comparison */
    cmp_result = v1 != v1;  /* Check for NaN elements */
    sum += ((int*)&cmp_result)[0] != 0 ? 1 : 0;
    
    /* Ordered comparison */
    cmp_result = v1 == v1;  /* Check for non-NaN elements */
    sum += ((int*)&cmp_result)[1] != 0 ? 2 : 0;
    
    /* Less-than with unordered */
    cmp_result = v1 < v2;
    sum += ((int*)&cmp_result)[2] != 0 ? 4 : 0;
    
    /* Greater-than with unordered */
    cmp_result = v1 > v2;
    sum += ((int*)&cmp_result)[3] != 0 ? 8 : 0;
    
    /* Not-equal with unordered */
    cmp_result = v1 != v2;
    sum += ((int*)&cmp_result)[0] != 0 ? 16 : 0;
    
    return sum;
}

/* Test function 4: AVX intrinsics for direct control */
#ifdef __AVX__
__attribute__((noinline))
static int test_avx_intrinsics(__m256 a, __m256 b) {
    int sum = 0;
    __m256 cmp;
    
    /* _CMP_UNORD_Q: unordered (quiet) */
    cmp = _mm256_cmp_ps(a, b, _CMP_UNORD_Q);
    sum += _mm256_movemask_ps(cmp);
    
    /* _CMP_NEQ_UQ: not equal (unordered, quiet) */
    cmp = _mm256_cmp_ps(a, b, _CMP_NEQ_UQ);
    sum += _mm256_movemask_ps(cmp);
    
    /* _CMP_NLT_UQ: not less-than (unordered, quiet) - should generate UNGE */
    cmp = _mm256_cmp_ps(a, b, _CMP_NLT_UQ);
    sum += _mm256_movemask_ps(cmp);
    
    /* _CMP_NLE_UQ: not less-or-equal (unordered, quiet) - should generate UNGT */
    cmp = _mm256_cmp_ps(a, b, _CMP_NLE_UQ);
    sum += _mm256_movemask_ps(cmp);
    
    return sum;
}
#endif

/* Test function 5: Chain of comparisons with fast-math assumptions */
__attribute__((noinline))
static int test_fast_math_chain(float a, float b, float c, float d, float e) {
    int sum = 0;
    
    /* Under -ffast-math, these comparisons can be optimized to unordered variants */
    if (a == b) sum += 1;      /* May become UNEQ */
    if (a != b) sum += 2;      /* May become LTGT */
    if (a >= b) sum += 4;      /* May become UNGE */
    if (a > b)  sum += 8;      /* May become UNGT */
    if (a <= b) sum += 16;     /* May become UNLE */
    if (a < b)  sum += 32;     /* May become UNLT */
    
    /* Chain comparisons */
    if ((a < b) && (c > d)) sum += 64;
    if ((a == b) || (c != d)) sum += 128;
    if (!(a != a) && (b == b)) sum += 256;  /* Check for non-NaN */
    
    /* Complex expression that might generate multiple condition codes */
    sum += (a < b) ? ((c > d) ? 512 : 1024) : ((e != e) ? 2048 : 4096);
    
    return sum;
}

/* Main test driver */
int main(int argc, char **argv) {
    volatile int iterations = (argc > 1) ? atoi(argv[1]) : 100;
    int total_sum = 0;
    
    /* Initialize test data with pattern that includes NaN for some iterations */
    float float_data[10];
    double double_data[10];
    
    for (int i = 0; i < 10; i++) {
        float_data[i] = (i % 3 == 0) ? (float)i : 
                       (i % 3 == 1) ? -(float)i : 
                       __builtin_nanf("");  /* NaN for some elements */
        
        double_data[i] = (i % 4 == 0) ? (double)i * 1.5 : 
                        (i % 4 == 1) ? -(double)i * 1.5 :
                        (i % 4 == 2) ? 0.0 :
                        __builtin_nan("");  /* NaN for some elements */
    }
    
    /* Initialize vector data */
    v4sf vec1 = {1.0f, 2.0f, __builtin_nanf(""), 4.0f};
    v4sf vec2 = {1.0f, 3.0f, 3.0f, __builtin_nanf("")};
    v2df dvec1 = {1.0, __builtin_nan("")};
    v2df dvec2 = {__builtin_nan(""), 2.0};
    
    /* Main test loop - volatile prevents excessive unrolling */
    for (volatile int iter = 0; iter < iterations; iter++) {
        int idx = iter % 8;
        
        /* Call different test functions based on iteration */
        switch (iter % 5) {
            case 0:
                total_sum += test_unordered_comparisons(
                    float_data[idx], float_data[idx+1],
                    double_data[idx], double_data[idx+1]);
                break;
                
            case 1:
                total_sum += test_mixed_conditionals(
                    float_data[idx], float_data[idx+1], float_data[idx+2],
                    float_data[idx+3], float_data[idx+4], float_data[idx+5]);
                break;
                
            case 2:
                total_sum += test_vector_comparisons(vec1, vec2, dvec1, dvec2);
                break;
                
            case 3:
#ifdef __AVX__
                __m256 avx1 = _mm256_set_ps(1.0f, 2.0f, __builtin_nanf(""), 4.0f,
                                           5.0f, 6.0f, 7.0f, 8.0f);
                __m256 avx2 = _mm256_set_ps(1.0f, 3.0f, 3.0f, __builtin_nanf(""),
                                           5.0f, 5.0f, 7.0f, 9.0f);
                total_sum += test_avx_intrinsics(avx1, avx2);
#endif
                break;
                
            case 4:
                total_sum += test_fast_math_chain(
                    float_data[idx], float_data[idx+1], float_data[idx+2],
                    float_data[idx+3], float_data[idx+4]);
                break;
        }
        
        /* Modify data slightly each iteration to prevent constant folding */
        float_data[idx] += 0.1f;
        double_data[idx] += 0.1;
    }
    
    printf("Final checksum: %d\n", total_sum);
    return total_sum != 0 ? 0 : 1;
}
