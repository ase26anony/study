#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* Feature detection and fallbacks */
#ifdef __DECIMAL_BID_FORMAT__
    #define HAS_DFP 1
    #include <decimal/decimal.h>
#else
    #define HAS_DFP 0
#endif

#ifdef __STDC_IEC_559_COMPLEX__
    #include <complex.h>
    #define HAS_COMPLEX 1
#else
    #define HAS_COMPLEX 0
#endif

/* Vector type definitions */
#if defined(__GNUC__) && (defined(__x86_64__) || defined(__i386__) || defined(__aarch64__))
    #define HAS_VECTORS 1
    typedef int32_t int32x4_t __attribute__((vector_size(16)));
    typedef int64_t int64x2_t __attribute__((vector_size(16)));
    typedef double float64x2_t __attribute__((vector_size(16)));
#else
    #define HAS_VECTORS 0
#endif

/* DFP fallback using integer arrays */
#if !HAS_DFP
typedef struct {
    uint64_t lo;
    uint64_t hi;
} decimal64_fb;

typedef struct {
    uint64_t parts[2];
} decimal128_fb;

static decimal64_fb dfp64_add_fb(decimal64_fb a, decimal64_fb b) {
    decimal64_fb result;
    result.lo = a.lo + b.lo;
    result.hi = a.hi + b.hi + (result.lo < a.lo);
    return result;
}

static decimal64_fb dfp64_mul_fb(decimal64_fb a, decimal64_fb b) {
    /* Simplified multiplication for fallback */
    decimal64_fb result;
    result.lo = a.lo * b.lo;
    result.hi = a.hi * b.hi;
    return result;
}
#endif

/* Complex fallback */
#if !HAS_COMPLEX
typedef struct {
    long double real;
    long double imag;
} complex_ld_fb;
#endif

/* Helper function with 11 arguments - marked noinline to prevent optimization */
static __attribute__((noinline)) long double helper_11_args(
#if HAS_DFP
    _Decimal64 d1, _Decimal64 d2, _Decimal128 d3,
#else
    decimal64_fb d1, decimal64_fb d2, decimal128_fb d3,
#endif
#if HAS_COMPLEX
    long double _Complex c1, long double _Complex c2,
#else
    complex_ld_fb c1, complex_ld_fb c2,
#endif
#if HAS_VECTORS
    int32x4_t v1, float64x2_t v2,
#else
    int32_t v1[4], double v2[2],
#endif
    long double scalar1, long double scalar2, int modifier)
{
    long double result = 0.0L;
    
    /* Process DFP arguments */
#if HAS_DFP
    result += (long double)d1 + (long double)d2;
#else
    result += (long double)d1.lo + (long double)d2.lo;
#endif
    
    /* Process complex arguments */
#if HAS_COMPLEX
    result += creall(c1) + cimagl(c2);
#else
    result += c1.real + c2.imag;
#endif
    
    /* Process vector arguments */
#if HAS_VECTORS
    for (int i = 0; i < 4; i++) result += v1[i];
    for (int i = 0; i < 2; i++) result += v2[i];
#else
    for (int i = 0; i < 4; i++) result += v1[i];
    for (int i = 0; i < 2; i++) result += v2[i];
#endif
    
    result += scalar1 * scalar2;
    result *= (modifier % 10) + 1;
    
    return result;
}

/* Another helper with 10 arguments */
static __attribute__((noinline)) long double helper_10_args(
    long double a1, long double a2, long double a3,
    long double a4, long double a5, long double a6,
    long double a7, long double a8, long double a9,
    long double a10)
{
    /* Complex expression that may expand to many operands */
    return ((a1 * a2) + (a3 / a4) - (a5 * a6) + (a7 - a8) * (a9 + a10)) /
           ((a1 + a2) * (a3 - a4) + (a5 / a6) - (a7 * a8) + (a9 - a10));
}

/* Vector reduction with accumulation */
static long double vector_reduction(
#if HAS_VECTORS
    int32x4_t vec
#else
    int32_t vec[4]
#endif
) {
    long double sum = 0.0L;
    
#if HAS_VECTORS
    /* This may expand to multiple operations */
    sum = vec[0] + vec[1] + vec[2] + vec[3];
#else
    for (int i = 0; i < 4; i++) {
        sum += vec[i];
    }
#endif
    
    return sum;
}

int main(int argc, char *argv[]) {
    int seed = 12345;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    srand(seed);
    
    /* Prevent optimization */
    volatile long double global_accumulator = 0.0L;
    long double results[5] = {0};
    
    /* Initialize base variables */
#if HAS_DFP
    _Decimal64 d64_a = 1.23456789dd;
    _Decimal64 d64_b = 9.87654321dd;
    _Decimal64 d64_c = 5.55555555dd;
    _Decimal64 d64_d = 3.33333333dd;
    _Decimal128 d128_a = 1.2345678901234567dl;
    _Decimal128 d128_b = 9.8765432109876543dl;
#else
    decimal64_fb d64_a = {123456789ULL, 0};
    decimal64_fb d64_b = {987654321ULL, 0};
    decimal64_fb d64_c = {555555555ULL, 0};
    decimal64_fb d64_d = {333333333ULL, 0};
    decimal128_fb d128_a = {{12345678901234567ULL, 0}};
    decimal128_fb d128_b = {{98765432109876543ULL, 0}};
#endif

#if HAS_COMPLEX
    long double _Complex ca = 1.0L + 2.0LI;
    long double _Complex cb = 3.0L - 4.0LI;
    long double _Complex cc = 5.0L + 6.0LI;
    long double _Complex cd = 7.0L - 8.0LI;
#else
    complex_ld_fb ca = {1.0L, 2.0L};
    complex_ld_fb cb = {3.0L, -4.0L};
    complex_ld_fb cc = {5.0L, 6.0L};
    complex_ld_fb cd = {7.0L, -8.0L};
#endif

#if HAS_VECTORS
    int32x4_t vec1 = {1, 2, 3, 4};
    int32x4_t vec2 = {5, 6, 7, 8};
    float64x2_t fvec1 = {1.5, 2.5};
    float64x2_t fvec2 = {3.5, 4.5};
#else
    int32_t vec1[4] = {1, 2, 3, 4};
    int32_t vec2[4] = {5, 6, 7, 8};
    double fvec1[2] = {1.5, 2.5};
    double fvec2[2] = {3.5, 4.5};
#endif

    /* Main computation loop */
    for (int iter = 0; iter < 3; iter++) {
        long double iter_result = 0.0L;
        
        /* Conditional execution based on volatile variable */
        volatile int condition = iter % 2;
        
        if (condition) {
            /* DFP arithmetic - may expand to many operands */
#if HAS_DFP
            _Decimal64 d64_result = d64_a * d64_b + d64_c / d64_d;
            _Decimal128 d128_result = d128_a * d128_b + (_Decimal128)d64_c;
            iter_result += (long double)d64_result + (long double)d128_result;
#else
            decimal64_fb d64_result = dfp64_add_fb(
                dfp64_mul_fb(d64_a, d64_b),
                dfp64_mul_fb(d64_c, d64_d)  /* Simplified division */
            );
            iter_result += (long double)d64_result.lo;
#endif
        } else {
            /* Complex arithmetic */
#if HAS_COMPLEX
            long double _Complex c_result = (ca * cb) / (cc - cd);
            iter_result += creall(c_result) + cimagl(c_result);
#else
            complex_ld_fb c_result;
            c_result.real = (ca.real * cb.real - ca.imag * cb.imag) /
                           ((cc.real - cd.real) * (cc.real - cd.real) + 
                            (cc.imag - cd.imag) * (cc.imag - cd.imag));
            c_result.imag = (ca.real * cb.imag + ca.imag * cb.real) /
                           ((cc.real - cd.real) * (cc.real - cd.real) + 
                            (cc.imag - cd.imag) * (cc.imag - cd.imag));
            iter_result += c_result.real + c_result.imag;
#endif
        }
        
        /* Vector reduction with accumulation */
        long double vec_sum1 = vector_reduction(vec1);
        long double vec_sum2 = vector_reduction(vec2);
        iter_result += vec_sum1 * vec_sum2;
        
        /* Call helper with many arguments */
        long double helper_result = helper_11_args(
#if HAS_DFP
            d64_a, d64_b, d128_a,
#else
            d64_a, d64_b, d128_a,
#endif
#if HAS_COMPLEX
            ca, cb,
#else
            ca, cb,
#endif
#if HAS_VECTORS
            vec1, fvec1,
#else
            vec1, fvec1,
#endif
            vec_sum1, vec_sum2, iter
        );
        
        iter_result += helper_result;
        
        /* Another helper call with 10 arguments */
        iter_result += helper_10_args(
            iter_result, vec_sum1, vec_sum2,
            (long double)iter, (long double)(iter + 1),
            (long double)(iter + 2), (long double)(iter + 3),
            (long double)(iter + 4), (long double)(iter + 5),
            (long double)(iter + 6)
        );
        
        /* Store result to prevent elimination */
        results[iter] = iter_result;
        global_accumulator += iter_result;
        
        /* Modify variables for next iteration */
#if HAS_VECTORS
        vec1[0] += iter;
        vec2[1] += iter;
#endif
    }
    
    /* Compute checksum */
    long double checksum = 0.0L;
    for (int i = 0; i < 3; i++) {
        checksum += results[i];
    }
    
    /* Print deterministic result */
    printf("Checksum: %.15Lf\n", checksum);
    printf("Global accumulator: %.15Lf\n", (long double)global_accumulator);
    
    return 0;
}
