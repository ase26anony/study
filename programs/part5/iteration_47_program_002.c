/* 
 * Program to trigger x86 condition code mnemonics in i386.cc
 * Compile with: gcc -O3 -ffast-math -mavx2 -fdump-rtl-final -o test_conds test_conds.c
 */

#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <immintrin.h>

/* Vector types for AVX/SSE */
typedef float v4sf __attribute__((vector_size(16)));
typedef float v8sf __attribute__((vector_size(32)));
typedef double v2df __attribute__((vector_size(16)));
typedef double v4df __attribute__((vector_size(32)));

/* Prevent optimization */
static volatile int sink = 0;

/* Test function 1: Direct unordered comparisons */
__attribute__((noinline))
static int test_unordered_comparisons(float a, float b, double c, double d) {
    int result = 0;
    
    /* Use standard macros that generate unordered comparisons */
    result += isunordered(a, b) ? 1 : 0;        /* Should generate UNORDERED */
    result += !isunordered(c, d) ? 2 : 0;       /* Should generate ORDERED */
    result += islessgreater(a, b) ? 4 : 0;      /* Should generate LTGT */
    
    /* Direct NaN checks */
    result += (a != a) ? 8 : 0;                 /* UNORDERED when a is NaN */
    result += !(c == c) ? 16 : 0;               /* UNORDERED when c is NaN */
    
    return result;
}

/* Test function 2: Mixed comparisons in conditional expressions */
__attribute__((noinline))
static int test_mixed_comparisons(float x, float y, float z, float w) {
    int result = 0;
    
    /* Complex conditional with different comparison types */
    if ((x < y) ? (z != w) : (x >= y)) {
        result += 1;  /* Mix of LT, NEQ, GE */
    }
    
    /* Nested ternary with unordered possibility */
    result += (x == y) ? ((z < w) ? 2 : 4) : ((z > w) ? 8 : 16);
    
    /* Chain of comparisons that might generate UNEQ/UNGE/etc */
    if ((x <= y) && !(z >= w) && (x != z)) {
        result += 32;
    }
    
    return result;
}

/* Test function 3: Vectorized comparisons using GCC vector extensions */
__attribute__((noinline))
static int test_vector_comparisons(v4sf va, v4sf vb, v2df vc, v2df vd) {
    int result = 0;
    
    /* Vector comparisons generate multiple condition codes */
    v4sf cmp1 = va < vb;    /* Element-wise less than */
    v4sf cmp2 = va == vb;   /* Element-wise equality */
    v4sf cmp3 = va != vb;   /* Element-wise inequality */
    
    /* Check results */
    for (int i = 0; i < 4; i++) {
        result += cmp1[i] ? (1 << i) : 0;
        result += cmp2[i] ? (1 << (i + 4)) : 0;
        result += cmp3[i] ? (1 << (i + 8)) : 0;
    }
    
    /* Double vector comparisons */
    v2df dcmp1 = vc <= vd;
    v2df dcmp2 = vc > vd;
    
    for (int i = 0; i < 2; i++) {
        result += dcmp1[i] ? (1 << (i + 12)) : 0;
        result += dcmp2[i] ? (1 << (i + 14)) : 0;
    }
    
    return result;
}

/* Test function 4: AVX intrinsics for explicit unordered comparisons */
#ifdef __AVX__
__attribute__((noinline))
static int test_avx_intrinsics(__m256 a, __m256 b, __m256d c, __m256d d) {
    int result = 0;
    
    /* Use intrinsics that map to unordered comparisons */
    __m256 cmp_unord = _mm256_cmp_ps(a, b, _CMP_UNORD_Q);   /* UNORDERED */
    __m256 cmp_ord = _mm256_cmp_ps(a, b, _CMP_ORD_Q);       /* ORDERED */
    __m256 cmp_neq_uq = _mm256_cmp_ps(a, b, _CMP_NEQ_UQ);   /* UNEQ */
    __m256 cmp_nlt_uq = _mm256_cmp_ps(a, b, _CMP_NLT_UQ);   /* UNGE */
    __m256 cmp_nle_uq = _mm256_cmp_ps(a, b, _CMP_NLE_UQ);   /* UNGT */
    
    /* Extract results */
    float* f_unord = (float*)&cmp_unord;
    float* f_ord = (float*)&cmp_ord;
    
    for (int i = 0; i < 8; i++) {
        result += (f_unord[i] != 0.0f) ? (1 << i) : 0;
        result += (f_ord[i] != 0.0f) ? (1 << (i + 8)) : 0;
    }
    
    /* Double precision unordered comparisons */
    __m256d cmp_unord_d = _mm256_cmp_pd(c, d, _CMP_UNORD_Q);
    __m256d cmp_neq_uq_d = _mm256_cmp_pd(c, d, _CMP_NEQ_UQ);
    
    double* d_unord = (double*)&cmp_unord_d;
    double* d_neq = (double*)&cmp_neq_uq_d;
    
    for (int i = 0; i < 4; i++) {
        result += (d_unord[i] != 0.0) ? (1 << (i + 16)) : 0;
        result += (d_neq[i] != 0.0) ? (1 << (i + 20)) : 0;
    }
    
    return result;
}
#endif

/* Test function 5: Fast-math optimized comparisons */
__attribute__((noinline))
static int test_fast_math_optimizations(float a, float b, float c, float d,
                                        double e, double f, double g, double h) {
    int result = 0;
    
    /* Under fast-math, these may generate UNEQ/LTGT codes */
    result += (a == b) ? 1 : 0;      /* May become UNEQ with fast-math */
    result += (c != d) ? 2 : 0;      /* May become LTGT with fast-math */
    
    /* Mixed precision with fast-math assumptions */
    if ((a < b) && (e > f) && (c <= d) && (g >= h)) {
        result += 4;
    }
    
    /* Complex expression that fast-math might transform */
    float temp1 = (a * b) + (c * d);
    double temp2 = (e * f) - (g * h);
    result += (temp1 == temp1) ? 8 : 0;   /* Check for NaN */
    result += (temp2 != temp2) ? 16 : 0;  /* Check for NaN */
    
    /* Chain that encourages UNLE/UNLT generation */
    if (!(a > b) || !(c < d)) {
        result += 32;
    }
    
    return result;
}

/* Test function 6: NaN propagation tests */
__attribute__((noinline))
static int test_nan_propagation(float* fvals, double* dvals, int n) {
    int result = 0;
    
    for (int i = 0; i < n - 1; i++) {
        /* These comparisons with NaN should generate unordered codes */
        result += (fvals[i] == fvals[i+1]) ? (1 << (i % 16)) : 0;
        result += (fvals[i] < fvals[i+1]) ? (1 << ((i+4) % 16)) : 0;
        result += (fvals[i] > fvals[i+1]) ? (1 << ((i+8) % 16)) : 0;
        
        /* Double comparisons */
        result += (dvals[i] != dvals[i+1]) ? (1 << ((i+12) % 16)) : 0;
        result += (dvals[i] <= dvals[i+1]) ? (1 << ((i+14) % 16)) : 0;
    }
    
    return result;
}

int main(int argc, char** argv) {
    int total_result = 0;
    
    /* Patterned data including NaN values */
    float fdata[16];
    double ddata[16];
    
    /* Initialize with pattern: normal numbers, zeros, and some NaNs */
    for (int i = 0; i < 16; i++) {
        fdata[i] = (i % 3 == 0) ? (float)i : 
                   (i % 3 == 1) ? 0.0f : 
                   (float)(i * 0.5);
        
        ddata[i] = (i % 4 == 0) ? (double)i : 
                   (i % 4 == 1) ? 0.0 : 
                   (i % 4 == 2) ? __builtin_nan("") :  /* Explicit NaN */
                   (double)(i * 0.25);
        
        /* Add some NaN values in float array too */
        if (i % 5 == 0) {
            fdata[i] = __builtin_nanf("");
        }
    }
    
    /* Use argc to prevent excessive loop unrolling */
    int iterations = (argc > 1) ? atoi(argv[1]) : 100;
    if (iterations < 1) iterations = 100;
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Vary inputs slightly each iteration */
        float offset = (iter % 10) * 0.1f;
        double doffset = (iter % 7) * 0.07;
        
        /* Test 1: Unordered comparisons */
        total_result += test_unordered_comparisons(
            fdata[iter % 16] + offset,
            fdata[(iter + 1) % 16],
            ddata[iter % 16] + doffset,
            ddata[(iter + 2) % 16]
        );
        
        /* Test 2: Mixed comparisons */
        total_result += test_mixed_comparisons(
            fdata[(iter + 3) % 16],
            fdata[(iter + 4) % 16],
            fdata[(iter + 5) % 16],
            fdata[(iter + 6) % 16]
        );
        
        /* Test 3: Vector comparisons */
        v4sf va = {fdata[0], fdata[1], fdata[2], fdata[3]};
        v4sf vb = {fdata[4], fdata[5], fdata[6], fdata[7]};
        v2df vc = {ddata[0], ddata[1]};
        v2df vd = {ddata[2], ddata[3]};
        
        total_result += test_vector_comparisons(va, vb, vc, vd);
        
        /* Test 4: AVX intrinsics (if available) */
        #ifdef __AVX__
        __m256 avx_a = _mm256_set_ps(fdata[0], fdata[1], fdata[2], fdata[3],
                                     fdata[4], fdata[5], fdata[6], fdata[7]);
        __m256 avx_b = _mm256_set_ps(fdata[8], fdata[9], fdata[10], fdata[11],
                                     fdata[12], fdata[13], fdata[14], fdata[15]);
        __m256d avx_c = _mm256_set_pd(ddata[0], ddata[1], ddata[2], ddata[3]);
        __m256d avx_d = _mm256_set_pd(ddata[4], ddata[5], ddata[6], ddata[7]);
        
        total_result += test_avx_intrinsics(avx_a, avx_b, avx_c, avx_d);
        #endif
        
        /* Test 5: Fast-math optimizations */
        total_result += test_fast_math_optimizations(
            fdata[(iter + 7) % 16],
            fdata[(iter + 8) % 16],
            fdata[(iter + 9) % 16],
            fdata[(iter + 10) % 16],
            ddata[(iter + 7) % 16],
            ddata[(iter + 8) % 16],
            ddata[(iter + 9) % 16],
            ddata[(iter + 10) % 16]
        );
        
        /* Test 6: NaN propagation */
        total_result += test_nan_propagation(fdata, ddata, 16);
        
        /* Prevent dead code elimination */
        sink = total_result;
    }
    
    printf("Final checksum: %d\n", total_result);
    return total_result != 0 ? 0 : 1;
}
