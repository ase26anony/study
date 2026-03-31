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
typedef int32_t int32x4_t __attribute__((vector_size(16)));
typedef float float32x4_t __attribute__((vector_size(16)));
typedef double float64x2_t __attribute__((vector_size(16)));

/* Global volatile to prevent optimization */
volatile uint64_t g_result_accumulator = 0;

/* Helper function with 11 arguments - marked noinline */
static __attribute__((noinline)) 
long double complex_helper_11(
    long double a1, long double a2, long double a3,
    long double a4, long double a5, long double a6,
    long double a7, long double a8, long double a9,
    long double a10, long double a11)
{
    /* Complex operation that might expand to many operands */
    return (a1 * a2) + (a3 * a4) - (a5 * a6) + 
           (a7 / a8) - (a9 / a10) + a11;
}

/* DFP helper with 10 arguments */
#if HAS_DFP
static __attribute__((noinline))
_Decimal128 dfp_helper_10(
    _Decimal64 d1, _Decimal64 d2, _Decimal128 d3,
    _Decimal128 d4, _Decimal64 d5, _Decimal64 d6,
    _Decimal128 d7, _Decimal128 d8, _Decimal64 d9,
    _Decimal64 d10)
{
    /* Complex DFP expression that may require many operands */
    return ((_Decimal128)d1 * (_Decimal128)d2) + 
           (d3 * d4) - 
           ((_Decimal128)d5 * (_Decimal128)d6) +
           (d7 / d8) - 
           ((_Decimal128)d9 / (_Decimal128)d10);
}
#endif

/* Fallback DFP emulation using integer arrays */
typedef struct {
    uint64_t lo;
    uint64_t hi;
} dfp128_emu_t;

static __attribute__((noinline))
dfp128_emu_t dfp_emu_helper_10(
    dfp128_emu_t d1, dfp128_emu_t d2, dfp128_emu_t d3,
    dfp128_emu_t d4, dfp128_emu_t d5, dfp128_emu_t d6,
    dfp128_emu_t d7, dfp128_emu_t d8, dfp128_emu_t d9,
    dfp128_emu_t d10)
{
    /* Simulate complex DFP operation with integer arithmetic */
    dfp128_emu_t result;
    result.lo = d1.lo + d2.lo + d3.lo + d4.lo + d5.lo + 
                d6.lo + d7.lo + d8.lo + d9.lo + d10.lo;
    result.hi = d1.hi + d2.hi + d3.hi + d4.hi + d5.hi + 
                d6.hi + d7.hi + d8.hi + d9.hi + d10.hi;
    return result;
}

/* Vector reduction with accumulation */
static float vector_reduce_accumulate(float64x2_t v1, float64x2_t v2,
                                      float64x2_t v3, float64x2_t v4)
{
    /* Horizontal reduction that may expand to many operations */
    float64x2_t sum1 = v1 + v2;
    float64x2_t sum2 = v3 + v4;
    float64x2_t total = sum1 + sum2;
    
    /* Extract and sum elements - may create many intermediate operands */
    float result = 0.0f;
    for (int i = 0; i < 2; i++) {
        result += total[i];
    }
    
    /* Additional complex operation to increase operand count */
    result = result * result - result / 2.0f + sqrt(fabs(result));
    
    return result;
}

/* Main computation function */
static void perform_computations(int seed, int iterations)
{
    /* Initialize deterministic values */
    srand(seed);
    
    /* Base values for DFP */
#if HAS_DFP
    _Decimal64 d64_vals[10];
    _Decimal128 d128_vals[10];
    
    for (int i = 0; i < 10; i++) {
        double temp = (rand() % 1000) / 100.0;
        d64_vals[i] = (_Decimal64)temp;
        d128_vals[i] = (_Decimal128)(temp * 10.0);
    }
#else
    /* Fallback: use emulated DFP */
    dfp128_emu_t dfp_emu_vals[10];
    for (int i = 0; i < 10; i++) {
        dfp_emu_vals[i].lo = rand() % 1000;
        dfp_emu_vals[i].hi = rand() % 1000;
    }
#endif

    /* Complex number values */
#if HAS_COMPLEX
    long double complex cl_vals[10];
    for (int i = 0; i < 10; i++) {
        cl_vals[i] = (rand() % 1000) / 100.0 + 
                     (rand() % 1000) / 100.0 * I;
    }
#else
    long double cl_real[10], cl_imag[10];
    for (int i = 0; i < 10; i++) {
        cl_real[i] = (rand() % 1000) / 100.0;
        cl_imag[i] = (rand() % 1000) / 100.0;
    }
#endif

    /* Vector values */
    float64x2_t vec_vals[4];
    for (int i = 0; i < 4; i++) {
        vec_vals[i][0] = (rand() % 1000) / 100.0;
        vec_vals[i][1] = (rand() % 1000) / 100.0;
    }

    /* Volatile condition to prevent constant folding */
    volatile int condition = seed % 2;
    
    /* Main computation loop */
    for (int iter = 0; iter < iterations; iter++) {
        long double result = 0.0;
        
        /* Conditional execution block */
        if (condition) {
            /* Complex DFP arithmetic - may expand to many operands */
#if HAS_DFP
            _Decimal128 d128_result = 
                d128_vals[0] * d128_vals[1] + 
                d128_vals[2] / d128_vals[3] - 
                d128_vals[4] * d128_vals[5] +
                d128_vals[6] / d128_vals[7] - 
                d128_vals[8] * d128_vals[9];
            
            /* Call DFP helper with 10 arguments */
            _Decimal128 d128_helper_result = dfp_helper_10(
                d64_vals[0], d64_vals[1], d128_vals[2],
                d128_vals[3], d64_vals[4], d64_vals[5],
                d128_vals[6], d128_vals[7], d64_vals[8],
                d64_vals[9]);
            
            result += (long double)d128_result + 
                     (long double)d128_helper_result;
#else
            /* Fallback DFP emulation */
            dfp128_emu_t dfp_result = dfp_emu_helper_10(
                dfp_emu_vals[0], dfp_emu_vals[1], dfp_emu_vals[2],
                dfp_emu_vals[3], dfp_emu_vals[4], dfp_emu_vals[5],
                dfp_emu_vals[6], dfp_emu_vals[7], dfp_emu_vals[8],
                dfp_emu_vals[9]);
            
            result += (long double)dfp_result.lo + 
                     (long double)dfp_result.hi;
#endif
        } else {
            /* Complex number operations */
#if HAS_COMPLEX
            long double complex cl_result = 
                (cl_vals[0] * cl_vals[1]) / (cl_vals[2] - cl_vals[3]) +
                (cl_vals[4] * cl_vals[5]) / (cl_vals[6] - cl_vals[7]) -
                (cl_vals[8] * cl_vals[9]);
            
            /* Complex power operation - may expand to many operands */
            long double complex cl_pow = cpow(cl_vals[0], cl_vals[1]);
            
            result += creal(cl_result) + cimag(cl_result) +
                     creal(cl_pow) + cimag(cl_pow);
#else
            /* Fallback complex emulation */
            long double cl_real_result = 
                (cl_real[0] * cl_real[1] - cl_imag[0] * cl_imag[1]) /
                (cl_real[2] - cl_real[3]) +
                (cl_real[4] * cl_real[5] - cl_imag[4] * cl_imag[5]) /
                (cl_real[6] - cl_real[7]) -
                (cl_real[8] * cl_real[9] - cl_imag[8] * cl_imag[9]);
            
            result += cl_real_result;
#endif
        }
        
        /* Vector reduction - always executed */
        float vec_result = vector_reduce_accumulate(
            vec_vals[0], vec_vals[1], vec_vals[2], vec_vals[3]);
        
        result += vec_result;
        
        /* Call helper with 11 arguments - mixing different computations */
        long double helper_result = complex_helper_11(
            result, result * 0.5, result * 0.25,
            result * 0.125, result * 0.0625, result * 0.03125,
            result * 0.015625, result * 0.0078125, result * 0.00390625,
            result * 0.001953125, result * 0.0009765625);
        
        /* Accumulate result to prevent optimization */
        g_result_accumulator += (uint64_t)helper_result;
        
        /* Modify values slightly for next iteration */
        for (int i = 0; i < 10; i++) {
#if HAS_DFP
            d64_vals[i] = d64_vals[i] * (_Decimal64)1.01;
            d128_vals[i] = d128_vals[i] * (_Decimal128)1.01;
#else
            dfp_emu_vals[i].lo = dfp_emu_vals[i].lo * 101 / 100;
            dfp_emu_vals[i].hi = dfp_emu_vals[i].hi * 101 / 100;
#endif
            
#if HAS_COMPLEX
            cl_vals[i] = cl_vals[i] * (1.0 + 0.01 * I);
#else
            cl_real[i] = cl_real[i] * 1.01;
            cl_imag[i] = cl_imag[i] * 1.01;
#endif
        }
        
        /* Toggle condition */
        condition = !condition;
    }
}

int main(int argc, char *argv[])
{
    int seed = 42;
    int iterations = 3;
    
    /* Use command line seed if provided */
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    if (argc > 2) {
        iterations = atoi(argv[2]);
        if (iterations < 1) iterations = 1;
        if (iterations > 5) iterations = 5;
    }
    
    printf("Starting computations with seed=%d, iterations=%d\n", 
           seed, iterations);
    
    /* Perform the computations */
    perform_computations(seed, iterations);
    
    /* Compute and print checksum */
    uint64_t checksum = g_result_accumulator;
    printf("Result checksum: %llu\n", (unsigned long long)checksum);
    
    return 0;
}
