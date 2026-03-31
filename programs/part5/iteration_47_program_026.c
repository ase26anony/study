/* Compile with:
   gcc -O3 -ffast-math -march=x86-64 -mavx2 -fdump-rtl-final -o test_conds test_conds.c
   Additional flags for debugging: -O1 -da -fno-trapping-math
*/

#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Vector types for AVX/SSE */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Prevent inlining to ensure RTL generation */
static __attribute__((noinline)) 
int test_unordered_comparisons(float a, float b, double c, double d) {
    int sum = 0;
    
    /* Use standard unordered macros */
    sum += isunordered(a, b) ? 1 : 0;
    sum += islessgreater(a, b) ? 2 : 0;
    
    /* Direct unordered comparisons */
    sum += !(a <= b) ? 4 : 0;  /* May generate UNGT */
    sum += !(a >= b) ? 8 : 0;  /* May generate UNLT */
    
    /* NaN checks */
    sum += (a != a) ? 16 : 0;  /* Always true for NaN */
    sum += !(c == c) ? 32 : 0; /* Always true for NaN */
    
    /* Mixed ordered/unordered */
    sum += (isunordered(a, b) || (c > d)) ? 64 : 0;
    
    return sum;
}

static __attribute__((noinline))
int test_ltgt_uneq(float a, float b, float c, float d) {
    int sum = 0;
    
    /* These should generate LTGT/UNEQ under -ffast-math */
    sum += (a != b) ? 1 : 0;    /* May become UNEQ or LTGT */
    sum += (c == d) ? 2 : 0;    /* May become UNEQ */
    
    /* Complex expression that might generate multiple condition codes */
    sum += ((a < b) ? (c != d) : (a > b)) ? 4 : 0;
    
    return sum;
}

static __attribute__((noinline))
int test_vector_comparisons(v4sf va, v4sf vb, v2df vc, v2df vd) {
    int sum = 0;
    
    /* Vector comparisons - these often use unordered condition codes */
    v4sf mask1 = va < vb;    /* Element-wise less than */
    v4sf mask2 = va != vb;   /* Element-wise not equal */
    v2df mask3 = vc >= vd;   /* Element-wise greater or equal */
    
    /* Extract results to prevent elimination */
    float m1 = mask1[0] + mask1[1] + mask1[2] + mask1[3];
    float m2 = mask2[0] + mask2[1] + mask2[2] + mask2[3];
    double m3 = mask3[0] + mask3[1];
    
    sum += (m1 > 0) ? 1 : 0;
    sum += (m2 != 0) ? 2 : 0;
    sum += (m3 != 0) ? 4 : 0;
    
    return sum;
}

static __attribute__((noinline))
int test_mixed_conditions(float a, double b, float c, double d, float e) {
    int sum = 0;
    
    /* Complex chain of comparisons */
    if (isunordered(a, (float)b) || islessgreater(c, (float)d)) {
        sum += 1;
    }
    
    /* Ternary with different comparison types */
    sum += (a < b) ? ((c != d) ? 2 : 4) : ((e >= a) ? 8 : 16);
    
    /* Nested comparisons */
    sum += ((a == b) && (c != d) && !(e < a)) ? 32 : 0;
    
    return sum;
}

static __attribute__((noinline))
int test_ordered_vs_unordered(float a, float b, double c, double d) {
    int sum = 0;
    
    /* Ordered comparisons */
    sum += (a > b) ? 1 : 0;
    sum += (c <= d) ? 2 : 0;
    
    /* Unordered comparisons */
    sum += !(a >= b) ? 4 : 0;  /* UNLT */
    sum += !(c < d) ? 8 : 0;   /* UNGE */
    
    /* EQ/NEQ that might become UNEQ/LTGT */
    sum += (a == b) ? 16 : 0;
    sum += (c != d) ? 32 : 0;
    
    return sum;
}

int main(int argc, char **argv) {
    /* Use argc to prevent excessive loop unrolling */
    int iterations = argc > 1 ? atoi(argv[1]) : 100;
    if (iterations < 1) iterations = 1;
    if (iterations > 1000) iterations = 1000;
    
    /* Patterned data including NaN values */
    volatile float fdata[8];
    volatile double ddata[8];
    
    /* Initialize with pattern: normal, zero, NaN, inf */
    for (int i = 0; i < 8; i++) {
        fdata[i] = (i % 4 == 0) ? 1.0f / (i + 1) :
                   (i % 4 == 1) ? 0.0f :
                   (i % 4 == 2) ? __builtin_nanf("") :
                   __builtin_inff();
        
        ddata[i] = (i % 4 == 0) ? 1.0 / (i + 1) :
                   (i % 4 == 1) ? 0.0 :
                   (i % 4 == 2) ? __builtin_nan("") :
                   __builtin_inf();
    }
    
    /* Vector data */
    v4sf v1 = {1.0f, 2.0f, __builtin_nanf(""), 4.0f};
    v4sf v2 = {4.0f, 2.0f, 3.0f, __builtin_nanf("")};
    v2df vd1 = {1.0, __builtin_nan("")};
    v2df vd2 = {__builtin_nan(""), 2.0};
    
    int total_sum = 0;
    
    /* Main test loop - volatile counter prevents optimization */
    for (volatile int i = 0; i < iterations; i++) {
        int idx = i % 8;
        
        /* Call all test functions with different data patterns */
        total_sum += test_unordered_comparisons(
            fdata[idx], fdata[(idx + 1) % 8],
            ddata[idx], ddata[(idx + 2) % 8]
        );
        
        total_sum += test_ltgt_uneq(
            fdata[(idx + 1) % 8], fdata[(idx + 3) % 8],
            fdata[(idx + 2) % 8], fdata[(idx + 4) % 8]
        );
        
        total_sum += test_vector_comparisons(v1, v2, vd1, vd2);
        
        total_sum += test_mixed_conditions(
            fdata[idx], ddata[(idx + 1) % 8],
            fdata[(idx + 2) % 8], ddata[(idx + 3) % 8],
            fdata[(idx + 4) % 8]
        );
        
        total_sum += test_ordered_vs_unordered(
            fdata[idx], fdata[(idx + 2) % 8],
            ddata[idx], ddata[(idx + 2) % 8]
        );
        
        /* Modify data slightly each iteration */
        fdata[idx] += 0.1f;
        ddata[idx] += 0.1;
    }
    
    printf("Final checksum: %d\n", total_sum);
    return total_sum != 0 ? 0 : 1;
}
