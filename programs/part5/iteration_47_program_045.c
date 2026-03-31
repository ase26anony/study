/* Compile with:
   -O2 -march=x86-64 -ffast-math -mavx2 -fdump-rtl-final
   -O3 -ffast-math -mavx2 -fdump-rtl-expand
   -O1 -da -fno-trapping-math
*/

#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <x86intrin.h>

/* Vector types for AVX/SSE */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Prevent inlining to ensure RTL generation */
__attribute__((noinline))
static int test_unordered_comparisons(float a, float b, double c, double d) {
    int sum = 0;
    
    /* Direct unordered comparisons using standard macros */
    sum += isunordered(a, b) ? 1 : 0;
    sum += islessgreater(a, b) ? 2 : 0;
    
    /* Mixed ordered/unordered comparisons */
    if ((a < b) ? isunordered(c, d) : (c >= d)) {
        sum += 4;
    }
    
    /* Complex conditional with multiple FP comparisons */
    sum += ((a != a) || (b != b)) ? 8 : 0;  /* NaN checks */
    sum += (!(c == c) && !(d == d)) ? 16 : 0;
    
    return sum;
}

__attribute__((noinline))
static int test_fast_math_optimizations(float f1, float f2, float f3, float f4) {
    int sum = 0;
    
    /* These often generate UNEQ/LTGT under -ffast-math */
    sum += (f1 == f2) ? 1 : 0;
    sum += (f1 != f2) ? 2 : 0;
    sum += (f1 < f2) ? 4 : 0;
    sum += (f1 > f2) ? 8 : 0;
    sum += (f1 <= f2) ? 16 : 0;
    sum += (f1 >= f2) ? 32 : 0;
    
    /* Chain of comparisons that might generate multiple condition codes */
    if ((f1 < f2) && (f3 > f4)) {
        sum += 64;
    }
    
    /* Ternary with different comparison types */
    sum += (f1 != f2) ? (f3 < f4 ? 128 : 256) : (f3 >= f4 ? 512 : 1024);
    
    return sum;
}

__attribute__((noinline))
static int test_vector_comparisons(v4sf va, v4sf vb, v2df vc, v2df vd) {
    int sum = 0;
    
    /* Vector comparisons using GCC extensions */
    v4sf vcmp1 = va < vb;      /* Should generate UNLT/UNLE */
    v4sf vcmp2 = va == vb;     /* Should generate UNEQ */
    v4sf vcmp3 = va != vb;     /* Should generate LTGT */
    
    /* Extract results to prevent elimination */
    float fcmp1[4], fcmp2[4], fcmp3[4];
    memcpy(fcmp1, &vcmp1, sizeof(v4sf));
    memcpy(fcmp2, &vcmp2, sizeof(v4sf));
    memcpy(fcmp3, &vcmp3, sizeof(v4sf));
    
    for (int i = 0; i < 4; i++) {
        sum += (fcmp1[i] != 0.0f) ? (1 << i) : 0;
        sum += (fcmp2[i] != 0.0f) ? (1 << (i + 4)) : 0;
        sum += (fcmp3[i] != 0.0f) ? (1 << (i + 8)) : 0;
    }
    
    /* AVX intrinsics for explicit unordered comparisons */
    __m128 av = _mm_loadu_ps((float*)&va);
    __m128 bv = _mm_loadu_ps((float*)&vb);
    
    /* _CMP_UNORD_Q = unordered (NaN) */
    __m128 unord_mask = _mm_cmp_ps(av, bv, _CMP_UNORD_Q);
    /* _CMP_NEQ_UQ = not equal unordered */
    __m128 neq_uq_mask = _mm_cmp_ps(av, bv, _CMP_NEQ_UQ);
    
    float unord_res[4], neq_uq_res[4];
    _mm_storeu_ps(unord_res, unord_mask);
    _mm_storeu_ps(neq_uq_res, neq_uq_mask);
    
    for (int i = 0; i < 4; i++) {
        sum += (unord_res[i] != 0.0f) ? (1 << (i + 12)) : 0;
        sum += (neq_uq_res[i] != 0.0f) ? (1 << (i + 16)) : 0;
    }
    
    return sum;
}

__attribute__((noinline))
static int test_mixed_precision(float a, double b, float c, double d) {
    int sum = 0;
    
    /* Mixed float/double comparisons */
    sum += ((double)a < b) ? 1 : 0;
    sum += (a < (float)b) ? 2 : 0;
    
    /* Complex expression with multiple condition codes */
    if (((a != a) || (b != b)) && ((c == c) && (d == d))) {
        sum += 4;
    }
    
    /* Ordered/unordered mix in ternary */
    float result = (isunordered(a, (float)b)) ? 
                   (islessgreater(c, (float)d) ? 8.0f : 16.0f) :
                   (isless(c, (float)d) ? 32.0f : 64.0f);
    
    sum += (int)result;
    
    return sum;
}

__attribute__((noinline))
static int test_nan_handling(float f1, float f2, double d1, double d2) {
    int sum = 0;
    
    /* Explicit NaN checks that generate unordered comparisons */
    sum += (f1 != f1) ? 1 : 0;      /* true if f1 is NaN */
    sum += (!(f2 == f2)) ? 2 : 0;   /* true if f2 is NaN */
    
    /* Ordered comparisons that might be optimized */
    sum += (f1 < f2) ? 4 : 0;
    sum += (d1 > d2) ? 8 : 0;
    
    /* Guarded fast-math path */
    volatile float vf1 = f1;
    volatile float vf2 = f2;
    if (vf1 == vf1 && vf2 == vf2) {  /* Both are not NaN */
        /* Fast-math optimizations apply here */
        sum += (f1 * f2 < f1 / f2) ? 16 : 0;
    }
    
    return sum;
}

int main(int argc, char *argv[]) {
    int total_sum = 0;
    
    /* Patterned data including NaN values */
    float float_data[] = {
        1.0f, 2.0f, 3.0f, 4.0f,
        0.0f, -0.0f, __builtin_nanf(""), INFINITY,
        5.0f, 6.0f, 7.0f, 8.0f
    };
    
    double double_data[] = {
        1.0, 2.0, __builtin_nan(""), 4.0,
        0.0, -0.0, 3.0, INFINITY
    };
    
    /* Use argc to prevent excessive loop unrolling */
    int iterations = (argc > 1) ? (argc % 8) + 4 : 8;
    
    for (int i = 0; i < iterations; i++) {
        /* Test unordered comparisons */
        total_sum += test_unordered_comparisons(
            float_data[i % 12],
            float_data[(i + 1) % 12],
            double_data[i % 8],
            double_data[(i + 1) % 8]
        );
        
        /* Test fast-math optimizations */
        total_sum += test_fast_math_optimizations(
            float_data[i % 12],
            float_data[(i + 2) % 12],
            float_data[(i + 3) % 12],
            float_data[(i + 4) % 12]
        );
        
        /* Test vector comparisons */
        v4sf va = {float_data[i % 12], float_data[(i + 1) % 12], 
                   float_data[(i + 2) % 12], float_data[(i + 3) % 12]};
        v4sf vb = {float_data[(i + 4) % 12], float_data[(i + 5) % 12],
                   float_data[(i + 6) % 12], float_data[(i + 7) % 12]};
        v2df vc = {double_data[i % 8], double_data[(i + 1) % 8]};
        v2df vd = {double_data[(i + 2) % 8], double_data[(i + 3) % 8]};
        
        total_sum += test_vector_comparisons(va, vb, vc, vd);
        
        /* Test mixed precision */
        total_sum += test_mixed_precision(
            float_data[i % 12],
            double_data[i % 8],
            float_data[(i + 1) % 12],
            double_data[(i + 1) % 8]
        );
        
        /* Test NaN handling */
        total_sum += test_nan_handling(
            float_data[i % 12],
            float_data[(i + 2) % 12],
            double_data[i % 8],
            double_data[(i + 2) % 8]
        );
    }
    
    printf("Total checksum: %d\n", total_sum);
    return total_sum != 0 ? 0 : 1;
}
