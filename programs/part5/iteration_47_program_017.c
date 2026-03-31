#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <immintrin.h>

/* Prevent optimization from removing critical comparisons */
static volatile int force_volatile = 0;

/* Vector types for AVX/SSE comparisons */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Test functions with different comparison patterns */

/* Function 1: Direct unordered comparisons using math.h macros */
__attribute__((noinline))
static int test_unordered_comparisons(float a, float b, double c, double d) {
    int sum = 0;
    
    /* These should generate UNORDERED/ORDERED condition codes */
    sum += isunordered(a, b) ? 1 : 0;
    sum += isordered(a, b) ? 2 : 0;
    
    /* UNEQ: unordered or equal */
    sum += !(c < d) && !(c > d) ? 4 : 0;  /* c == d or unordered */
    
    /* UNGE: not less than (greater or equal or unordered) */
    sum += !(c < d) ? 8 : 0;
    
    /* UNGT: not less or equal (greater or unordered) */
    sum += !(c <= d) ? 16 : 0;
    
    /* UNLE: less or equal or unordered */
    sum += !(c > d) ? 32 : 0;
    
    /* UNLT: less than or unordered */
    sum += !(c >= d) ? 64 : 0;
    
    /* LTGT: less than or greater than (ordered and not equal) */
    sum += islessgreater(c, d) ? 128 : 0;
    
    return sum;
}

/* Function 2: Mixed comparisons in conditional expressions */
__attribute__((noinline))
static int test_mixed_conditionals(float a, float b, float c, float d, float e, float f) {
    int sum = 0;
    
    /* Complex ternary with different comparison types */
    for (int i = 0; i < (force_volatile & 3); i++) {
        /* This should generate multiple condition codes */
        sum += (a < b) ? (c != d) : (e >= f);
        sum += (a == b) ? (c > d) : (e <= f);
        sum += (a != b) ? (c < d) : (e == f);
    }
    
    /* Chain of comparisons */
    if ((a < b) && (c > d) && (e != f)) {
        sum += 1;
    }
    
    /* Nested conditionals with different operators */
    sum += (a <= b) ? ((c >= d) ? 2 : 4) : ((e == f) ? 8 : 16);
    
    return sum;
}

/* Function 3: Vectorized comparisons using GCC vector extensions */
__attribute__((noinline))
static int test_vector_comparisons(v4sf va, v4sf vb, v2df vc, v2df vd) {
    int sum = 0;
    
    /* Element-wise unordered comparisons */
    v4sf cmp_unord = __builtin_ia32_cmpunordps(va, vb);
    v4sf cmp_ord = __builtin_ia32_cmpordps(va, vb);
    v4sf cmp_neq_uq = __builtin_ia32_cmpneqps(va, vb);  /* UNEQ */
    
    /* Extract results to prevent elimination */
    float res[4];
    memcpy(res, &cmp_unord, sizeof(res));
    sum += (res[0] != 0.0f) ? 1 : 0;
    sum += (res[1] != 0.0f) ? 2 : 0;
    
    memcpy(res, &cmp_ord, sizeof(res));
    sum += (res[2] != 0.0f) ? 4 : 0;
    
    memcpy(res, &cmp_neq_uq, sizeof(res));
    sum += (res[3] != 0.0f) ? 8 : 0;
    
    /* Double vector comparisons */
    v2df cmp_nlt = __builtin_ia32_cmpnltpd(vc, vd);  /* UNGE */
    v2df cmp_nle = __builtin_ia32_cmpnlepd(vc, vd);  /* UNGT */
    v2df cmp_ule = __builtin_ia32_cmpnltpd(vd, vc);  /* UNLE (swapped) */
    
    double dres[2];
    memcpy(dres, &cmp_nlt, sizeof(dres));
    sum += (dres[0] != 0.0) ? 16 : 0;
    
    memcpy(dres, &cmp_nle, sizeof(dres));
    sum += (dres[1] != 0.0) ? 32 : 0;
    
    memcpy(dres, &cmp_ule, sizeof(dres));
    sum += (dres[0] != 0.0) ? 64 : 0;
    
    return sum;
}

/* Function 4: Fast-math optimized comparisons */
__attribute__((noinline))
static int test_fast_math_comparisons(float a, float b, float c, float d) {
    int sum = 0;
    
    /* With -ffast-math, these may generate UNEQ/LTGT codes */
    sum += (a == b) ? 1 : 0;
    sum += (a != b) ? 2 : 0;
    sum += (a < b) ? 4 : 0;
    sum += (a > b) ? 8 : 0;
    sum += (a <= b) ? 16 : 0;
    sum += (a >= b) ? 32 : 0;
    
    /* Combined comparisons that might generate LTGT */
    if ((c < d) || (c > d)) {
        sum += 64;
    }
    
    /* Inverse that might generate UNEQ */
    if (!((c < d) || (c > d))) {
        sum += 128;
    }
    
    return sum;
}

/* Function 5: NaN checks and unordered detection */
__attribute__((noinline))
static int test_nan_checks(float a, double b) {
    int sum = 0;
    
    /* Direct NaN checks - should generate unordered comparisons */
    sum += (a != a) ? 1 : 0;      /* true if a is NaN */
    sum += !(b == b) ? 2 : 0;     /* true if b is NaN */
    
    /* Mixed NaN and normal comparisons */
    float zero = 0.0f;
    sum += (a < zero) ? 4 : 0;
    sum += (a > zero) ? 8 : 0;
    sum += (a == zero) ? 16 : 0;
    sum += (a != zero) ? 32 : 0;
    
    /* Check for infinity (may generate ordered comparisons) */
    sum += isinf(b) ? 64 : 0;
    
    return sum;
}

int main(int argc, char **argv) {
    int total_sum = 0;
    
    /* Patterned data including NaN values */
    float float_data[] = {
        1.0f, 2.0f, 3.0f, 4.0f,
        0.0f, -0.0f, __builtin_nanf(""), INFINITY,
        5.0f, 6.0f, 7.0f, 8.0f
    };
    
    double double_data[] = {
        1.0, 2.0, __builtin_nan(""), 4.0,
        0.0, -0.0, INFINITY, -INFINITY
    };
    
    /* Initialize vector data */
    v4sf vec1 = {1.0f, 2.0f, __builtin_nanf(""), 4.0f};
    v4sf vec2 = {4.0f, 3.0f, 2.0f, 1.0f};
    v2df dvec1 = {1.0, __builtin_nan("")};
    v2df dvec2 = {__builtin_nan(""), 1.0};
    
    /* Use argc to prevent loop unrolling from eliminating comparisons */
    int iterations = (argc > 1) ? (argc & 7) + 1 : 4;
    
    for (int i = 0; i < iterations; i++) {
        int idx = i % 8;
        int f_idx = i % 12;
        
        /* Call all test functions with different data patterns */
        total_sum += test_unordered_comparisons(
            float_data[f_idx],
            float_data[(f_idx + 1) % 12],
            double_data[idx],
            double_data[(idx + 1) % 8]
        );
        
        total_sum += test_mixed_conditionals(
            float_data[f_idx],
            float_data[(f_idx + 2) % 12],
            float_data[(f_idx + 3) % 12],
            float_data[(f_idx + 4) % 12],
            float_data[(f_idx + 5) % 12],
            float_data[(f_idx + 6) % 12]
        );
        
        total_sum += test_vector_comparisons(vec1, vec2, dvec1, dvec2);
        
        total_sum += test_fast_math_comparisons(
            float_data[f_idx],
            float_data[(f_idx + 7) % 12],
            float_data[(f_idx + 8) % 12],
            float_data[(f_idx + 9) % 12]
        );
        
        total_sum += test_nan_checks(
            float_data[f_idx],
            double_data[idx]
        );
        
        /* Modify vectors slightly each iteration */
        vec1[0] += 0.1f;
        vec2[1] -= 0.1f;
    }
    
    printf("Total checksum: %d\n", total_sum);
    return total_sum != 0 ? 0 : 1;
}
