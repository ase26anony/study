#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Vector types for SSE/AVX comparisons */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Prevent optimization */
static volatile int sink = 0;

/* Function to generate UNORDERED/ORDERED condition codes */
__attribute__((noinline))
static int test_unordered_ordered(float a, float b, double c, double d) {
    int sum = 0;
    
    /* Direct unordered checks - should generate "unord" */
    if (isunordered(a, b)) {
        sum += 1;
    }
    
    /* Ordered check - should generate "ord" */
    if (!isunordered(a, b)) {
        sum += 2;
    }
    
    /* NaN checks using self-comparison */
    if (a != a) {  /* true if a is NaN */
        sum += 4;
    }
    
    if (c == c) {  /* false if c is NaN */
        sum += 8;
    }
    
    /* Mixed ordered/unordered */
    if ((isunordered(a, b) || (c > d)) && !isunordered(c, d)) {
        sum += 16;
    }
    
    return sum;
}

/* Function to generate UNEQ/UNGE/UNGT/UNLE/UNLT condition codes */
__attribute__((noinline))
static int test_uneq_unge_ungt_unle_unlt(float a, float b, float c, float d) {
    int sum = 0;
    
    /* Complex conditional that may generate UNEQ (!= but unordered allowed) */
    if ((a == b) ? (c != d) : (a < b)) {
        sum += 1;
    }
    
    /* Chain that could generate UNGE (not less than, unordered allowed) */
    if (!(a < b) && !isunordered(a, b)) {
        sum += 2;
    }
    
    /* Could generate UNGT (not less or equal, unordered allowed) */
    if (!(a <= b) && (c >= d || isunordered(c, d))) {
        sum += 4;
    }
    
    /* Could generate UNLE (less or equal, unordered allowed) */
    if ((a <= b) || isunordered(a, b)) {
        sum += 8;
    }
    
    /* Could generate UNLT (less than, unordered allowed) */
    if ((a < b) || (isunordered(a, b) && (c > d))) {
        sum += 16;
    }
    
    return sum;
}

/* Function to generate LTGT condition code */
__attribute__((noinline))
static int test_ltgt(double a, double b, double c, double d) {
    int sum = 0;
    
    /* islessgreater generates LTGT (less or greater, but not equal, unordered not allowed) */
    if (islessgreater(a, b)) {
        sum += 1;
    }
    
    /* Complex expression that might compile to LTGT under fast-math */
    if ((a < b) != (a > b) && !isunordered(a, b)) {
        sum += 2;
    }
    
    /* Mixed comparisons that could yield LTGT */
    if ((a != b) && !isunordered(a, b)) {
        sum += 4;
    }
    
    /* Ternary with different comparisons */
    sum += ((c < d) ? 8 : 16) * !isunordered(c, d);
    
    return sum;
}

/* Vectorized comparisons using GCC vector extensions */
__attribute__((noinline))
static int test_vector_comparisons(v4sf va, v4sf vb, v2df vc, v2df vd) {
    int sum = 0;
    
    /* Vector comparisons generate various condition codes */
    v4sf cmp_unord = __builtin_ia32_cmpunordps(va, vb);
    v4sf cmp_ord = __builtin_ia32_cmpordps(va, vb);
    v4sf cmp_neq_uq = __builtin_ia32_cmpneqps(va, vb);  /* Not equal (unordered quiet) */
    
    /* Extract results to prevent elimination */
    float res[4];
    memcpy(res, &cmp_unord, sizeof(res));
    sum += (res[0] != 0.0f) * 1;
    
    memcpy(res, &cmp_ord, sizeof(res));
    sum += (res[1] != 0.0f) * 2;
    
    memcpy(res, &cmp_neq_uq, sizeof(res));
    sum += (res[2] != 0.0f) * 4;
    
    /* Double vector comparisons */
    v2df cmp_unord_d = __builtin_ia32_cmpunordpd(vc, vd);
    v2df cmp_ltgt_d = __builtin_ia32_cmpneqpd(vc, vd);  /* Not equal (ordered) */
    
    double dres[2];
    memcpy(dres, &cmp_unord_d, sizeof(dres));
    sum += (dres[0] != 0.0) * 8;
    
    memcpy(dres, &cmp_ltgt_d, sizeof(dres));
    sum += (dres[1] != 0.0) * 16;
    
    return sum;
}

/* SSE intrinsics version */
#ifdef __SSE__
__attribute__((noinline))
static int test_sse_intrinsics(__m128 a, __m128 b) {
    int sum = 0;
    
    /* Generate various condition codes */
    __m128 cmp_unord = _mm_cmpunord_ps(a, b);
    __m128 cmp_ord = _mm_cmpord_ps(a, b);
    __m128 cmp_neq_uq = _mm_cmpneq_ps(a, b);
    __m128 cmp_nlt = _mm_cmpnlt_ps(a, b);  /* UNGE */
    __m128 cmp_nle = _mm_cmpnle_ps(a, b);  /* UNGT */
    
    /* Extract mask bits */
    int mask_unord = _mm_movemask_ps(cmp_unord);
    int mask_ord = _mm_movemask_ps(cmp_ord);
    int mask_neq = _mm_movemask_ps(cmp_neq_uq);
    int mask_nlt = _mm_movemask_ps(cmp_nlt);
    int mask_nle = _mm_movemask_ps(cmp_nle);
    
    sum = mask_unord + (mask_ord << 4) + (mask_neq << 8) + 
          (mask_nlt << 12) + (mask_nle << 16);
    
    return sum;
}
#endif

/* Main test driver */
int main(int argc, char **argv) {
    int total_sum = 0;
    
    /* Patterned data including NaN values */
    float fdata[] = {
        1.0f, 2.0f, 3.0f, 4.0f,
        0.0f, -0.0f, __builtin_nanf(""), 5.0f,
        __builtin_inff(), -__builtin_inff(), 6.0f, 7.0f
    };
    
    double ddata[] = {
        1.0, 2.0, __builtin_nan(""), 4.0,
        0.0, -0.0, 3.0, __builtin_nan(""),
        __builtin_inf(), -__builtin_inf(), 5.0, 6.0
    };
    
    /* Use argc to prevent excessive unrolling */
    int iterations = (argc > 1) ? (argc % 8) + 4 : 8;
    
    for (int i = 0; i < iterations; i++) {
        int idx = i % 12;
        int idx2 = (i + 1) % 12;
        int idx3 = (i + 2) % 12;
        int idx4 = (i + 3) % 12;
        
        /* Test 1: Unordered/Ordered condition codes */
        total_sum += test_unordered_ordered(
            fdata[idx], fdata[idx2],
            ddata[idx3], ddata[idx4]
        );
        
        /* Test 2: UNEQ/UNGE/UNGT/UNLE/UNLT */
        total_sum += test_uneq_unge_ungt_unle_unlt(
            fdata[idx], fdata[idx2],
            fdata[idx3], fdata[idx4]
        );
        
        /* Test 3: LTGT */
        total_sum += test_ltgt(
            ddata[idx], ddata[idx2],
            ddata[idx3], ddata[idx4]
        );
        
        /* Test 4: Vector comparisons */
        v4sf va = {fdata[idx], fdata[idx2], fdata[idx3], fdata[idx4]};
        v4sf vb = {fdata[idx2], fdata[idx3], fdata[idx4], fdata[idx]};
        v2df vc = {ddata[idx], ddata[idx2]};
        v2df vd = {ddata[idx2], ddata[idx]};
        
        total_sum += test_vector_comparisons(va, vb, vc, vd);
        
        #ifdef __SSE__
        /* Test 5: SSE intrinsics */
        __m128 a = _mm_set_ps(fdata[idx4], fdata[idx3], fdata[idx2], fdata[idx]);
        __m128 b = _mm_set_ps(fdata[idx], fdata[idx4], fdata[idx3], fdata[idx2]);
        total_sum += test_sse_intrinsics(a, b);
        #endif
        
        /* Prevent loop optimization */
        sink = total_sum;
    }
    
    /* Complex conditional with mixed operators */
    float x = fdata[0], y = fdata[1];
    double u = ddata[0], v = ddata[1];
    
    for (int i = 0; i < 4; i++) {
        /* This complex expression may generate multiple condition codes */
        if ((x < y) ? (u != v) : (x >= y)) {
            total_sum += i;
        }
        
        if ((isunordered(x, y) || (u > v)) && !(x == y)) {
            total_sum += i * 2;
        }
        
        if (!(x <= y) && !isunordered(x, y) && (u < v || isunordered(u, v))) {
            total_sum += i * 4;
        }
        
        /* Modify values to change comparison results */
        x += 1.0f;
        y -= 0.5f;
        u *= 1.1;
        v /= 1.1;
    }
    
    printf("Result checksum: %d\n", total_sum);
    return total_sum != 0 ? 0 : 1;
}
