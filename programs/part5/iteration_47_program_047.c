#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <immintrin.h>

/* Vector types for SSE/AVX comparisons */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Prevent optimization of inputs */
static volatile float vf1, vf2;
static volatile double vd1, vd2;

/* Test function for unordered comparisons */
__attribute__((noinline))
static int test_unordered(float a, float b, double c, double d) {
    int sum = 0;
    
    /* Direct unordered checks - should generate "unord" */
    if (isunordered(a, b)) sum |= 1;
    if (!isunordered(c, d)) sum |= 2;  /* Should generate "ord" */
    
    /* NaN checks using self-comparison */
    if (a != a) sum |= 4;      /* true if a is NaN */
    if (!(c == c)) sum |= 8;   /* true if c is NaN */
    
    /* Mixed ordered/unordered in conditional */
    float t = (isunordered(a, b) ? b : a);
    sum += (int)(t * 100);
    
    return sum;
}

/* Test function for UNEQ/UNGE/UNGT/UNLE/UNLT comparisons */
__attribute__((noinline))
static int test_uneq_unge(float a, float b, float c, float d) {
    int sum = 0;
    
    /* Complex conditional that may generate UNEQ */
    if ((a == b) || isunordered(a, b)) sum |= 1;
    
    /* May generate UNGE (not less than) */
    if (!(a < b) || isunordered(a, b)) sum |= 2;
    
    /* May generate UNGT (not less or equal) */
    if (!(a <= b) || isunordered(a, b)) sum |= 4;
    
    /* May generate UNLE (unordered or less or equal) */
    if ((a <= b) || isunordered(a, b)) sum |= 8;
    
    /* May generate UNLT (unordered or less than) */
    if ((a < b) || isunordered(a, b)) sum |= 16;
    
    /* Ternary with mixed comparisons */
    float r = (a < b) ? ((c >= d) || isunordered(c, d) ? c : d) : a;
    sum += (int)(r * 10);
    
    return sum;
}

/* Test function for LTGT (unordered or not equal) */
__attribute__((noinline))
static int test_ltgt(double a, double b, double c, double d) {
    int sum = 0;
    
    /* Using islessgreater macro - should map to LTGT */
    if (islessgreater(a, b)) sum |= 1;
    
    /* Alternative formulation */
    if ((a < b) || (a > b)) sum |= 2;
    
    /* Complex chain */
    if ((a != b) && !isunordered(a, b)) sum |= 4;
    
    /* Mixed in conditional expression */
    double t = (islessgreater(a, b) ? 
               (isunordered(c, d) ? c : d) : 
               (c == d ? c : d));
    sum += (int)(t * 1000);
    
    return sum;
}

/* Vectorized comparison tests using GCC vector extensions */
__attribute__((noinline))
static int test_vector_comparisons(v4sf va, v4sf vb) {
    int sum = 0;
    
    /* Element-wise unordered check */
    v4sf mask_unord = __builtin_ia32_cmpunordps(va, vb);
    sum += ((int*)&mask_unord)[0] & 1;
    
    /* Element-wise ordered check */
    v4sf mask_ord = __builtin_ia32_cmpordps(va, vb);
    sum += ((int*)&mask_ord)[1] & 2;
    
    /* Not equal unordered */
    v4sf mask_neq_uq = __builtin_ia32_cmpneqps(va, vb);
    sum += ((int*)&mask_neq_uq)[2] & 4;
    
    /* Greater or equal unordered */
    v4sf mask_ge_uq = __builtin_ia32_cmpnltps(va, vb);
    sum += ((int*)&mask_ge_uq)[3] & 8;
    
    return sum;
}

/* AVX intrinsics for more condition codes */
#ifdef __AVX__
__attribute__((noinline))
static int test_avx_comparisons(__m256 va, __m256 vb) {
    int sum = 0;
    
    /* _CMP_UNORD_Q - unordered (non-signaling) */
    __m256 mask_unord = _mm256_cmp_ps(va, vb, _CMP_UNORD_Q);
    sum += _mm256_movemask_ps(mask_unord) & 0xF;
    
    /* _CMP_NEQ_UQ - not equal unordered (non-signaling) */
    __m256 mask_neq_uq = _mm256_cmp_ps(va, vb, _CMP_NEQ_UQ);
    sum += _mm256_movemask_ps(mask_neq_uq) & 0xF0;
    
    /* _CMP_NLT_UQ - not less than unordered (non-signaling) */
    __m256 mask_nlt_uq = _mm256_cmp_ps(va, vb, _CMP_NLT_UQ);
    sum += _mm256_movemask_ps(mask_nlt_uq) << 4;
    
    /* _CMP_NLE_UQ - not less or equal unordered (non-signaling) */
    __m256 mask_nle_uq = _mm256_cmp_ps(va, vb, _CMP_NLE_UQ);
    sum += _mm256_movemask_ps(mask_nle_uq) << 8;
    
    return sum;
}
#endif

/* Complex function with mixed comparison types */
__attribute__((noinline))
static int test_mixed_comparisons(float a, float b, float c, float d, 
                                  double e, double f, double g, double h) {
    int sum = 0;
    
    /* Chain of different comparisons */
    sum += (a < b) ? ((c != d) ? 1 : 2) : ((e >= f) ? 3 : 4);
    
    /* Nested conditionals with unordered checks */
    if (isunordered(a, b)) {
        sum += (g == h) ? 5 : 6;
    } else {
        sum += (islessgreater(e, f) ? 7 : 8);
    }
    
    /* Loop with varying comparisons */
    for (int i = 0; i < 4; i++) {
        switch (i) {
            case 0: sum += isunordered(a + i, b) ? 1 : 0; break;
            case 1: sum += (c != d + i) ? 2 : 0; break;
            case 2: sum += (e < f + i) ? 4 : 0; break;
            case 3: sum += islessgreater(g, h + i) ? 8 : 0; break;
        }
    }
    
    return sum;
}

int main(int argc, char **argv) {
    int total_sum = 0;
    
    /* Patterned data including NaN values */
    float fdata[] = {1.0f, 2.0f, 0.0f, -0.0f, __builtin_nanf(""), 3.0f, 4.0f, 5.0f};
    double ddata[] = {1.0, 2.0, 0.0, -0.0, __builtin_nan(""), 3.0, 4.0, 5.0};
    
    /* Use argc to prevent excessive loop unrolling */
    int iterations = (argc > 1) ? (argc & 7) : 8;
    
    for (int i = 0; i < iterations; i++) {
        /* Volatile to prevent constant propagation */
        vf1 = fdata[i % 8];
        vf2 = fdata[(i + 1) % 8];
        vd1 = ddata[i % 8];
        vd2 = ddata[(i + 2) % 8];
        
        /* Call all test functions */
        total_sum += test_unordered(vf1, vf2, vd1, vd2);
        total_sum += test_uneq_unge(vf1, vf2, fdata[(i + 3) % 8], fdata[(i + 4) % 8]);
        total_sum += test_ltgt(vd1, vd2, ddata[(i + 5) % 8], ddata[(i + 6) % 8]);
        
        /* Vector tests */
        v4sf va = {vf1, vf2, fdata[(i + 2) % 8], fdata[(i + 3) % 8]};
        v4sf vb = {vf2, vf1, fdata[(i + 4) % 8], fdata[(i + 5) % 8]};
        total_sum += test_vector_comparisons(va, vb);
        
        #ifdef __AVX__
        __m256 avx_a = _mm256_set_ps(vf1, vf2, fdata[0], fdata[1], 
                                     fdata[2], fdata[3], fdata[4], fdata[5]);
        __m256 avx_b = _mm256_set_ps(vf2, vf1, fdata[1], fdata[2], 
                                     fdata[3], fdata[4], fdata[5], fdata[6]);
        total_sum += test_avx_comparisons(avx_a, avx_b);
        #endif
        
        /* Mixed comparisons with more arguments */
        total_sum += test_mixed_comparisons(
            vf1, vf2, fdata[(i + 1) % 8], fdata[(i + 2) % 8],
            vd1, vd2, ddata[(i + 3) % 8], ddata[(i + 4) % 8]
        );
    }
    
    printf("Total checksum: %d\n", total_sum);
    return total_sum != 0 ? 0 : 1;
}
