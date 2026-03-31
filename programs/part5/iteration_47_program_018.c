/* Condition code test program targeting i386.cc lines 13992-14017 */
#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <string.h>
#include <xmmintrin.h>
#include <immintrin.h>

/* Prevent optimization from removing test cases */
static volatile int force_volatile = 0;

/* Vector types for AVX/SSE comparisons */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Test 1: Direct unordered comparisons using math.h macros */
__attribute__((noinline))
static int test_unordered_comparisons(float a, float b, double c, double d) {
    int sum = 0;
    
    /* These should generate UNORDERED/ORDERED condition codes */
    sum += isunordered(a, b) ? 1 : 0;
    sum += isordered(a, b) ? 2 : 0;
    
    /* UNEQ: unordered or equal */
    sum += !isgreater(c, d) && !isless(c, d) ? 4 : 0;
    
    /* UNGE: not less than (unordered or greater or equal) */
    sum += !isless(c, d) ? 8 : 0;
    
    /* UNGT: not less or equal (unordered or greater) */
    sum += !islessequal(c, d) ? 16 : 0;
    
    /* UNLE: unordered or less or equal */
    sum += !isgreater(c, d) ? 32 : 0;
    
    /* UNLT: unordered or less than */
    sum += !isgreaterequal(c, d) ? 64 : 0;
    
    /* LTGT: less or greater (ordered and not equal) */
    sum += islessgreater(a, b) ? 128 : 0;
    
    return sum;
}

/* Test 2: Mixed comparisons in complex conditional expressions */
__attribute__((noinline))
static int test_mixed_conditionals(float a, float b, float c, float d, float e, float f) {
    int sum = 0;
    
    /* Complex ternary with different comparison types */
    for (int i = 0; i < 3; i++) {
        /* This should generate multiple condition codes */
        sum += ((a < b) ? (c != d) : (e >= f)) ? (1 << i) : 0;
        
        /* Another mixed expression */
        sum += ((a == b) ? (c > d) : (e <= f)) ? (2 << i) : 0;
        
        /* Chain of comparisons */
        sum += (a != a) || (b != b) ? (4 << i) : 0;  /* NaN checks */
        sum += (c == c) && (d == d) ? (8 << i) : 0;  /* Not NaN checks */
    }
    
    return sum;
}

/* Test 3: Vectorized comparisons using GCC vector extensions */
__attribute__((noinline))
static int test_vector_comparisons(v4sf va, v4sf vb, v2df vc, v2df vd) {
    int sum = 0;
    
    /* Vector comparisons generate UNORD/ORD/etc condition codes */
    v4sf mask_unord = __builtin_ia32_cmpunordps(va, vb);
    v4sf mask_ord = __builtin_ia32_cmpordps(va, vb);
    v4sf mask_neq = __builtin_ia32_cmpneqps(va, vb);
    v4sf mask_nlt = __builtin_ia32_cmpnltps(va, vb);  /* UNGE */
    v4sf mask_nle = __builtin_ia32_cmpnleps(va, vb);  /* UNGT */
    
    /* Extract results to prevent elimination */
    float m[4];
    memcpy(m, &mask_unord, sizeof(m));
    sum += (int)(m[0] + m[1] + m[2] + m[3]);
    
    memcpy(m, &mask_ord, sizeof(m));
    sum += (int)(m[0] + m[1] + m[2] + m[3]) * 2;
    
    memcpy(m, &mask_neq, sizeof(m));
    sum += (int)(m[0] + m[1] + m[2] + m[3]) * 3;
    
    /* Double precision vector comparisons */
    __m128d cmp_unord = _mm_cmpunord_pd(vc, vd);
    __m128d cmp_neq_uq = _mm_cmpneq_pd(vc, vd);  /* UNEQ variant */
    
    double dm[2];
    memcpy(dm, &cmp_unord, sizeof(dm));
    sum += (int)(dm[0] + dm[1]) * 4;
    
    memcpy(dm, &cmp_neq_uq, sizeof(dm));
    sum += (int)(dm[0] + dm[1]) * 5;
    
    return sum;
}

/* Test 4: Fast-math optimized comparisons */
__attribute__((noinline))
static int test_fast_math_optimizations(float a, float b, double c, double d) {
    int sum = 0;
    
    /* With -ffast-math, these may generate UNEQ/LTGT codes */
    for (int i = 0; i < 2; i++) {
        /* Ordered comparisons that fast-math might transform */
        sum += (a == b) ? 1 : 0;
        sum += (c != d) ? 2 : 0;
        sum += (a < b) ? 4 : 0;
        sum += (c > d) ? 8 : 0;
        sum += (a <= b) ? 16 : 0;
        sum += (c >= d) ? 32 : 0;
        
        /* LTGT: ordered and not equal */
        sum += ((a < b) || (a > b)) ? 64 : 0;
        
        /* UNEQ: unordered or equal */
        sum += !((a < b) || (a > b)) ? 128 : 0;
    }
    
    return sum;
}

/* Test 5: NaN-producing comparisons */
__attribute__((noinline))
static int test_nan_comparisons(float a, float b, double c, double d) {
    int sum = 0;
    
    /* Explicit NaN checks */
    float nan_f = __builtin_nanf("");
    double nan_d = __builtin_nan("");
    
    /* These should generate unordered comparisons */
    sum += (a != a) ? 1 : 0;           /* a is NaN */
    sum += !(b == b) ? 2 : 0;          /* b is NaN */
    sum += isunordered(c, nan_d) ? 4 : 0;
    sum += isunordered(nan_f, d) ? 8 : 0;
    
    /* Mixed NaN and normal comparisons */
    sum += (a < nan_f) ? 16 : 0;       /* Always false with NaN */
    sum += (nan_d == c) ? 32 : 0;      /* Always false with NaN */
    sum += (nan_f != nan_f) ? 64 : 0;  /* NaN != NaN is true */
    sum += (nan_d == nan_d) ? 128 : 0; /* NaN == NaN is false */
    
    return sum;
}

int main(int argc, char **argv) {
    int total_sum = 0;
    
    /* Patterned data including normal numbers, zeros, and NaN */
    float float_data[] = {
        1.0f, 2.0f, 0.0f, -0.0f, 3.14f, -2.71f,
        __builtin_nanf(""), INFINITY, -INFINITY
    };
    
    double double_data[] = {
        1.0, 2.0, 0.0, -0.0, 3.1415926535, -2.7182818284,
        __builtin_nan(""), (double)INFINITY, -(double)INFINITY
    };
    
    /* Vector data */
    v4sf vec1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec2 = {4.0f, 3.0f, 2.0f, 1.0f};
    v2df dvec1 = {1.0, 2.0};
    v2df dvec2 = {2.0, 1.0};
    
    /* Use argc to prevent excessive unrolling */
    int iterations = (argc > 1) ? (argc % 5) + 1 : 3;
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Vary inputs slightly each iteration */
        int idx = iter % 9;
        int idx2 = (iter + 1) % 9;
        int idx3 = (iter + 2) % 9;
        int idx4 = (iter + 3) % 9;
        
        /* Test 1: Unordered comparisons */
        total_sum += test_unordered_comparisons(
            float_data[idx], float_data[idx2],
            double_data[idx3], double_data[idx4]
        );
        
        /* Test 2: Mixed conditionals */
        total_sum += test_mixed_conditionals(
            float_data[idx], float_data[idx2], float_data[idx3],
            float_data[idx4], float_data[(idx + 4) % 9],
            float_data[(idx + 5) % 9]
        );
        
        /* Test 3: Vector comparisons */
        total_sum += test_vector_comparisons(vec1, vec2, dvec1, dvec2);
        
        /* Test 4: Fast-math optimizations */
        total_sum += test_fast_math_optimizations(
            float_data[idx], float_data[idx2],
            double_data[idx3], double_data[idx4]
        );
        
        /* Test 5: NaN comparisons */
        total_sum += test_nan_comparisons(
            float_data[idx], float_data[idx2],
            double_data[idx3], double_data[idx4]
        );
        
        /* Modify vectors slightly */
        vec1[0] += 0.1f;
        vec2[1] -= 0.1f;
        dvec1[0] += 0.01;
        dvec2[1] -= 0.01;
    }
    
    /* Add volatile to prevent dead code elimination */
    total_sum += force_volatile;
    
    printf("Total checksum: %d\n", total_sum);
    return total_sum != 0 ? 0 : 1;
}
