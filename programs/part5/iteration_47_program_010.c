/* test_fp_conditions.c - Trigger x86 condition code mnemonics generation */
#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <string.h>
#include <xmmintrin.h>
#include <immintrin.h>

/* Prevent optimization from removing critical comparisons */
static volatile int vol_counter = 0;

/* Vector types for AVX/SSE comparisons */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Test function 1: Direct unordered comparisons using math.h macros */
__attribute__((noinline))
static int test_unordered_comparisons(float a, float b, double c, double d) {
    int sum = 0;
    
    /* These should generate UNORDERED/ORDERED condition codes */
    sum += isunordered(a, b) ? 1 : 0;
    sum += isordered(a, b) ? 2 : 0;
    
    /* UNEQ (unordered or equal) */
    sum += !(c < d) && !(c > d) ? 4 : 0;  /* Equivalent to !(c != c) && c == d */
    
    /* UNGE (not less than) - unordered or greater or equal */
    sum += !(c < d) ? 8 : 0;
    
    /* UNGT (not less or equal) - unordered or greater */
    sum += !(c <= d) ? 16 : 0;
    
    /* UNLE (unordered or less or equal) */
    sum += !(c > d) ? 32 : 0;
    
    /* UNLT (unordered or less than) */
    sum += !(c >= d) ? 64 : 0;
    
    /* LTGT (less or greater) - ordered and not equal */
    sum += islessgreater(c, d) ? 128 : 0;
    
    return sum;
}

/* Test function 2: Mixed comparisons in conditional expressions */
__attribute__((noinline))
static int test_mixed_conditionals(float a, float b, float c, float d, float e, float f) {
    int sum = 0;
    
    /* Complex ternary with different comparison types */
    for (int i = 0; i < 3; i++) {
        /* This should generate multiple condition codes */
        sum += ((a < b) ? (c != d) : (e >= f)) ? (1 << i) : 0;
        
        /* Another mixed conditional */
        sum += ((a == b) ? (c > d) : (e <= f)) ? (2 << i) : 0;
        
        /* Nested conditionals */
        sum += ((a != a) /* NaN check */ ? (b < c) : (d > e)) ? (4 << i) : 0;
    }
    
    return sum;
}

/* Test function 3: Vectorized comparisons using GCC vector extensions */
__attribute__((noinline))
static int test_vector_comparisons(v4sf va, v4sf vb, v2df vc, v2df vd) {
    int sum = 0;
    
    /* Vector comparisons generate UNORD/ORD/etc condition codes */
    v4sf vcmp_unord = __builtin_ia32_cmpunordps(va, vb);
    v4sf vcmp_ord = __builtin_ia32_cmpordps(va, vb);
    v4sf vcmp_neq_uq = __builtin_ia32_cmpneqps(va, vb);  /* UNEQ */
    v4sf vcmp_nlt = __builtin_ia32_cmpnltps(va, vb);     /* UNGE */
    v4sf vcmp_nle = __builtin_ia32_cmpnleps(va, vb);     /* UNGT */
    
    /* Extract results to prevent elimination */
    float res[4];
    memcpy(res, &vcmp_unord, sizeof(res));
    sum += (res[0] != 0.0f) ? 1 : 0;
    
    memcpy(res, &vcmp_ord, sizeof(res));
    sum += (res[1] != 0.0f) ? 2 : 0;
    
    memcpy(res, &vcmp_neq_uq, sizeof(res));
    sum += (res[2] != 0.0f) ? 4 : 0;
    
    memcpy(res, &vcmp_nlt, sizeof(res));
    sum += (res[3] != 0.0f) ? 8 : 0;
    
    /* Double precision vector comparisons */
    v2df vcmp_unord_pd = __builtin_ia32_cmpunordpd(vc, vd);
    v2df vcmp_ltgt = __builtin_ia32_cmpneqpd(vc, vd);    /* LTGT */
    
    double dres[2];
    memcpy(dres, &vcmp_unord_pd, sizeof(dres));
    sum += (dres[0] != 0.0) ? 16 : 0;
    
    memcpy(dres, &vcmp_ltgt, sizeof(dres));
    sum += (dres[1] != 0.0) ? 32 : 0;
    
    return sum;
}

/* Test function 4: Fast-math optimized comparisons */
__attribute__((noinline))
static int test_fast_math_optimizations(float a, float b, double c, double d) {
    int sum = 0;
    
    /* With -ffast-math, these may generate UNEQ/LTGT codes */
    sum += (a == b) ? 1 : 0;      /* May become UNEQ */
    sum += (a != b) ? 2 : 0;      /* May become LTGT */
    sum += (c >= d) ? 4 : 0;      /* May become UNLT inverse */
    sum += (c <= d) ? 8 : 0;      /* May become UNGT inverse */
    
    /* Chain of comparisons */
    if ((a < b) && (c > d)) sum += 16;
    if ((a > b) || (c < d)) sum += 32;
    
    /* NaN checks that survive fast-math */
    volatile float v = a;
    if (v != v) sum += 64;        /* NaN check */
    
    return sum;
}

/* Test function 5: Complex floating-point decision logic */
__attribute__((noinline))
static int test_decision_logic(float f1, float f2, float f3, 
                               double d1, double d2, double d3) {
    int result = 0;
    
    /* Decision tree with multiple comparison types */
    if (isunordered(f1, f2)) {
        result |= 1;
        if (d1 != d2) result |= 2;
    } else if (islessgreater(f1, f2)) {
        result |= 4;
        if (!(d1 < d3)) result |= 8;      /* UNGE */
    } else if (!(f1 > f3)) {              /* UNLE */
        result |= 16;
        if (!(d2 >= d3)) result |= 32;    /* UNLT */
    }
    
    /* Switch-like behavior based on comparisons */
    int choice = 0;
    choice += (f1 == f2) ? 0 : 1;
    choice += (d1 < d2) ? 0 : 2;
    choice += (f3 != f3) ? 0 : 4;  /* NaN check */
    
    switch (choice) {
        case 0: result |= 64; break;
        case 1: result |= 128; break;
        case 2: result |= 256; break;
        case 3: result |= 512; break;
        case 4: result |= 1024; break;
        case 5: result |= 2048; break;
        case 6: result |= 4096; break;
        case 7: result |= 8192; break;
    }
    
    return result;
}

int main(int argc, char *argv[]) {
    int total_sum = 0;
    
    /* Patterned data including NaN values */
    float float_data[] = {
        1.0f, 2.0f, 3.0f, 4.0f,
        0.0f, -0.0f, __builtin_nanf(""), INFINITY,
        5.0f, 6.0f, 7.0f, 8.0f
    };
    
    double double_data[] = {
        1.0, 2.0, 3.0, 4.0,
        0.0, -0.0, __builtin_nan(""), __builtin_inf(),
        5.0, 6.0, 7.0, 8.0
    };
    
    /* Vector data */
    v4sf vec1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec2 = {4.0f, 3.0f, 2.0f, 1.0f};
    v2df dvec1 = {1.0, 2.0};
    v2df dvec2 = {2.0, 1.0};
    
    /* Use argc to prevent loop unrolling from eliminating comparisons */
    int iterations = (argc > 1) ? (argc % 8) + 1 : 4;
    
    for (int i = 0; i < iterations; i++) {
        vol_counter = i;  /* Volatile to prevent optimization */
        
        /* Test 1: Unordered comparisons */
        int idx = i % 12;
        total_sum += test_unordered_comparisons(
            float_data[idx], 
            float_data[(idx + 1) % 12],
            double_data[idx],
            double_data[(idx + 2) % 12]
        );
        
        /* Test 2: Mixed conditionals */
        total_sum += test_mixed_conditionals(
            float_data[(idx + 0) % 12],
            float_data[(idx + 1) % 12],
            float_data[(idx + 2) % 12],
            float_data[(idx + 3) % 12],
            float_data[(idx + 4) % 12],
            float_data[(idx + 5) % 12]
        );
        
        /* Test 3: Vector comparisons */
        total_sum += test_vector_comparisons(vec1, vec2, dvec1, dvec2);
        
        /* Test 4: Fast-math optimizations */
        total_sum += test_fast_math_optimizations(
            float_data[(idx + 6) % 12],
            float_data[(idx + 7) % 12],
            double_data[(idx + 6) % 12],
            double_data[(idx + 7) % 12]
        );
        
        /* Test 5: Decision logic */
        total_sum += test_decision_logic(
            float_data[(idx + 0) % 12],
            float_data[(idx + 3) % 12],
            float_data[(idx + 6) % 12],
            double_data[(idx + 1) % 12],
            double_data[(idx + 4) % 12],
            double_data[(idx + 7) % 12]
        );
    }
    
    printf("Total checksum: %d\n", total_sum);
    return total_sum != 0 ? 0 : 1;
}
