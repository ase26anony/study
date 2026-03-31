#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <xmmintrin.h>
#include <immintrin.h>

/* Prevent optimization of critical sections */
static volatile int sink;

/* Vector types for AVX/SSE operations */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Test function 1: Direct unordered comparisons using math.h macros */
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

/* Test function 2: Mixed comparisons in conditional expressions */
__attribute__((noinline))
static int test_mixed_comparisons(float a, float b, float c, float d, float e, float f) {
    int sum = 0;
    
    /* Complex conditional with mixed operators */
    for (int i = 0; i < 4; i++) {
        float t = (i & 1) ? a : b;
        float u = (i & 2) ? c : d;
        
        /* Ternary with different comparison types */
        sum += ((t < u) ? (e != f) : (e >= f)) ? (1 << i) : 0;
        
        /* Nested comparisons */
        sum += ((t == u) && (e < f)) || ((t != u) && (e > f)) ? (16 << i) : 0;
    }
    
    return sum;
}

/* Test function 3: Vectorized comparisons using GCC vector extensions */
__attribute__((noinline))
static int test_vector_comparisons(v4sf va, v4sf vb, v2df vc, v2df vd) {
    int sum = 0;
    
    /* Element-wise unordered comparisons */
    v4sf mask_unord = __builtin_ia32_cmpunordps(va, vb);
    v4sf mask_ord = __builtin_ia32_cmpordps(va, vb);
    v4sf mask_neq = __builtin_ia32_cmpneqps(va, vb);
    v4sf mask_nlt = __builtin_ia32_cmpnltps(va, vb);
    v4sf mask_nle = __builtin_ia32_cmpnleps(va, vb);
    
    /* Extract results to integer */
    float m[4];
    memcpy(m, &mask_unord, sizeof(m));
    sum += (m[0] != 0.0f) ? 1 : 0;
    sum += (m[1] != 0.0f) ? 2 : 0;
    sum += (m[2] != 0.0f) ? 4 : 0;
    sum += (m[3] != 0.0f) ? 8 : 0;
    
    memcpy(m, &mask_ord, sizeof(m));
    sum += (m[0] != 0.0f) ? 16 : 0;
    sum += (m[1] != 0.0f) ? 32 : 0;
    
    /* Double precision vector comparisons */
    double md[2];
    v2df mask_d_unord = __builtin_ia32_cmpunordsd(vc, vd);
    memcpy(md, &mask_d_unord, sizeof(md));
    sum += (md[0] != 0.0) ? 64 : 0;
    sum += (md[1] != 0.0) ? 128 : 0;
    
    return sum;
}

/* Test function 4: Fast-math optimizations that generate UNEQ/LTGT */
__attribute__((noinline, optimize("fast-math")))
static int test_fast_math_comparisons(float a, float b, float c, float d) {
    int sum = 0;
    
    /* Under fast-math, these may generate UNEQ/LTGT condition codes */
    sum += (a == b) ? 1 : 0;        /* May become UNEQ with fast-math */
    sum += (a != b) ? 2 : 0;        /* May become LTGT with fast-math */
    sum += (a < b) ? 4 : 0;
    sum += (a > b) ? 8 : 0;
    sum += (a <= b) ? 16 : 0;
    sum += (a >= b) ? 32 : 0;
    
    /* Chain of comparisons */
    sum += ((a < b) && (c > d)) ? 64 : 0;
    sum += ((a == b) || (c == d)) ? 128 : 0;
    
    return sum;
}

/* Test function 5: NaN checks and explicit unordered tests */
__attribute__((noinline))
static int test_nan_checks(float a, double b, float c, double d) {
    int sum = 0;
    
    /* Explicit NaN checks using a != a idiom */
    sum += (a != a) ? 1 : 0;        /* UNORDERED check */
    sum += !(b == b) ? 2 : 0;       /* Another UNORDERED check */
    
    /* Mixed NaN and normal comparisons */
    sum += ((a != a) || (c == 0.0f)) ? 4 : 0;
    sum += ((b == b) && (d != 0.0)) ? 8 : 0;
    
    /* Complex expression with potential unordered results */
    sum += ((a < c) || (a != a) || (c != c)) ? 16 : 0;
    sum += ((b > d) && (b == b) && (d == d)) ? 32 : 0;
    
    return sum;
}

/* Main test driver */
int main(int argc, char *argv[]) {
    int total_sum = 0;
    
    /* Patterned data including NaN values */
    float float_data[] = {
        1.0f, 2.0f, 0.0f, -1.0f,
        __builtin_nanf(""), 3.0f, __builtin_nanf("quiet"), 4.0f,
        INFINITY, -INFINITY, 5.0f, 6.0f
    };
    
    double double_data[] = {
        1.0, 2.0, 0.0, -1.0,
        __builtin_nan(""), 3.0, __builtin_nan("quiet"), 4.0,
        INFINITY, -INFINITY, 5.0, 6.0
    };
    
    /* Use argc to prevent excessive loop unrolling */
    int iterations = (argc > 1) ? (argc & 3) + 1 : 4;
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Vary inputs each iteration to prevent constant folding */
        int idx = iter % 8;
        
        /* Test 1: Unordered comparisons */
        total_sum += test_unordered_comparisons(
            float_data[idx], float_data[idx + 1],
            double_data[idx], double_data[idx + 2]
        );
        
        /* Test 2: Mixed comparisons */
        total_sum += test_mixed_comparisons(
            float_data[idx], float_data[idx + 1], float_data[idx + 2],
            float_data[idx + 3], float_data[idx + 4], float_data[idx + 5]
        );
        
        /* Test 3: Vector comparisons */
        v4sf va = {float_data[idx], float_data[idx + 1], 
                   float_data[idx + 2], float_data[idx + 3]};
        v4sf vb = {float_data[idx + 4], float_data[idx + 5],
                   float_data[idx + 6], float_data[idx + 7]};
        v2df vc = {double_data[idx], double_data[idx + 1]};
        v2df vd = {double_data[idx + 2], double_data[idx + 3]};
        
        total_sum += test_vector_comparisons(va, vb, vc, vd);
        
        /* Test 4: Fast-math comparisons */
        total_sum += test_fast_math_comparisons(
            float_data[idx], float_data[idx + 1],
            float_data[idx + 2], float_data[idx + 3]
        );
        
        /* Test 5: NaN checks */
        total_sum += test_nan_checks(
            float_data[idx], double_data[idx],
            float_data[idx + 4], double_data[idx + 4]
        );
        
        /* Prevent dead code elimination */
        sink = total_sum;
    }
    
    printf("Total checksum: %d\n", total_sum);
    return total_sum != 0 ? 0 : 1;
}
