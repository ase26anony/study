#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <xmmintrin.h>
#include <immintrin.h>

/* Vector types for triggering vector comparison RTL */
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
static int test_uneq_unge_ungt(float a, float b, float c, float d) {
    int sum = 0;
    
    /* Complex conditional to trigger multiple condition codes */
    sum += ((a <= b) ? (c != d) : (a >= b)) ? 1 : 0;
    
    /* Chain of comparisons */
    if ((a < b) && !(c > d) && (a != c)) {
        sum += 2;
    }
    
    /* Ternary with different operators */
    float result = (a == b) ? c : d;
    sum += (result > 0.0f) ? 4 : 0;
    
    return sum;
}

/* Function specifically for LTGT (unordered not equal) */
__attribute__((noinline))
static int test_ltgt(double a, double b, double c, double d) {
    int sum = 0;
    
    /* With -ffast-math, != can become LTGT */
    if (a != b) {
        sum += 1;
    }
    
    /* Complex expression that might generate LTGT */
    if ((a < b) != (c > d)) {
        sum += 2;
    }
    
    /* Mixed comparisons */
    if (!(a == b) && (c <= d)) {
        sum += 4;
    }
    
    return sum;
}

/* Vectorized comparisons using GCC vector extensions */
__attribute__((noinline))
static int test_vector_comparisons(v4sf va, v4sf vb, v2df vc, v2df vd) {
    int sum = 0;
    
    /* Vector comparisons generate specific condition codes */
    v4sf cmp_result = va < vb;
    v2df cmp_result_d = vc > vd;
    
    /* Extract results to prevent elimination */
    float temp[4];
    memcpy(temp, &cmp_result, sizeof(temp));
    sum += (temp[0] != 0.0f);
    
    double temp_d[2];
    memcpy(temp_d, &cmp_result_d, sizeof(temp_d));
    sum += (temp_d[1] != 0.0) * 2;
    
    /* Unordered vector comparison */
    v4sf unord_cmp = va == va;  /* false for NaN elements */
    memcpy(temp, &unord_cmp, sizeof(temp));
    sum += (temp[2] != 0.0f) ? 4 : 0;
    
    return sum;
}

/* AVX intrinsics for direct unordered comparisons */
#ifdef __AVX__
__attribute__((noinline))
static int test_avx_unordered(float* fa, float* fb) {
    int sum = 0;
    
    __m128 a = _mm_loadu_ps(fa);
    __m128 b = _mm_loadu_ps(fb);
    
    /* Compare unordered - should map to UNORD condition */
    __m128 cmp_unord = _mm_cmpunord_ps(a, b);
    
    /* Compare ordered */
    __m128 cmp_ord = _mm_cmpord_ps(a, b);
    
    /* Compare not equal unordered (NEQ_UQ) */
    __m128 cmp_neq_uq = _mm_cmpneq_ps(a, b);
    
    /* Extract results */
    float res[4];
    _mm_storeu_ps(res, cmp_unord);
    sum += (res[0] != 0.0f) ? 1 : 0;
    
    _mm_storeu_ps(res, cmp_ord);
    sum += (res[1] != 0.0f) ? 2 : 0;
    
    _mm_storeu_ps(res, cmp_neq_uq);
    sum += (res[2] != 0.0f) ? 4 : 0;
    
    return sum;
}
#endif

/* Mixed type comparisons in loop */
__attribute__((noinline))
static int test_mixed_in_loop(float* farr, double* darr, int n) {
    int sum = 0;
    
    for (int i = 0; i < n - 1; i++) {
        /* Different comparison types in conditional */
        if ((farr[i] < farr[i+1]) ? 
            (darr[i] != darr[i+1]) : 
            (farr[i] >= farr[i+1])) {
            sum += 1;
        }
        
        /* Unordered check in loop */
        if (isunordered(farr[i], farr[i+1])) {
            sum += 2;
        }
        
        /* Fast-math optimized comparison */
        if (farr[i] != farr[i+1] && darr[i] <= darr[i+1]) {
            sum += 4;
        }
    }
    
    return sum;
}

int main(int argc, char** argv) {
    int total_sum = 0;
    
    /* Patterned data including NaN values */
    float fdata[16];
    double ddata[16];
    
    /* Initialize with pattern: normal numbers, zeros, NaN */
    for (int i = 0; i < 16; i++) {
        fdata[i] = (i % 3 == 0) ? (float)i : 
                   (i % 3 == 1) ? 0.0f : 
                   (float)(i - 8);
        
        ddata[i] = (i % 4 == 0) ? (double)i : 
                   (i % 4 == 1) ? __builtin_nan("") :  /* NaN */
                   (i % 4 == 2) ? 0.0 : 
                   (double)(i * 0.5);
    }
    
    /* Use argc to prevent loop unrolling */
    int iterations = (argc > 1) ? atoi(argv[1]) : 10;
    if (iterations <= 0) iterations = 10;
    
    for (int iter = 0; iter < iterations; iter++) {
        sink = iter;  /* Volatile to prevent optimization */
        
        /* Test various comparison patterns */
        total_sum += test_unordered_ordered(
            fdata[iter % 16], 
            fdata[(iter + 1) % 16],
            ddata[iter % 16],
            ddata[(iter + 3) % 16]
        );
        
        total_sum += test_uneq_unge_ungt(
            fdata[(iter + 2) % 16],
            fdata[(iter + 3) % 16],
            fdata[(iter + 4) % 16],
            fdata[(iter + 5) % 16]
        );
        
        total_sum += test_ltgt(
            ddata[(iter + 1) % 16],
            ddata[(iter + 2) % 16],
            ddata[(iter + 3) % 16],
            ddata[(iter + 4) % 16]
        );
        
        /* Vector tests */
        v4sf va = {fdata[0], fdata[1], fdata[2], fdata[3]};
        v4sf vb = {fdata[4], fdata[5], fdata[6], fdata[7]};
        v2df vc = {ddata[0], ddata[1]};
        v2df vd = {ddata[2], ddata[3]};
        
        total_sum += test_vector_comparisons(va, vb, vc, vd);
        
        #ifdef __AVX__
        total_sum += test_avx_unordered(fdata, &fdata[4]);
        #endif
        
        /* Mixed comparisons in loop */
        total_sum += test_mixed_in_loop(fdata, ddata, 8);
    }
    
    printf("Final checksum: %d\n", total_sum);
    return total_sum != 0 ? 0 : 1;
}
