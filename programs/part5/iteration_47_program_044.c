/* test_condition_codes.c */
#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Prevent optimization of critical functions */
#define NOINLINE __attribute__((noinline))
#define VOLATILE_MATH volatile

/* Vector types for SSE/AVX operations */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Test 1: Direct unordered comparisons using math.h macros */
NOINLINE static int test_unordered_comparisons(float a, float b, double c, double d) {
    int sum = 0;
    
    /* These should generate UNORDERED/ORDERED condition codes */
    sum += isunordered(a, b) ? 1 : 0;
    sum += isordered(a, b) ? 2 : 0;
    
    /* UNEQ - unordered or equal */
    sum += !isgreater(c, d) && !isless(c, d) ? 4 : 0;
    
    /* UNGE - unordered or greater or equal (not less than) */
    sum += !isless(c, d) ? 8 : 0;
    
    /* UNGT - unordered or greater (not less or equal) */
    sum += !islessequal(c, d) ? 16 : 0;
    
    /* UNLE - unordered or less or equal */
    sum += !isgreater(c, d) ? 32 : 0;
    
    /* UNLT - unordered or less than */
    sum += !isgreaterequal(c, d) ? 64 : 0;
    
    /* LTGT - less or greater (unordered equal) */
    sum += islessgreater(a, b) ? 128 : 0;
    
    return sum;
}

/* Test 2: Mixed comparisons in conditional expressions */
NOINLINE static int test_mixed_comparisons(float f1, float f2, float f3, float f4,
                                          double d1, double d2) {
    int result = 0;
    
    /* Complex conditional with mixed operators */
    for (int i = 0; i < 3; i++) {
        /* This should generate various condition codes */
        if ((f1 < f2) ? (f3 != f4) : (f1 >= f2)) {
            result += 1;
        }
        
        /* Ternary with different comparison types */
        float temp = (d1 == d2) ? f1 : 
                    ((d1 > d2) ? f2 : 
                    ((d1 < d2) ? f3 : f4));
        VOLATILE_MATH float vtemp = temp;
        result += (int)vtemp;
        
        /* Chain of comparisons */
        if (!(f1 == f1) || (f2 > f3) || (f4 <= f1)) {
            result += 2;
        }
    }
    
    return result;
}

/* Test 3: Vectorized comparisons using GCC vector extensions */
NOINLINE static int test_vector_comparisons(v4sf vec1, v4sf vec2, v2df dvec1, v2df dvec2) {
    int sum = 0;
    
    /* Vector comparisons generate UNORD/ORD/etc condition codes */
    v4sf cmp_unord = vec1 != vec1;  /* NaN check - should use UNORD */
    v4sf cmp_eq = vec1 == vec2;     /* Equality - may use UNEQ with fast-math */
    v4sf cmp_neq = vec1 != vec2;    /* Inequality - may use LTGT */
    v4sf cmp_lt = vec1 < vec2;      /* Less than */
    v4sf cmp_le = vec1 <= vec2;     /* Less or equal */
    v4sf cmp_gt = vec1 > vec2;      /* Greater than */
    v4sf cmp_ge = vec1 >= vec2;     /* Greater or equal */
    
    /* Extract results to prevent elimination */
    float temp[4];
    memcpy(temp, &cmp_unord, sizeof(temp));
    for (int i = 0; i < 4; i++) {
        sum += (temp[i] != 0.0f) ? (1 << i) : 0;
    }
    
    memcpy(temp, &cmp_eq, sizeof(temp));
    for (int i = 0; i < 4; i++) {
        sum += (temp[i] != 0.0f) ? (1 << (i + 4)) : 0;
    }
    
    /* Double vector comparisons */
    double dtemp[2];
    v2df dcmp = dvec1 <= dvec2;  /* UNLE condition code */
    memcpy(dtemp, &dcmp, sizeof(dtemp));
    sum += (dtemp[0] != 0.0) ? 256 : 0;
    sum += (dtemp[1] != 0.0) ? 512 : 0;
    
    return sum;
}

/* Test 4: SSE/AVX intrinsics for explicit unordered comparisons */
#ifdef __SSE__
NOINLINE static int test_sse_comparisons(__m128 a, __m128 b) {
    int sum = 0;
    
    /* These intrinsics map directly to condition codes */
    __m128 cmp_unord = _mm_cmpunord_ps(a, b);  /* UNORDERED */
    __m128 cmp_ord = _mm_cmpord_ps(a, b);      /* ORDERED */
    __m128 cmp_neq = _mm_cmpneq_ps(a, b);      /* NEQ (may use UNEQ/LTGT) */
    __m128 cmp_nlt = _mm_cmpnlt_ps(a, b);      /* UNGE */
    __m128 cmp_nle = _mm_cmpnle_ps(a, b);      /* UNGT */
    __m128 cmp_ule = _mm_cmpule_ps(a, b);      /* UNLE - Note: SSE doesn't have direct UNLE */
    
    /* Extract mask bits */
    int mask_unord = _mm_movemask_ps(cmp_unord);
    int mask_ord = _mm_movemask_ps(cmp_ord);
    int mask_neq = _mm_movemask_ps(cmp_neq);
    int mask_nlt = _mm_movemask_ps(cmp_nlt);
    int mask_nle = _mm_movemask_ps(cmp_nle);
    
    sum = mask_unord + (mask_ord << 4) + (mask_neq << 8) + 
          (mask_nlt << 12) + (mask_nle << 16);
    
    return sum;
}
#endif

/* Test 5: Fast-math optimized comparisons */
NOINLINE static int test_fast_math_comparisons(float a, float b, float c, float d) {
    int result = 0;
    
    /* With -ffast-math, these may generate UNEQ/LTGT codes */
    if (a == b) result += 1;      /* May become UNEQ */
    if (a != b) result += 2;      /* May become LTGT */
    if (a < b) result += 4;       /* Regular LT */
    if (a > b) result += 8;       /* Regular GT */
    if (a <= b) result += 16;     /* May become UNLE */
    if (a >= b) result += 32;     /* May become UNGE */
    
    /* Mixed comparisons that fast-math can optimize */
    float temp = (a < b) ? c : d;
    if (temp == a || temp == b) {
        result += 64;
    }
    
    /* Check for NaN using unordered comparison */
    if (a != a) result += 128;    /* Always true for NaN - uses UNORD */
    if (b == b) result += 256;    /* Always true for non-NaN - uses ORD */
    
    return result;
}

/* Main test driver */
int main(int argc, char *argv[]) {
    int total_sum = 0;
    
    /* Patterned data including NaN values */
    float float_data[] = {
        1.0f, 2.0f, 3.0f, 4.0f,
        0.0f, -0.0f, __builtin_nanf(""), INFINITY,
        5.0f, 6.0f, 7.0f, 8.0f
    };
    
    double double_data[] = {
        1.0, 2.0, __builtin_nan(""), 4.0,
        0.0, -0.0, INFINITY, -INFINITY
    };
    
    /* Use argc to prevent loop unrolling */
    int iterations = (argc > 1) ? (argc % 4) + 1 : 3;
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Test 1: Unordered comparisons */
        int idx = iter % 8;
        total_sum += test_unordered_comparisons(
            float_data[idx], float_data[idx + 1],
            double_data[idx % 6], double_data[(idx + 1) % 6]
        );
        
        /* Test 2: Mixed comparisons */
        total_sum += test_mixed_comparisons(
            float_data[0], float_data[1], float_data[2], float_data[3],
            double_data[0], double_data[1]
        );
        
        /* Test 3: Vector comparisons */
        v4sf vec1 = {float_data[0], float_data[1], float_data[2], float_data[3]};
        v4sf vec2 = {float_data[4], float_data[5], float_data[6], float_data[7]};
        v2df dvec1 = {double_data[0], double_data[1]};
        v2df dvec2 = {double_data[2], double_data[3]};
        total_sum += test_vector_comparisons(vec1, vec2, dvec1, dvec2);
        
        /* Test 4: SSE comparisons */
        #ifdef __SSE__
        __m128 sse_vec1 = _mm_set_ps(float_data[3], float_data[2], 
                                    float_data[1], float_data[0]);
        __m128 sse_vec2 = _mm_set_ps(float_data[7], float_data[6], 
                                    float_data[5], float_data[4]);
        total_sum += test_sse_comparisons(sse_vec1, sse_vec2);
        #endif
        
        /* Test 5: Fast-math comparisons */
        total_sum += test_fast_math_comparisons(
            float_data[idx], float_data[(idx + 2) % 8],
            float_data[(idx + 4) % 8], float_data[(idx + 6) % 8]
        );
        
        /* Modify data slightly each iteration */
        float_data[idx] += 0.5f;
        double_data[idx % 6] += 0.25;
    }
    
    printf("Total checksum: %d\n", total_sum);
    return total_sum != 0 ? 0 : 1;
}
