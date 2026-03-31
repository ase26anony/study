/* 
 * Program to trigger x86 condition code mnemonics in i386.cc
 * Compile with: gcc -O3 -ffast-math -march=x86-64 -mavx2 -fdump-rtl-final
 */

#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <immintrin.h>

/* Vector types for SSE/AVX comparisons */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Prevent optimization of critical functions */
#define NOINLINE __attribute__((noinline))
#define VOLATILE_VAR(x) volatile auto x##_vol = x; x = x##_vol

/* Test function 1: Direct unordered comparisons using math.h macros */
NOINLINE static int test_unordered_comparisons(float a, float b, double c, double d) {
    int sum = 0;
    
    /* These should generate UNORDERED/ORDERED condition codes */
    sum += isunordered(a, b) ? 1 : 0;
    sum += isordered(a, b) ? 2 : 0;
    
    /* These should generate UNEQ/LTGT condition codes */
    sum += !islessgreater(c, d) ? 4 : 0;  /* UNEQ */
    sum += islessgreater(c, d) ? 8 : 0;   /* LTGT */
    
    /* Mixed comparisons that might generate UNGE/UNGT/UNLE/UNLT */
    sum += !isless(a, b) ? 16 : 0;        /* UNGE (nlt) */
    sum += !islessequal(a, b) ? 32 : 0;   /* UNGT (nle) */
    sum += islessequal(a, b) ? 64 : 0;    /* UNLE (ule) */
    sum += isless(a, b) ? 128 : 0;        /* UNLT (ult) */
    
    return sum;
}

/* Test function 2: Complex conditional expressions with mixed operators */
NOINLINE static int test_mixed_conditionals(float a, float b, float c, float d, float e, float f) {
    int sum = 0;
    
    /* Complex ternary with different comparison types */
    for (int i = 0; i < 3; i++) {
        float t = (i == 0) ? a : (i == 1) ? b : c;
        
        /* This complex expression should generate multiple condition codes */
        if ((a < b) ? (c != d) : (e >= f)) {
            sum += 1;
        }
        
        /* Nested conditionals with different operators */
        sum += ((t == a) && (b != c)) || ((d > e) != (f < a)) ? 2 : 0;
    }
    
    /* Direct NaN checks that might generate unordered comparisons */
    sum += (a != a) ? 4 : 0;      /* Always true if a is NaN */
    sum += !(b == b) ? 8 : 0;     /* Always true if b is NaN */
    
    return sum;
}

/* Test function 3: Vectorized comparisons using GCC vector extensions */
NOINLINE static int test_vector_comparisons(v4sf va, v4sf vb, v2df vc, v2df vd) {
    int sum = 0;
    
    /* Element-wise vector comparisons */
    v4sf vcmp_unord = __builtin_ia32_cmpunordps(va, vb);  /* UNORDERED */
    v4sf vcmp_ord = __builtin_ia32_cmpordps(va, vb);      /* ORDERED */
    v4sf vcmp_neq_uq = __builtin_ia32_cmpneqps(va, vb);   /* UNEQ */
    v4sf vcmp_nlt = __builtin_ia32_cmpnltps(va, vb);      /* UNGE (nlt) */
    v4sf vcmp_nle = __builtin_ia32_cmpnleps(va, vb);      /* UNGT (nle) */
    v4sf vcmp_ule = (va <= vb);                          /* UNLE using GCC vectors */
    v4sf vcmp_ult = (va < vb);                           /* UNLT using GCC vectors */
    v4sf vcmp_une = __builtin_ia32_cmpneqps(va, vb);     /* LTGT (une) */
    
    /* Extract results to prevent elimination */
    float results[8];
    memcpy(results, &vcmp_unord, sizeof(float) * 4);
    memcpy(results + 4, &vcmp_ord, sizeof(float) * 4);
    
    for (int i = 0; i < 8; i++) {
        sum += (results[i] != 0.0f) ? (1 << i) : 0;
    }
    
    return sum;
}

/* Test function 4: AVX intrinsics for specific condition codes */
#ifdef __AVX__
NOINLINE static int test_avx_intrinsics(__m128 a, __m128 b) {
    int sum = 0;
    
    /* Generate specific condition codes using AVX intrinsics */
    __m128 cmp_unord = _mm_cmp_unord_ps(a, b);    /* UNORDERED */
    __m128 cmp_ord = _mm_cmp_ord_ps(a, b);        /* ORDERED */
    __m128 cmp_ueq = _mm_cmp_ueq_ps(a, b);        /* UNEQ */
    __m128 cmp_nlt = _mm_cmp_nlt_ps(a, b);        /* UNGE (nlt) */
    __m128 cmp_nle = _mm_cmp_nle_ps(a, b);        /* UNGT (nle) */
    __m128 cmp_ule = _mm_cmp_ule_ps(a, b);        /* UNLE */
    __m128 cmp_ult = _mm_cmp_ult_ps(a, b);        /* UNLT */
    __m128 cmp_une = _mm_cmp_neq_ps(a, b);        /* LTGT (une) */
    
    /* Use results to prevent elimination */
    float res[4];
    _mm_store_ps(res, cmp_unord);
    sum += (res[0] != 0.0f) ? 1 : 0;
    
    _mm_store_ps(res, cmp_ord);
    sum += (res[1] != 0.0f) ? 2 : 0;
    
    _mm_store_ps(res, cmp_ueq);
    sum += (res[2] != 0.0f) ? 4 : 0;
    
    return sum;
}
#endif

/* Test function 5: Chain of comparisons in a loop */
NOINLINE static int test_comparison_chain(float *arr, int n) {
    int sum = 0;
    
    for (int i = 0; i < n - 1; i++) {
        /* Chain of different comparisons */
        if (isunordered(arr[i], arr[i + 1])) {
            sum += 1;
        } else if (!islessgreater(arr[i], arr[i + 1])) {  /* UNEQ */
            sum += 2;
        } else if (isless(arr[i], arr[i + 1])) {          /* UNLT */
            sum += 4;
        } else if (!islessequal(arr[i], arr[i + 1])) {    /* UNGT */
            sum += 8;
        }
        
        /* Additional mixed comparison */
        sum += (arr[i] == arr[i + 1]) ? 16 : 0;
        sum += (arr[i] != arr[i + 1]) ? 32 : 0;
    }
    
    return sum;
}

int main(int argc, char **argv) {
    int total_sum = 0;
    
    /* Patterned data including normal numbers, zeros, and NaN */
    float float_data[] = {
        1.0f, 2.0f, 0.0f, -0.0f,
        __builtin_nanf(""), 3.14f, INFINITY, -INFINITY,
        100.0f, 200.0f, 300.0f, 400.0f
    };
    
    double double_data[] = {
        1.0, 2.0, 0.0, -0.0,
        __builtin_nan(""), 3.1415926535, INFINITY, -INFINITY
    };
    
    /* Vector data */
    v4sf vec1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec2 = {4.0f, 3.0f, 2.0f, 1.0f};
    v2df dvec1 = {1.0, 2.0};
    v2df dvec2 = {2.0, 1.0};
    
    /* Use argc to prevent excessive loop unrolling */
    volatile int iterations = (argc > 1) ? argc : 4;
    if (iterations > 10) iterations = 10;
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Vary inputs slightly each iteration */
        float f1 = float_data[iter % 12] + (iter * 0.1f);
        float f2 = float_data[(iter + 1) % 12] - (iter * 0.1f);
        float f3 = float_data[(iter + 2) % 12] * (1.0f + iter * 0.01f);
        float f4 = float_data[(iter + 3) % 12] / (1.0f + iter * 0.01f);
        double d1 = double_data[iter % 8] + (iter * 0.01);
        double d2 = double_data[(iter + 1) % 8] - (iter * 0.01);
        
        /* Call test functions with varying inputs */
        total_sum += test_unordered_comparisons(f1, f2, d1, d2);
        total_sum += test_mixed_conditionals(f1, f2, f3, f4, 
                                           float_data[(iter + 4) % 12],
                                           float_data[(iter + 5) % 12]);
        total_sum += test_vector_comparisons(vec1, vec2, dvec1, dvec2);
        
        #ifdef __AVX__
        __m128 avx1 = _mm_set_ps(f1, f2, f3, f4);
        __m128 avx2 = _mm_set_ps(f2, f3, f4, f1);
        total_sum += test_avx_intrinsics(avx1, avx2);
        #endif
        
        total_sum += test_comparison_chain(float_data, 
                                         (iter % 8) + 4);
    }
    
    /* Print result to ensure all code has observable effects */
    printf("Total checksum: %d\n", total_sum);
    
    return total_sum != 0 ? 0 : 1;
}
