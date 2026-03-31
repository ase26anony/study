/* Compile with: gcc -O3 -ffast-math -mavx2 -fdump-rtl-final -o test test.c */
#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <x86intrin.h>

/* Prevent optimization of critical values */
static volatile float vf = 0.0f;
static volatile double vd = 0.0;

/* Vector types for AVX comparisons */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Test function 1: Direct unordered comparisons using math.h macros */
__attribute__((noinline))
static int test_unordered_comparisons(float a, float b, double c, double d) {
    int sum = 0;
    
    /* These should generate UNORDERED/ORDERED condition codes */
    sum += isunordered(a, b) ? 1 : 0;      /* unord */
    sum += isordered(a, b) ? 2 : 0;        /* ord */
    sum += !isunordered(a, b) ? 4 : 0;     /* ord (inverted) */
    
    /* UNEQ: unordered or equal */
    sum += !islessgreater(c, d) ? 8 : 0;   /* uneq */
    
    /* UNGE: not less than (unordered or greater or equal) */
    sum += !isless(c, d) ? 16 : 0;         /* nlt */
    
    /* UNGT: not less or equal (unordered or greater) */
    sum += !islessequal(c, d) ? 32 : 0;    /* nle */
    
    /* UNLE: unordered or less or equal */
    sum += !isgreater(c, d) ? 64 : 0;      /* ule */
    
    /* UNLT: unordered or less than */
    sum += !isgreaterequal(c, d) ? 128 : 0; /* ult */
    
    /* LTGT: less or greater (ordered and not equal) */
    sum += islessgreater(a, b) ? 256 : 0;  /* une */
    
    return sum;
}

/* Test function 2: Mixed comparisons in conditional expressions */
__attribute__((noinline))
static int test_mixed_comparisons(float a, float b, float c, float d, 
                                  double e, double f, double g, double h) {
    int sum = 0;
    
    /* Complex ternary with different comparison types */
    for (int i = 0; i < 4; i++) {
        /* This should generate multiple condition codes */
        sum += ((a < b) ? (c != d) : (e >= f)) ? 1 : 0;
        
        /* Another mixed expression */
        sum += ((g == h) ? (a > b) : (c <= d)) ? 2 : 0;
        
        /* Combine ordered and unordered checks */
        sum += (isunordered(a, b) || (c == d)) ? 4 : 0;
        sum += (!isunordered(e, f) && (g != h)) ? 8 : 0;
        
        /* Rotate values to prevent constant folding */
        float tmp = a;
        a = b; b = c; c = d; d = tmp;
    }
    
    return sum;
}

/* Test function 3: Vectorized comparisons using GCC vector extensions */
__attribute__((noinline))
static int test_vector_comparisons(v4sf va, v4sf vb, v2df vc, v2df vd) {
    int sum = 0;
    
    /* Vector comparisons generate condition codes for each element */
    v4sf cmp_unord = __builtin_ia32_cmpunordps(va, vb);  /* UNORDERED */
    v4sf cmp_ord = __builtin_ia32_cmpordps(va, vb);      /* ORDERED */
    v4sf cmp_neq_uq = __builtin_ia32_cmpneqps(va, vb);   /* UNEQ */
    v4sf cmp_nlt = __builtin_ia32_cmpnltps(va, vb);      /* UNGE (nlt) */
    v4sf cmp_nle = __builtin_ia32_cmpnleps(va, vb);      /* UNGT (nle) */
    v4sf cmp_ule = (va <= vb);                           /* UNLE */
    v4sf cmp_ult = (va < vb);                            /* UNLT */
    v4sf cmp_une = __builtin_ia32_cmpneqps(va, vb);      /* LTGT (une) */
    
    /* Extract results to prevent elimination */
    float res[4];
    memcpy(res, &cmp_unord, sizeof(res));
    sum += (res[0] != 0.0f) ? 1 : 0;
    
    memcpy(res, &cmp_ord, sizeof(res));
    sum += (res[1] != 0.0f) ? 2 : 0;
    
    memcpy(res, &cmp_neq_uq, sizeof(res));
    sum += (res[2] != 0.0f) ? 4 : 0;
    
    memcpy(res, &cmp_nlt, sizeof(res));
    sum += (res[3] != 0.0f) ? 8 : 0;
    
    return sum;
}

/* Test function 4: NaN checks and fast-math optimizations */
__attribute__((noinline))
static int test_nan_checks(float a, double b, float c, double d) {
    int sum = 0;
    
    /* Direct NaN checks (a != a is true iff a is NaN) */
    sum += (a != a) ? 1 : 0;          /* Should generate unordered check */
    sum += !(b == b) ? 2 : 0;         /* Another NaN check */
    
    /* Under fast-math, these may use UNEQ/LTGT codes */
    sum += (a == c) ? 4 : 0;          /* May become UNEQ with -ffast-math */
    sum += (a != c) ? 8 : 0;          /* May become LTGT with -ffast-math */
    
    /* Mixed NaN and normal comparisons */
    sum += ((a != a) || (b > d)) ? 16 : 0;
    sum += ((b == b) && (c < a)) ? 32 : 0;
    
    return sum;
}

/* Test function 5: Chain of floating-point comparisons */
__attribute__((noinline))
static int test_comparison_chain(float a, float b, float c, float d,
                                 double e, double f, double g, double h) {
    int sum = 0;
    
    /* Chain comparisons that might generate various condition codes */
    if (a < b) sum += 1;
    if (c >= d) sum += 2;
    if (e != f) sum += 4;
    if (g == h) sum += 8;
    
    /* Nested comparisons */
    sum += (a < b) ? ((c > d) ? 16 : 32) : ((e == f) ? 64 : 128);
    
    /* Logical combinations */
    sum += ((a <= b) && (c >= d)) ? 256 : 0;
    sum += ((e != f) || (g == h)) ? 512 : 0;
    
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
        1.0, 2.0, 3.0, 4.0,
        0.0, -0.0, __builtin_nan(""), __builtin_inf(),
        5.0, 6.0, 7.0, 8.0
    };
    
    /* Use argc to prevent loop unrolling from eliminating comparisons */
    int iterations = (argc > 1) ? argc : 4;
    iterations = iterations % 8 + 1;  /* Ensure at least 1 iteration */
    
    for (int i = 0; i < iterations; i++) {
        /* Test 1: Unordered comparisons */
        total_sum += test_unordered_comparisons(
            float_data[i % 12] + vf,
            float_data[(i + 1) % 12],
            double_data[i % 12] + vd,
            double_data[(i + 2) % 12]
        );
        
        /* Test 2: Mixed comparisons */
        total_sum += test_mixed_comparisons(
            float_data[(i + 0) % 12], float_data[(i + 1) % 12],
            float_data[(i + 2) % 12], float_data[(i + 3) % 12],
            double_data[(i + 4) % 12], double_data[(i + 5) % 12],
            double_data[(i + 6) % 12], double_data[(i + 7) % 12]
        );
        
        /* Test 3: Vector comparisons */
        v4sf va = {float_data[(i + 0) % 12], float_data[(i + 1) % 12],
                   float_data[(i + 2) % 12], float_data[(i + 3) % 12]};
        v4sf vb = {float_data[(i + 4) % 12], float_data[(i + 5) % 12],
                   float_data[(i + 6) % 12], float_data[(i + 7) % 12]};
        v2df vc = {double_data[(i + 0) % 12], double_data[(i + 1) % 12]};
        v2df vd = {double_data[(i + 2) % 12], double_data[(i + 3) % 12]};
        
        total_sum += test_vector_comparisons(va, vb, vc, vd);
        
        /* Test 4: NaN checks */
        total_sum += test_nan_checks(
            float_data[(i + 0) % 12] + vf,
            double_data[(i + 1) % 12] + vd,
            float_data[(i + 2) % 12],
            double_data[(i + 3) % 12]
        );
        
        /* Test 5: Comparison chain */
        total_sum += test_comparison_chain(
            float_data[(i + 0) % 12], float_data[(i + 1) % 12],
            float_data[(i + 2) % 12], float_data[(i + 3) % 12],
            double_data[(i + 4) % 12], double_data[(i + 5) % 12],
            double_data[(i + 6) % 12], double_data[(i + 7) % 12]
        );
        
        /* Modify volatile values to prevent constant propagation */
        vf += 0.1f;
        vd += 0.1;
    }
    
    printf("Total checksum: %d\n", total_sum);
    return total_sum != 0 ? 0 : 1;
}
