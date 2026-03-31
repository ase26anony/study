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
#if defined(__GNUC__) && (defined(__x86_64__) || defined(__i386__) || defined(__aarch64__))
    #define VECTOR_SUPPORTED 1
    typedef int32_t int32x8_t __attribute__((vector_size(32)));
    typedef double float64x4_t __attribute__((vector_size(32)));
#else
    #define VECTOR_SUPPORTED 0
    typedef struct { int32_t data[8]; } int32x8_t;
    typedef struct { double data[4]; } float64x4_t;
#endif

/* DFP fallback structures */
#if !DFP_SUPPORTED
typedef union {
    unsigned long long ull[2];
    double d;
} decimal64_fallback;

typedef union {
    unsigned long long ull[4];
    long double ld;
} decimal128_fallback;

#define _Decimal64 decimal64_fallback
#define _Decimal128 decimal128_fallback
#endif

/* Complex fallback */
#if !COMPLEX_SUPPORTED
typedef struct {
    long double real;
    long double imag;
} long_double_complex;
#define long double _Complex long_double_complex
#endif

/* Helper function with 11 arguments - marked noinline to prevent optimization */
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

/* Helper function with 10 arguments */
static __attribute__((noinline))
_Decimal128 helper_10_args_dfp(
    _Decimal128 d1, _Decimal128 d2, _Decimal128 d3,
    _Decimal128 d4, _Decimal128 d5, _Decimal128 d6,
    _Decimal128 d7, _Decimal128 d8, _Decimal128 d9,
    _Decimal128 d10)
{
#if DFP_SUPPORTED
    /* DFP arithmetic that may require multi-operand expansion */
    return ((d1 * d2) + (d3 * d4) - (d5 / d6) + 
            (d7 - d8) * (d9 + d10));
#else
    /* Fallback using integer arithmetic */
    decimal128_fallback result;
    result.ull[0] = d1.ull[0] + d2.ull[0] + d3.ull[0] + d4.ull[0];
    result.ull[1] = d5.ull[1] + d6.ull[1] + d7.ull[1] + d8.ull[1];
    result.ull[2] = d9.ull[2] + d10.ull[2];
    result.ull[3] = 0;
    return result;
#endif
}

/* Vector reduction with accumulation */
static long double vector_reduction(float64x4_t vec, long double accum)
{
#if VECTOR_SUPPORTED
    /* Horizontal reduction that may expand to multiple operations */
    float64x4_t temp = vec + __builtin_shuffle(vec, (int32x8_t){2,3,0,1,6,7,4,5});
    temp = temp + __builtin_shuffle(temp, (int32x8_t){1,0,3,2,5,4,7,6});
    return accum + temp[0] + temp[2];
#else
    /* Manual reduction */
    long double sum = accum;
    for (int i = 0; i < 4; i++) {
        sum += vec.data[i];
    }
    return sum;
#endif
}

/* Complex number operations */
static long double complex complex_operations(
    long double complex a, 
    long double complex b,
    long double complex c,
    long double complex d)
{
#if COMPLEX_SUPPORTED
    /* Complex expression that may expand real/imag parts separately */
    return (a * b) / (c - d) + csqrt(a + b) * cpow(c, d);
#else
    /* Manual complex arithmetic */
    long_double_complex result;
    result.real = (a.real * b.real - a.imag * b.imag) / 
                  (c.real - d.real);
    result.imag = (a.real * b.imag + a.imag * b.real) / 
                  (c.imag - d.imag);
    return result;
#endif
}

/* Volatile storage to prevent dead code elimination */
static volatile long double result_store[10];
static volatile int store_idx = 0;

int main(int argc, char *argv[])
{
    int seed = (argc > 1) ? atoi(argv[1]) : 42;
    srand(seed);
    
    /* Initialize base variables */
#if DFP_SUPPORTED
    _Decimal64 d64_a = 1.23456789dd;
    _Decimal64 d64_b = 9.87654321dd;
    _Decimal128 d128_a = 1.2345678901234567dl;
    _Decimal128 d128_b = 9.8765432109876543dl;
    _Decimal128 d128_c = 5.5555555555555555dl;
    _Decimal128 d128_d = 3.3333333333333333dl;
#else
    _Decimal64 d64_a = {{123456789ULL, 0}};
    _Decimal64 d64_b = {{987654321ULL, 0}};
    _Decimal128 d128_a = {{12345678901234567ULL, 0, 0, 0}};
    _Decimal128 d128_b = {{98765432109876543ULL, 0, 0, 0}};
    _Decimal128 d128_c = {{55555555555555555ULL, 0, 0, 0}};
    _Decimal128 d128_d = {{33333333333333333ULL, 0, 0, 0}};
#endif
    
#if COMPLEX_SUPPORTED
    long double complex ca = 1.0L + 2.0LI;
    long double complex cb = 3.0L + 4.0LI;
    long double complex cc = 5.0L + 6.0LI;
    long double complex cd = 7.0L + 8.0LI;
#else
    long_double_complex ca = {1.0L, 2.0L};
    long_double_complex cb = {3.0L, 4.0L};
    long_double_complex cc = {5.0L, 6.0L};
    long_double_complex cd = {7.0L, 8.0L};
#endif
    
#if VECTOR_SUPPORTED
    float64x4_t vec1 = {1.0, 2.0, 3.0, 4.0};
    float64x4_t vec2 = {5.0, 6.0, 7.0, 8.0};
#else
    float64x4_t vec1 = {{1.0, 2.0, 3.0, 4.0}};
    float64x4_t vec2 = {{5.0, 6.0, 7.0, 8.0}};
#endif
    
    long double accum = 0.0L;
    long double checksum = 0.0L;
    
    /* Main computation loop */
    for (int iter = 0; iter < 4; iter++) {
        volatile int condition = (iter % 2) ? 1 : 0;
        
        if (condition) {
            /* DFP arithmetic block */
#if DFP_SUPPORTED
            _Decimal128 d128_result = (d128_a * d128_b) + 
                                     (d128_c / d128_d) - 
                                     (d128_a + d128_b) * 
                                     (d128_c - d128_d);
            
            /* Call helper with 10 DFP arguments */
            _Decimal128 helper_result = helper_10_args_dfp(
                d128_a, d128_b, d128_c, d128_d,
                d128_result, d128_a * d128_c,
                d128_b / d128_d, d128_result + d128_a,
                d128_c - d128_result, d128_d * d128_b);
            
            /* Convert to long double for storage */
            accum += (long double)helper_result;
#else
            /* Fallback DFP-like arithmetic */
            decimal128_fallback temp;
            temp.ull[0] = d128_a.ull[0] * d128_b.ull[0] + 
                         d128_c.ull[0] / d128_d.ull[0];
            temp.ull[1] = d128_a.ull[1] + d128_b.ull[1] - 
                         d128_c.ull[1] * d128_d.ull[1];
            accum += (long double)temp.ull[0] + (long double)temp.ull[1];
#endif
        } else {
            /* Complex arithmetic block */
            long double complex cl_result = complex_operations(ca, cb, cc, cd);
            
            /* Additional complex operations */
            long double complex cl_temp = (ca * cb) / (cc - cd) + 
                                         csqrt(ca + cb) * cpow(cc, cd);
            
#if COMPLEX_SUPPORTED
            accum += creall(cl_result) + cimagl(cl_temp);
#else
            accum += ((long_double_complex)cl_result).real + 
                    ((long_double_complex)cl_temp).imag;
#endif
        }
        
        /* Vector reduction */
        float64x4_t vec_sum;
#if VECTOR_SUPPORTED
        vec_sum = vec1 + vec2 * (float64x4_t){1.0, 2.0, 3.0, 4.0};
#else
        for (int i = 0; i < 4; i++) {
            vec_sum.data[i] = vec1.data[i] + vec2.data[i] * (i + 1.0);
        }
#endif
        
        accum = vector_reduction(vec_sum, accum);
        
        /* Call helper with 11 arguments using mixed computations */
        long double helper_val = helper_11_args(
            accum, (long double)iter * 1.5L,
            creall(ca), cimagl(cb),
            (long double)d64_a.ull[0] / 1000000000.0L,
            (long double)d64_b.ull[0] / 1000000000.0L,
            vec_sum.data[0], vec_sum.data[1],
            vec_sum.data[2], vec_sum.data[3],
            (long double)seed / 100.0L);
        
        /* Store result to prevent elimination */
        if (store_idx < 10) {
            result_store[store_idx++] = helper_val;
            checksum += helper_val;
        }
        
        /* Modify variables for next iteration */
#if DFP_SUPPORTED
        d128_a = d128_a * 1.1dl;
        d128_b = d128_b / 1.1dl;
#else
        d128_a.ull[0] = (uint64_t)(d128_a.ull[0] * 1.1);
        d128_b.ull[0] = (uint64_t)(d128_b.ull[0] / 1.1);
#endif
        
#if COMPLEX_SUPPORTED
        ca = ca * (1.0L + 0.1LI);
        cb = cb / (1.0L + 0.1LI);
#else
        ca.real *= 1.1L;
        cb.imag /= 1.1L;
#endif
    }
    
    /* Final checksum and output */
    for (int i = 0; i < store_idx; i++) {
        checksum += result_store[i];
    }
    
    printf("Checksum: %.15Lf\n", checksum);
    printf("Result count: %d\n", store_idx);
    
    return 0;
}
