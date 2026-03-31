/* test_condition_codes.c */
#include <stdio.h>
#include <math.h>
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

/* Global to prevent constant propagation */
volatile int g_volatile = 0;

/* Function 1: Direct unordered comparisons using math.h macros */
NOINLINE static int test_unordered_comparisons(float a, float b, double c, double d)
{
    int sum = 0;
    
    /* These should generate UNORDERED/ORDERED condition codes */
    sum += isunordered(a, b) ? 1 : 0;
    sum += isordered(a, b) ? 2 : 0;
    
    /* UNEQ: unordered or equal */
    sum += !(c < d) && !(c > d) ? 4 : 0;  /* May become UNEQ with fast-math */
    
    /* LTGT: less or greater (ordered and not equal) */
    sum += islessgreater(c, d) ? 8 : 0;
    
    return sum;
}

/* Function 2: Mixed ordered/unordered comparisons in complex expressions */
NOINLINE static int test_mixed_comparisons(float a, float b, float c, float d, float e, float f)
{
    int sum = 0;
    
    /* Complex conditional that may generate multiple condition codes */
    sum += ((a < b) ? (c != d) : (e >= f)) ? 1 : 0;
    
    /* Nested comparisons that could become UNGE/UNGT/etc */
    sum += (!(a >= b) && !isnan(a) && !isnan(b)) ? 2 : 0;  /* May become UNLT */
    sum += (!(a <= b) && !isnan(a) && !isnan(b)) ? 4 : 0;  /* May become UNGT */
    
    /* UNLE/UNLT patterns */
    sum += ((a == a) && (b == b) && !(a > b)) ? 8 : 0;    /* May become UNLE */
    sum += ((a == a) && (b == b) && !(a >= b)) ? 16 : 0;  /* May become UNLT */
    
    return sum;
}

/* Function 3: Vectorized comparisons using GCC vector extensions */
NOINLINE static int test_vector_comparisons(v4sf va, v4sf vb, v2df vc, v2df vd)
{
    int sum = 0;
    
    /* Element-wise unordered comparison */
    v4sf vcmp_unord = (va != va) | (vb != vb);  /* UNORDERED check */
    sum += vcmp_unord[0] ? 1 : 0;
    sum += vcmp_unord[1] ? 2 : 0;
    
    /* Ordered comparison */
    v4sf vcmp_ord = (va == va) & (vb == vb);    /* ORDERED check */
    sum += vcmp_ord[2] ? 4 : 0;
    
    /* UNEQ: unordered or equal */
    v4sf vcmp_ueq = !(va < vb) & !(va > vb);    /* May compile to UNEQ */
    sum += vcmp_ueq[3] ? 8 : 0;
    
    /* LTGT: less or greater (ordered and not equal) */
    v2df vcmp_ltgt = (vc < vd) | (vc > vd);     /* May compile to LTGT */
    sum += vcmp_ltgt[0] ? 16 : 0;
    
    return sum;
}

/* Function 4: SSE/AVX intrinsics for precise condition code generation */
#ifdef __SSE__
NOINLINE static int test_sse_intrinsics(__m128 a, __m128 b)
{
    int sum = 0;
    
    /* Generate specific comparison predicates */
    __m128 cmp_unord = _mm_cmpunord_ps(a, b);   /* UNORDERED */
    __m128 cmp_ord   = _mm_cmpord_ps(a, b);     /* ORDERED */
    __m128 cmp_neq_uq = _mm_cmpneq_ps(a, b);    /* NEQ_UQ (unordered, non-equal) */
    
    /* Extract results to prevent elimination */
    float res[4];
    _mm_storeu_ps(res, cmp_unord);
    sum += (res[0] != 0.0f) ? 1 : 0;
    
    _mm_storeu_ps(res, cmp_ord);
    sum += (res[1] != 0.0f) ? 2 : 0;
    
    _mm_storeu_ps(res, cmp_neq_uq);
    sum += (res[2] != 0.0f) ? 4 : 0;
    
    return sum;
}
#endif

/* Function 5: Fast-math optimized comparisons */
NOINLINE static int test_fast_math_optimizations(float a, float b, double c, double d)
{
    int sum = 0;
    
    /* With -ffast-math, these may compile to UNEQ/LTGT */
    VOLATILE_MATH float fa = a;
    VOLATILE_MATH float fb = b;
    
    /* UNEQ pattern: !(a < b) && !(a > b) */
    sum += !(fa < fb) && !(fa > fb) ? 1 : 0;
    
    /* LTGT pattern: (a < b) || (a > b) */
    sum += (fa < fb) || (fa > fb) ? 2 : 0;
    
    /* UNGE: !(a < b) */
    sum += !(c < d) ? 4 : 0;
    
    /* UNGT: !(a <= b) */
    sum += !(c <= d) ? 8 : 0;
    
    /* UNLE: !(a > b) */
    sum += !(fa > fb) ? 16 : 0;
    
    /* UNLT: !(a >= b) */
    sum += !(fa >= fb) ? 32 : 0;
    
    return sum;
}

/* Function 6: NaN checks that force unordered comparisons */
NOINLINE static int test_nan_checks(float a, float b, double c, double d)
{
    int sum = 0;
    
    /* Direct NaN checks using IEEE 754 property */
    sum += (a != a) ? 1 : 0;        /* true if a is NaN */
    sum += !(c == c) ? 2 : 0;       /* true if c is NaN */
    
    /* Combined NaN checks with comparisons */
    sum += ((a != a) || (b != b) || (a < b)) ? 4 : 0;
    sum += ((c == c) && (d == d) && (c > d)) ? 8 : 0;
    
    /* Complex NaN-aware comparison */
    sum += (!(a >= b) || (a != a) || (b != b)) ? 16 : 0;
    
    return sum;
}

int main(int argc, char *argv[])
{
    /* Patterned data including normal numbers, zeros, and NaN */
    float fdata[] = {
        1.0f, -1.0f, 0.0f, -0.0f,
        __builtin_nanf(""), 2.5f, -2.5f, INFINITY,
        3.14f, -3.14f, 100.0f, -100.0f,
        1e-10f, -1e-10f, 1e10f, -1e10f
    };
    
    double ddata[] = {
        1.0, -1.0, 0.0, -0.0,
        __builtin_nan(""), 2.5, -2.5, INFINITY,
        3.141592653589793, -3.141592653589793,
        1e-100, -1e-100, 1e100, -1e100
    };
    
    /* Vector data */
    v4sf v1 = {1.0f, 2.0f, __builtin_nanf(""), 4.0f};
    v4sf v2 = {4.0f, 2.0f, 3.0f, __builtin_nanf("")};
    v2df vd1 = {1.0, __builtin_nan("")};
    v2df vd2 = {__builtin_nan(""), 2.0};
    
    int total_sum = 0;
    
    /* Use argc to prevent loop unrolling */
    int iterations = (argc > 1) ? (argc % 8) + 4 : 8;
    
    for (int i = 0; i < iterations; i++) {
        /* Vary inputs using loop index to prevent constant folding */
        int idx1 = (i * 3) % 16;
        int idx2 = (i * 5) % 16;
        int idx3 = (i * 7) % 14;
        int idx4 = (i * 11) % 14;
        
        /* Call all test functions with varying inputs */
        total_sum += test_unordered_comparisons(
            fdata[idx1], fdata[idx2],
            ddata[idx3], ddata[idx4]
        );
        
        total_sum += test_mixed_comparisons(
            fdata[(idx1 + 1) % 16], fdata[(idx2 + 2) % 16],
            fdata[(idx3 + 3) % 16], fdata[(idx4 + 4) % 16],
            fdata[(idx1 + 5) % 16], fdata[(idx2 + 6) % 16]
        );
        
        total_sum += test_vector_comparisons(v1, v2, vd1, vd2);
        
#ifdef __SSE__
        __m128 sse_a = _mm_set_ps(fdata[idx1], fdata[idx2], fdata[idx3], fdata[idx4]);
        __m128 sse_b = _mm_set_ps(fdata[idx2], fdata[idx3], fdata[idx4], fdata[idx1]);
        total_sum += test_sse_intrinsics(sse_a, sse_b);
#endif
        
        total_sum += test_fast_math_optimizations(
            fdata[idx1] + i * 0.1f,
            fdata[idx2] - i * 0.1f,
            ddata[idx3] + i * 0.01,
            ddata[idx4] - i * 0.01
        );
        
        total_sum += test_nan_checks(
            fdata[idx1] * (1 + (i % 3)),
            fdata[idx2] / (1 + (i % 5)),
            ddata[idx3] * (1 + (i % 7)),
            ddata[idx4] / (1 + (i % 11))
        );
        
        /* Mix in some volatile operations to prevent optimization */
        total_sum += g_volatile;
    }
    
    printf("Final checksum: %d\n", total_sum);
    return total_sum != 0 ? 0 : 1;
}
