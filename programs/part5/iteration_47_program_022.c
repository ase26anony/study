/* 
 * Program to trigger x86 condition code mnemonics in i386.cc
 * Compile with: gcc -O3 -ffast-math -march=x86-64 -mavx2 -fdump-rtl-final
 */

#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <xmmintrin.h>
#include <immintrin.h>

/* Vector types for AVX operations */
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
    sum += !(c > d) && !(c < d) ? 4 : 0;  /* May generate UNEQ under fast-math */
    
    /* LTGT: less than or greater than (ordered and not equal) */
    sum += islessgreater(c, d) ? 8 : 0;
    
    return sum;
}

/* Function 2: Mixed ordered/unordered comparisons in complex expressions */
__attribute__((noinline))
static int test_mixed_comparisons(float a, float b, float c, float d, float e, float f) {
    int sum = 0;
    
    /* Complex conditional with different comparison types */
    /* May generate multiple distinct condition codes */
    sum += ((a < b) ? (c != d) : (e >= f)) ? 1 : 0;
    
    /* Nested comparisons that could use UNGE/UNGT/UNLE/UNLT */
    sum += !(a >= b) ? 2 : 0;  /* Could become UNLT (nlt) */
    sum += !(a > b) ? 4 : 0;   /* Could become UNLE (nle) */
    sum += !(a <= b) ? 8 : 0;  /* Could become UNGT (ngt) */
    sum += !(a < b) ? 16 : 0;  /* Could become UNGE (nge) */
    
    /* Direct unordered comparisons */
    sum += (a != a) || (b != b) ? 32 : 0;  /* NaN check */
    
    return sum;
}

/* Function 3: Vectorized comparisons using GCC vector extensions */
__attribute__((noinline))
static int test_vector_comparisons(v4sf va, v4sf vb, v2df vc, v2df vd) {
    int sum = 0;
    
    /* Element-wise vector comparisons */
    v4sf vcmp_unord = __builtin_ia32_cmpunordps(va, vb);  /* UNORDERED */
    v4sf vcmp_ord = __builtin_ia32_cmpordps(va, vb);      /* ORDERED */
    v4sf vcmp_neq_uq = __builtin_ia32_cmpneqps(va, vb);   /* UNEQ */
    v4sf vcmp_nlt = __builtin_ia32_cmpnltps(va, vb);      /* UNGE (nlt) */
    v4sf vcmp_nle = __builtin_ia32_cmpnleps(va, vb);      /* UNGT (nle) */
    
    /* Extract results to prevent optimization */
    float res[4];
    memcpy(res, &vcmp_unord, sizeof(res));
    sum += (res[0] != 0.0f) ? 1 : 0;
    
    memcpy(res, &vcmp_ord, sizeof(res));
    sum += (res[1] != 0.0f) ? 2 : 0;
    
    memcpy(res, &vcmp_neq_uq, sizeof(res));
    sum += (res[2] != 0.0f) ? 4 : 0;
    
    memcpy(res, &vcmp_nlt, sizeof(res));
    sum += (res[3] != 0.0f) ? 8 : 0;
    
    /* Double vector comparisons */
    v2df vcmp_une = __builtin_ia32_cmpunordsd(vc, vd);    /* UNORDERED for doubles */
    double dres[2];
    memcpy(dres, &vcmp_une, sizeof(dres));
    sum += (dres[0] != 0.0) ? 16 : 0;
    
    return sum;
}

/* Function 4: Chain of comparisons with multiple parameters */
__attribute__((noinline))
static int test_comparison_chain(float a, float b, float c, float d, 
                                 float e, float f, float g, float h) {
    int sum = 0;
    
    /* Chain that might generate LTGT (une) */
    sum += ((a < b) != (c > d)) ? 1 : 0;
    
    /* Mixed comparisons that could use UNLE/UNLT */
    sum += ((e <= f) && (g >= h)) ? 2 : 0;
    sum += ((e < f) || (g > h)) ? 4 : 0;
    
    /* Complex ternary with floating comparisons */
    float result = (a != b) ? ((c == d) ? e : f) : ((g < h) ? e : f);
    sum += (result > 0.0f) ? 8 : 0;
    
    /* Explicit NaN checks that generate unordered comparisons */
    sum += (a != a) ? 16 : 0;
    sum += !(b == b) ? 32 : 0;
    
    return sum;
}

/* Function 5: Fast-math optimized comparisons */
__attribute__((noinline))
static int test_fast_math_comparisons(double a, double b, double c, double d) {
    int sum = 0;
    
    /* Under -ffast-math, these may generate UNEQ/LTGT codes */
    sum += (a == b) ? 1 : 0;      /* May become UNEQ */
    sum += (a != b) ? 2 : 0;      /* May become LTGT (une) */
    
    /* Ordered comparisons that fast-math might transform */
    sum += (a < b) ? 4 : 0;
    sum += (a > b) ? 8 : 0;
    sum += (a <= b) ? 16 : 0;
    sum += (a >= b) ? 32 : 0;
    
    /* Combined expression */
    sum += ((a < b) && (c > d)) ? 64 : 0;
    sum += ((a == b) || (c == d)) ? 128 : 0;
    
    return sum;
}

int main(int argc, char **argv) {
    /* Use argc to prevent excessive loop unrolling */
    int iterations = (argc > 1) ? (argc % 10) + 1 : 5;
    int total_sum = 0;
    
    /* Initialize test data with pattern including NaN */
    float float_data[16];
    double double_data[16];
    
    for (int i = 0; i < 16; i++) {
        float_data[i] = (i % 3 == 0) ? (float)i : 
                       (i % 3 == 1) ? -(float)i : 
                       __builtin_nanf("");
        
        double_data[i] = (i % 4 == 0) ? (double)i * 1.5 : 
                        (i % 4 == 1) ? -(double)i * 1.5 :
                        (i % 4 == 2) ? 0.0 : 
                        __builtin_nan("");
    }
    
    /* Main test loop - prevents dead code elimination */
    for (int iter = 0; iter < iterations; iter++) {
        volatile int idx = iter;  /* Volatile to prevent optimization */
        
        /* Test 1: Unordered comparisons */
        total_sum += test_unordered_comparisons(
            float_data[idx % 16],
            float_data[(idx + 1) % 16],
            double_data[idx % 16],
            double_data[(idx + 2) % 16]
        );
        
        /* Test 2: Mixed comparisons */
        total_sum += test_mixed_comparisons(
            float_data[(idx + 1) % 16],
            float_data[(idx + 2) % 16],
            float_data[(idx + 3) % 16],
            float_data[(idx + 4) % 16],
            float_data[(idx + 5) % 16],
            float_data[(idx + 6) % 16]
        );
        
        /* Test 3: Vector comparisons */
        v4sf va = {float_data[0], float_data[1], float_data[2], float_data[3]};
        v4sf vb = {float_data[4], float_data[5], float_data[6], float_data[7]};
        v2df vc = {double_data[0], double_data[1]};
        v2df vd = {double_data[2], double_data[3]};
        total_sum += test_vector_comparisons(va, vb, vc, vd);
        
        /* Test 4: Comparison chain */
        total_sum += test_comparison_chain(
            float_data[(idx + 0) % 16],
            float_data[(idx + 1) % 16],
            float_data[(idx + 2) % 16],
            float_data[(idx + 3) % 16],
            float_data[(idx + 4) % 16],
            float_data[(idx + 5) % 16],
            float_data[(idx + 6) % 16],
            float_data[(idx + 7) % 16]
        );
        
        /* Test 5: Fast-math comparisons */
        total_sum += test_fast_math_comparisons(
            double_data[(idx + 0) % 16],
            double_data[(idx + 1) % 16],
            double_data[(idx + 2) % 16],
            double_data[(idx + 3) % 16]
        );
    }
    
    /* Print result to ensure observable effect */
    printf("Total checksum: %d\n", total_sum);
    
    return total_sum != 0 ? 0 : 1;
}
