#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Feature detection and fallbacks */
#ifdef __DECIMAL_BID_FORMAT__
    #define DFP_SUPPORTED 1
    #include <decimal/decimal.h>
#else
    #define DFP_SUPPORTED 0
#endif

#ifdef __STDC_IEC_559_COMPLEX__
    #include <complex.h>
    #define COMPLEX_SUPPORTED 1
#else
    #define COMPLEX_SUPPORTED 0
#endif

/* Vector type definitions */
#if defined(__GNUC__) && defined(__VECTOR_TYPES_SUPPORTED__)
    typedef int32_t int32x8_t __attribute__((vector_size(32)));
    typedef float float64x4_t __attribute__((vector_size(32)));
    #define VECTOR_SUPPORTED 1
#else
    #define VECTOR_SUPPORTED 0
#endif

/* Fallback types for unsupported features */
#if !DFP_SUPPORTED
    typedef struct {
        uint64_t lo;
        uint64_t hi;
    } decimal128_fallback;
    
    typedef struct {
        uint64_t value;
    } decimal64_fallback;
#endif

#if !COMPLEX_SUPPORTED
    typedef struct {
        long double real;
        long double imag;
    } complex_fallback;
#endif

#if !VECTOR_SUPPORTED
    typedef struct {
        int32_t data[8];
    } int32x8_fallback;
    
    typedef struct {
        double data[4];
    } float64x4_fallback;
#endif

/* Helper functions with 10+ arguments (marked noinline) */
static __attribute__((noinline)) 
long double helper_10_args(long double a1, long double a2, long double a3,
                          long double a4, long double a5, long double a6,
                          long double a7, long double a8, long double a9,
                          long double a10) {
    /* Complex expression to prevent optimization */
    volatile long double result = 0;
    result = a1 + a2 - a3 * a4 / a5 + a6 * a7 - a8 / a9 + a10;
    return result;
}

static __attribute__((noinline))
long double helper_11_args(long double a1, long double a2, long double a3,
                          long double a4, long double a5, long double a6,
                          long double a7, long double a8, long double a9,
                          long double a10, long double a11) {
    /* Even more complex expression */
    volatile long double result = 0;
    result = (a1 * a2) + (a3 / a4) - (a5 * a6) + (a7 / a8) - (a9 * a10) + a11;
    return result;
}

/* DFP operations with fallbacks */
#if DFP_SUPPORTED
static _Decimal128 dfp_complex_operation(_Decimal128 a, _Decimal128 b,
                                        _Decimal128 c, _Decimal128 d) {
    /* Complex DFP expression that may expand to many operands */
    volatile _Decimal128 result;
    result = (a * b) + (c / d) - (a + b) * (c - d);
    return result;
}
#else
static decimal128_fallback dfp_complex_operation_fallback(
    decimal128_fallback a, decimal128_fallback b,
    decimal128_fallback c, decimal128_fallback d) {
    /* Simulate DFP with integer arithmetic */
    decimal128_fallback result;
    /* Simple emulation - actual DFP would be more complex */
    result.lo = a.lo + b.lo - c.lo + d.lo;
    result.hi = a.hi + b.hi - c.hi + d.hi;
    return result;
}
#endif

/* Complex number operations */
#if COMPLEX_SUPPORTED
static long double _Complex complex_operation(long double _Complex a,
                                             long double _Complex b,
                                             long double _Complex c,
                                             long double _Complex d) {
    /* Complex expression that may generate many operands */
    volatile long double _Complex result;
    result = (a * b) / (c - d) + (a + b) * (c * d);
    return result;
}
#else
static complex_fallback complex_operation_fallback(complex_fallback a,
                                                  complex_fallback b,
                                                  complex_fallback c,
                                                  complex_fallback d) {
    complex_fallback result;
    result.real = (a.real * b.real - a.imag * b.imag) / 
                  (c.real - d.real) + (a.real + b.real) * (c.real * d.real);
    result.imag = (a.real * b.imag + a.imag * b.real) / 
                  (c.imag - d.imag) + (a.imag + b.imag) * (c.imag * d.imag);
    return result;
}
#endif

/* Vector reduction */
#if VECTOR_SUPPORTED
static int32_t vector_reduction(int32x8_t v) {
    /* Horizontal reduction that may expand to many operations */
    volatile int32_t result = 0;
    result = v[0] + v[1] + v[2] + v[3] + v[4] + v[5] + v[6] + v[7];
    return result;
}

static double vector_reduction_double(float64x4_t v) {
    volatile double result = 0.0;
    result = v[0] + v[1] + v[2] + v[3];
    return result;
}
#else
static int32_t vector_reduction_fallback(int32x8_fallback v) {
    int32_t result = 0;
    for (int i = 0; i < 8; i++) {
        result += v.data[i];
    }
    return result;
}

static double vector_reduction_double_fallback(float64x4_fallback v) {
    double result = 0.0;
    for (int i = 0; i < 4; i++) {
        result += v.data[i];
    }
    return result;
}
#endif

/* Main computation function */
static long double compute_sequence(int seed) {
    volatile long double accumulator = 0.0;
    
    /* Initialize variables with seed-dependent values */
#if DFP_SUPPORTED
    _Decimal128 d128_a = (_Decimal128)seed * 1.23456789DL;
    _Decimal128 d128_b = (_Decimal128)(seed + 1) * 2.34567890DL;
    _Decimal128 d128_c = (_Decimal128)(seed + 2) * 3.45678901DL;
    _Decimal128 d128_d = (_Decimal128)(seed + 3) * 4.56789012DL;
#else
    decimal128_fallback d128_a = {seed * 123456789ULL, 0};
    decimal128_fallback d128_b = {(seed + 1) * 234567890ULL, 0};
    decimal128_fallback d128_c = {(seed + 2) * 345678901ULL, 0};
    decimal128_fallback d128_d = {(seed + 3) * 456789012ULL, 0};
#endif

#if COMPLEX_SUPPORTED
    long double _Complex ca = (seed * 1.5L) + (seed * 0.5L) * I;
    long double _Complex cb = ((seed + 1) * 2.5L) + ((seed + 1) * 1.5L) * I;
    long double _Complex cc = ((seed + 2) * 3.5L) + ((seed + 2) * 2.5L) * I;
    long double _Complex cd = ((seed + 3) * 4.5L) + ((seed + 3) * 3.5L) * I;
#else
    complex_fallback ca = {seed * 1.5L, seed * 0.5L};
    complex_fallback cb = {(seed + 1) * 2.5L, (seed + 1) * 1.5L};
    complex_fallback cc = {(seed + 2) * 3.5L, (seed + 2) * 2.5L};
    complex_fallback cd = {(seed + 3) * 4.5L, (seed + 3) * 3.5L};
#endif

#if VECTOR_SUPPORTED
    int32x8_t vec_int = {seed, seed+1, seed+2, seed+3, 
                         seed+4, seed+5, seed+6, seed+7};
    float64x4_t vec_double = {seed * 1.1, seed * 2.2, 
                              seed * 3.3, seed * 4.4};
#else
    int32x8_fallback vec_int = {{seed, seed+1, seed+2, seed+3, 
                                 seed+4, seed+5, seed+6, seed+7}};
    float64x4_fallback vec_double = {{seed * 1.1, seed * 2.2, 
                                      seed * 3.3, seed * 4.4}};
#endif

    /* Perform computations in a loop */
    for (int i = 0; i < 3; i++) {
        volatile int condition = seed % 2;
        
        if (condition) {
            /* DFP arithmetic branch */
#if DFP_SUPPORTED
            _Decimal128 dfp_result = dfp_complex_operation(
                d128_a, d128_b, d128_c, d128_d);
            accumulator += (long double)dfp_result;
#else
            decimal128_fallback dfp_result = dfp_complex_operation_fallback(
                d128_a, d128_b, d128_c, d128_d);
            accumulator += (long double)dfp_result.lo;
#endif
        } else {
            /* Complex arithmetic branch */
#if COMPLEX_SUPPORTED
            long double _Complex cplx_result = complex_operation(
                ca, cb, cc, cd);
            accumulator += creal(cplx_result) + cimag(cplx_result);
#else
            complex_fallback cplx_result = complex_operation_fallback(
                ca, cb, cc, cd);
            accumulator += cplx_result.real + cplx_result.imag;
#endif
        }
        
        /* Vector reduction */
#if VECTOR_SUPPORTED
        int32_t vec_result_int = vector_reduction(vec_int);
        double vec_result_double = vector_reduction_double(vec_double);
        accumulator += vec_result_int + vec_result_double;
#else
        int32_t vec_result_int = vector_reduction_fallback(vec_int);
        double vec_result_double = vector_reduction_double_fallback(vec_double);
        accumulator += vec_result_int + vec_result_double;
#endif
        
        /* Call helper functions with many arguments */
        long double helper_result_10 = helper_10_args(
            accumulator, accumulator * 1.1L, accumulator * 1.2L,
            accumulator * 1.3L, accumulator * 1.4L, accumulator * 1.5L,
            accumulator * 1.6L, accumulator * 1.7L, accumulator * 1.8L,
            accumulator * 1.9L);
            
        long double helper_result_11 = helper_11_args(
            accumulator, accumulator * 2.1L, accumulator * 2.2L,
            accumulator * 2.3L, accumulator * 2.4L, accumulator * 2.5L,
            accumulator * 2.6L, accumulator * 2.7L, accumulator * 2.8L,
            accumulator * 2.9L, accumulator * 3.0L);
            
        accumulator += helper_result_10 + helper_result_11;
        
        /* Update variables for next iteration */
        seed += 10;
    }
    
    return accumulator;
}

int main(int argc, char *argv[]) {
    int seed = 42;  /* Default seed */
    
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    printf("Starting computation with seed: %d\n", seed);
    
    /* Perform computation and store result in volatile to prevent optimization */
    volatile long double final_result = compute_sequence(seed);
    
    /* Compute checksum */
    uint64_t checksum = 0;
    unsigned char *bytes = (unsigned char*)&final_result;
    for (size_t i = 0; i sizeof(long double); i++) {
        checksum += bytes[i];
    }
    
    printf("Final checksum: %llu\n", (unsigned long long)checksum);
    
    return 0;
}
