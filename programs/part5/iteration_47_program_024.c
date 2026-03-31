/* Compile with:
   gcc -O3 -ffast-math -mavx2 -fdump-rtl-final -o test_conds test_conds.c
   For debugging: gcc -O1 -da -fno-trapping-math -o test_conds_debug test_conds.c
*/

#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <immintrin.h>

/* Prevent excessive optimization */
static volatile int force_volatile = 0;

/* Vector types for AVX/SSE operations */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Function 1: Direct unordered comparisons using math.h macros */
__attribute__((noinline))
static int test_unordered_comparisons(float a, float b, double c, double d) {
    int sum = 0;
    
    /* These should generate UNORDERED/ORDERED condition codes */
    sum += isunordered(a, b) ? 1 : 0;
    sum += isordered(a, b) ? 2 : 0;
    
    /* UNEQ: unordered or equal */
    sum += !islessgreater(c, d) ? 4 : 0;
    
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

/* Function 2: Mixed comparisons in complex conditional expressions */
__attribute__((noinline))
static int test_mixed_conditionals(float a, float b, float c, float d, float e, float f) {
    int sum = 0;
    
    /* Complex ternary with different comparison types */
    for (int i = 0; i < 4; i++) {
        float t = (float)i;
        
        /* This should generate multiple condition codes */
        if ((a < b + t) ? (c != d) : (e >= f - t)) {
            sum += 1;
        }
        
        /* Nested conditionals with different operators */
        sum += ((a == b) && (c != d)) || ((e < f) != (a > b)) ? 2 : 0;
        
        /* Chain of comparisons */
        sum += (a < b) < (c > d) ? 4 : 0;  /* Comparison of boolean results */
    }
    
    return sum;
}

/* Function 3: Vectorized comparisons using GCC vector extensions */
__attribute__((noinline))
static int test_vector_comparisons(v4sf va, v4sf vb, v2df vc, v2df vd) {
    int sum = 0;
    
    /* Element-wise vector comparisons */
    v4sf cmp_unord = __builtin_ia32_cmpunordps(va, vb);  /* UNORDERED */
    v4sf cmp_ord = __builtin_ia32_cmpordps(va, vb);      /* ORDERED */
    v4sf cmp_neq_uq = __builtin_ia32_cmpneqps(va, vb);   /* UNEQ */
    v4sf cmp_nlt = __builtin_ia32_cmpnltps(va, vb);      /* UNGE (nlt) */
    v4sf cmp_nle = __builtin_ia32_cmpnleps(va, vb);      /* UNGT (nle) */
    v4sf cmp_ule = (va <= vb);                           /* UNLE */
    v4sf cmp_ult = (va < vb);                            /* UNLT */
    v4sf cmp_une = __builtin_ia32_cmpunordps(va, vb) | __builtin_ia32_cmpneqps(va, vb); /* LTGT approximation */
    
    /* Extract results to prevent elimination */
    float results[8];
    memcpy(results, &cmp_unord, sizeof(cmp_unord));
    memcpy(results + 4, &cmp_ord, sizeof(cmp_ord));
    
    for (int i = 0; i < 8; i++) {
        sum += (results[i] != 0.0f) ? (1 << i) : 0;
    }
    
    return sum;
}

/* Function 4: NaN checks and fast-math optimizations */
__attribute__((noinline))
static int test_nan_fastmath(float a, double b, float c, double d) {
    int sum = 0;
    
    /* NaN checks that might generate unordered comparisons */
    sum += (a != a) ? 1 : 0;           /* true if a is NaN */
    sum += !(b == b) ? 2 : 0;          /* true if b is NaN */
    
    /* Under fast-math, these might use UNEQ/LTGT codes */
    sum += (c == d) ? 4 : 0;
    sum += (c != d) ? 8 : 0;
    
    /* Mixed with ordered comparisons */
    if ((a < b) && !(c != c)) {
        sum += 16;
    }
    
    /* Complex expression that fast-math might transform */
    sum += ((a >= b) == (c <= d)) ? 32 : 0;
    
    return sum;
}

/* Function 5: Chain of floating-point comparisons */
__attribute__((noinline))
static int test_comparison_chain(float a, float b, float c, float d, 
                                 float e, float f, float g, float h) {
    int sum = 0;
    
    /* Long chain of different comparisons */
    sum += (a < b) ? 1 : 0;
    sum += (b > c) ? 2 : 0;
    sum += (c <= d) ? 4 : 0;
    sum += (d >= e) ? 8 : 0;
    sum += (e == f) ? 16 : 0;
    sum += (f != g) ? 32 : 0;
    
    /* Combined with logical operators */
    if ((a < b) && (c > d) && (e != f) && (g <= h)) {
        sum += 64;
    }
    
    /* Ternary with comparison in result */
    sum += (a < b) ? (c > d) : (e < f);
    
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
        0.0, -0.0, 3.0, INFINITY
    };
    
    /* Initialize vector data */
    v4sf v1 = {1.0f, 2.0f, __builtin_nanf(""), 4.0f};
    v4sf v2 = {4.0f, 3.0f, 2.0f, 1.0f};
    v2df vd1 = {1.0, __builtin_nan("")};
    v2df vd2 = {__builtin_nan(""), 1.0};
    
    /* Use argc to prevent loop unrolling from collapsing comparisons */
    int iterations = (argc > 1) ? (argc % 8) + 1 : 4;
    
    for (int i = 0; i < iterations; i++) {
        int idx = i % 8;
        int idx2 = (i + 1) % 8;
        int idx3 = (i + 2) % 8;
        int idx4 = (i + 3) % 8;
        
        /* Test 1: Unordered comparisons */
        total_sum += test_unordered_comparisons(
            float_data[idx], float_data[idx2],
            double_data[idx], double_data[idx2]
        );
        
        /* Test 2: Mixed conditionals */
        total_sum += test_mixed_conditionals(
            float_data[idx], float_data[idx2],
            float_data[idx3], float_data[idx4],
            float_data[(idx + 4) % 8], float_data[(idx + 5) % 8]
        );
        
        /* Test 3: Vector comparisons */
        total_sum += test_vector_comparisons(v1, v2, vd1, vd2);
        
        /* Test 4: NaN and fast-math */
        total_sum += test_nan_fastmath(
            float_data[idx], double_data[idx2],
            float_data[idx3], double_data[idx4]
        );
        
        /* Test 5: Comparison chain */
        total_sum += test_comparison_chain(
            float_data[0], float_data[1], float_data[2], float_data[3],
            float_data[4], float_data[5], float_data[6], float_data[7]
        );
        
        /* Modify data slightly each iteration */
        float_data[idx] += 0.1f * i;
        double_data[idx2] += 0.05 * i;
    }
    
    /* Add volatile to prevent dead code elimination */
    total_sum += force_volatile;
    
    printf("Total checksum: %d\n", total_sum);
    return total_sum != 0 ? 0 : 1;
}
