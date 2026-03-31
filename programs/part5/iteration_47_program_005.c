/* Compile with: gcc -O3 -ffast-math -mavx2 -fdump-rtl-final -o test_conds test_conds.c */
#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <string.h>
#include <xmmintrin.h>
#include <immintrin.h>

/* Prevent optimization of critical values */
static volatile float vf1, vf2;
static volatile double vd1, vd2;

/* Vector types for SSE/AVX comparisons */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Test function 1: Direct unordered comparisons using math.h macros */
__attribute__((noinline))
static int test_unordered_comparisons(float a, float b, double c, double d) {
    int sum = 0;
    
    /* These should generate UNORDERED/ORDERED condition codes */
    sum += isunordered(a, b) ? 1 : 0;
    sum += isordered(a, b) ? 2 : 0;
    
    /* UNEQ: unordered or equal */
    sum += !(c > d) && !(c < d) ? 4 : 0;  /* Equivalent to UNEQ under fast-math */
    
    /* UNGE: unordered or greater-or-equal */
    sum += !(c < d) ? 8 : 0;
    
    /* UNGT: unordered or greater */
    sum += !(c <= d) ? 16 : 0;
    
    /* UNLE: unordered or less-or-equal */
    sum += !(c > d) ? 32 : 0;
    
    /* UNLT: unordered or less */
    sum += !(c >= d) ? 64 : 0;
    
    /* LTGT: less or greater (ordered and not equal) */
    sum += islessgreater(c, d) ? 128 : 0;
    
    return sum;
}

/* Test function 2: Mixed comparison types in complex conditionals */
__attribute__((noinline))
static int test_mixed_comparisons(float a, float b, float c, float d, float e, float f) {
    int sum = 0;
    
    /* Complex ternary with different comparison types */
    for (int i = 0; i < 3; i++) {
        /* This should generate multiple condition codes */
        sum += ((a < b) ? (c != d) : (e >= f)) ? (1 << i) : 0;
        
        /* Another mixed comparison */
        sum += ((a == b) ? (c > d) : (e <= f)) ? (2 << i) : 0;
        
        /* Chain of comparisons */
        sum += (a != b) && (c < d) && (e > f) ? (4 << i) : 0;
    }
    
    /* NaN checks that should generate unordered comparisons */
    sum += (a != a) ? 256 : 0;      /* Check for NaN */
    sum += !(b == b) ? 512 : 0;     /* Another NaN check */
    
    return sum;
}

/* Test function 3: Vectorized comparisons using GCC vector extensions */
__attribute__((noinline))
static int test_vector_comparisons(v4sf va, v4sf vb, v2df vc, v2df vd) {
    int sum = 0;
    
    /* Element-wise vector comparisons */
    v4sf cmp_unord = __builtin_ia32_cmpunordps(va, vb);  /* UNORDERED */
    v4sf cmp_ord = __builtin_ia32_cmpordps(va, vb);      /* ORDERED */
    v4sf cmp_neq = __builtin_ia32_cmpneqps(va, vb);      /* UNEQ inverse? */
    v4sf cmp_nlt = __builtin_ia32_cmpnltps(va, vb);      /* UNGE */
    v4sf cmp_nle = __builtin_ia32_cmpnleps(va, vb);      /* UNGT */
    
    /* Extract results to scalar */
    float res[4];
    memcpy(res, &cmp_unord, sizeof(res));
    for (int i = 0; i < 4; i++) {
        sum += (res[i] != 0.0f) ? (1 << i) : 0;
    }
    
    memcpy(res, &cmp_ord, sizeof(res));
    for (int i = 0; i < 4; i++) {
        sum += (res[i] != 0.0f) ? (1 << (i + 4)) : 0;
    }
    
    /* Double vector comparisons */
    v2df cmp_uneq = __builtin_ia32_cmpneqpd(vc, vd);     /* UNEQ */
    v2df cmp_ltgt = (vc < vd) | (vc > vd);               /* LTGT - less or greater */
    
    double dres[2];
    memcpy(dres, &cmp_uneq, sizeof(dres));
    sum += (dres[0] != 0.0) ? 1024 : 0;
    sum += (dres[1] != 0.0) ? 2048 : 0;
    
    return sum;
}

/* Test function 4: Fast-math optimized comparisons */
__attribute__((noinline))
static int test_fastmath_comparisons(float a, float b, double c, double d) {
    int sum = 0;
    
    /* Under -ffast-math, these may be transformed to unordered comparisons */
    
    /* UNEQ: (a == b) under fast-math might use unordered equal */
    sum += (a == b) ? 1 : 0;
    
    /* LTGT: (a != b) under fast-math might use less or greater */
    sum += (a != b) ? 2 : 0;
    
    /* UNGE: (a >= b) might become !(a < b) */
    sum += (a >= b) ? 4 : 0;
    
    /* UNGT: (a > b) might become !(a <= b) */
    sum += (a > b) ? 8 : 0;
    
    /* Complex expression that should generate multiple condition codes */
    sum += ((c == d) || (c != d)) ? 16 : 0;  /* Always true but compiler doesn't know */
    sum += ((c < d) && (c > d)) ? 32 : 0;    /* Always false but compiler doesn't know */
    
    return sum;
}

/* Test function 5: NaN-producing comparisons */
__attribute__((noinline))
static int test_nan_comparisons(float a, float b) {
    int sum = 0;
    
    /* Generate NaN values */
    float nan1 = __builtin_nanf("");
    float nan2 = __builtin_nanf("0x1234");
    double nan3 = __builtin_nan("");
    
    /* Comparisons with NaN should generate unordered condition codes */
    sum += (a == nan1) ? 1 : 0;      /* Always false when a is not NaN */
    sum += (b != nan2) ? 2 : 0;      /* Always true when b is not NaN */
    sum += (nan1 == nan2) ? 4 : 0;   /* Always false (NaN != NaN) */
    sum += (nan1 != nan1) ? 8 : 0;   /* Always true (NaN != NaN) */
    
    /* Mixed NaN and normal comparisons */
    sum += (a < nan3) ? 16 : 0;      /* Always false */
    sum += (nan3 > b) ? 32 : 0;      /* Always false */
    
    return sum;
}

int main(int argc, char **argv) {
    int total_sum = 0;
    
    /* Patterned data including normal numbers, zeros, and special values */
    float float_data[] = {1.0f, -1.0f, 0.0f, -0.0f, 3.14159f, 2.71828f, 100.0f, -100.0f};
    double double_data[] = {1.0, -1.0, 0.0, -0.0, 3.141592653589793, 2.718281828459045, 
                           1e100, -1e100};
    
    /* Initialize volatile values to prevent constant propagation */
    vf1 = float_data[argc % 8];
    vf2 = float_data[(argc + 1) % 8];
    vd1 = double_data[argc % 8];
    vd2 = double_data[(argc + 2) % 8];
    
    /* Vector data */
    v4sf vec1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec2 = {4.0f, 3.0f, 2.0f, 1.0f};
    v2df dvec1 = {1.0, 2.0};
    v2df dvec2 = {2.0, 1.0};
    
    /* Loop with volatile counter to prevent excessive unrolling */
    volatile int iterations = (argc > 1) ? 4 : 8;
    
    for (volatile int i = 0; i < iterations; i++) {
        int idx = i % 8;
        
        /* Call all test functions with varying inputs */
        total_sum += test_unordered_comparisons(
            float_data[idx], 
            float_data[(idx + 1) % 8],
            double_data[idx],
            double_data[(idx + 2) % 8]
        );
        
        total_sum += test_mixed_comparisons(
            float_data[idx],
            float_data[(idx + 3) % 8],
            float_data[(idx + 1) % 8],
            float_data[(idx + 4) % 8],
            float_data[(idx + 2) % 8],
            float_data[(idx + 5) % 8]
        );
        
        total_sum += test_vector_comparisons(vec1, vec2, dvec1, dvec2);
        
        total_sum += test_fastmath_comparisons(
            float_data[idx],
            float_data[(idx + 6) % 8],
            double_data[idx],
            double_data[(idx + 7) % 8]
        );
        
        total_sum += test_nan_comparisons(
            float_data[idx],
            float_data[(idx + 7) % 8]
        );
        
        /* Modify vectors slightly each iteration */
        vec1[0] += 0.1f;
        vec2[1] -= 0.1f;
        dvec1[0] *= 1.1;
        dvec2[1] /= 1.1;
    }
    
    printf("Total checksum: %d\n", total_sum);
    return total_sum != 0 ? 0 : 1;
}
