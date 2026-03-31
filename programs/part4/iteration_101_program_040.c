/* Test program to cover 10/11 operand expansion in optabs.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <immintrin.h>

/* Prevent optimization */
#define NOINLINE __attribute__((noinline, noclone))
#define BARRIER() asm volatile("" ::: "memory")

/* Vector types using GCC extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));
typedef double v4df __attribute__((vector_size(32)));
typedef int v8si __attribute__((vector_size(32)));
typedef long long v4di __attribute__((vector_size(32)));

/* Complex operation that should generate many operands */
NOINLINE static v4si test_10_operands(v4si a, v4si b, v4si c, v4si d, 
                                      v4si mask1, v4si mask2)
{
    volatile v4si temp1, temp2, temp3;
    v4si result;
    
    /* Chain of operations that may require many operands */
    
    /* 1. Shuffle with runtime mask - potentially many operands */
    v4si shuffled = __builtin_shuffle(a, b, mask1);
    BARRIER();
    
    /* 2. Conditional select with vector comparison */
    v4si cmp = (a > b) ? shuffled : c;
    BARRIER();
    
    /* 3. Another shuffle with different mask */
    v4si shuffled2 = __builtin_shuffle(cmp, d, mask2);
    BARRIER();
    
    /* 4. Complex arithmetic with multiple operands */
    temp1 = a + b * shuffled - c / (d + 1);
    BARRIER();
    
    /* 5. Blend operation using conditional */
    v4si blend_mask = (mask1 > mask2);
    temp2 = blend_mask ? temp1 : shuffled2;
    BARRIER();
    
    /* 6. Final combination - this may expand to insn with many operands */
    result = (temp2 + a) * (b - c) / (d + shuffled);
    BARRIER();
    
    /* Force memory store/load */
    volatile v4si* volatile_ptr = &temp3;
    *volatile_ptr = result;
    BARRIER();
    
    return *volatile_ptr;
}

/* Test with AVX types for more operands */
#ifdef __AVX__
NOINLINE static v8si test_11_operands(v8si a, v8si b, v8si c, v8si d,
                                      v8si e, v8si mask)
{
    volatile v8si temp1, temp2;
    v8si result;
    
    /* Complex expression that may require many operands */
    
    /* 1. Multiple arithmetic operations */
    v8si t1 = a + b;
    v8si t2 = c - d;
    v8si t3 = e * mask;
    BARRIER();
    
    /* 2. Conditional with vector comparison */
    v8si cmp = (t1 > t2);
    v8si t4 = cmp ? t3 : a;
    BARRIER();
    
    /* 3. Shuffle large vector - may need many operands */
    /* Create a complex shuffle mask */
    v8si shuffle_mask = {0, 7, 1, 6, 2, 5, 3, 4};
    v8si shuffled = __builtin_shufflevector(t4, b, 0, 7, 1, 6, 2, 5, 3, 4);
    BARRIER();
    
    /* 4. Blend with mask */
    v8si blend_result;
    for (int i = 0; i < 8; i++) {
        blend_result[i] = (mask[i] > 0) ? shuffled[i] : t4[i];
    }
    BARRIER();
    
    /* 5. Final complex computation */
    result = (blend_result * a) + (shuffled * b) - (t4 * c) / (d + 1);
    BARRIER();
    
    /* Force through memory */
    temp1 = result;
    temp2 = temp1;
    BARRIER();
    
    return temp2;
}
#endif

/* Test with double vectors */
NOINLINE static v4df test_double_vectors(v4df a, v4df b, v4df c, v4df d,
                                         v4di mask)
{
    volatile v4df temp;
    v4df result;
    
    /* Complex floating point vector operations */
    
    /* 1. Conditional with FP comparison */
    v4df cmp = (a > b);
    v4df t1 = cmp ? a * c : b * d;
    BARRIER();
    
    /* 2. Shuffle FP vectors */
    v4df shuffled = __builtin_shuffle(t1, c, (v4si){3, 2, 1, 0});
    BARRIER();
    
    /* 3. Blend based on integer mask */
    v4df blended;
    for (int i = 0; i < 4; i++) {
        blended[i] = (mask[i] & 1) ? shuffled[i] : t1[i];
    }
    BARRIER();
    
    /* 4. Complex FP expression */
    result = blended + (a - b) * (c / d);
    BARRIER();
    
    temp = result;
    return temp;
}

/* Main test function */
NOINLINE static int run_tests(void)
{
    int checksum = 0;
    
    /* Initialize vectors with pattern */
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = {9, 10, 11, 12};
    v4si d = {13, 14, 15, 16};
    v4si mask1 = {3, 2, 1, 0};
    v4si mask2 = {0, 1, 2, 3};
    
    /* Test 10-operand path */
    v4si res1 = test_10_operands(a, b, c, d, mask1, mask2);
    BARRIER();
    
    /* Extract and sum results */
    for (int i = 0; i < 4; i++) {
        checksum += res1[i];
    }
    
#ifdef __AVX__
    /* Test 11-operand path with AVX */
    v8si avx_a = {1, 2, 3, 4, 5, 6, 7, 8};
    v8si avx_b = {9, 10, 11, 12, 13, 14, 15, 16};
    v8si avx_c = {17, 18, 19, 20, 21, 22, 23, 24};
    v8si avx_d = {25, 26, 27, 28, 29, 30, 31, 32};
    v8si avx_e = {33, 34, 35, 36, 37, 38, 39, 40};
    v8si avx_mask = {1, 0, 1, 0, 1, 0, 1, 0};
    
    v8si res2 = test_11_operands(avx_a, avx_b, avx_c, avx_d, avx_e, avx_mask);
    BARRIER();
    
    for (int i = 0; i < 8; i++) {
        checksum += res2[i];
    }
#endif
    
    /* Test with double vectors */
    v4df da = {1.0, 2.0, 3.0, 4.0};
    v4df db = {5.0, 6.0, 7.0, 8.0};
    v4df dc = {9.0, 10.0, 11.0, 12.0};
    v4df dd = {13.0, 14.0, 15.0, 16.0};
    v4di dmask = {1, 0, 1, 0};
    
    v4df res3 = test_double_vectors(da, db, dc, dd, dmask);
    BARRIER();
    
    for (int i = 0; i < 4; i++) {
        checksum += (int)res3[i];
    }
    
    return checksum;
}

int main(void)
{
    int result = run_tests();
    
    /* Use result to prevent optimization */
    printf("Checksum: %d\n", result);
    
    /* Return based on result to ensure execution */
    if (result != 0) {
        return 0;  /* Success */
    } else {
        return 1;  /* Shouldn't happen with our test patterns */
    }
}
