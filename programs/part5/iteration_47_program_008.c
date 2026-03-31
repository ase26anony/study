#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Vector types for SSE/AVX operations */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Test functions with different comparison patterns */
static int __attribute__((noinline)) 
test_unordered_comparisons(float a, float b, double c, double d) {
    int sum = 0;
    
    /* Direct unordered checks using standard macros */
    sum += isunordered(a, b) ? 1 : 0;
    sum += isunordered(c, d) ? 2 : 0;
    
    /* NaN checks using self-comparison */
    sum += (a != a) ? 4 : 0;  /* true if a is NaN */
    sum += !(c == c) ? 8 : 0; /* true if c is NaN */
    
    /* Ordered checks */
    sum += isordered(a, b) ? 16 : 0;
    sum += isordered(c, d) ? 32 : 0;
    
    return sum;
}

static int __attribute__((noinline))
test_mixed_comparisons(float a, float b, float c, float d) {
    int sum = 0;
    
    /* Complex conditional with mixed comparisons */
    sum += ((a < b) ? (c != d) : (d >= c)) ? 1 : 0;
    sum += ((a == b) ? (c > d) : (d <= c)) ? 2 : 0;
    
    /* Nested ternary with different operators */
    float res = (a != b) ? ((c < d) ? 1.0f : 2.0f) 
                         : ((c > d) ? 3.0f : 4.0f);
    sum += (int)res;
    
    return sum;
}

static int __attribute__((noinline))
test_vector_comparisons(v4sf va, v4sf vb) {
    int sum = 0;
    
    /* Vector comparisons using GCC extensions */
    v4sf cmp_result = va < vb;
    sum += ((int*)&cmp_result)[0] != 0 ? 1 : 0;
    
    cmp_result = va == vb;
    sum += ((int*)&cmp_result)[1] != 0 ? 2 : 0;
    
    cmp_result = va != vb;
    sum += ((int*)&cmp_result)[2] != 0 ? 4 : 0;
    
    cmp_result = va >= vb;
    sum += ((int*)&cmp_result)[3] != 0 ? 8 : 0;
    
    return sum;
}

static int __attribute__((noinline))
test_fast_math_patterns(float a, float b, float c, float d) {
    int sum = 0;
    
    /* These patterns often generate UNEQ/LTGT under -ffast-math */
    sum += (a == b) ? 1 : 0;
    sum += (c != d) ? 2 : 0;
    
    /* Chained comparisons */
    sum += (a < b && c > d) ? 4 : 0;
    sum += (a >= b || c <= d) ? 8 : 0;
    
    /* Mixed with integer conversion */
    sum += ((int)(a + b) > (int)(c + d)) ? 16 : 0;
    
    return sum;
}

static int __attribute__((noinline))
test_sse_intrinsics(__m128 x, __m128 y) {
    int sum = 0;
    
    /* SSE intrinsics that map to condition codes */
    __m128 cmp;
    
    /* Compare unordered */
    cmp = _mm_cmpunord_ps(x, y);
    sum += _mm_movemask_ps(cmp);
    
    /* Compare not equal (unordered or ordered) */
    cmp = _mm_cmpneq_ps(x, y);
    sum += _mm_movemask_ps(cmp) << 4;
    
    /* Compare less than */
    cmp = _mm_cmplt_ps(x, y);
    sum += _mm_movemask_ps(cmp) << 8;
    
    /* Compare greater than or equal */
    cmp = _mm_cmpge_ps(x, y);
    sum += _mm_movemask_ps(cmp) << 12;
    
    return sum;
}

static int __attribute__((noinline))
test_double_comparisons(double a, double b, double c, double d, double e, double f) {
    int sum = 0;
    
    /* Multiple double comparisons in complex expression */
    sum += ((a < b) != (c > d)) ? 1 : 0;
    sum += ((a == b) == (c == d)) ? 2 : 0;
    
    /* Use all comparison operators */
    sum += (a != b) ? 4 : 0;
    sum += (c <= d) ? 8 : 0;
    sum += (e >= f) ? 16 : 0;
    
    /* Combined with logical operators */
    sum += ((a < b) && (c > d) && (e != f)) ? 32 : 0;
    sum += ((a == b) || (c == d) || (e == f)) ? 64 : 0;
    
    return sum;
}

int main(int argc, char **argv) {
    volatile int iterations = (argc > 1) ? atoi(argv[1]) : 100;
    int total_sum = 0;
    
    /* Patterned data including NaN values */
    float fdata[] = {1.0f, 2.0f, __builtin_nanf(""), 0.0f, -1.0f, INFINITY};
    double ddata[] = {1.0, 2.0, __builtin_nan(""), 0.0, -1.0, INFINITY};
    
    /* SSE vectors */
    __m128 sse_vec1 = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    __m128 sse_vec2 = _mm_set_ps(4.0f, 3.0f, 2.0f, 1.0f);
    
    /* GCC vector types */
    v4sf gcc_vec1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf gcc_vec2 = {4.0f, 3.0f, 2.0f, 1.0f};
    
    for (volatile int i = 0; i < iterations; i++) {
        /* Vary inputs slightly each iteration */
        float f1 = fdata[i % 6] + (i * 0.1f);
        float f2 = fdata[(i + 1) % 6] - (i * 0.1f);
        float f3 = fdata[(i + 2) % 6] * (1.0f + i * 0.01f);
        float f4 = fdata[(i + 3) % 6] / (1.0f + i * 0.01f);
        
        double d1 = ddata[i % 6] + (i * 0.1);
        double d2 = ddata[(i + 1) % 6] - (i * 0.1);
        double d3 = ddata[(i + 2) % 6] * (1.0 + i * 0.01);
        double d4 = ddata[(i + 3) % 6] / (1.0 + i * 0.01);
        double d5 = ddata[(i + 4) % 6];
        double d6 = ddata[(i + 5) % 6];
        
        /* Call all test functions */
        total_sum += test_unordered_comparisons(f1, f2, d1, d2);
        total_sum += test_mixed_comparisons(f1, f2, f3, f4);
        total_sum += test_vector_comparisons(gcc_vec1, gcc_vec2);
        total_sum += test_fast_math_patterns(f1, f2, f3, f4);
        total_sum += test_sse_intrinsics(sse_vec1, sse_vec2);
        total_sum += test_double_comparisons(d1, d2, d3, d4, d5, d6);
        
        /* Modify vectors slightly */
        gcc_vec1[0] += 0.1f;
        gcc_vec2[1] -= 0.1f;
    }
    
    printf("Final checksum: %d\n", total_sum);
    return total_sum != 0 ? 0 : 1;
}
