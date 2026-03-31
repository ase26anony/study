/* 
 * Program to trigger x86 condition code mnemonics in i386.cc
 * Compile with: gcc -O3 -ffast-math -march=x86-64 -mavx2 -fdump-rtl-final
 */

#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <x86intrin.h>

/* Vector types for AVX comparisons */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Prevent optimization */
static volatile int sink;

/* Function 1: Direct unordered comparisons using math.h macros */
__attribute__((noinline))
static int test_unordered_comparisons(float a, float b, double c, double d) {
    int sum = 0;
    
    /* These should generate UNORDERED/ORDERED condition codes */
    sum += isunordered(a, b) ? 1 : 0;
    sum += isordered(a, b) ? 2 : 0;
    
    /* LTGT (unordered not equal) */
    sum += islessgreater(c, d) ? 4 : 0;
    
    /* UNEQ (unordered or equal) - not a standard macro, but can be synthesized */
    sum += (!islessgreater(a, b) || isunordered(a, b)) ? 8 : 0;
    
    return sum;
}

/* Function 2: Mixed comparisons in conditional expressions */
__attribute__((noinline))
static int test_mixed_comparisons(float a, float b, float c, float d, float e, float f) {
    int sum = 0;
    
    /* Complex conditional with different comparison types */
    sum += ((a < b) ? (c != d) : (e >= f)) ? 1 : 0;
    
    /* Nested ternary with unordered checks */
    sum += (a == a) ? ((b != b) ? 2 : ((c < d) ? 4 : 8)) : 16;
    
    /* Chain of comparisons that might generate UNGE/UNLE etc. */
    sum += (!(a >= b) || (a != a)) ? 32 : 0;  /* Could generate UNGE (nlt) */
    sum += (!(a <= b) || (b != b)) ? 64 : 0;  /* Could generate UNGT (nle) */
    
    return sum;
}

/* Function 3: Vectorized comparisons using GCC vector extensions */
__attribute__((noinline))
static int test_vector_comparisons(v4sf va, v4sf vb, v2df vc, v2df vd) {
    int sum = 0;
    
    /* Element-wise unordered comparison */
    v4sf mask_unord = __builtin_ia32_cmpunordps(va, vb);
    sum += ((int)mask_unord[0]) & 1;
    
    /* Element-wise ordered comparison */
    v4sf mask_ord = __builtin_ia32_cmpordps(va, vb);
    sum += ((int)mask_ord[1]) & 2;
    
    /* Not equal (unordered) - could generate UNEQ */
    v4sf mask_neq = va != vb;
    sum += ((int)mask_neq[2]) & 4;
    
    /* Less than or greater than (LTGT) */
    v4sf mask_ltgt = (va < vb) | (va > vb);
    sum += ((int)mask_ltgt[3]) & 8;
    
    return sum;
}

/* Function 4: Fast-math optimized comparisons */
__attribute__((noinline))
static int test_fastmath_comparisons(float a, float b, double c, double d) {
    int sum = 0;
    
    /* With -ffast-math, these might use UNEQ/LTGT condition codes */
    sum += (a == b) ? 1 : 0;      /* Could become UNEQ */
    sum += (a != b) ? 2 : 0;      /* Could become LTGT */
    
    /* Inequalities that might use unordered variants */
    sum += (a >= b) ? 4 : 0;      /* Could become UNLT (nge) */
    sum += (a <= b) ? 8 : 0;      /* Could become UNGT (nle) */
    sum += (a > b) ? 16 : 0;      /* Could become UNLE (ngt) */
    sum += (a < b) ? 32 : 0;      /* Could become UNGE (nlt) */
    
    return sum;
}

/* Function 5: NaN checks that force unordered comparisons */
__attribute__((noinline))
static int test_nan_checks(float a, float b, double c, double d) {
    int sum = 0;
    
    /* Direct NaN checks - always unordered */
    sum += (a != a) ? 1 : 0;      /* true if a is NaN */
    sum += (b == b) ? 2 : 0;      /* false if b is NaN */
    sum += !(c == c) ? 4 : 0;     /* true if c is NaN */
    sum += !(d != d) ? 8 : 0;     /* false if d is NaN */
    
    /* Combined NaN and value checks */
    sum += ((a != a) || (a > b)) ? 16 : 0;    /* UNGT? */
    sum += ((b != b) || (a <= b)) ? 32 : 0;   /* UNLE? */
    
    return sum;
}

/* Main test driver */
int main(int argc, char **argv) {
    /* Patterned data including NaN values */
    float fdata[16];
    double ddata[16];
    
    /* Initialize with pattern: normals, zeros, NaN */
    for (int i = 0; i < 16; i++) {
        fdata[i] = (i % 3 == 0) ? __builtin_nanf("") : 
                   (i % 3 == 1) ? 0.0f : (float)i;
        ddata[i] = (i % 4 == 0) ? __builtin_nan("") : 
                   (i % 4 == 1) ? -0.0 : (double)i * 1.5;
    }
    
    int total_sum = 0;
    
    /* Use argc to prevent loop unrolling from collapsing comparisons */
    int iterations = (argc > 1) ? argc : 10;
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Vary inputs slightly each iteration */
        int idx = iter % 16;
        
        /* Test 1: Unordered comparisons */
        total_sum += test_unordered_comparisons(
            fdata[idx], fdata[(idx + 1) % 16],
            ddata[idx], ddata[(idx + 2) % 16]
        );
        
        /* Test 2: Mixed comparisons */
        total_sum += test_mixed_comparisons(
            fdata[idx], fdata[(idx + 3) % 16],
            fdata[(idx + 1) % 16], fdata[(idx + 4) % 16],
            fdata[(idx + 2) % 16], fdata[(idx + 5) % 16]
        );
        
        /* Test 3: Vector comparisons */
        v4sf va = {fdata[idx], fdata[(idx + 1) % 16], 
                   fdata[(idx + 2) % 16], fdata[(idx + 3) % 16]};
        v4sf vb = {fdata[(idx + 4) % 16], fdata[(idx + 5) % 16],
                   fdata[(idx + 6) % 16], fdata[(idx + 7) % 16]};
        v2df vc = {ddata[idx], ddata[(idx + 1) % 16]};
        v2df vd = {ddata[(idx + 2) % 16], ddata[(idx + 3) % 16]};
        
        total_sum += test_vector_comparisons(va, vb, vc, vd);
        
        /* Test 4: Fast-math comparisons */
        total_sum += test_fastmath_comparisons(
            fdata[(idx + 8) % 16], fdata[(idx + 9) % 16],
            ddata[(idx + 4) % 16], ddata[(idx + 5) % 16]
        );
        
        /* Test 5: NaN checks */
        total_sum += test_nan_checks(
            fdata[(idx + 10) % 16], fdata[(idx + 11) % 16],
            ddata[(idx + 6) % 16], ddata[(idx + 7) % 16]
        );
        
        /* Prevent dead code elimination */
        sink = total_sum;
    }
    
    printf("Final checksum: %d\n", total_sum);
    return total_sum != 0 ? 0 : 1;
}
