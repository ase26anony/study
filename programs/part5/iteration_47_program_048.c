/* Compile with: gcc -O3 -ffast-math -march=x86-64 -mavx2 -fdump-rtl-final -o test_cond test_cond.c */
#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <immintrin.h>

/* Prevent optimization of critical values */
static volatile float vnan = __builtin_nanf("");
static volatile double dnan = __builtin_nan("");

/* Vector types for AVX comparisons */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Function to trigger unordered comparisons */
__attribute__((noinline))
static int test_unordered(float a, float b, double c, double d) {
    int sum = 0;
    
    /* Direct unordered checks */
    sum += isunordered(a, b) ? 1 : 0;
    sum += isunordered(c, d) ? 2 : 0;
    
    /* NaN checks using self-comparison */
    sum += (a != a) ? 4 : 0;
    sum += !(c == c) ? 8 : 0;
    
    /* Mixed ordered/unordered in conditional */
    sum += (isunordered(a, b) ? (c < d) : (c > d)) ? 16 : 0;
    
    return sum;
}

/* Function to trigger LTGT (unordered and not equal) */
__attribute__((noinline))
static int test_ltgt(float a, float b, double c, double d) {
    int sum = 0;
    
    /* islessgreater generates LTGT under fast-math */
    sum += islessgreater(a, b) ? 1 : 0;
    sum += islessgreater(c, d) ? 2 : 0;
    
    /* Complex expression that might generate LTGT */
    sum += ((a > b) != (a < b)) ? 4 : 0;
    sum += ((c != d) && !isunordered(c, d)) ? 8 : 0;
    
    return sum;
}

/* Function to trigger UNEQ/UNGE/UNGT/UNLE/UNLT */
__attribute__((noinline))
static int test_mixed_unordered(float a, float b, float c, float d) {
    int sum = 0;
    
    /* Chain of comparisons that might generate various unordered codes */
    if (isunordered(a, b) || a >= b) sum += 1;  /* Could generate UNGE */
    if (!isunordered(c, d) && c <= d) sum += 2; /* Could generate UNLE */
    
    /* Ternary with different comparison types */
    sum += (a < b) ? (c != d ? 4 : 0) : (isunordered(a, c) ? 8 : 0);
    
    /* Complex conditional expression */
    sum += ((a > b) && !isunordered(a, b)) ? 16 : 0;
    sum += ((c <= d) || isunordered(c, d)) ? 32 : 0;
    
    return sum;
}

/* Function using vector extensions to trigger vector condition codes */
__attribute__((noinline))
static int test_vector_comparisons(v4sf va, v4sf vb, v2df vc, v2df vd) {
    int sum = 0;
    
    /* Vector comparisons generate various condition codes */
    v4sf vcmp = va < vb;
    v2df vcmpd = vc > vd;
    
    /* Extract results to prevent elimination */
    float farr[4];
    double darr[2];
    memcpy(farr, &vcmp, sizeof(vcmp));
    memcpy(darr, &vcmpd, sizeof(vcmpd));
    
    for (int i = 0; i < 4; i++) sum += (farr[i] != 0.0f) ? (1 << i) : 0;
    for (int i = 0; i < 2; i++) sum += (darr[i] != 0.0) ? (1 << (i + 4)) : 0;
    
    /* Unordered vector comparison */
    v4sf vunord = (va != va) | (vb != vb);
    memcpy(farr, &vunord, sizeof(vunord));
    for (int i = 0; i < 4; i++) sum += (farr[i] != 0.0f) ? (1 << (i + 6)) : 0;
    
    return sum;
}

/* Function using AVX intrinsics for explicit unordered comparisons */
#ifdef __AVX__
__attribute__((noinline))
static int test_avx_intrinsics(float *fa, float *fb, double *da, double *db) {
    int sum = 0;
    
    __m128 veca = _mm_loadu_ps(fa);
    __m128 vecb = _mm_loadu_ps(fb);
    __m128d vecda = _mm_loadu_pd(da);
    __m128d vecdb = _mm_loadu_pd(db);
    
    /* Generate UNORD comparison */
    __m128 unord_mask = _mm_cmpunord_ps(veca, vecb);
    __m128d unord_maskd = _mm_cmpunord_pd(vecda, vecdb);
    
    /* Generate NEQ_UQ (not equal unordered) */
    __m128 neq_mask = _mm_cmpneq_ps(veca, vecb);
    __m128d neq_maskd = _mm_cmpneq_pd(vecda, vecdb);
    
    /* Extract masks */
    float farr[4];
    double darr[2];
    _mm_storeu_ps(farr, unord_mask);
    _mm_storeu_pd(darr, unord_maskd);
    
    for (int i = 0; i < 4; i++) sum += (farr[i] != 0.0f) ? 1 : 0;
    for (int i = 0; i < 2; i++) sum += (darr[i] != 0.0) ? 2 : 0;
    
    _mm_storeu_ps(farr, neq_mask);
    _mm_storeu_pd(darr, neq_maskd);
    
    for (int i = 0; i < 4; i++) sum += (farr[i] != 0.0f) ? 4 : 0;
    for (int i = 0; i < 2; i++) sum += (darr[i] != 0.0) ? 8 : 0;
    
    return sum;
}
#endif

/* Complex function with multiple parameters and comparison chain */
__attribute__((noinline))
static int test_complex_chain(float a, float b, float c, float d, 
                              double e, double f, double g, double h) {
    int sum = 0;
    
    /* Chain of comparisons that might generate various condition codes */
    int cond1 = (a < b) && !isunordered(a, b);
    int cond2 = isunordered(c, d) || (c >= d);
    int cond3 = (e != f) ? (g < h) : (isunordered(g, h));
    int cond4 = islessgreater(a, c) && (b != d);
    
    sum = cond1 + (cond2 << 1) + (cond3 << 2) + (cond4 << 3);
    
    /* Additional mixed comparisons */
    if ((a == b) ? (c <= d) : (e > f)) sum += 16;
    if ((isunordered(g, h) || (g >= h)) && !isunordered(e, f)) sum += 32;
    
    return sum;
}

int main(int argc, char **argv) {
    int total_sum = 0;
    
    /* Patterned data including NaN, normal numbers, and zeros */
    float fdata[16];
    double ddata[16];
    
    /* Initialize with pattern: normal, zero, NaN, normal... */
    for (int i = 0; i < 16; i++) {
        if (i % 4 == 0) {
            fdata[i] = (float)(i * 1.5);
            ddata[i] = i * 2.5;
        } else if (i % 4 == 1) {
            fdata[i] = 0.0f;
            ddata[i] = 0.0;
        } else if (i % 4 == 2) {
            fdata[i] = __builtin_nanf("");
            ddata[i] = __builtin_nan("");
        } else {
            fdata[i] = (float)(-i * 0.5);
            ddata[i] = -i * 0.75;
        }
    }
    
    /* Use argc to prevent excessive unrolling */
    int iterations = (argc > 1) ? (argc % 8) + 4 : 8;
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Vary inputs slightly each iteration */
        float f1 = fdata[iter % 16] + (iter * 0.1f);
        float f2 = fdata[(iter + 1) % 16] - (iter * 0.05f);
        float f3 = fdata[(iter + 2) % 16];
        float f4 = fdata[(iter + 3) % 16];
        
        double d1 = ddata[iter % 16] + (iter * 0.01);
        double d2 = ddata[(iter + 1) % 16] - (iter * 0.005);
        double d3 = ddata[(iter + 2) % 16];
        double d4 = ddata[(iter + 3) % 16];
        
        /* Call all test functions */
        total_sum += test_unordered(f1, f2, d1, d2);
        total_sum += test_ltgt(f2, f3, d2, d3);
        total_sum += test_mixed_unordered(f1, f3, f2, f4);
        total_sum += test_complex_chain(f1, f2, f3, f4, d1, d2, d3, d4);
        
        /* Vector tests */
        v4sf va = {f1, f2, f3, f4};
        v4sf vb = {f2, f3, f4, f1};
        v2df vc = {d1, d2};
        v2df vd = {d2, d3};
        
        total_sum += test_vector_comparisons(va, vb, vc, vd);
        
        #ifdef __AVX__
        total_sum += test_avx_intrinsics(&f1, &f2, &d1, &d2);
        #endif
        
        /* Force compiler to consider NaN paths */
        if (iter % 3 == 0) {
            total_sum += test_unordered(vnan, f1, dnan, d1);
        }
    }
    
    printf("Final checksum: %d\n", total_sum);
    return total_sum != 0 ? 0 : 1;
}
