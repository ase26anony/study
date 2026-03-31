#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* Feature detection and fallbacks */
#ifdef __STDC_IEC_559_COMPLEX__
#include <complex.h>
#endif

/* Decimal Floating Point support detection */
#ifdef __DECIMAL_BID_FORMAT__
#define DFP_SUPPORTED 1
#else
#define DFP_SUPPORTED 0
#endif

/* Vector extensions support */
#ifdef __VECTOR_TYPES_SUPPORTED__
#define VECTOR_SUPPORTED 1
#else
#define VECTOR_SUPPORTED 0
#endif

/* Global volatile to prevent optimization */
volatile uint64_t g_result = 0;

/* Helper function with 11 arguments - matches case 11 in optabs.cc */
static __attribute__((noinline)) 
long double helper_11_args(
    long double a1, long double a2, long double a3,
    long double a4, long double a5, long double a6,
    long double a7, long double a8, long double a9,
    long double a10, long double a11)
{
    /* Complex expression that may expand to many operands */
    return ((a1 * a2) + (a3 / a4) - (a5 * a6) + 
            (a7 - a8) * (a9 + a10) / a11);
}

/* Helper function with 10 arguments - matches case 10 in optabs.cc */
static __attribute__((noinline))
double helper_10_args(
    double b1, double b2, double b3, double b4, double b5,
    double b6, double b7, double b8, double b9, double b10)
{
    /* Multi-operand expression */
    return b1 + b2 - b3 * b4 / b5 + b6 * b7 - b8 + b9 * b10;
}

#if DFP_SUPPORTED
/* DFP helper with many operands */
static __attribute__((noinline))
_Decimal128 dfp_helper(
    _Decimal64 d1, _Decimal64 d2, _Decimal64 d3,
    _Decimal64 d4, _Decimal64 d5, _Decimal64 d6,
    _Decimal128 d7, _Decimal128 d8, _Decimal128 d9,
    _Decimal128 d10)
{
    /* Complex DFP expression that may require many operands */
    return ((_Decimal128)d1 * (_Decimal128)d2 + 
            (_Decimal128)d3 / (_Decimal128)d4 -
            (_Decimal128)d5 * (_Decimal128)d6 +
            d7 * d8 - d9 / d10);
}
#else
/* Fallback for DFP using integer arrays */
typedef struct {
    uint64_t lo;
    uint64_t hi;
} dfp128_fallback;

static __attribute__((noinline))
dfp128_fallback dfp_fallback_helper(
    uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4,
    uint64_t a5, uint64_t a6, uint64_t a7, uint64_t a8,
    uint64_t a9, uint64_t a10)
{
    /* Simulate multi-operand arithmetic */
    dfp128_fallback result;
    result.lo = a1 + a2 - a3 * a4 + a5 / (a6 + 1) + a7 - a8 * a9 + a10;
    result.hi = a1 * a2 + a3 - a4 / (a5 + 1) + a6 * a7 - a8 + a9 * a10;
    return result;
}
#endif

#if defined(__STDC_IEC_559_COMPLEX__) && defined(complex) && defined(I)
/* Complex number helper */
static __attribute__((noinline))
long double _Complex complex_helper(
    long double _Complex c1, long double _Complex c2,
    long double _Complex c3, long double _Complex c4,
    long double _Complex c5, long double _Complex c6,
    long double _Complex c7, long double _Complex c8,
    long double _Complex c9, long double _Complex c10)
{
    /* Complex expression that may expand to many real/imaginary operands */
    return (c1 * c2 + c3 / c4 - c5 * c6 + c7 * c8 - c9 / c10);
}
#endif

#if VECTOR_SUPPORTED
/* Vector types using GCC extensions */
typedef int32_t v8si __attribute__((vector_size(32)));  /* 8 x int32 */
typedef double v4df __attribute__((vector_size(32)));   /* 4 x double */

/* Vector reduction with accumulation */
static __attribute__((noinline))
double vector_reduce(v4df vec, double accumulator)
{
    /* Horizontal reduction that may expand to many operations */
    double sum = accumulator;
    sum += vec[0] + vec[1] + vec[2] + vec[3];
    
    /* Additional operations to increase operand count */
    v4df temp = vec * vec + vec;
    sum += temp[0] - temp[1] + temp[2] - temp[3];
    
    return sum;
}
#endif

int main(int argc, char *argv[])
{
    int seed = 12345;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    srand(seed);
    
    /* Storage for results to prevent dead code elimination */
    volatile double results[10] = {0};
    int result_idx = 0;
    
    /* Initialize base variables */
    double base1 = 1.23456789;
    double base2 = 9.87654321;
    long double ld1 = 3.141592653589793238L;
    long double ld2 = 2.718281828459045235L;
    
#if DFP_SUPPORTED
    /* DFP variables */
    _Decimal64 df64_vals[6];
    _Decimal128 df128_vals[4];
    
    for (int i = 0; i < 6; i++) {
        df64_vals[i] = (_Decimal64)(i + 1) * 1.5DF;
    }
    for (int i = 0; i < 4; i++) {
        df128_vals[i] = (_Decimal128)(i + 1) * 2.5DL;
    }
#else
    /* Fallback DFP simulation */
    uint64_t dfp_sim[10];
    for (int i = 0; i < 10; i++) {
        dfp_sim[i] = (uint64_t)(i + 1) * seed;
    }
#endif

#if defined(__STDC_IEC_559_COMPLEX__) && defined(complex) && defined(I)
    /* Complex variables */
    long double _Complex cvals[10];
    for (int i = 0; i < 10; i++) {
        cvals[i] = (i + 1) * 0.5L + (i * 0.3L) * I;
    }
#endif

#if VECTOR_SUPPORTED
    /* Vector variables */
    v4df vec1 = {1.0, 2.0, 3.0, 4.0};
    v4df vec2 = {5.0, 6.0, 7.0, 8.0};
    double vec_accumulator = 0.0;
#endif

    /* Main computation loop - 4 iterations */
    for (int iter = 0; iter < 4; iter++) {
        volatile int condition = seed + iter;  /* Prevent constant folding */
        
        if (condition & 1) {
            /* Branch 1: Call helper with 11 arguments */
            long double arg1 = ld1 * iter;
            long double arg2 = ld2 / (iter + 1);
            long double arg3 = base1 + iter;
            long double arg4 = base2 - iter;
            long double arg5 = arg1 * arg2;
            long double arg6 = arg3 / arg4;
            long double arg7 = arg5 + arg6;
            long double arg8 = arg5 - arg6;
            long double arg9 = arg7 * arg8;
            long double arg10 = arg7 / (arg8 + 1.0L);
            long double arg11 = arg9 + arg10;
            
            long double res1 = helper_11_args(
                arg1, arg2, arg3, arg4, arg5,
                arg6, arg7, arg8, arg9, arg10,
                arg11);
            
            results[result_idx++] = (double)res1;
        } else {
            /* Branch 2: Call helper with 10 arguments */
            double d1 = base1 * iter;
            double d2 = base2 / (iter + 1);
            double d3 = d1 + d2;
            double d4 = d1 - d2;
            double d5 = d3 * d4;
            double d6 = d3 / (d4 + 1.0);
            double d7 = d5 + d6;
            double d8 = d5 - d6;
            double d9 = d7 * d8;
            double d10 = d7 / (d8 + 1.0);
            
            double res2 = helper_10_args(
                d1, d2, d3, d4, d5,
                d6, d7, d8, d9, d10);
            
            results[result_idx++] = res2;
        }
        
        /* DFP operations */
#if DFP_SUPPORTED
        if (condition & 2) {
            _Decimal128 dfp_res = dfp_helper(
                df64_vals[0], df64_vals[1], df64_vals[2],
                df64_vals[3], df64_vals[4], df64_vals[5],
                df128_vals[0], df128_vals[1], df128_vals[2],
                df128_vals[3]);
            
            /* Convert DFP to double for storage */
            results[result_idx++] = (double)dfp_res;
        }
#else
        if (condition & 2) {
            dfp128_fallback fb_res = dfp_fallback_helper(
                dfp_sim[0], dfp_sim[1], dfp_sim[2], dfp_sim[3],
                dfp_sim[4], dfp_sim[5], dfp_sim[6], dfp_sim[7],
                dfp_sim[8], dfp_sim[9]);
            
            results[result_idx++] = (double)(fb_res.lo + fb_res.hi);
        }
#endif

        /* Complex number operations */
#if defined(__STDC_IEC_559_COMPLEX__) && defined(complex) && defined(I)
        if (condition & 4) {
            /* Create complex expressions that may expand to many operands */
            long double _Complex cexpr1 = cvals[0] * cvals[1] + cvals[2] / cvals[3];
            long double _Complex cexpr2 = cvals[4] - cvals[5] * cvals[6];
            long double _Complex cexpr3 = cvals[7] / cvals[8] + cvals[9];
            
            long double _Complex cexpr4 = (cexpr1 * cexpr2) / (cexpr3 + 1.0L * I);
            long double _Complex cexpr5 = cexpr1 / cexpr2 - cexpr3 * (1.0L - I);
            
            long double _Complex cres = complex_helper(
                cexpr1, cexpr2, cexpr3, cexpr4, cexpr5,
                cvals[0], cvals[1], cvals[2], cvals[3], cvals[4]);
            
            results[result_idx++] = (double)creal(cres) + (double)cimag(cres);
        }
#endif

        /* Vector operations */
#if VECTOR_SUPPORTED
        if (condition & 8) {
            /* Vector operations that may expand to many scalar operations */
            v4df vec_op1 = vec1 * vec2 + vec1;
            v4df vec_op2 = vec1 / (vec2 + 1.0) - vec2;
            v4df vec_op3 = vec_op1 * vec_op2 + vec1 / vec2;
            
            vec_accumulator = vector_reduce(vec_op1, vec_accumulator);
            vec_accumulator = vector_reduce(vec_op2, vec_accumulator);
            vec_accumulator = vector_reduce(vec_op3, vec_accumulator);
            
            results[result_idx++] = vec_accumulator;
        }
#endif
        
        /* Prevent result_idx from going out of bounds */
        if (result_idx >= 9) result_idx = 0;
    }
    
    /* Compute checksum */
    uint64_t checksum = 0;
    for (int i = 0; i < 10; i++) {
        /* Access through volatile pointer to ensure reads */
        volatile double *volatile ptr = &results[i];
        checksum += (uint64_t)(*ptr * 1000000.0);
    }
    
    g_result = checksum;
    printf("Checksum: %llu\n", (unsigned long long)checksum);
    
    return 0;
}
