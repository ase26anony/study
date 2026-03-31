/* test_fp_conditions.c - Generate floating-point comparisons to trigger x86 condition code output */
#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Prevent optimization of critical functions */
#define NOINLINE __attribute__((noinline))
#define VOLATILE_VAR(v) volatile auto v

/* Vector types for SSE/AVX comparisons */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Test function 1: Unordered comparisons using standard macros */
NOINLINE static int test_unordered_comparisons(float a, float b, double c, double d) {
    int sum = 0;
    
    /* Generate unordered comparisons */
    sum += isunordered(a, b) ? 1 : 0;      /* Should generate "unord" */
    sum += isordered(a, b) ? 2 : 0;        /* Should generate "ord" */
    
    /* Mixed ordered/unordered comparisons */
    if ((a != a) || (b != b)) {            /* NaN checks */
        sum += 4;
    }
    
    /* Complex conditional with unordered result */
    sum += ((a < b) && !isunordered(a, b)) ? 8 : 0;
    sum += ((c >= d) || isunordered(c, d)) ? 16 : 0;
    
    return sum;
}

/* Test function 2: LTGT and UNEQ comparisons (common with -ffast-math) */
NOINLINE static int test_ltgt_uneq(float x, float y, double p, double q) {
    int sum = 0;
    
    /* These comparisons often generate LTGT/UNEQ with fast-math */
    sum += (x != y) ? 1 : 0;               /* May generate "une" (LTGT) */
    sum += (x == y) ? 2 : 0;               /* May generate "ueq" (UNEQ) with fast-math */
    
    /* Chained comparisons that might produce UNGE/UNGT/UNLE/UNLT */
    sum += (x > y) ? 4 : 0;
    sum += (x <= y) ? 8 : 0;
    sum += (p < q) ? 16 : 0;
    sum += (p >= q) ? 32 : 0;
    
    /* Ternary with different comparison types */
    sum += ((x < y) ? (p != q) : (x == y)) ? 64 : 0;
    
    return sum;
}

/* Test function 3: Vectorized comparisons using GCC vector extensions */
NOINLINE static int test_vector_comparisons(v4sf va, v4sf vb, v2df vc, v2df vd) {
    int sum = 0;
    
    /* Element-wise vector comparisons */
    v4sf vcmp_eq = va == vb;      /* Element-wise equality */
    v4sf vcmp_neq = va != vb;     /* Element-wise inequality */
    v4sf vcmp_lt = va < vb;       /* Element-wise less than */
    v4sf vcmp_ge = va >= vb;      /* Element-wise greater or equal */
    
    /* Extract results to scalar */
    float* feq = (float*)&vcmp_eq;
    float* fneq = (float*)&vcmp_neq;
    
    for (int i = 0; i < 4; i++) {
        sum += (feq[i] != 0.0f) ? (1 << i) : 0;
        sum += (fneq[i] != 0.0f) ? (1 << (i + 4)) : 0;
    }
    
    /* Double vector comparisons */
    v2df vcmp_d_eq = vc == vd;
    v2df vcmp_d_lt = vc < vd;
    
    double* deq = (double*)&vcmp_d_eq;
    sum += (deq[0] != 0.0) ? 256 : 0;
    sum += (deq[1] != 0.0) ? 512 : 0;
    
    return sum;
}

/* Test function 4: SSE/AVX intrinsics for direct control */
#ifdef __SSE__
NOINLINE static int test_sse_intrinsics(__m128 a, __m128 b) {
    int sum = 0;
    
    /* Various comparison intrinsics */
    __m128 cmp_unord = _mm_cmpunord_ps(a, b);   /* UNORD comparison */
    __m128 cmp_ord = _mm_cmpord_ps(a, b);       /* ORD comparison */
    __m128 cmp_neq = _mm_cmpneq_ps(a, b);       /* NEQ (may generate LTGT) */
    __m128 cmp_nlt = _mm_cmpnlt_ps(a, b);       /* NLT (UNGE) */
    __m128 cmp_nle = _mm_cmpnle_ps(a, b);       /* NLE (UNGT) */
    
    /* Extract mask bits */
    int mask_unord = _mm_movemask_ps(cmp_unord);
    int mask_ord = _mm_movemask_ps(cmp_ord);
    int mask_neq = _mm_movemask_ps(cmp_neq);
    
    sum += mask_unord;
    sum += mask_ord * 16;
    sum += mask_neq * 256;
    
    return sum;
}
#endif

/* Test function 5: Complex conditional expressions with mixed types */
NOINLINE static int test_mixed_conditionals(float f1, float f2, double d1, double d2) {
    int sum = 0;
    
    /* Nested ternary with different comparisons */
    sum += ((f1 < f2) ? 
            ((d1 != d2) ? 1 : 2) : 
            ((f1 == f2) ? 3 : 4));
    
    /* Logical combinations */
    sum += ((f1 > f2) && (d1 <= d2)) ? 8 : 0;
    sum += ((f1 != f2) || (d1 == d2)) ? 16 : 0;
    
    /* Comparison chain */
    if (f1 < f2) {
        if (d1 > d2) sum += 32;
        else if (d1 == d2) sum += 64;
    } else if (f1 > f2) {
        if (d1 < d2) sum += 128;
    } else { /* f1 == f2 */
        if (d1 != d2) sum += 256;
    }
    
    return sum;
}

/* Main test driver */
int main(int argc, char** argv) {
    int total_sum = 0;
    
    /* Patterned data including NaN values */
    float float_data[] = {
        1.0f, 2.0f, 3.0f, 4.0f,
        0.0f, -0.0f, __builtin_nanf(""), __builtin_inff(),
        -__builtin_inff(), 5.0f, 6.0f, 7.0f
    };
    
    double double_data[] = {
        1.0, 2.0, 3.0, 4.0,
        0.0, -0.0, __builtin_nan(""), __builtin_inf(),
        -__builtin_inf(), 5.0, 6.0, 7.0
    };
    
    /* Vector data */
    v4sf vec1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec2 = {4.0f, 3.0f, 2.0f, 1.0f};
    v2df dvec1 = {1.0, 2.0};
    v2df dvec2 = {2.0, 1.0};
    
#ifdef __SSE__
    __m128 sse1 = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    __m128 sse2 = _mm_set_ps(4.0f, 3.0f, 2.0f, 1.0f);
#endif
    
    /* Use argc to prevent loop unrolling and dead code elimination */
    VOLATILE_VAR(int iterations) = (argc > 1) ? argc : 10;
    if (iterations > 100) iterations = 100; /* Safety limit */
    
    for (int i = 0; i < iterations; i++) {
        /* Cycle through different data patterns */
        int idx = i % 8;
        int didx = i % 8;
        
        /* Call all test functions with varying inputs */
        total_sum += test_unordered_comparisons(
            float_data[idx], 
            float_data[idx + 1],
            double_data[didx],
            double_data[didx + 1]
        );
        
        total_sum += test_ltgt_uneq(
            float_data[idx + 2],
            float_data[idx + 3],
            double_data[didx + 2],
            double_data[didx + 3]
        );
        
        total_sum += test_vector_comparisons(vec1, vec2, dvec1, dvec2);
        
#ifdef __SSE__
        total_sum += test_sse_intrinsics(sse1, sse2);
#endif
        
        total_sum += test_mixed_conditionals(
            float_data[idx],
            float_data[idx + 4],
            double_data[didx],
            double_data[didx + 4]
        );
        
        /* Modify vectors slightly each iteration */
        vec1[0] += 0.1f;
        vec2[1] -= 0.1f;
        dvec1[0] += 0.05;
        dvec2[1] -= 0.05;
    }
    
    printf("Total checksum: %d\n", total_sum);
    return total_sum != 0 ? 0 : 1;
}
