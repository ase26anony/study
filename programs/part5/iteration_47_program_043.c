/* Compile with: gcc -O3 -ffast-math -march=x86-64 -mavx2 -fdump-rtl-final -o test_conds test_conds.c -lm */
#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <immintrin.h>

/* Prevent optimization of critical functions */
#define NOINLINE __attribute__((noinline))

/* Vector types for AVX comparisons */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Patterned data with NaN values */
static float fdata[16] = {
    1.0f, 2.0f, 3.0f, 4.0f,
    0.0f, -0.0f, __builtin_nanf(""), 8.0f,
    9.0f, 10.0f, 11.0f, 12.0f,
    __builtin_inff(), -__builtin_inff(), 15.0f, 16.0f
};

static double ddata[16] = {
    1.0, 2.0, 3.0, 4.0,
    0.0, -0.0, __builtin_nan(""), 8.0,
    9.0, 10.0, 11.0, 12.0,
    __builtin_inf(), -__builtin_inf(), 15.0, 16.0
};

/* Function 1: Direct unordered comparisons using math.h macros */
NOINLINE static int test_unordered_comparisons(float a, float b, double c, double d) {
    int sum = 0;
    
    /* These should generate UNORDERED condition codes */
    sum += isunordered(a, b) ? 1 : 0;
    sum += isunordered(c, d) ? 2 : 0;
    
    /* ORDERED condition codes */
    sum += isordered(a, b) ? 4 : 0;
    sum += isordered(c, d) ? 8 : 0;
    
    /* UNEQ (unordered or equal) */
    sum += !(a < b) && !(a > b) ? 16 : 0;  /* May become UNEQ with fast-math */
    
    return sum;
}

/* Function 2: Mixed comparison types in conditional expressions */
NOINLINE static int test_mixed_conditionals(float a, float b, float c, float d, float e, float f) {
    int sum = 0;
    
    /* Complex conditional with different comparison types */
    sum += ((a < b) ? (c != d) : (e >= f)) ? 1 : 0;
    
    /* Nested ternary with unordered checks */
    sum += (isunordered(a, b) ? (c == d) : (e != f)) ? 2 : 0;
    
    /* Chain of comparisons that may generate LTGT */
    sum += ((a != b) && (a < b || a > b)) ? 4 : 0;  /* Could become LTGT with fast-math */
    
    return sum;
}

/* Function 3: Vectorized comparisons using GCC vector extensions */
NOINLINE static int test_vector_comparisons(void) {
    v4sf v1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf v2 = {4.0f, 3.0f, 2.0f, 1.0f};
    v4sf v3 = {1.0f, __builtin_nanf(""), 3.0f, 4.0f};
    
    int sum = 0;
    
    /* Element-wise comparisons - may generate various condition codes */
    v4sf cmp1 = v1 < v2;    /* Less than */
    v4sf cmp2 = v1 == v3;   /* Equal (with NaN) */
    v4sf cmp3 = v1 != v3;   /* Not equal (with NaN) */
    
    /* Extract results to prevent elimination */
    float* f1 = (float*)&cmp1;
    float* f2 = (float*)&cmp2;
    float* f3 = (float*)&cmp3;
    
    for (int i = 0; i < 4; i++) {
        sum += (f1[i] != 0.0f) ? (1 << i) : 0;
        sum += (f2[i] != 0.0f) ? (1 << (i + 4)) : 0;
        sum += (f3[i] != 0.0f) ? (1 << (i + 8)) : 0;
    }
    
    return sum;
}

/* Function 4: AVX intrinsics for explicit unordered comparisons */
#ifdef __AVX__
NOINLINE static int test_avx_intrinsics(void) {
    __m128 a = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    __m128 b = _mm_set_ps(4.0f, 3.0f, 2.0f, 1.0f);
    __m128 c = _mm_set_ps(1.0f, __builtin_nanf(""), 3.0f, 4.0f);
    
    int sum = 0;
    
    /* These intrinsics map directly to condition codes */
    __m128 cmp1 = _mm_cmp_ps(a, b, _CMP_UNORD_Q);   /* Unordered */
    __m128 cmp2 = _mm_cmp_ps(a, c, _CMP_NEQ_UQ);    /* Not equal unordered */
    __m128 cmp3 = _mm_cmp_ps(a, b, _CMP_NLE_UQ);    /* Not less or equal unordered */
    __m128 cmp4 = _mm_cmp_ps(a, b, _CMP_NLT_UQ);    /* Not less than unordered */
    
    /* Extract mask bits */
    int mask1 = _mm_movemask_ps(cmp1);
    int mask2 = _mm_movemask_ps(cmp2);
    int mask3 = _mm_movemask_ps(cmp3);
    int mask4 = _mm_movemask_ps(cmp4);
    
    sum = mask1 + mask2 * 16 + mask3 * 256 + mask4 * 4096;
    
    return sum;
}
#endif

/* Function 5: Chain of floating-point comparisons with multiple parameters */
NOINLINE static int test_comparison_chain(float a, float b, float c, float d, 
                                          float e, float f, float g, float h) {
    int sum = 0;
    
    /* Chain that may generate UNGE, UNGT, UNLE, UNLT */
    sum += (a >= b) ? 1 : 0;      /* May become UNGE with fast-math */
    sum += (c > d) ? 2 : 0;       /* May become UNGT */
    sum += (e <= f) ? 4 : 0;      /* May become UNLE */
    sum += (g < h) ? 8 : 0;       /* May become UNLT */
    
    /* Combined comparisons for LTGT */
    sum += ((a != b) && (c != d)) ? 16 : 0;
    
    /* Explicit NaN checks */
    sum += (a != a) ? 32 : 0;     /* Always false unless a is NaN */
    sum += !(b == b) ? 64 : 0;    /* Another NaN check */
    
    return sum;
}

/* Function 6: Fast-math optimized comparisons */
NOINLINE static int test_fast_math_optimizations(float a, float b, double c, double d) {
    int sum = 0;
    
    /* With -ffast-math, these may use different condition codes */
    sum += (a == b) ? 1 : 0;      /* May become UNEQ */
    sum += (a != b) ? 2 : 0;      /* May become LTGT */
    sum += (c < d) ? 4 : 0;       /* Ordered less than */
    sum += (c >= d) ? 8 : 0;      /* May become UNGE */
    
    /* Complex expression that fast-math might transform */
    sum += ((a < b) || (a > b)) ? 16 : 0;  /* Could become ordered comparison */
    
    return sum;
}

int main(int argc, char** argv) {
    /* Use argc to prevent excessive loop unrolling */
    int iterations = (argc > 1) ? atoi(argv[1]) : 100;
    if (iterations < 1) iterations = 100;
    
    int total_sum = 0;
    
    /* Volatile counter to prevent optimization */
    volatile int vol_iter = iterations;
    
    for (int i = 0; i < vol_iter; i++) {
        /* Use different data patterns each iteration */
        int idx = i % 12;
        
        /* Test 1: Unordered comparisons */
        total_sum += test_unordered_comparisons(
            fdata[idx], fdata[idx + 1], 
            ddata[idx], ddata[idx + 2]
        );
        
        /* Test 2: Mixed conditionals */
        total_sum += test_mixed_conditionals(
            fdata[idx], fdata[idx + 1], fdata[idx + 2],
            fdata[idx + 3], fdata[idx + 4], fdata[idx + 5]
        );
        
        /* Test 3: Vector comparisons */
        if (i % 3 == 0) {
            total_sum += test_vector_comparisons();
        }
        
        #ifdef __AVX__
        /* Test 4: AVX intrinsics */
        if (i % 4 == 0) {
            total_sum += test_avx_intrinsics();
        }
        #endif
        
        /* Test 5: Comparison chain */
        total_sum += test_comparison_chain(
            fdata[idx], fdata[idx + 1], fdata[idx + 2], fdata[idx + 3],
            fdata[idx + 4], fdata[idx + 5], fdata[idx + 6], fdata[idx + 7]
        );
        
        /* Test 6: Fast-math optimizations */
        total_sum += test_fast_math_optimizations(
            fdata[idx + 2], fdata[idx + 3],
            ddata[idx + 1], ddata[idx + 4]
        );
    }
    
    printf("Final checksum: %d\n", total_sum);
    return 0;
}
