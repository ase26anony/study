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
static volatile int sink;

/* Test function 1: Direct unordered comparisons using math.h macros */
__attribute__((noinline))
static int test_unordered_comparisons(float a, float b, double c, double d) {
    int sum = 0;
    
    /* These should generate UNORDERED/ORDERED condition codes */
    sum += isunordered(a, b) ? 1 : 0;
    sum += isordered(a, b) ? 2 : 0;
    
    /* UNEQ: unordered or equal */
    sum += !(c > d) && !(c < d) ? 4 : 0;  /* Equivalent to UNEQ under fast-math */
    
    /* LTGT: less than or greater than (ordered and not equal) */
    sum += islessgreater(c, d) ? 8 : 0;
    
    return sum;
}

/* Test function 2: Mixed comparisons in conditional expressions */
__attribute__((noinline))
static int test_mixed_comparisons(float a, float b, float c, float d, float e, float f) {
    int sum = 0;
    
    /* Complex conditional that may generate multiple condition codes */
    for (int i = 0; i < 3; i++) {
        /* This ternary mixes different comparison types */
        sum += ((a < b) ? (c != d) : (e >= f)) ? (1 << i) : 0;
        
        /* Another mixed expression */
        sum += ((a == b) ? (c > d) : (e <= f)) ? (2 << i) : 0;
        
        /* UNGE: not less than (unordered or greater or equal) */
        sum += !(a < b) ? (4 << i) : 0;
        
        /* UNGT: not less than or equal */
        sum += !(a <= b) ? (8 << i) : 0;
        
        /* UNLE: unordered or less or equal */
        sum += !(a > b) ? (16 << i) : 0;
        
        /* UNLT: unordered or less than */
        sum += !(a >= b) ? (32 << i) : 0;
    }
    
    return sum;
}

/* Test function 3: Vectorized comparisons using GCC vector extensions */
__attribute__((noinline))
static int test_vector_comparisons(v4sf va, v4sf vb, v2df vc, v2df vd) {
    int sum = 0;
    
    /* Element-wise vector comparisons */
    v4sf cmp_unord = __builtin_ia32_cmpunordps(va, vb);  /* UNORDERED */
    v4sf cmp_ord = __builtin_ia32_cmpordps(va, vb);      /* ORDERED */
    v4sf cmp_neq = __builtin_ia32_cmpneqps(va, vb);      /* UNEQ under fast-math? */
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
    v2df cmp_ltgt = __builtin_ia32_cmpneqpd(vc, vd) & __builtin_ia32_cmpordpd(vc, vd); /* LTGT */
    
    double dres[2];
    memcpy(dres, &cmp_uneq, sizeof(dres));
    sum += (dres[0] != 0.0) ? 256 : 0;
    sum += (dres[1] != 0.0) ? 512 : 0;
    
    return sum;
}

/* Test function 4: NaN checks and fast-math optimizations */
__attribute__((noinline))
static int test_nan_checks(float a, double b, float c, double d) {
    int sum = 0;
    
    /* Explicit NaN checks - may generate unordered comparisons */
    sum += (a != a) ? 1 : 0;          /* true if a is NaN */
    sum += !(b == b) ? 2 : 0;         /* true if b is NaN */
    
    /* Under fast-math, these may become UNEQ/LTGT */
    sum += (c == c) ? 4 : 0;          /* false if c is NaN */
    sum += (d != d) ? 8 : 0;          /* true if d is NaN */
    
    /* Complex expression that fast-math might transform */
    float temp = a * c;
    sum += (temp == temp) ? 16 : 0;   /* check for NaN result */
    
    /* Ordered comparisons that fast-math might convert */
    sum += (a < c) ? 32 : 0;
    sum += (b > d) ? 64 : 0;
    
    /* UNLE/UNLT through negation */
    sum += !(a > c) ? 128 : 0;        /* UNLE: unordered or less or equal */
    sum += !(a >= c) ? 256 : 0;       /* UNLT: unordered or less than */
    
    return sum;
}

/* Test function 5: Chain of comparisons in function arguments */
__attribute__((noinline))
static int test_comparison_chain(float a, float b, float c, float d, 
                                 double e, double f, double g, double h) {
    /* This complex chain should generate multiple condition codes */
    int result = 0;
    
    if ((a < b) && (c > d)) {
        result |= 1;
    }
    
    if ((e == f) || (g != h)) {
        result |= 2;
    }
    
    if (!(a >= b) && !(c <= d)) {  /* UNLT && UNGT */
        result |= 4;
    }
    
    if ((a != a) || (b != b)) {    /* Either is NaN */
        result |= 8;
    }
    
    /* Mixed float/double comparisons */
    result |= ((double)a < e) ? 16 : 0;
    result |= ((float)g > c) ? 32 : 0;
    
    return result;
}

int main(int argc, char **argv) {
    int total_sum = 0;
    
    /* Patterned data including normal numbers, zeros, and NaN */
    float float_data[] = {
        1.0f, 2.0f, 0.0f, -0.0f,
        3.14f, -2.71f, __builtin_nanf(""), 100.0f,
        INFINITY, -INFINITY
    };
    
    double double_data[] = {
        1.0, 2.0, 0.0, -0.0,
        3.1415926535, -2.7182818284, __builtin_nan(""), 1000.0,
        INFINITY, -INFINITY
    };
    
    /* Vector data */
    v4sf vec1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec2 = {4.0f, 3.0f, 2.0f, 1.0f};
    v2df dvec1 = {1.0, 2.0};
    v2df dvec2 = {2.0, 1.0};
    
    /* Use argc to prevent excessive loop unrolling */
    int iterations = (argc > 1) ? (argc % 5) + 1 : 3;
    
    for (int i = 0; i < iterations; i++) {
        /* Vary the inputs slightly each iteration */
        float f1 = float_data[i % 10] + (i * 0.1f);
        float f2 = float_data[(i + 1) % 10] - (i * 0.1f);
        float f3 = float_data[(i + 2) % 10] * (1.0f + i * 0.01f);
        float f4 = float_data[(i + 3) % 10] / (1.0f + i * 0.01f);
        
        double d1 = double_data[i % 10] + (i * 0.1);
        double d2 = double_data[(i + 1) % 10] - (i * 0.1);
        double d3 = double_data[(i + 2) % 10] * (1.0 + i * 0.01);
        double d4 = double_data[(i + 3) % 10] / (1.0 + i * 0.01);
        
        /* Call all test functions */
        total_sum += test_unordered_comparisons(f1, f2, d1, d2);
        total_sum += test_mixed_comparisons(f1, f2, f3, f4, f1, f3);
        total_sum += test_vector_comparisons(vec1, vec2, dvec1, dvec2);
        total_sum += test_nan_checks(f1, d1, f3, d3);
        total_sum += test_comparison_chain(f1, f2, f3, f4, d1, d2, d3, d4);
        
        /* Modify vectors slightly */
        vec1[0] += 0.1f;
        vec2[1] -= 0.1f;
        dvec1[0] += 0.01;
        dvec2[1] -= 0.01;
    }
    
    /* Prevent dead code elimination */
    sink = total_sum;
    
    printf("Result checksum: %d\n", total_sum);
    return total_sum != 0 ? 0 : 1;
}
