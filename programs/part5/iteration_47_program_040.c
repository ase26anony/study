/* Compile with:
   gcc -O3 -ffast-math -march=x86-64 -mavx2 -fdump-rtl-final -o test_conds test_conds.c
   gcc -O2 -ffast-math -march=x86-64 -fdump-rtl-expand -o test_conds2 test_conds.c
*/

#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#ifdef __SSE__
#include <xmmintrin.h>
#endif

#ifdef __AVX__
#include <immintrin.h>
#endif

/* Vector types for triggering vector comparisons */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Prevent inlining to ensure RTL generation */
__attribute__((noinline))
static int test_unordered_comparisons(float a, float b, double c, double d) {
    int sum = 0;
    
    /* Direct unordered checks using standard macros */
    sum += isunordered(a, b) ? 1 : 0;
    sum += isunordered(c, d) ? 2 : 0;
    
    /* Ordered checks */
    sum += isordered(a, b) ? 4 : 0;
    sum += isordered(c, d) ? 8 : 0;
    
    /* Less/greater unordered comparisons */
    sum += islessgreater(a, b) ? 16 : 0;
    sum += islessgreater(c, d) ? 32 : 0;
    
    /* Unordered comparisons using fast-math optimizations */
    if (!(a < b)) sum += 64;      /* May generate UNGE */
    if (!(a <= b)) sum += 128;    /* May generate UNGT */
    if (!(a > b)) sum += 256;     /* May generate UNLE */
    if (!(a >= b)) sum += 512;    /* May generate UNLT */
    
    return sum;
}

__attribute__((noinline))
static int test_mixed_conditionals(float a, float b, float c, float d, 
                                   double e, double f, double g, double h) {
    int sum = 0;
    
    /* Complex conditional expressions mixing different comparisons */
    sum += ((a < b) ? (c != d) : (e >= f)) ? 1 : 0;
    sum += ((a == b) ? (c > d) : (e <= f)) ? 2 : 0;
    sum += ((a != b) ? (c < d) : (e > f)) ? 4 : 0;
    
    /* Nested conditionals */
    sum += (a < b) ? ((c == d) ? 8 : 16) : ((e != f) ? 32 : 64);
    
    /* Chain of comparisons */
    sum += (a < b && c > d) ? 128 : 0;
    sum += (a == b || c != d) ? 256 : 0;
    sum += (e < f && g > h) ? 512 : 0;
    
    /* Ternary with unordered possibility */
    sum += (isunordered(a, b) ? (c < d) : (e > f)) ? 1024 : 0;
    
    return sum;
}

__attribute__((noinline))
static int test_vector_comparisons(v4sf va, v4sf vb, v2df vc, v2df vd) {
    int sum = 0;
    
#ifdef __SSE__
    /* Vector comparisons that may generate UNORD/ORD condition codes */
    v4sf vcmp = va < vb;
    float *f = (float*)&vcmp;
    sum += (f[0] != 0.0f) ? 1 : 0;
    sum += (f[1] != 0.0f) ? 2 : 0;
    sum += (f[2] != 0.0f) ? 4 : 0;
    sum += (f[3] != 0.0f) ? 8 : 0;
    
    vcmp = va == vb;
    f = (float*)&vcmp;
    sum += (f[0] != 0.0f) ? 16 : 0;
    sum += (f[1] != 0.0f) ? 32 : 0;
    
    /* Unordered vector comparisons */
    vcmp = !(va <= vb);  /* May generate UNGT in vector form */
    f = (float*)&vcmp;
    sum += (f[2] != 0.0f) ? 64 : 0;
    sum += (f[3] != 0.0f) ? 128 : 0;
#endif
    
#ifdef __AVX__
    /* Use AVX intrinsics for explicit unordered comparisons */
    __m128 av = _mm_load_ps((float*)&va);
    __m128 bv = _mm_load_ps((float*)&vb);
    __m128 cmp = _mm_cmp_ps(av, bv, _CMP_UNORD_Q);  /* UNORD */
    float *cf = (float*)&cmp;
    sum += (cf[0] != 0.0f) ? 256 : 0;
    sum += (cf[1] != 0.0f) ? 512 : 0;
    
    cmp = _mm_cmp_ps(av, bv, _CMP_NEQ_UQ);  /* UNEQ variant */
    cf = (float*)&cmp;
    sum += (cf[2] != 0.0f) ? 1024 : 0;
    sum += (cf[3] != 0.0f) ? 2048 : 0;
#endif
    
    return sum;
}

__attribute__((noinline))
static int test_nan_handling(float a, double b) {
    int sum = 0;
    
    /* Explicit NaN checks */
    sum += (a != a) ? 1 : 0;          /* true if a is NaN */
    sum += (!(b == b)) ? 2 : 0;       /* true if b is NaN */
    
    /* Mixed NaN and normal comparisons */
    float nan_f = __builtin_nanf("");
    double nan_d = __builtin_nan("");
    
    sum += (a == nan_f) ? 4 : 0;      /* Always false, even if a is NaN */
    sum += (a != nan_f) ? 8 : 0;      /* Always true, even if a is NaN */
    sum += (b < nan_d) ? 16 : 0;      /* Unordered comparison */
    sum += (b > nan_d) ? 32 : 0;      /* Unordered comparison */
    
    /* LTGT (not equal and ordered) */
    sum += (islessgreater(a, nan_f)) ? 64 : 0;
    sum += (islessgreater(b, nan_d)) ? 128 : 0;
    
    return sum;
}

__attribute__((noinline))
static int test_fast_math_optimizations(float a, float b, float c, float d) {
    int sum = 0;
    
    /* These comparisons may be optimized to UNEQ/LTGT under -ffast-math */
    sum += (a == b) ? 1 : 0;
    sum += (a != b) ? 2 : 0;
    sum += (a < b) ? 4 : 0;
    sum += (a > b) ? 8 : 0;
    sum += (a <= b) ? 16 : 0;
    sum += (a >= b) ? 32 : 0;
    
    /* Combined comparisons that might generate UNGE/UNLE/etc */
    if (!(a < b)) sum += 64;      /* UNGE: not less => greater or equal or unordered */
    if (!(a > b)) sum += 128;     /* UNLE: not greater => less or equal or unordered */
    if (!(a <= b)) sum += 256;    /* UNGT: not less-or-equal => greater or unordered */
    if (!(a >= b)) sum += 512;    /* UNLT: not greater-or-equal => less or unordered */
    
    /* UNEQ: unordered or equal */
    if (isunordered(a, b) || a == b) sum += 1024;
    
    return sum;
}

int main(int argc, char **argv) {
    /* Use argc to prevent excessive loop unrolling */
    volatile int iterations = (argc > 1) ? 100 : 50;
    int total_sum = 0;
    
    /* Initialize test data with pattern */
    float float_data[8];
    double double_data[8];
    
    for (int i = 0; i < 8; i++) {
        float_data[i] = (i % 2 == 0) ? (float)i * 1.5f : (float)i * -0.5f;
        double_data[i] = (i % 3 == 0) ? (double)i * 2.0 : (double)i * -1.0;
    }
    
    /* Add some NaN values (except when using -ffast-math which assumes no NaNs) */
#ifndef __FAST_MATH__
    float_data[3] = __builtin_nanf("");
    double_data[5] = __builtin_nan("");
#endif
    
    /* Initialize vector data */
    v4sf vec_f1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec_f2 = {4.0f, 3.0f, 2.0f, 1.0f};
    v2df vec_d1 = {1.0, 2.0};
    v2df vec_d2 = {2.0, 1.0};
    
    /* Main test loop */
    for (volatile int i = 0; i < iterations; i++) {
        int idx = i % 7;
        
        /* Call different test functions to trigger various condition codes */
        total_sum += test_unordered_comparisons(
            float_data[idx], float_data[idx+1],
            double_data[idx], double_data[idx+1]);
        
        total_sum += test_mixed_conditionals(
            float_data[idx], float_data[(idx+2)%8],
            float_data[(idx+3)%8], float_data[(idx+4)%8],
            double_data[idx], double_data[(idx+2)%8],
            double_data[(idx+3)%8], double_data[(idx+4)%8]);
        
        total_sum += test_vector_comparisons(vec_f1, vec_f2, vec_d1, vec_d2);
        
        total_sum += test_nan_handling(float_data[idx], double_data[idx]);
        
        total_sum += test_fast_math_optimizations(
            float_data[idx], float_data[(idx+1)%8],
            float_data[(idx+2)%8], float_data[(idx+3)%8]);
        
        /* Modify data slightly each iteration to prevent constant folding */
        vec_f1[0] += 0.001f;
        vec_d1[0] += 0.0001;
    }
    
    printf("Total checksum: %d\n", total_sum);
    return total_sum != 0 ? 0 : 1;
}
