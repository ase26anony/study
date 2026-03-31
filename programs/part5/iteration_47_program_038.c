/* 
 * Program to trigger x86 condition code mnemonics in i386.cc
 * Compile with: gcc -O3 -ffast-math -march=x86-64 -mavx2 -fdump-rtl-final
 */

#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <x86intrin.h>

/* Vector types for AVX/SSE comparisons */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Prevent optimization of critical functions */
#define NOINLINE __attribute__((noinline))
#define VOLATILE_MATH volatile

/* Test function 1: Direct unordered comparisons using math.h macros */
NOINLINE static int test_unordered_comparisons(float a, float b, double c, double d) {
    int sum = 0;
    
    /* These should generate UNORDERED/ORDERED condition codes */
    sum += isunordered(a, b) ? 1 : 0;
    sum += isordered(a, b) ? 2 : 0;
    
    /* UNEQ: unordered or equal */
    sum += !islessgreater(c, d) ? 4 : 0;  /* Equivalent to UNEQ */
    
    /* UNGE: unordered or greater-or-equal (nlt) */
    sum += !isless(c, d) ? 8 : 0;
    
    /* UNGT: unordered or greater (nle) */
    sum += !islessequal(c, d) ? 16 : 0;
    
    /* UNLE: unordered or less-or-equal */
    sum += islessequal(c, d) ? 32 : 0;
    
    /* UNLT: unordered or less */
    sum += isless(c, d) ? 64 : 0;
    
    /* LTGT: less or greater (une) */
    sum += islessgreater(a, b) ? 128 : 0;
    
    return sum;
}

/* Test function 2: Mixed conditional expressions */
NOINLINE static int test_mixed_conditionals(float a, float b, float c, float d, float e, float f) {
    int sum = 0;
    
    /* Complex ternary with different comparison types */
    for (int i = 0; i < 4; i++) {
        float t = (float)i;
        sum += ((a + t < b) ? (c != d) : (e >= f)) ? 1 : 0;
        sum += ((a == b) ? (c > d) : (e <= f)) ? 2 : 0;
        sum += ((a != b) ? (c < d) : (e == f)) ? 4 : 0;
    }
    
    /* Nested conditionals */
    sum += (a < b) ? ((c > d) ? 8 : 16) : ((e != f) ? 32 : 64);
    
    return sum;
}

/* Test function 3: Vectorized comparisons using GCC vector extensions */
NOINLINE static int test_vector_comparisons(v4sf va, v4sf vb, v2df vc, v2df vd) {
    int sum = 0;
    
    /* Vector unordered comparison */
    v4sf vcmp_unord = __builtin_ia32_cmpunordps(va, vb);
    sum += ((int*)&vcmp_unord)[0] != 0 ? 1 : 0;
    
    /* Vector ordered comparison */
    v4sf vcmp_ord = __builtin_ia32_cmpordps(va, vb);
    sum += ((int*)&vcmp_ord)[1] != 0 ? 2 : 0;
    
    /* Vector not-equal unordered (UNEQ pattern) */
    v2df vcmp_ueq = __builtin_ia32_cmpunordsd(vc, vd);
    sum += ((int64_t*)&vcmp_ueq)[0] != 0 ? 4 : 0;
    
    /* Vector not-less-than (UNGE: nlt) */
    v4sf vcmp_nlt = __builtin_ia32_cmpnltps(va, vb);
    sum += ((int*)&vcmp_nlt)[2] != 0 ? 8 : 0;
    
    /* Vector not-less-or-equal (UNGT: nle) */
    v4sf vcmp_nle = __builtin_ia32_cmpnleps(va, vb);
    sum += ((int*)&vcmp_nle)[3] != 0 ? 16 : 0;
    
    return sum;
}

/* Test function 4: NaN checks and fast-math optimizations */
NOINLINE static int test_nan_fastmath(float a, double b, float c, double d) {
    int sum = 0;
    
    /* Direct NaN checks - may generate unordered comparisons */
    sum += (a != a) ? 1 : 0;      /* true if a is NaN */
    sum += !(b == b) ? 2 : 0;     /* true if b is NaN */
    
    /* Under fast-math, these may use UNEQ/LTGT codes */
    sum += (c == c) ? 4 : 0;      /* false if c is NaN */
    sum += (d != d) ? 8 : 0;      /* true if d is NaN */
    
    /* Mixed with regular comparisons */
    VOLATILE_MATH float v1 = a;
    VOLATILE_MATH double v2 = b;
    
    if (v1 < v2 || v1 != v1) {
        sum += 16;
    }
    
    if (v2 >= v1 && v2 == v2) {
        sum += 32;
    }
    
    return sum;
}

/* Test function 5: Complex chain of floating comparisons */
NOINLINE static int test_comparison_chain(float a, float b, float c, 
                                          double d, double e, double f) {
    int result = 0;
    
    /* Chain of comparisons that might generate various condition codes */
    int cmp1 = (a < b) && !isunordered(a, b);
    int cmp2 = (c >= a) || isunordered(c, a);
    int cmp3 = (d == e) && !islessgreater(d, e);
    int cmp4 = (f != d) || islessgreater(f, d);
    
    /* Combine with arithmetic to prevent optimization */
    result = cmp1 * 1 + cmp2 * 2 + cmp3 * 4 + cmp4 * 8;
    
    /* Additional complex expression */
    result += ((a < b) ? (c > a) : (b != c)) ? 16 : 0;
    result += ((d == e) ? (f < d) : (e >= f)) ? 32 : 0;
    
    return result;
}

/* Main test driver */
int main(int argc, char **argv) {
    int total_sum = 0;
    
    /* Patterned data including normal numbers, zeros, and NaN */
    float float_data[] = {
        1.0f, 2.0f, 0.0f, -0.0f,
        3.14f, -2.71f, __builtin_nanf(""), 100.0f
    };
    
    double double_data[] = {
        1.0, 2.0, 0.0, -0.0,
        3.1415926535, -2.7182818284, __builtin_nan(""), 1000.0
    };
    
    /* Vector data */
    v4sf vec1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec2 = {4.0f, 3.0f, 2.0f, 1.0f};
    v2df dvec1 = {1.0, 2.0};
    v2df dvec2 = {2.0, 1.0};
    
    /* Use argc to prevent loop unrolling from collapsing comparisons */
    int iterations = (argc > 1) ? argc : 4;
    if (iterations > 100) iterations = 100; /* Safety limit */
    
    for (int i = 0; i < iterations; i++) {
        /* Cycle through different data patterns */
        int idx = i % 8;
        int didx = i % 8;
        
        /* Call all test functions with varying inputs */
        total_sum += test_unordered_comparisons(
            float_data[idx], 
            float_data[(idx + 1) % 8],
            double_data[didx],
            double_data[(didx + 2) % 8]
        );
        
        total_sum += test_mixed_conditionals(
            float_data[idx],
            float_data[(idx + 2) % 8],
            float_data[(idx + 3) % 8],
            float_data[(idx + 4) % 8],
            float_data[(idx + 5) % 8],
            float_data[(idx + 6) % 8]
        );
        
        /* Modify vectors slightly each iteration */
        vec1[0] += 0.1f * i;
        vec2[1] -= 0.1f * i;
        
        total_sum += test_vector_comparisons(vec1, vec2, dvec1, dvec2);
        
        total_sum += test_nan_fastmath(
            float_data[idx],
            double_data[didx],
            float_data[(idx + 3) % 8],
            double_data[(didx + 4) % 8]
        );
        
        total_sum += test_comparison_chain(
            float_data[idx],
            float_data[(idx + 1) % 8],
            float_data[(idx + 2) % 8],
            double_data[didx],
            double_data[(didx + 1) % 8],
            double_data[(didx + 2) % 8]
        );
    }
    
    /* Print result to prevent dead code elimination */
    printf("Total checksum: %d\n", total_sum);
    
    return total_sum != 0 ? 0 : 1;
}
