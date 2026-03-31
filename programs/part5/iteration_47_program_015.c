#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Prevent optimization of critical values */
static volatile float vf = 0.0f;
static volatile double vd = 0.0;

/* Vector types for AVX/SSE */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Test function 1: Unordered comparisons using standard macros */
__attribute__((noinline))
static int test_unordered_comparisons(float a, float b, double c, double d) {
    int sum = 0;
    
    /* These should generate UNORDERED condition codes */
    if (isunordered(a, b)) sum += 1;
    if (!isunordered(c, d)) sum += 2;  /* ORDERED */
    
    /* UNEQ: unordered or equal */
    if (!(a < b) && !(a > b)) sum += 4;
    
    /* UNGE: not less than (unordered or greater or equal) */
    if (!(a < b)) sum += 8;
    
    /* UNGT: not less or equal (unordered or greater) */
    if (!(a <= b)) sum += 16;
    
    /* UNLE: unordered or less or equal */
    if (!(a > b)) sum += 32;
    
    /* UNLT: unordered or less than */
    if (!(a >= b)) sum += 64;
    
    /* LTGT: less or greater (unordered excluded) */
    if (islessgreater(c, d)) sum += 128;
    
    return sum;
}

/* Test function 2: Mixed comparisons in conditional expressions */
__attribute__((noinline))
static int test_mixed_comparisons(float a, float b, float c, float d, float e, float f) {
    int sum = 0;
    
    /* Complex conditional with different comparison types */
    for (int i = 0; i < 3; i++) {
        /* This should generate multiple condition codes */
        int cond = ((a < b) ? (c != d) : (e >= f)) ? 1 : 0;
        sum += cond;
        
        /* Another mixed expression */
        cond = ((a == b) && (c > d)) || ((e != f) && (a <= c));
        sum += cond * 2;
        
        /* Ternary with unordered check */
        cond = (a != a) ? (b < c) : (d > e);  /* a != a checks for NaN */
        sum += cond * 3;
    }
    
    return sum;
}

/* Test function 3: Vectorized comparisons */
__attribute__((noinline))
static int test_vector_comparisons(v4sf va, v4sf vb, v2df vc, v2df vd) {
    int sum = 0;
    
    /* Element-wise vector comparisons */
    v4sf cmp_result = va < vb;      /* Should use UNLT/UNLE */
    v4sf cmp_result2 = va >= vb;    /* Should use UNGE/UNGT */
    v4sf cmp_result3 = va == vb;    /* Should use UNEQ */
    
    /* Extract results to prevent elimination */
    float temp[4];
    memcpy(temp, &cmp_result, sizeof(temp));
    for (int i = 0; i < 4; i++) {
        sum += (temp[i] != 0.0f);
    }
    
    memcpy(temp, &cmp_result2, sizeof(temp));
    for (int i = 0; i < 4; i++) {
        sum += (temp[i] != 0.0f) * 2;
    }
    
    /* Double vector comparisons */
    v2df cmp_dbl = vc != vd;        /* Should use UNEQ */
    v2df cmp_dbl2 = vc < vd;        /* Should use UNLT */
    
    double tempd[2];
    memcpy(tempd, &cmp_dbl, sizeof(tempd));
    sum += (tempd[0] != 0.0) * 4 + (tempd[1] != 0.0) * 8;
    
    return sum;
}

/* Test function 4: Fast-math optimizations */
__attribute__((noinline))
static int test_fast_math_optimizations(float a, float b, double c, double d) {
    int sum = 0;
    
    /* Under fast-math, these may use UNEQ/LTGT codes */
    if (a == b) sum += 1;      /* May become UNEQ */
    if (a != b) sum += 2;      /* May become LTGT */
    
    /* Chained comparisons */
    if (a < b && b < c) sum += 4;
    if (c >= d || d <= a) sum += 8;
    
    /* Complex floating expression with comparisons */
    float temp = (a < b) ? c : d;
    sum += (temp > 0.0f) ? 16 : 0;
    
    return sum;
}

/* Test function 5: NaN handling and ordered/unordered checks */
__attribute__((noinline))
static int test_nan_handling(float a, float b, double c, double d) {
    int sum = 0;
    
    /* Explicit NaN checks */
    if (a != a) sum += 1;           /* Always true if a is NaN */
    if (!(c == c)) sum += 2;        /* Always true if c is NaN */
    
    /* Ordered comparisons that must handle NaN */
    if (a < b) sum += 4;            /* False if either is NaN */
    if (c > d) sum += 8;            /* False if either is NaN */
    
    /* Unordered comparisons */
    if (!(a >= b)) sum += 16;       /* True if a or b is NaN */
    if (!(c <= d)) sum += 32;       /* True if c or d is NaN */
    
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
    
    /* Use argc to prevent excessive loop unrolling */
    int iterations = (argc > 1) ? (argc % 5) + 1 : 3;
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Vary inputs slightly each iteration */
        float f1 = float_data[iter % 12] + vf;
        float f2 = float_data[(iter + 1) % 12] + vf;
        float f3 = float_data[(iter + 2) % 12] + vf;
        float f4 = float_data[(iter + 3) % 12] + vf;
        float f5 = float_data[(iter + 4) % 12] + vf;
        float f6 = float_data[(iter + 5) % 12] + vf;
        
        double d1 = double_data[iter % 12] + vd;
        double d2 = double_data[(iter + 1) % 12] + vd;
        double d3 = double_data[(iter + 2) % 12] + vd;
        double d4 = double_data[(iter + 3) % 12] + vd;
        
        /* Vector data */
        v4sf vf1 = {f1, f2, f3, f4};
        v4sf vf2 = {f2, f3, f4, f1};
        v2df vd1 = {d1, d2};
        v2df vd2 = {d2, d3};
        
        /* Call all test functions */
        total_sum += test_unordered_comparisons(f1, f2, d1, d2);
        total_sum += test_mixed_comparisons(f1, f2, f3, f4, f5, f6);
        total_sum += test_vector_comparisons(vf1, vf2, vd1, vd2);
        total_sum += test_fast_math_optimizations(f3, f4, d3, d4);
        total_sum += test_nan_handling(f5, f6, d1, d4);
    }
    
    printf("Total checksum: %d\n", total_sum);
    return total_sum != 0 ? 0 : 1;
}
