#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>

#ifdef __AVX__
#include <immintrin.h>
#endif

/* Vector types for triggering vector comparisons */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Prevent optimization of inputs */
static volatile float vf1, vf2;
static volatile double vd1, vd2;

/* Test function 1: Direct unordered comparisons using math.h macros */
__attribute__((noinline))
static int test_unordered_comparisons(float a, float b, double c, double d) {
    int sum = 0;
    
    /* These should generate UNORDERED/ORDERED condition codes */
    sum += isunordered(a, b) ? 1 : 0;
    sum += isordered(a, b) ? 2 : 0;
    
    /* UNEQ: unordered or equal */
    sum += !(c > d) && !(c < d) ? 4 : 0;  /* a == b or unordered */
    
    /* UNGE: not less than (greater or equal or unordered) */
    sum += !(c < d) ? 8 : 0;
    
    /* UNGT: not less or equal (greater or unordered) */
    sum += !(c <= d) ? 16 : 0;
    
    /* UNLE: less or equal or unordered */
    sum += !(c > d) ? 32 : 0;
    
    /* UNLT: less than or unordered */
    sum += !(c >= d) ? 64 : 0;
    
    /* LTGT: less than or greater than (ordered and not equal) */
    sum += islessgreater(c, d) ? 128 : 0;
    
    return sum;
}

/* Test function 2: Mixed comparisons in conditional expressions */
__attribute__((noinline))
static int test_mixed_comparisons(float a, float b, float c, float d, float e, float f) {
    int sum = 0;
    
    /* Complex conditional with different comparison types */
    for (int i = 0; i < 3; i++) {
        /* This ternary mixes different comparison operators */
        sum += ((a < b) ? (c != d) : (e >= f)) ? (1 << i) : 0;
        
        /* Another mixed expression */
        sum += ((a == b) ? (c > d) : (e <= f)) ? (2 << i) : 0;
        
        /* Chain of comparisons */
        sum += (a != b) && (c < d) || (e > f) ? (4 << i) : 0;
    }
    
    return sum;
}

/* Test function 3: Vector comparisons using GCC vector extensions */
__attribute__((noinline))
static int test_vector_comparisons(v4sf va, v4sf vb, v2df vc, v2df vd) {
    int sum = 0;
    
    /* Vector comparisons that should generate condition codes */
    v4sf cmp_result;
    
    /* Unordered comparison */
    cmp_result = va == vb;  /* May generate UNEQ in fast-math */
    sum += ((int*)&cmp_result)[0] != 0 ? 1 : 0;
    
    /* Ordered less-than */
    cmp_result = va < vb;
    sum += ((int*)&cmp_result)[1] != 0 ? 2 : 0;
    
    /* Not less than or equal (UNGT with fast-math) */
    cmp_result = !(va <= vb);
    sum += ((int*)&cmp_result)[2] != 0 ? 4 : 0;
    
    /* Double vector comparisons */
    v2df dbl_cmp = vc > vd;
    sum += ((int64_t*)&dbl_cmp)[0] != 0 ? 8 : 0;
    
    dbl_cmp = vc != vd;  /* May generate LTGT/UNE */
    sum += ((int64_t*)&dbl_cmp)[1] != 0 ? 16 : 0;
    
    return sum;
}

/* Test function 4: NaN checks and fast-math optimizations */
__attribute__((noinline))
static int test_nan_fastmath(float a, float b, double c, double d) {
    int sum = 0;
    
    /* NaN checks - these may generate UNORDERED comparisons */
    sum += a != a ? 1 : 0;      /* true if a is NaN */
    sum += !(c == c) ? 2 : 0;   /* true if c is NaN */
    
    /* With fast-math, these may use UNEQ/LTGT */
    sum += (a == b) ? 4 : 0;
    sum += (c != d) ? 8 : 0;
    
    /* Mixed NaN and normal comparisons */
    float temp = vf1;
    double dtemp = vd1;
    sum += (temp == a) && (dtemp != c) ? 16 : 0;
    sum += (temp < b) || (dtemp > d) ? 32 : 0;
    
    return sum;
}

#ifdef __SSE__
/* Test function 5: SSE intrinsics for direct control */
__attribute__((noinline))
static int test_sse_intrinsics(__m128 a, __m128 b) {
    int sum = 0;
    __m128 cmp;
    
    /* CMPUNORDPS - unordered comparison */
    cmp = _mm_cmpunord_ps(a, b);
    sum += _mm_movemask_ps(cmp) & 1 ? 1 : 0;
    
    /* CMPORDPS - ordered comparison */
    cmp = _mm_cmpord_ps(a, b);
    sum += _mm_movemask_ps(cmp) & 2 ? 2 : 0;
    
    /* CMPNEQPS - not equal (may use UNEQ/LTGT) */
    cmp = _mm_cmpneq_ps(a, b);
    sum += _mm_movemask_ps(cmp) & 4 ? 4 : 0;
    
    /* CMPNLTPS - not less than (UNGE) */
    cmp = _mm_cmpnlt_ps(a, b);
    sum += _mm_movemask_ps(cmp) & 8 ? 8 : 0;
    
    /* CMPNLEPS - not less or equal (UNGT) */
    cmp = _mm_cmpnle_ps(a, b);
    sum += _mm_movemask_ps(cmp) & 16 ? 16 : 0;
    
    return sum;
}
#endif

int main(int argc, char *argv[]) {
    int total_sum = 0;
    
    /* Initialize test data with pattern including potential NaN */
    float fa[8], fb[8];
    double da[8], db[8];
    
    for (int i = 0; i < 8; i++) {
        fa[i] = (i - 3) * 1.5f;
        fb[i] = (i - 4) * 2.0f;
        da[i] = (i - 2) * 1.7;
        db[i] = (i - 5) * 2.3;
    }
    
    /* Insert some NaN values if not using -ffast-math */
#ifndef __FAST_MATH__
    fa[2] = __builtin_nanf("");
    da[3] = __builtin_nan("");
#endif
    
    /* Initialize vector data */
    v4sf vfa = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vfb = {4.0f, 3.0f, 2.0f, 1.0f};
    v2df vda = {1.0, 2.0};
    v2df vdb = {2.0, 1.0};
    
    /* Use argc to prevent loop unrolling from eliminating comparisons */
    int iterations = (argc > 1) ? (argc % 5) + 1 : 3;
    
    for (int iter = 0; iter < iterations; iter++) {
        int idx = iter % 8;
        
        /* Test 1: Unordered comparisons */
        total_sum += test_unordered_comparisons(fa[idx], fb[idx], da[idx], db[idx]);
        
        /* Test 2: Mixed comparisons */
        total_sum += test_mixed_comparisons(fa[idx], fb[idx], 
                                          fa[(idx+1)%8], fb[(idx+1)%8],
                                          fa[(idx+2)%8], fb[(idx+2)%8]);
        
        /* Test 3: Vector comparisons */
        total_sum += test_vector_comparisons(vfa, vfb, vda, vdb);
        
        /* Test 4: NaN and fast-math tests */
        total_sum += test_nan_fastmath(fa[idx], fb[idx], da[idx], db[idx]);
        
#ifdef __SSE__
        /* Test 5: SSE intrinsics */
        __m128 sse_a = _mm_set_ps(fa[0], fa[1], fa[2], fa[3]);
        __m128 sse_b = _mm_set_ps(fb[0], fb[1], fb[2], fb[3]);
        total_sum += test_sse_intrinsics(sse_a, sse_b);
#endif
        
        /* Modify data slightly each iteration to prevent constant folding */
        vf1 = fa[idx] * 0.9f;
        vd1 = da[idx] * 0.9;
    }
    
    printf("Total checksum: %d\n", total_sum);
    return total_sum != 0 ? 0 : 1;
}
