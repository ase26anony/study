/* Test program to cover 10/11 operand expansion in optabs.cc */
#include <stdio.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <immintrin.h>

/* Prevent optimization */
#define NOOPT __attribute__((noinline, noclone))
#define BARRIER() asm volatile("" ::: "memory")

/* Vector types using GCC extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));

/* AVX types */
typedef float v8sf __attribute__((vector_size(32)));
typedef double v4df __attribute__((vector_size(32)));
typedef int v8si __attribute__((vector_size(32)));

/* Complex operation that may require many operands during expansion */
NOOPT v4si test_10_operands(v4si a, v4si b, v4si c, v4si d, 
                           v4si mask1, v4si mask2, v4si shuffle_mask) {
    volatile v4si temp1, temp2, temp3, temp4;
    
    /* Force memory operations */
    temp1 = a;
    temp2 = b;
    BARRIER();
    
    /* Complex shuffle chain - may expand to many operands */
    v4si shuffled1 = __builtin_shuffle(temp1, temp2, shuffle_mask);
    BARRIER();
    
    /* Vector conditional with comparison */
    v4si cmp_result = (shuffled1 > mask1) ? a * b : c + d;
    temp3 = cmp_result;
    BARRIER();
    
    /* Another shuffle with different inputs */
    v4si shuffled2 = __builtin_shuffle(temp3, mask2, shuffle_mask);
    
    /* Blend-like operation using conditional */
    v4si blend_mask = (mask1 > mask2);
    v4si result = blend_mask ? shuffled1 : shuffled2;
    temp4 = result;
    BARRIER();
    
    return temp4;
}

/* Test with floating point vectors - different expansion path */
NOOPT v4df test_11_operands(v4df a, v4df b, v4df c, v4df d,
                           v4df e, v4si int_mask) {
    volatile v4df temp1, temp2, temp3;
    
    /* Convert int mask to double for comparison */
    v4df mask_df = __builtin_convertvector(int_mask, v4df);
    temp1 = mask_df;
    BARRIER();
    
    /* Complex conditional expression with arithmetic */
    v4df cmp = (temp1 > 0.0) ? a * b : c / d;
    temp2 = cmp;
    BARRIER();
    
    /* Chain of operations that may require many temporaries */
    v4df result = temp2 + e - a * 2.0 + b / 3.0;
    
    /* Use builtin for rounding (may require mode operand) */
    result = __builtin_ia32_roundpd256(result, 0); /* _MM_FROUND_TO_NEAREST_INT */
    
    temp3 = result;
    BARRIER();
    
    return temp3;
}

/* Test with AVX-512 style 512-bit vectors if supported */
#ifdef __AVX512F__
typedef double v8df __attribute__((vector_size(64)));
typedef int v16si __attribute__((vector_size(64)));

NOOPT v8df test_many_operands_avx512(v8df a, v8df b, v8df c, 
                                    v8df d, v8df e, v8df f) {
    volatile v8df temp1, temp2;
    
    /* Very complex expression that may need many registers */
    v8df result = (a > b) ? (c * d + e) : (f - a / b);
    
    /* Multiple operations chained */
    result = result * 2.0 - a + b / 3.0;
    
    temp1 = result;
    BARRIER();
    
    /* Conditional move style operation */
    v8df mask = (temp1 > 0.0);
    v8df alt = c * d;
    result = mask ? temp1 : alt;
    
    temp2 = result;
    BARRIER();
    
    return temp2;
}
#endif

/* Main test harness */
int main(void) {
    /* Initialize test vectors with pattern */
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = {9, 10, 11, 12};
    v4si d = {13, 14, 15, 16};
    v4si mask1 = {0, 1, 0, 1};
    v4si mask2 = {1, 0, 1, 0};
    v4si shuffle_mask = {3, 2, 1, 0}; /* Reverse */
    
    /* Test 10 operand path */
    v4si result1 = test_10_operands(a, b, c, d, mask1, mask2, shuffle_mask);
    
    /* Initialize FP vectors */
    v4df ad = {1.0, 2.0, 3.0, 4.0};
    v4df bd = {5.0, 6.0, 7.0, 8.0};
    v4df cd = {9.0, 10.0, 11.0, 12.0};
    v4df dd = {13.0, 14.0, 15.0, 16.0};
    v4df ed = {17.0, 18.0, 19.0, 20.0};
    
    /* Test 11 operand path */
    v4df result2 = test_11_operands(ad, bd, cd, dd, ed, mask1);
    
#ifdef __AVX512F__
    /* Test with even more operands if available */
    v8df a8 = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0};
    v8df b8 = {9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0, 16.0};
    v8df c8 = {17.0, 18.0, 19.0, 20.0, 21.0, 22.0, 23.0, 24.0};
    v8df d8 = {25.0, 26.0, 27.0, 28.0, 29.0, 30.0, 31.0, 32.0};
    v8df e8 = {33.0, 34.0, 35.0, 36.0, 37.0, 38.0, 39.0, 40.0};
    v8df f8 = {41.0, 42.0, 43.0, 44.0, 45.0, 46.0, 47.0, 48.0};
    
    v8df result3 = test_many_operands_avx512(a8, b8, c8, d8, e8, f8);
#endif
    
    /* Compute checksums to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < 4; i++) {
        checksum += result1[i];
        checksum += (int)result2[i];
    }
    
#ifdef __AVX512F__
    for (int i = 0; i < 8; i++) {
        checksum += (int)result3[i];
    }
#endif
    
    printf("Checksum: %d\n", checksum);
    
    /* Return based on checksum to ensure execution */
    return (checksum > 0) ? 0 : 1;
}
