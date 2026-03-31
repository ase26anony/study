/* test_condition_codes.c - Program to trigger x86 floating-point condition code generation */
#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <string.h>
#include <xmmintrin.h>
#include <immintrin.h>

/* Prevent optimization of critical functions */
#define NOINLINE __attribute__((noinline))
#define VOLATILE_MATH volatile

/* Vector types for SSE/AVX operations */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Test function 1: Direct unordered comparisons using math.h macros */
NOINLINE static int test_unordered_comparisons(float a, float b, double c, double d) {
    int sum = 0;
    
    /* These should generate UNORDERED/ORDERED condition codes */
    sum += isunordered(a, b) ? 1 : 0;
    sum += isordered(a, b) ? 2 : 0;
    
    /* UNEQ: unordered or equal */
    sum += !(c > d) && !(c < d) ? 4 : 0;  /* Equivalent to UNEQ under fast-math */
    
    /* UNGE: not less than (unordered or greater or equal) */
    sum += !(c < d) ? 8 : 0;
    
    /* UNGT: not less or equal (unordered or greater) */
    sum += !(c <= d) ? 16 : 0;
    
    /* UNLE: unordered or less or equal */
    sum += !(c > d) ? 32 : 0;
    
    /* UNLT: unordered or less than */
    sum += !(c >= d) ? 64 : 0;
    
    /* LTGT: less or greater (ordered and not equal) */
    sum += islessgreater(c, d) ? 128 : 0;
    
    return sum;
}

/* Test function 2: Mixed comparisons in conditional expressions */
NOINLINE static int test_mixed_conditionals(float a, float b, float c, float d, float e, float f) {
    int sum = 0;
    
    /* Complex conditional mixing different comparison types */
    for (int i = 0; i < 3; i++) {
        /* This ternary mixes <, >=, and != comparisons */
        sum += ((a < b) ? (c != d) : (e >= f)) ? (1 << i) : 0;
        
        /* Another mixed conditional */
        sum += ((a == b) ? (c > d) : (e <= f)) ? (2 << i) : 0;
        
        /* Chain of comparisons */
        sum += (a < b && c > d) || (e != f) ? (4 << i) : 0;
        
        /* Rotate values to create variation */
        float temp = a;
        a = b; b = c; c = d; d = e; e = f; f = temp;
    }
    
    return sum;
}

/* Test function 3: Vectorized comparisons using GCC vector extensions */
NOINLINE static int test_vector_comparisons(v4sf va, v4sf vb, v2df vc, v2df vd) {
    int sum = 0;
    
    /* Element-wise unordered comparison */
    v4sf vcmp_unord = __builtin_ia32_cmpunordps(va, vb);
    int mask_unord = __builtin_ia32_movmskps(vcmp_unord);
    sum += mask_unord * 1;
    
    /* Element-wise ordered comparison */
    v4sf vcmp_ord = __builtin_ia32_cmpordps(va, vb);
    int mask_ord = __builtin_ia32_movmskps(vcmp_ord);
    sum += mask_ord * 2;
    
    /* Not equal (unordered or not equal) */
    v4sf vcmp_neq = __builtin_ia32_cmpneqps(va, vb);
    int mask_neq = __builtin_ia32_movmskps(vcmp_neq);
    sum += mask_neq * 4;
    
    /* Not less than (unordered or greater or equal) */
    v4sf vcmp_nlt = __builtin_ia32_cmpnltps(va, vb);
    int mask_nlt = __builtin_ia32_movmskps(vcmp_nlt);
    sum += mask_nlt * 8;
    
    /* Double precision vector comparisons */
    v2df vcmp_unord_pd = __builtin_ia32_cmpunordpd(vc, vd);
    int mask_unord_pd = __builtin_ia32_movmskpd(vcmp_unord_pd);
    sum += mask_unord_pd * 16;
    
    return sum;
}

/* Test function 4: Fast-math optimized comparisons */
NOINLINE static int test_fast_math_optimizations(float a, float b, double c, double d) {
    int sum = 0;
    
    /* Under -ffast-math, these may generate UNEQ/LTGT codes */
    
    /* UNEQ: (a == b) || (a != a) || (b != b) */
    /* Compiler may optimize to unordered equal under fast-math */
    if (!(a > b) && !(a < b)) {
        sum += 1;
    }
    
    /* LTGT: (a < b) || (a > b) but both ordered */
    /* Compiler may optimize to ordered not-equal under fast-math */
    if (a < b || a > b) {
        sum += 2;
    }
    
    /* Complex expression that might generate multiple condition codes */
    sum += ((c == d) ? 4 : 0) + ((c != d) ? 8 : 0);
    sum += ((c < d) ? 16 : 0) + ((c > d) ? 32 : 0);
    sum += ((c <= d) ? 64 : 0) + ((c >= d) ? 128 : 0);
    
    return sum;
}

/* Test function 5: NaN checks and unordered semantics */
NOINLINE static int test_nan_handling(float a, float b, double c, double d) {
    int sum = 0;
    
    /* Explicit NaN checks using a != a idiom */
    sum += (a != a) ? 1 : 0;      /* true if a is NaN */
    sum += (b == b) ? 2 : 0;      /* false if b is NaN */
    sum += !(c == c) ? 4 : 0;     /* true if c is NaN */
    sum += !(d != d) ? 8 : 0;     /* false if d is NaN */
    
    /* Mixed NaN and normal comparisons */
    sum += (a < b) ? 16 : 0;
    sum += (a > b) ? 32 : 0;
    sum += (a <= b) ? 64 : 0;
    sum += (a >= b) ? 128 : 0;
    
    /* Unordered comparison with potential NaN */
    sum += isunordered(c, d) ? 256 : 0;
    sum += isordered(c, d) ? 512 : 0;
    
    return sum;
}

/* Main test driver */
int main(int argc, char *argv[]) {
    int total_sum = 0;
    
    /* Patterned data including normal numbers, zeros, and NaN */
    float float_data[] = {
        1.0f, 2.0f, 0.0f, -0.0f,
        3.14f, -2.71f, __builtin_nanf(""), 100.0f,
        __builtin_inff(), -__builtin_inff(), 1.0e-10f, 1.0e10f
    };
    
    double double_data[] = {
        1.0, 2.0, 0.0, -0.0,
        3.141592653589793, -2.718281828459045,
        __builtin_nan(""), 100.0,
        __builtin_inf(), -__builtin_inf(), 1.0e-100, 1.0e100
    };
    
    /* Vector data */
    v4sf vec1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec2 = {4.0f, 3.0f, 2.0f, 1.0f};
    v4sf vec_nan = {__builtin_nanf(""), 1.0f, __builtin_nanf(""), 2.0f};
    
    v2df dvec1 = {1.0, 2.0};
    v2df dvec2 = {2.0, 1.0};
    v2df dvec_nan = {__builtin_nan(""), 3.0};
    
    /* Use argc to prevent excessive loop unrolling */
    int iterations = (argc > 1) ? (argc % 8) + 1 : 4;
    
    for (int i = 0; i < iterations; i++) {
        /* Cycle through different data combinations */
        int idx = i % 12;
        int idx2 = (i + 3) % 12;
        int idx3 = (i + 6) % 12;
        int idx4 = (i + 9) % 12;
        
        /* Test 1: Unordered comparisons */
        total_sum += test_unordered_comparisons(
            float_data[idx], float_data[idx2],
            double_data[idx3], double_data[idx4]
        );
        
        /* Test 2: Mixed conditionals */
        total_sum += test_mixed_conditionals(
            float_data[idx], float_data[idx2],
            float_data[idx3], float_data[idx4],
            float_data[(idx + 1) % 12], float_data[(idx + 2) % 12]
        );
        
        /* Test 3: Vector comparisons (alternate between normal and NaN vectors) */
        if (i % 2 == 0) {
            total_sum += test_vector_comparisons(vec1, vec2, dvec1, dvec2);
        } else {
            total_sum += test_vector_comparisons(vec_nan, vec2, dvec_nan, dvec2);
        }
        
        /* Test 4: Fast-math optimizations */
        total_sum += test_fast_math_optimizations(
            float_data[idx], float_data[idx2],
            double_data[idx3], double_data[idx4]
        );
        
        /* Test 5: NaN handling */
        total_sum += test_nan_handling(
            float_data[idx], float_data[idx2],
            double_data[idx3], double_data[idx4]
        );
    }
    
    /* Print result to prevent dead code elimination */
    printf("Total checksum: %d\n", total_sum);
    
    return total_sum != 0 ? 0 : 1;
}
