/* 
 * Program to trigger x86 condition code mnemonics in i386.cc
 * Compile with: gcc -O3 -ffast-math -mavx2 -fdump-rtl-final -o test_conds test_conds.c
 */

#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <immintrin.h>

/* Vector types for SSE/AVX comparisons */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Prevent optimization of inputs */
static volatile float vf1, vf2;
static volatile double vd1, vd2;

/* Function 1: Direct unordered comparisons using math.h macros */
__attribute__((noinline))
static int test_unordered_comparisons(float a, float b, double c, double d) {
    int sum = 0;
    
    /* These should generate UNORDERED/ORDERED condition codes */
    sum += isunordered(a, b) ? 1 : 0;
    sum += isordered(a, b) ? 2 : 0;
    
    /* UNEQ: unordered or equal */
    sum += !(c > d) && !(c < d) ? 4 : 0;  /* Equivalent to UNEQ */
    
    /* UNGE: not less than (unordered or greater or equal) */
    sum += !(a < b) ? 8 : 0;
    
    /* UNGT: not less or equal (unordered or greater) */
    sum += !(a <= b) ? 16 : 0;
    
    /* UNLE: unordered or less or equal */
    sum += !(a > b) ? 32 : 0;
    
    /* UNLT: unordered or less than */
    sum += !(a >= b) ? 64 : 0;
    
    /* LTGT: less or greater (ordered and not equal) */
    sum += islessgreater(c, d) ? 128 : 0;
    
    return sum;
}

/* Function 2: Mixed comparisons in conditional expressions */
__attribute__((noinline))
static int test_mixed_conditionals(float a, float b, float c, float d, 
                                   double e, double f) {
    int sum = 0;
    
    /* Complex conditional mixing different comparisons */
    for (int i = 0; i < 3; i++) {
        /* Ternary with different FP comparisons in branches */
        sum += (a < b) ? (c != d) * 1 : (e >= f) * 2;
        
        /* Nested conditionals */
        if ((a == b) || (c != c)) {  /* c != c checks for NaN */
            sum += (e < f) ? 4 : 8;
        } else if (!(a <= b) && (d == d)) {
            sum += (e > f) ? 16 : 32;
        }
        
        /* Rotate values to create different comparison scenarios */
        float tmp = a;
        a = b; b = c; c = d; d = tmp;
    }
    
    return sum;
}

/* Function 3: Vectorized comparisons using GCC vector extensions */
__attribute__((noinline))
static int test_vector_comparisons(v4sf va, v4sf vb, v2df vc, v2df vd) {
    int sum = 0;
    
    /* Element-wise vector comparisons */
    v4sf cmp_unord = __builtin_ia32_cmpunordps(va, vb);
    v4sf cmp_ord = __builtin_ia32_cmpordps(va, vb);
    v4sf cmp_neq = __builtin_ia32_cmpneqps(va, vb);
    v4sf cmp_nlt = __builtin_ia32_cmpnltps(va, vb);
    v4sf cmp_nle = __builtin_ia32_cmpnleps(va, vb);
    
    /* Extract masks from comparisons */
    int mask_unord = __builtin_ia32_movmskps(cmp_unord);
    int mask_ord = __builtin_ia32_movmskps(cmp_ord);
    int mask_neq = __builtin_ia32_movmskps(cmp_neq);
    int mask_nlt = __builtin_ia32_movmskps(cmp_nlt);
    int mask_nle = __builtin_ia32_movmskps(cmp_nle);
    
    sum = mask_unord + mask_ord * 2 + mask_neq * 4 + 
          mask_nlt * 8 + mask_nle * 16;
    
    /* Double precision vector comparisons */
    v2df cmp_unord_d = __builtin_ia32_cmpunordsd(vc, vd);
    v2df cmp_neq_uq = __builtin_ia32_cmpneqsd(vc, vd);
    
    /* Use results to prevent elimination */
    sum += ((int)cmp_unord_d[0]) * 32;
    sum += ((int)cmp_neq_uq[0]) * 64;
    
    return sum;
}

/* Function 4: Fast-math optimized comparisons */
__attribute__((noinline))
static int test_fast_math_optimizations(float a, float b, float c, float d) {
    int sum = 0;
    
    /* With -ffast-math, these may generate UNEQ/LTGT codes */
    
    /* Chain of comparisons that fast-math can optimize */
    sum += (a == b) ? 1 : 0;
    sum += (a != b) ? 2 : 0;
    sum += (a < b) ? 4 : 0;
    sum += (a > b) ? 8 : 0;
    sum += (a <= b) ? 16 : 0;
    sum += (a >= b) ? 32 : 0;
    
    /* Combined comparisons that might generate LTGT */
    int cmp1 = (a < b) || (a > b);  /* LTGT when ordered */
    int cmp2 = (c == d) || (c != d); /* Always true, but compiler might not see */
    
    sum += cmp1 ? 64 : 0;
    sum += cmp2 ? 128 : 0;
    
    /* Check for NaN using volatile to prevent optimization */
    if (vf1 != vf1) {  /* Always false for non-NaN, but compiler must check */
        sum += 256;
    }
    
    return sum;
}

/* Function 5: NaN handling with ordered/unordered semantics */
__attribute__((noinline))
static int test_nan_handling(float a, float b, double c, double d) {
    int sum = 0;
    
    /* Generate NaN values */
    float nan_f = __builtin_nanf("");
    double nan_d = __builtin_nan("");
    
    /* Comparisons with NaN - should generate unordered codes */
    sum += (a == nan_f) ? 1 : 0;      /* false for NaN */
    sum += (a != nan_f) ? 2 : 0;      /* true for NaN */
    sum += (a < nan_f) ? 4 : 0;       /* false, unordered */
    sum += (a > nan_f) ? 8 : 0;       /* false, unordered */
    
    /* Ordered comparisons that fail with NaN */
    sum += (c <= d) ? 16 : 0;
    sum += (c >= d) ? 32 : 0;
    
    /* Explicit unordered check */
    sum += isunordered(c, nan_d) ? 64 : 0;
    sum += isordered(c, d) ? 128 : 0;
    
    return sum;
}

int main(int argc, char **argv) {
    int total_sum = 0;
    
    /* Initialize test data with pattern */
    float f_data[8];
    double d_data[8];
    
    for (int i = 0; i < 8; i++) {
        f_data[i] = (i * 1.5f) - 3.0f;
        d_data[i] = (i * 2.3) - 5.0;
    }
    
    /* Add some NaN values if not using -ffast-math */
    if (argc > 1) {
        f_data[2] = __builtin_nanf("");
        d_data[5] = __builtin_nan("");
    }
    
    /* Set volatile values */
    vf1 = f_data[0];
    vf2 = f_data[1];
    vd1 = d_data[0];
    vd2 = d_data[1];
    
    /* Loop with volatile counter to prevent unrolling */
    volatile int iterations = (argc > 2) ? atoi(argv[2]) : 5;
    if (iterations < 1) iterations = 1;
    if (iterations > 100) iterations = 100;
    
    for (volatile int iter = 0; iter < iterations; iter++) {
        int idx = iter % 6;
        
        /* Test 1: Unordered comparisons */
        total_sum += test_unordered_comparisons(
            f_data[idx], f_data[idx+1],
            d_data[idx], d_data[idx+1]
        );
        
        /* Test 2: Mixed conditionals */
        total_sum += test_mixed_conditionals(
            f_data[idx], f_data[(idx+1)%8], f_data[(idx+2)%8], f_data[(idx+3)%8],
            d_data[idx], d_data[(idx+1)%8]
        );
        
        /* Test 3: Vector comparisons */
        v4sf va = {f_data[0], f_data[1], f_data[2], f_data[3]};
        v4sf vb = {f_data[4], f_data[5], f_data[6], f_data[7]};
        v2df vc = {d_data[0], d_data[1]};
        v2df vd = {d_data[2], d_data[3]};
        
        total_sum += test_vector_comparisons(va, vb, vc, vd);
        
        /* Test 4: Fast-math optimizations */
        total_sum += test_fast_math_optimizations(
            f_data[(idx+4)%8], f_data[(idx+5)%8],
            f_data[(idx+6)%8], f_data[(idx+7)%8]
        );
        
        /* Test 5: NaN handling */
        total_sum += test_nan_handling(
            f_data[idx], f_data[(idx+2)%8],
            d_data[idx], d_data[(idx+2)%8]
        );
    }
    
    printf("Total checksum: %d\n", total_sum);
    
    /* Use result to prevent dead code elimination */
    if (total_sum == 0) {
        printf("Warning: All comparisons returned zero\n");
    }
    
    return total_sum != 0 ? 0 : 1;
}
