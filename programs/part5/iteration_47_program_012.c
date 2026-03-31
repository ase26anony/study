/* test_fp_conditions.c - Target i386.cc lines 13992-14017 */
#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <string.h>
#include <xmmintrin.h>
#include <immintrin.h>

/* Prevent optimization of critical functions */
#define NOINLINE __attribute__((noinline))
#define VOLATILE_MATH volatile

/* Vector types for SSE/AVX comparisons */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Test 1: Direct unordered comparisons using math.h macros */
NOINLINE static int test_unordered_comparisons(float a, float b, double c, double d) {
    int sum = 0;
    
    /* These should generate UNORDERED/ORDERED condition codes */
    sum += isunordered(a, b) ? 1 : 0;
    sum += isordered(a, b) ? 2 : 0;
    
    /* UNEQ - unordered or equal */
    sum += !(c < d) && !(c > d) ? 4 : 0;  /* Equivalent to !(c < d) && !(c > d) */
    
    /* LTGT - less than or greater than (ordered and not equal) */
    sum += islessgreater(c, d) ? 8 : 0;
    
    /* UNGE - unordered or greater than or equal */
    sum += !(c < d) ? 16 : 0;  /* Under fast-math: nlt */
    
    /* UNGT - unordered or greater than */
    sum += !(c <= d) ? 32 : 0; /* Under fast-math: nle */
    
    /* UNLE - unordered or less than or equal */
    sum += !(c > d) ? 64 : 0;  /* ule */
    
    /* UNLT - unordered or less than */
    sum += !(c >= d) ? 128 : 0; /* ult */
    
    return sum;
}

/* Test 2: Mixed comparisons in conditional expressions */
NOINLINE static int test_mixed_conditionals(float a, float b, float c, float d, float e, float f) {
    int sum = 0;
    
    /* Complex ternary with different comparison types */
    for (int i = 0; i < 4; i++) {
        VOLATILE_MATH float t = a + i;
        /* This should generate multiple condition codes */
        sum += ((t < b) ? (c != d) : (e >= f)) ? (1 << i) : 0;
        
        /* Nested conditionals with different operators */
        sum += ((t == b) ? (c < d) : (e > f)) ? (2 << i) : 0;
    }
    
    /* Chain of comparisons */
    sum += (a < b) && (b > c) && (c != d) && (d <= e) && (e >= f) ? 256 : 0;
    
    return sum;
}

/* Test 3: Vectorized comparisons using GCC vector extensions */
NOINLINE static int test_vector_comparisons(v4sf va, v4sf vb, v2df vc, v2df vd) {
    int sum = 0;
    
    /* Element-wise unordered comparisons */
    v4sf mask_unord = __builtin_ia32_cmpunordps(va, vb);
    v4sf mask_ord = __builtin_ia32_cmpordps(va, vb);
    v4sf mask_neq_uq = __builtin_ia32_cmpneqps(va, vb);  /* UNEQ */
    
    /* Extract results */
    float m1[4], m2[4], m3[4];
    memcpy(m1, &mask_unord, 16);
    memcpy(m2, &mask_ord, 16);
    memcpy(m3, &mask_neq_uq, 16);
    
    for (int i = 0; i < 4; i++) {
        sum += (m1[i] != 0.0f) ? 1 : 0;
        sum += (m2[i] != 0.0f) ? 2 : 0;
        sum += (m3[i] != 0.0f) ? 4 : 0;
    }
    
    /* Double precision vector comparisons */
    v2df mask_ltgt = __builtin_ia32_cmpneqpd(vc, vd);  /* LTGT when both are ordered */
    v2df mask_unlt = __builtin_ia32_cmpnltpd(vc, vd);  /* UNLT: !(vc < vd) */
    v2df mask_unge = __builtin_ia32_cmpnlepd(vc, vd);  /* UNGE: !(vc <= vd) */
    
    double dm1[2], dm2[2], dm3[2];
    memcpy(dm1, &mask_ltgt, 16);
    memcpy(dm2, &mask_unlt, 16);
    memcpy(dm3, &mask_unge, 16);
    
    for (int i = 0; i < 2; i++) {
        sum += (dm1[i] != 0.0) ? 8 : 0;
        sum += (dm2[i] != 0.0) ? 16 : 0;
        sum += (dm3[i] != 0.0) ? 32 : 0;
    }
    
    return sum;
}

/* Test 4: NaN checks and fast-math optimizations */
NOINLINE static int test_nan_checks(float a, double b, float c, double d) {
    int sum = 0;
    
    /* Explicit NaN checks - should generate unordered comparisons */
    sum += (a != a) ? 1 : 0;           /* true if a is NaN */
    sum += !(b == b) ? 2 : 0;          /* true if b is NaN */
    
    /* Mixed NaN and normal comparisons */
    sum += (a != a) || (c > 0.0f) ? 4 : 0;
    sum += (b == b) && (d < 1.0) ? 8 : 0;
    
    /* Fast-math style: assume no NaN but include guarded path */
    VOLATILE_MATH float volatile_f = a;
    if (volatile_f == volatile_f) {  /* Ordered check */
        /* Under fast-math, these may use UNEQ/LTGT */
        sum += (c != 0.0f) ? 16 : 0;
        sum += (c > 0.0f || c < 0.0f) ? 32 : 0;  /* LTGT when c != 0 */
    }
    
    return sum;
}

/* Test 5: Function with multiple FP parameters and comparison chain */
NOINLINE static int test_comparison_chain(float a, float b, float c, float d, 
                                          double e, double f, double g, double h) {
    int result = 0;
    
    /* Chain of comparisons that should generate various condition codes */
    int cmp1 = (a < b);      /* LT */
    int cmp2 = (c >= d);     /* GE */
    int cmp3 = (e == f);     /* EQ */
    int cmp4 = (g != h);     /* NEQ */
    int cmp5 = !(a > b);     /* UNLE under certain conditions */
    int cmp6 = !(e < f);     /* UNGE under certain conditions */
    
    /* Combine with bitwise operations to prevent short-circuiting */
    result = (cmp1 << 0) | (cmp2 << 1) | (cmp3 << 2) | 
             (cmp4 << 3) | (cmp5 << 4) | (cmp6 << 5);
    
    /* Additional unordered checks */
    result |= isunordered(a, c) ? (1 << 6) : 0;
    result |= islessgreater(e, g) ? (1 << 7) : 0;
    
    return result;
}

/* Main test driver */
int main(int argc, char *argv[]) {
    int total_sum = 0;
    
    /* Patterned data including normal numbers, zeros, and NaN */
    float float_data[] = {
        1.0f, 2.0f, 0.0f, -1.0f,
        __builtin_nanf(""), 3.0f, -0.0f, __builtin_nanf("")
    };
    
    double double_data[] = {
        1.0, 2.0, 0.0, -1.0,
        __builtin_nan(""), 3.0, -0.0, __builtin_nan("")
    };
    
    /* Vector data */
    v4sf v1 = {1.0f, 2.0f, __builtin_nanf(""), 0.0f};
    v4sf v2 = {2.0f, 1.0f, 0.0f, __builtin_nanf("")};
    v2df vd1 = {1.0, __builtin_nan("")};
    v2df vd2 = {__builtin_nan(""), 1.0};
    
    /* Use argc to prevent loop unrolling and dead code elimination */
    int iterations = (argc > 1) ? (argc & 7) + 1 : 4;
    
    for (int i = 0; i < iterations; i++) {
        int idx = i % 8;
        
        /* Test 1: Unordered comparisons */
        total_sum += test_unordered_comparisons(
            float_data[idx], 
            float_data[(idx + 1) % 8],
            double_data[idx],
            double_data[(idx + 2) % 8]
        );
        
        /* Test 2: Mixed conditionals */
        total_sum += test_mixed_conditionals(
            float_data[idx],
            float_data[(idx + 1) % 8],
            float_data[(idx + 2) % 8],
            float_data[(idx + 3) % 8],
            float_data[(idx + 4) % 8],
            float_data[(idx + 5) % 8]
        );
        
        /* Test 3: Vector comparisons */
        total_sum += test_vector_comparisons(v1, v2, vd1, vd2);
        
        /* Test 4: NaN checks */
        total_sum += test_nan_checks(
            float_data[idx],
            double_data[idx],
            float_data[(idx + 2) % 8],
            double_data[(idx + 2) % 8]
        );
        
        /* Test 5: Comparison chain */
        total_sum += test_comparison_chain(
            float_data[idx],
            float_data[(idx + 1) % 8],
            float_data[(idx + 2) % 8],
            float_data[(idx + 3) % 8],
            double_data[idx],
            double_data[(idx + 1) % 8],
            double_data[(idx + 2) % 8],
            double_data[(idx + 3) % 8]
        );
    }
    
    /* Print result to prevent dead code elimination */
    printf("Total checksum: %d\n", total_sum);
    
    return total_sum != 0 ? 0 : 1;
}
