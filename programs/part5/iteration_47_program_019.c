#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Vector types for SSE/AVX operations */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Test functions with different comparison patterns */
static __attribute__((noinline)) 
int test_unordered_comparisons(float a, float b, double c, double d) {
    int sum = 0;
    
    /* Direct unordered checks using standard macros */
    sum += isunordered(a, b) ? 1 : 0;
    sum += isunordered(c, d) ? 2 : 0;
    
    /* NaN checks using self-comparison */
    sum += (a != a) ? 4 : 0;  /* true if a is NaN */
    sum += !(c == c) ? 8 : 0; /* true if c is NaN */
    
    /* Mixed ordered/unordered comparisons */
    if (isunordered(a, b) || (c > d)) {
        sum += 16;
    }
    
    return sum;
}

static __attribute__((noinline))
int test_ltgt_comparisons(float a, float b, float c, float d) {
    int sum = 0;
    
    /* islessgreater generates LTGT condition code */
    sum += islessgreater(a, b) ? 1 : 0;
    sum += islessgreater(c, d) ? 2 : 0;
    
    /* Equivalent to islessgreater: (a < b) || (a > b) */
    sum += ((a < b) || (a > b)) ? 4 : 0;
    
    /* Complex conditional with mixed comparisons */
    sum += ((a < b) ? islessgreater(c, d) : (a != b)) ? 8 : 0;
    
    return sum;
}

static __attribute__((noinline))
int test_mixed_ordered_unordered(float a, float b, double c, double d, float e) {
    int sum = 0;
    
    /* Chain of different comparison types */
    if ((a < b) && !isunordered(c, d)) {
        sum += 1;
    }
    
    if (isunordered(a, b) || (c >= d)) {
        sum += 2;
    }
    
    /* Ternary with different comparison operators */
    sum += ((a == b) ? (c != d) : (e > 0.0f)) ? 4 : 0;
    
    /* Nested conditionals */
    if ((a <= b) ? (c == d) : isunordered(e, 0.0f)) {
        sum += 8;
    }
    
    return sum;
}

static __attribute__((noinline))
int test_vector_comparisons(v4sf va, v4sf vb, v2df vc, v2df vd) {
    int sum = 0;
    
    /* Element-wise vector comparisons */
    v4sf vcmp1 = va < vb;      /* Generates ordered less-than */
    v4sf vcmp2 = va != vb;     /* Generates unordered not-equal */
    v2df vcmp3 = vc == vd;     /* Generates ordered equal */
    
    /* Extract results to scalar */
    float f1 = vcmp1[0] + vcmp1[1] + vcmp1[2] + vcmp1[3];
    float f2 = vcmp2[0] + vcmp2[1] + vcmp2[2] + vcmp2[3];
    double d1 = vcmp3[0] + vcmp3[1];
    
    sum += (f1 > 0.0f) ? 1 : 0;
    sum += (f2 > 0.0f) ? 2 : 0;
    sum += (d1 > 0.0) ? 4 : 0;
    
    return sum;
}

#ifdef __SSE__
static __attribute__((noinline))
int test_sse_intrinsics(__m128 a, __m128 b, __m128d c, __m128d d) {
    int sum = 0;
    
    /* SSE intrinsics with various comparison predicates */
    __m128 cmp1 = _mm_cmpneq_ps(a, b);      /* Not equal (unordered allowed) */
    __m128 cmp2 = _mm_cmpnlt_ps(a, b);      /* Not less-than (UNGE: nlt) */
    __m128 cmp3 = _mm_cmpnle_ps(a, b);      /* Not less-or-equal (UNGT: nle) */
    __m128d cmp4 = _mm_cmpunord_pd(c, d);   /* Unordered */
    __m128d cmp5 = _mm_cmpord_pd(c, d);     /* Ordered */
    
    /* Extract mask bits */
    sum += _mm_movemask_ps(cmp1);
    sum += _mm_movemask_ps(cmp2) << 4;
    sum += _mm_movemask_ps(cmp3) << 8;
    sum += _mm_movemask_pd(cmp4) << 12;
    sum += _mm_movemask_pd(cmp5) << 14;
    
    return sum & 0xFFFF;
}
#endif

static __attribute__((noinline))
int test_fast_math_patterns(float a, float b, float c, float d) {
    int sum = 0;
    
    /* These patterns often generate UNEQ/LTGT under -ffast-math */
    
    /* UNEQ: unordered or equal */
    if (!(a < b) && !(a > b)) {  /* Equivalent to isunordered(a,b) || a == b */
        sum += 1;
    }
    
    /* LTGT: less-than or greater-than (ordered, not equal) */
    if ((a < b) || (a > b)) {    /* Equivalent to islessgreater(a,b) */
        sum += 2;
    }
    
    /* UNLE: unordered or less-or-equal */
    if (!(a > b)) {              /* Equivalent to isunordered(a,b) || a <= b */
        sum += 4;
    }
    
    /* UNLT: unordered or less-than */
    if (!(a >= b)) {             /* Equivalent to isunordered(a,b) || a < b */
        sum += 8;
    }
    
    /* Complex expression mixing different comparisons */
    sum += ((a == c) ? (b != d) : ((a < c) && (b > d))) ? 16 : 0;
    
    return sum;
}

int main(int argc, char *argv[]) {
    volatile int iterations = (argc > 1) ? atoi(argv[1]) : 100;
    if (iterations < 1) iterations = 1;
    
    int total_sum = 0;
    
    /* Patterned data including NaN values */
    float float_data[] = {
        1.0f, 2.0f, 0.0f, -0.0f,
        __builtin_nanf(""), 3.0f, __builtin_nanf(""), 4.0f,
        INFINITY, -INFINITY, 5.0f, 6.0f
    };
    
    double double_data[] = {
        1.0, 2.0, 0.0, -0.0,
        __builtin_nan(""), 3.0, __builtin_nan(""), 4.0,
        INFINITY, -INFINITY, 5.0, 6.0
    };
    
    /* Initialize vector data */
    v4sf vec_float1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec_float2 = {4.0f, 3.0f, 2.0f, 1.0f};
    v2df vec_double1 = {1.0, 2.0};
    v2df vec_double2 = {2.0, 1.0};
    
#ifdef __SSE__
    __m128 sse_float1 = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    __m128 sse_float2 = _mm_set_ps(4.0f, 3.0f, 2.0f, 1.0f);
    __m128d sse_double1 = _mm_set_pd(1.0, 2.0);
    __m128d sse_double2 = _mm_set_pd(2.0, 1.0);
#endif
    
    for (volatile int i = 0; i < iterations; i++) {
        int idx = i % 8;
        
        /* Call test functions with different data patterns */
        total_sum += test_unordered_comparisons(
            float_data[idx], float_data[idx+1],
            double_data[idx], double_data[idx+2]
        );
        
        total_sum += test_ltgt_comparisons(
            float_data[idx], float_data[idx+2],
            float_data[idx+1], float_data[idx+3]
        );
        
        total_sum += test_mixed_ordered_unordered(
            float_data[idx], float_data[idx+3],
            double_data[idx], double_data[idx+1],
            float_data[idx+2]
        );
        
        total_sum += test_vector_comparisons(
            vec_float1, vec_float2,
            vec_double1, vec_double2
        );
        
#ifdef __SSE__
        total_sum += test_sse_intrinsics(
            sse_float1, sse_float2,
            sse_double1, sse_double2
        );
#endif
        
        total_sum += test_fast_math_patterns(
            float_data[idx], float_data[idx+1],
            float_data[idx+2], float_data[idx+3]
        );
        
        /* Modify data slightly to prevent complete optimization */
        vec_float1[0] += 0.1f;
        vec_double1[0] += 0.1;
    }
    
    printf("Total checksum: %d\n", total_sum);
    return total_sum != 0 ? 0 : 1;
}
