/* test_condition_codes.c */
#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <xmmintrin.h>
#include <immintrin.h>

/* Prevent optimization of critical functions */
#define NOINLINE __attribute__((noinline))
#define VOLATILE_MATH volatile

/* Vector types for SSE/AVX */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Global volatile to prevent constant propagation */
volatile int g_volatile = 0;

/* Test 1: Direct unordered comparisons using math.h macros */
NOINLINE static int test_unordered_comparisons(float a, float b, double c, double d)
{
    int sum = 0;
    
    /* These should generate 'unord' condition codes */
    sum += isunordered(a, b) ? 1 : 0;
    sum += isunordered(c, d) ? 2 : 0;
    
    /* These should generate 'ord' condition codes */
    sum += islessgreater(a, b) ? 4 : 0;  /* LTGT -> "une" */
    sum += islessequal(a, b) ? 8 : 0;
    sum += isless(a, b) ? 16 : 0;
    sum += isgreater(a, b) ? 32 : 0;
    
    /* Mixed ordered/unordered checks */
    if (isunordered(a, b) || islessgreater(c, d)) {
        sum += 64;
    }
    
    return sum;
}

/* Test 2: Fast-math optimizations that generate UNEQ, UNGE, UNGT, etc. */
NOINLINE static int test_fast_math_patterns(float f1, float f2, float f3, float f4)
{
    int sum = 0;
    
    /* Under -ffast-math, these may generate unordered comparisons */
    sum += (f1 == f2) ? 1 : 0;      /* Potentially UNEQ */
    sum += (f1 != f2) ? 2 : 0;      /* Potentially LTGT -> "une" */
    sum += (f1 >= f2) ? 4 : 0;      /* Potentially UNGE -> "nlt" */
    sum += (f1 > f2) ? 8 : 0;       /* Potentially UNGT -> "nle" */
    sum += (f1 <= f2) ? 16 : 0;     /* Potentially UNLE -> "ule" */
    sum += (f1 < f2) ? 32 : 0;      /* Potentially UNLT -> "ult" */
    
    /* Complex conditional expression mixing operators */
    sum += ((f1 < f2) ? (f3 != f4) : (f1 >= f3)) ? 64 : 0;
    
    /* Chain of comparisons */
    if ((f1 == f2) && (f3 > f4) && (f1 != f3)) {
        sum += 128;
    }
    
    return sum;
}

/* Test 3: NaN checks that force unordered comparisons */
NOINLINE static int test_nan_handling(float a, double b)
{
    int sum = 0;
    
    /* Direct NaN checks - these often generate unordered comparisons */
    sum += (a != a) ? 1 : 0;        /* true if a is NaN -> unord */
    sum += !(b == b) ? 2 : 0;       /* true if b is NaN -> unord */
    
    /* Ordered comparison that must handle NaN */
    sum += (a < b) ? 4 : 0;
    sum += (a > b) ? 8 : 0;
    
    /* Mixed NaN and normal comparisons */
    if ((a != a) || (a < b)) {
        sum += 16;
    }
    
    return sum;
}

/* Test 4: Vectorized comparisons using GCC vector extensions */
NOINLINE static int test_vector_comparisons(v4sf va, v4sf vb, v2df vc, v2df vd)
{
    int sum = 0;
    
    /* Vector comparisons generate condition codes for each element */
    v4sf vcmp_eq = va == vb;    /* Element-wise equality */
    v4sf vcmp_ne = va != vb;    /* Element-wise inequality -> potentially LTGT */
    v4sf vcmp_lt = va < vb;     /* Element-wise less than */
    v4sf vcmp_ge = va >= vb;    /* Element-wise greater or equal */
    
    /* Extract results to prevent elimination */
    float temp[4];
    memcpy(temp, &vcmp_eq, sizeof(v4sf));
    sum += (temp[0] != 0.0f) ? 1 : 0;
    sum += (temp[1] != 0.0f) ? 2 : 0;
    
    memcpy(temp, &vcmp_ne, sizeof(v4sf));
    sum += (temp[2] != 0.0f) ? 4 : 0;
    
    memcpy(temp, &vcmp_ge, sizeof(v4sf));
    sum += (temp[3] != 0.0f) ? 8 : 0;
    
    /* Double vector comparisons */
    double dtemp[2];
    v2df vcmp_dbl = vc <= vd;   /* Element-wise less or equal */
    memcpy(dtemp, &vcmp_dbl, sizeof(v2df));
    sum += (dtemp[0] != 0.0) ? 16 : 0;
    sum += (dtemp[1] != 0.0) ? 32 : 0;
    
    return sum;
}

/* Test 5: AVX intrinsics for explicit unordered comparisons */
#ifdef __AVX__
NOINLINE static int test_avx_intrinsics(__m256 a, __m256 b)
{
    int sum = 0;
    
    /* Direct unordered comparison */
    __m256 cmp_unord = _mm256_cmp_ps(a, b, _CMP_UNORD_Q);  /* unordered (quiet) */
    __m256 cmp_neq_uq = _mm256_cmp_ps(a, b, _CMP_NEQ_UQ);  /* not equal (unordered, quiet) */
    __m256 cmp_nlt_uq = _mm256_cmp_ps(a, b, _CMP_NLT_UQ);  /* not less than (unordered, quiet) -> UNGE */
    __m256 cmp_nle_uq = _mm256_cmp_ps(a, b, _CMP_NLE_UQ);  /* not less or equal (unordered, quiet) -> UNGT */
    
    /* Extract mask bits */
    int mask_unord = _mm256_movemask_ps(cmp_unord);
    int mask_neq = _mm256_movemask_ps(cmp_neq_uq);
    int mask_nlt = _mm256_movemask_ps(cmp_nlt_uq);
    int mask_nle = _mm256_movemask_ps(cmp_nle_uq);
    
    sum += mask_unord;
    sum += mask_neq * 2;
    sum += mask_nlt * 4;
    sum += mask_nle * 8;
    
    return sum;
}
#endif

/* Test 6: Complex conditional with mixed comparison types */
NOINLINE static int test_mixed_conditionals(float a, float b, float c, float d, 
                                           double e, double f, double g, double h)
{
    int sum = 0;
    
    /* Nested ternary with different comparison types */
    sum += (a < b) ? 
           ((c == d) ? 1 : ((e != f) ? 2 : 3)) : 
           ((g >= h) ? 4 : 5);
    
    /* Switch-like structure with floating comparisons */
    if (isunordered(a, b)) {
        sum += 10;
    } else if (islessgreater(c, d)) {  /* LTGT */
        sum += 20;
    } else if (e == f) {               /* Potentially UNEQ */
        sum += 30;
    } else if (g <= h) {               /* Potentially UNLE */
        sum += 40;
    } else if (a > b) {                /* Potentially UNGT */
        sum += 50;
    }
    
    /* Complex boolean expression */
    if ((a == b) && (c != d) && (e < f) && (g > h)) {
        sum += 100;
    }
    
    return sum;
}

/* Main test driver */
int main(int argc, char *argv[])
{
    int total_sum = 0;
    
    /* Patterned data including NaN values */
    float float_data[] = {
        1.0f, 2.0f, 3.0f, 4.0f,
        0.0f, -0.0f, __builtin_nanf(""), INFINITY,
        5.0f, 6.0f, 7.0f, 8.0f
    };
    
    double double_data[] = {
        1.0, 2.0, 3.0, 4.0,
        0.0, -0.0, __builtin_nan(""), (double)INFINITY,
        5.0, 6.0, 7.0, 8.0
    };
    
    /* Use argc to prevent loop unrolling from eliminating comparisons */
    int iterations = (argc > 1) ? (argc % 8) + 4 : 8;
    
    for (int i = 0; i < iterations; i++) {
        /* Test 1: Unordered comparisons */
        total_sum += test_unordered_comparisons(
            float_data[i % 12] + g_volatile,
            float_data[(i + 1) % 12],
            double_data[i % 12],
            double_data[(i + 2) % 12]
        );
        
        /* Test 2: Fast-math patterns */
        total_sum += test_fast_math_patterns(
            float_data[i % 12],
            float_data[(i + 3) % 12],
            float_data[(i + 6) % 12],
            float_data[(i + 9) % 12]
        );
        
        /* Test 3: NaN handling */
        total_sum += test_nan_handling(
            float_data[i % 12],
            double_data[(i + 4) % 12]
        );
        
        /* Test 4: Vector comparisons */
        v4sf va = {float_data[0], float_data[1], float_data[2], float_data[3]};
        v4sf vb = {float_data[4], float_data[5], float_data[6], float_data[7]};
        v2df vc = {double_data[0], double_data[1]};
        v2df vd = {double_data[2], double_data[3]};
        total_sum += test_vector_comparisons(va, vb, vc, vd);
        
        /* Test 5: AVX intrinsics (if available) */
        #ifdef __AVX__
        __m256 a = _mm256_set_ps(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f);
        __m256 b = _mm256_set_ps(8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f);
        total_sum += test_avx_intrinsics(a, b);
        #endif
        
        /* Test 6: Mixed conditionals */
        total_sum += test_mixed_conditionals(
            float_data[i % 12],
            float_data[(i + 1) % 12],
            float_data[(i + 2) % 12],
            float_data[(i + 3) % 12],
            double_data[i % 12],
            double_data[(i + 1) % 12],
            double_data[(i + 2) % 12],
            double_data[(i + 3) % 12]
        );
        
        /* Modify volatile to prevent optimization */
        g_volatile += i;
    }
    
    printf("Total checksum: %d\n", total_sum);
    return total_sum != 0 ? 0 : 1;
}
