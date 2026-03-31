#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Vector types for SSE/AVX operations */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Prevent optimization */
static volatile int sink = 0;

/* Test function 1: Direct unordered comparisons using math.h macros */
__attribute__((noinline))
static int test_unordered_comparisons(float a, float b, double c, double d) {
    int sum = 0;
    
    /* These should generate UNORDERED/ORDERED condition codes */
    sum += isunordered(a, b) ? 1 : 0;
    sum += isordered(a, b) ? 2 : 0;
    
    /* UNEQ: unordered or equal */
    sum += !isgreater(c, d) && !isless(c, d) ? 4 : 0;
    
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

/* Test function 2: Mixed comparisons in conditional expressions */
__attribute__((noinline))
static int test_mixed_comparisons(float a, float b, float c, float d, float e, float f) {
    int sum = 0;
    
    /* Complex conditional with different comparison types */
    for (int i = 0; i < 4; i++) {
        /* This should generate multiple condition codes */
        if ((a < b) ? (c != d) : (e >= f)) {
            sum += 1;
        }
        
        /* Another mixed expression */
        sum += ((a == b) && (c > d)) || ((e != f) && (a <= c)) ? 2 : 0;
        
        /* Ternary with different operators */
        float temp = (b != b) ? c : d;  /* NaN check */
        sum += (temp == temp) ? 4 : 0;
    }
    
    return sum;
}

/* Test function 3: Vectorized comparisons using GCC vector extensions */
__attribute__((noinline))
static int test_vector_comparisons(v4sf va, v4sf vb, v2df vc, v2df vd) {
    int sum = 0;
    
    /* Element-wise unordered comparison */
    v4sf mask_unord = __builtin_ia32_cmpunordps(va, vb);
    sum += ((int*)&mask_unord)[0] != 0 ? 1 : 0;
    
    /* Element-wise ordered comparison */
    v4sf mask_ord = __builtin_ia32_cmpordps(va, vb);
    sum += ((int*)&mask_ord)[1] != 0 ? 2 : 0;
    
    /* Not equal (unordered or not equal) */
    v4sf mask_neq = va != vb;
    sum += ((int*)&mask_neq)[2] != 0 ? 4 : 0;
    
    /* Greater than or equal (unordered) */
    v4sf mask_ge = va >= vb;
    sum += ((int*)&mask_ge)[3] != 0 ? 8 : 0;
    
    /* Double vector comparisons */
    v2df mask_dbl_unord = __builtin_ia32_cmpunordsd(vc, vd);
    sum += ((long long*)&mask_dbl_unord)[0] != 0 ? 16 : 0;
    
    return sum;
}

/* Test function 4: Fast-math optimizations that generate UNEQ/LTGT */
__attribute__((noinline))
static int test_fast_math_optimizations(float a, float b, float c, float d) {
    int sum = 0;
    
    /* Under -ffast-math, these may generate UNEQ/LTGT */
    sum += (a == b) ? 1 : 0;      /* May become UNEQ with fast-math */
    sum += (a != b) ? 2 : 0;      /* May become LTGT with fast-math */
    
    /* Chain of comparisons */
    if (a < b) {
        sum += (c > d) ? 4 : 0;
    } else if (a > b) {
        sum += (c <= d) ? 8 : 0;
    } else {
        /* a == b or unordered */
        sum += !(c == c) ? 16 : 0;  /* Check for NaN */
    }
    
    /* Complex expression that might optimize to unordered comparisons */
    sum += ((a <= b) && (c >= d)) || ((a >= b) && (c <= d)) ? 32 : 0;
    
    return sum;
}

/* Test function 5: NaN handling and explicit unordered checks */
__attribute__((noinline))
static int test_nan_handling(float a, float b, double c, double d) {
    int sum = 0;
    
    /* Explicit NaN checks - should generate unordered comparisons */
    sum += (a != a) ? 1 : 0;      /* true if a is NaN */
    sum += !(b == b) ? 2 : 0;     /* true if b is NaN */
    
    /* Mixed NaN and normal comparisons */
    sum += (a != a) || (c > d) ? 4 : 0;
    sum += (b == b) && (c < d) ? 8 : 0;
    
    /* Double NaN check */
    sum += (c != c) ? 16 : 0;
    sum += !(d == d) ? 32 : 0;
    
    /* Ordered comparison that must handle NaN */
    sum += (a < b) ? 64 : 0;
    sum += (c >= d) ? 128 : 0;
    
    return sum;
}

int main(int argc, char **argv) {
    int total_sum = 0;
    
    /* Patterned data including NaN values */
    float float_data[] = {
        1.0f, 2.0f, 3.0f, 4.0f,
        0.0f, -0.0f, __builtin_nanf(""), 5.0f,
        INFINITY, -INFINITY, 6.0f, 7.0f
    };
    
    double double_data[] = {
        1.0, 2.0, 3.0, 4.0,
        0.0, -0.0, __builtin_nan(""), 5.0,
        INFINITY, -INFINITY, 6.0, 7.0
    };
    
    /* Vector data */
    v4sf vec1 = {1.0f, 2.0f, __builtin_nanf(""), 4.0f};
    v4sf vec2 = {2.0f, 2.0f, 3.0f, __builtin_nanf("")};
    v2df dvec1 = {1.0, __builtin_nan("")};
    v2df dvec2 = {__builtin_nan(""), 2.0};
    
    /* Use argc to prevent excessive loop unrolling */
    int iterations = (argc > 1) ? (argc % 8) + 1 : 4;
    
    for (int i = 0; i < iterations; i++) {
        int idx = i % 12;
        int idx2 = (i + 1) % 12;
        int idx3 = (i + 2) % 12;
        int idx4 = (i + 3) % 12;
        
        /* Call all test functions with different data patterns */
        total_sum += test_unordered_comparisons(
            float_data[idx], float_data[idx2],
            double_data[idx3], double_data[idx4]
        );
        
        total_sum += test_mixed_comparisons(
            float_data[idx], float_data[idx2],
            float_data[idx3], float_data[idx4],
            float_data[(idx + 4) % 12], float_data[(idx + 5) % 12]
        );
        
        total_sum += test_vector_comparisons(vec1, vec2, dvec1, dvec2);
        
        total_sum += test_fast_math_optimizations(
            float_data[idx], float_data[idx2],
            float_data[idx3], float_data[idx4]
        );
        
        total_sum += test_nan_handling(
            float_data[idx], float_data[idx2],
            double_data[idx3], double_data[idx4]
        );
        
        /* Modify data slightly to vary comparisons */
        float_data[idx] += 0.1f;
        double_data[idx2] -= 0.1;
    }
    
    /* Prevent dead code elimination */
    sink = total_sum;
    
    printf("Result checksum: %d\n", total_sum);
    return total_sum != 0 ? 0 : 1;
}
