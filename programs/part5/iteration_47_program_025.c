#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Prevent optimization of critical functions */
#define NOINLINE __attribute__((noinline))
#define VOLATILE_MATH volatile

/* Vector types for AVX/SSE */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Test 1: Direct unordered comparisons using math.h macros */
NOINLINE static int test_unordered_comparisons(float a, float b, double c, double d) {
    int sum = 0;
    
    /* These should generate UNORDERED/ORDERED condition codes */
    sum += isunordered(a, b) ? 1 : 0;
    sum += isordered(a, b) ? 2 : 0;
    
    /* UNEQ - unordered or equal */
    sum += !isgreater(c, d) && !isless(c, d) ? 4 : 0;
    
    /* UNGE - unordered or greater or equal */
    sum += !isless(c, d) ? 8 : 0;
    
    /* UNGT - unordered or greater */
    sum += !islessequal(c, d) ? 16 : 0;
    
    /* UNLE - unordered or less or equal */
    sum += !isgreater(c, d) ? 32 : 0;
    
    /* UNLT - unordered or less */
    sum += !isgreaterequal(c, d) ? 64 : 0;
    
    /* LTGT - less or greater (ordered and not equal) */
    sum += islessgreater(a, b) ? 128 : 0;
    
    return sum;
}

/* Test 2: Mixed comparisons in conditional expressions */
NOINLINE static int test_mixed_conditionals(float a, float b, float c, float d, float e, float f) {
    int sum = 0;
    
    /* Complex conditional with different comparison types */
    for (int i = 0; i < 3; i++) {
        /* This should generate multiple condition codes */
        if ((a < b) ? (c != d) : (e >= f)) {
            sum += 1;
        }
        
        /* Nested ternary with unordered checks */
        sum += (a == a) ? (b != b ? 2 : (c > d ? 4 : 8)) : 16;
        
        /* Mixed ordered/unordered comparisons */
        if (!(a < b) && (c == c) && (d != d)) {
            sum += 32;
        }
    }
    
    return sum;
}

/* Test 3: Vectorized comparisons using GCC vector extensions */
NOINLINE static int test_vector_comparisons(v4sf va, v4sf vb, v2df vc, v2df vd) {
    int sum = 0;
    
    /* Element-wise unordered comparison */
    v4sf vcmp_unord = __builtin_ia32_cmpunordps(va, vb);
    int mask_unord = __builtin_ia32_movmskps(vcmp_unord);
    sum += mask_unord;
    
    /* Element-wise ordered comparison */
    v4sf vcmp_ord = __builtin_ia32_cmpordps(va, vb);
    int mask_ord = __builtin_ia32_movmskps(vcmp_ord);
    sum += mask_ord * 2;
    
    /* Not less than (UNGE) */
    v4sf vcmp_nlt = __builtin_ia32_cmpnltps(va, vb);
    int mask_nlt = __builtin_ia32_movmskps(vcmp_nlt);
    sum += mask_nlt * 4;
    
    /* Not less or equal (UNGT) */
    v4sf vcmp_nle = __builtin_ia32_cmpnleps(va, vb);
    int mask_nle = __builtin_ia32_movmskps(vcmp_nle);
    sum += mask_nle * 8;
    
    /* Unordered or less or equal (UNLE) */
    v4sf vcmp_ule = __builtin_ia32_cmpuleps(va, vb);
    int mask_ule = __builtin_ia32_movmskps(vcmp_ule);
    sum += mask_ule * 16;
    
    /* Unordered or less than (UNLT) */
    v4sf vcmp_ult = __builtin_ia32_cmpultps(va, vb);
    int mask_ult = __builtin_ia32_movmskps(vcmp_ult);
    sum += mask_ult * 32;
    
    /* Not equal (LTGT when used with fast-math) */
    v4sf vcmp_neq = __builtin_ia32_cmpneqps(va, vb);
    int mask_neq = __builtin_ia32_movmskps(vcmp_neq);
    sum += mask_neq * 64;
    
    return sum;
}

/* Test 4: Fast-math optimizations that generate UNEQ/LTGT */
NOINLINE static int test_fast_math_patterns(float a, float b, double c, double d) {
    int sum = 0;
    
    /* Under fast-math, these may generate UNEQ condition codes */
    if (a == b) {
        sum += 1;
    }
    
    /* This may generate LTGT (ordered and not equal) */
    if (a != b) {
        sum += 2;
    }
    
    /* Complex expression that fast-math can transform */
    if ((a > b) || (a <= b)) {
        sum += 4;
    }
    
    /* Check for NaN using volatile to prevent optimization */
    VOLATILE_MATH float volatile_a = a;
    if (volatile_a != volatile_a) {  /* Always true if a is NaN */
        sum += 8;
    }
    
    /* Ordered comparison that might be optimized */
    if (c < d || c >= d) {
        sum += 16;
    }
    
    return sum;
}

/* Test 5: Chain of comparisons in a function with multiple parameters */
NOINLINE static int test_comparison_chain(float a, float b, float c, float d, 
                                          float e, float f, float g, float h) {
    int result = 0;
    
    /* Chain of comparisons that should generate various condition codes */
    result |= (a < b) ? 0x01 : 0;
    result |= (c >= d) ? 0x02 : 0;
    result |= (e == f) ? 0x04 : 0;
    result |= (g != h) ? 0x08 : 0;
    result |= !(a > b) ? 0x10 : 0;
    result |= !(c <= d) ? 0x20 : 0;
    
    /* Mixed with unordered checks */
    if (isunordered(a, c) || isunordered(b, d)) {
        result |= 0x40;
    }
    
    /* LTGT pattern */
    if ((a < b) != (c > d)) {
        result |= 0x80;
    }
    
    return result;
}

int main(int argc, char *argv[]) {
    int total_sum = 0;
    
    /* Patterned data including NaN values */
    float float_data[] = {
        1.0f, 2.0f, 3.0f, 4.0f,
        0.0f, -0.0f, __builtin_nanf(""), 5.0f,
        INFINITY, -INFINITY, 6.0f, 7.0f
    };
    
    double double_data[] = {
        1.0, 2.0, __builtin_nan(""), 4.0,
        0.0, -0.0, 3.0, 5.0
    };
    
    /* Vector data */
    v4sf vec1 = {1.0f, 2.0f, __builtin_nanf(""), 4.0f};
    v4sf vec2 = {4.0f, 3.0f, 2.0f, 1.0f};
    v2df dvec1 = {1.0, __builtin_nan("")};
    v2df dvec2 = {__builtin_nan(""), 2.0};
    
    /* Use argc to prevent loop unrolling that might collapse comparisons */
    int iterations = (argc > 1) ? (argc % 5) + 1 : 3;
    
    for (int i = 0; i < iterations; i++) {
        /* Vary the inputs to prevent constant propagation */
        int idx = i % 8;
        
        /* Test 1: Unordered comparisons */
        total_sum += test_unordered_comparisons(
            float_data[idx], float_data[idx + 1],
            double_data[idx % 4], double_data[(idx + 1) % 4]
        );
        
        /* Test 2: Mixed conditionals */
        total_sum += test_mixed_conditionals(
            float_data[idx], float_data[idx + 1], float_data[idx + 2],
            float_data[idx + 3], float_data[(idx + 4) % 12], float_data[(idx + 5) % 12]
        );
        
        /* Test 3: Vector comparisons */
        total_sum += test_vector_comparisons(vec1, vec2, dvec1, dvec2);
        
        /* Test 4: Fast-math patterns */
        total_sum += test_fast_math_patterns(
            float_data[idx], float_data[(idx + 2) % 12],
            double_data[idx % 4], double_data[(idx + 2) % 4]
        );
        
        /* Test 5: Comparison chain */
        total_sum += test_comparison_chain(
            float_data[idx], float_data[idx + 1],
            float_data[idx + 2], float_data[idx + 3],
            float_data[(idx + 4) % 12], float_data[(idx + 5) % 12],
            float_data[(idx + 6) % 12], float_data[(idx + 7) % 12]
        );
    }
    
    /* Print result to prevent dead code elimination */
    printf("Total checksum: %d\n", total_sum);
    
    return total_sum != 0 ? 0 : 1;
}
