/* Test program to cover 10/11 operand expansion in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <immintrin.h>

/* Prevent optimization */
#define NOOPT __attribute__((noinline, noclone))

/* Vector types using GCC extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));

/* Larger vector types for AVX */
#ifdef __AVX__
typedef float v8sf __attribute__((vector_size(32)));
typedef double v4df __attribute__((vector_size(32)));
#endif

/* Compiler barrier */
#define BARRIER() asm volatile("" ::: "memory")

/* Force memory operations */
static volatile v4si v4si_sink;
static volatile v2df v2df_sink;
static volatile v4sf v4sf_sink;

/* Test function with many vector operations */
NOOPT v4si test_10_operands(v4si a, v4si b, v4si c, v4si d, 
                           v4si e, v4si f, v4si mask) {
    /* Complex shuffle with runtime mask - may need many operands */
    v4si shuffled = __builtin_shuffle(a, b, mask);
    BARRIER();
    
    /* Vector conditional with comparison */
    v4si cmp = a > b;
    v4si cond_result = cmp ? (c * d) : (e + f);
    BARRIER();
    
    /* Blend operation using conditional */
    v4si blended = __builtin_shufflevector(a, b, 
        (cmp[0] > 0) ? 0 : 4,
        (cmp[1] > 0) ? 1 : 5,
        (cmp[2] > 0) ? 2 : 6,
        (cmp[3] > 0) ? 3 : 7);
    BARRIER();
    
    /* Complex arithmetic chain */
    v4si t1 = shuffled * cond_result;
    v4si t2 = blended + t1;
    v4si t3 = t2 - a;
    v4si t4 = t3 & b;
    BARRIER();
    
    /* Force memory store */
    v4si_sink = t4;
    
    return t4;
}

NOOPT v2df test_11_operands(v2df a, v2df b, v2df c, v2df d,
                           v2df e, v2df f, v2df g, v2di mask) {
    /* Convert between types - may need extra operands */
    v2di int_vec = __builtin_convertvector(a, v2di);
    BARRIER();
    
    /* Complex shuffle with conversion */
    v2df shuffled = __builtin_shuffle(a, b, (v2di){mask[0] & 1, mask[1] & 1});
    BARRIER();
    
    /* Vector comparison with multiple conditions */
    v2df cmp1 = a > b;
    v2df cmp2 = c < d;
    v2df cmp = cmp1 & cmp2;
    BARRIER();
    
    /* Conditional with arithmetic in both branches */
    v2df true_val = (e * f) + g;
    v2df false_val = (e / f) - g;
    v2df cond_result = cmp ? true_val : false_val;
    BARRIER();
    
    /* Blend with shuffle */
    v2df blended = __builtin_shufflevector(shuffled, cond_result,
        (int_vec[0] > 0) ? 0 : 2,
        (int_vec[1] > 0) ? 1 : 3);
    BARRIER();
    
    /* Complex FP arithmetic chain */
    v2df t1 = blended * a;
    v2df t2 = t1 + b;
    v2df t3 = t2 - c;
    v2df t4 = t3 * d;
    v2df t5 = t4 / e;
    BARRIER();
    
    /* Force multiple memory operations */
    v2df_sink = t5;
    v2df_sink = blended;
    
    return t5;
}

#ifdef __AVX__
NOOPT v4sf test_avx_many_ops(v8sf a, v8sf b, v8sf c, v8sf mask) {
    /* Extract halves for operations */
    v4sf a_low = __builtin_shufflevector(a, a, 0, 1, 2, 3);
    v4sf a_high = __builtin_shufflevector(a, a, 4, 5, 6, 7);
    v4sf b_low = __builtin_shufflevector(b, b, 0, 1, 2, 3);
    BARRIER();
    
    /* Complex operation mixing halves */
    v4sf t1 = a_low * b_low;
    v4sf t2 = a_high + b_low;
    v4sf mask_low = __builtin_shufflevector(mask, mask, 0, 1, 2, 3);
    BARRIER();
    
    /* Conditional with mask */
    v4sf cmp = mask_low > (v4sf){0.5f, 0.5f, 0.5f, 0.5f};
    v4sf result = cmp ? t1 : t2;
    BARRIER();
    
    /* Store to volatile */
    v4sf_sink = result;
    
    return result;
}
#endif

/* Main test driver */
int main() {
    /* Initialize test vectors */
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = {9, 10, 11, 12};
    v4si d = {13, 14, 15, 16};
    v4si e = {17, 18, 19, 20};
    v4si f = {21, 22, 23, 24};
    v4si mask = {1, 0, 3, 2};
    
    v2df da = {1.0, 2.0};
    v2df db = {3.0, 4.0};
    v2df dc = {5.0, 6.0};
    v2df dd = {7.0, 8.0};
    v2df de = {9.0, 10.0};
    v2df df = {11.0, 12.0};
    v2df dg = {13.0, 14.0};
    v2di dmask = {0, 1};
    
    printf("Testing 10/11 operand expansion...\n");
    
    /* Call test functions multiple times with different inputs */
    v4si res1 = test_10_operands(a, b, c, d, e, f, mask);
    BARRIER();
    
    v2df res2 = test_11_operands(da, db, dc, dd, de, df, dg, dmask);
    BARRIER();
    
#ifdef __AVX__
    v8sf avx_a = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    v8sf avx_b = {9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f};
    v8sf avx_c = {17.0f, 18.0f, 19.0f, 20.0f, 21.0f, 22.0f, 23.0f, 24.0f};
    v8sf avx_mask = {0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f};
    
    v4sf res3 = test_avx_many_ops(avx_a, avx_b, avx_c, avx_mask);
    BARRIER();
#endif
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < 4; i++) {
        checksum += res1[i];
    }
    
    checksum += (int)(res2[0] + res2[1]);
    
#ifdef __AVX__
    for (int i = 0; i < 4; i++) {
        checksum += (int)res3[i];
    }
#endif
    
    printf("Checksum: %d\n", checksum);
    
    /* Return based on checksum to ensure execution */
    return (checksum > 1000) ? 0 : 1;
}
