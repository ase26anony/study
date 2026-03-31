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
    /* Fallback DFP types using unions */
    typedef union {
        uint64_t u64;
        double   f64;
    } decimal64_fallback;
    
    typedef union {
        struct {
            uint64_t lo;
            uint64_t hi;
        } parts;
        long double ld;
    } decimal128_fallback;
#endif

#ifdef __STDC_IEC_559_COMPLEX__
    #include <complex.h>
    #define HAS_COMPLEX 1
#else
    #define HAS_COMPLEX 0
    typedef struct {
        long double real;
        long double imag;
    } complex_fallback;
#endif

/* Vector extensions if supported */
#ifdef __VECTOR_TYPES_SUPPORTED__
    typedef int32_t int32x8_t __attribute__((vector_size(32)));
    typedef float float64x4_t __attribute__((vector_size(32)));
    #define HAS_VECTORS 1
#else
    #define HAS_VECTORS 0
    typedef struct {
        int32_t data[8];
    } int32x8_fallback;
    typedef struct {
        double data[4];
    } float64x4_fallback;
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

/* Another helper with 10 arguments */
static __attribute__((noinline))
long double helper_10_args(
    long double a1, long double a2, long double a3,
    long double a4, long double a5, long double a6,
    long double a7, long double a8, long double a9,
    long double a10)
{
    return (a1 + a2) * (a3 - a4) / (a5 * a6) + 
           (a7 / a8) - (a9 * a10);
}

/* DFP operations */
#if HAS_DFP
static _Decimal128 dfp_complex_operation(_Decimal64 a, _Decimal64 b, 
                                        _Decimal64 c, _Decimal64 d,
                                        _Decimal128 e, _Decimal128 f)
{
    /* This complex DFP expression may expand to many operands */
    _Decimal128 result = (_Decimal128)a * (_Decimal128)b + 
                        (_Decimal128)c / (_Decimal128)d;
    result = result * e - f / (_Decimal128)a;
    return result + (_Decimal128)b * (_Decimal128)c;
}
#else
static long double dfp_fallback_operation(decimal64_fallback a, 
                                         decimal64_fallback b,
                                         decimal64_fallback c,
                                         decimal64_fallback d,
                                         decimal128_fallback e,
                                         decimal128_fallback f)
{
    /* Simulate DFP with manual precision handling */
    long double a_ld = a.f64;
    long double b_ld = b.f64;
    long double c_ld = c.f64;
    long double d_ld = d.f64;
    long double e_ld = e.ld;
    long double f_ld = f.ld;
    
    return (a_ld * b_ld + c_ld / d_ld) * e_ld - 
           f_ld / a_ld + b_ld * c_ld;
}
#endif

/* Complex number operations */
#if HAS_COMPLEX
static long double _Complex complex_high_precision(
    long double _Complex a, long double _Complex b,
    long double _Complex c, long double _Complex d)
{
    /* Complex expression that may require many operands */
    long double _Complex result = (a * b) / (c - d);
    result = result + csqrt(a * b + c * d);
    return cpow(result, 2.0L);
}
#else
static complex_fallback complex_fallback_op(
    complex_fallback a, complex_fallback b,
    complex_fallback c, complex_fallback d)
{
    /* Manual complex arithmetic */
    complex_fallback result;
    long double denom_real = c.real - d.real;
    long double denom_imag = c.imag - d.imag;
    long double denom_sq = denom_real * denom_real + denom_imag * denom_imag;
    
    /* (a * b) / (c - d) */
    long double ab_real = a.real * b.real - a.imag * b.imag;
    long double ab_imag = a.real * b.imag + a.imag * b.real;
    
    result.real = (ab_real * denom_real + ab_imag * denom_imag) / denom_sq;
    result.imag = (ab_imag * denom_real - ab_real * denom_imag) / denom_sq;
    
    return result;
}
#endif

/* Vector reduction */
#if HAS_VECTORS
static int32_t vector_reduction(int32x8_t vec, int32_t accumulator)
{
    /* Horizontal reduction that may expand to many operations */
    int32_t sum = vec[0] + vec[1] + vec[2] + vec[3] + 
                  vec[4] + vec[5] + vec[6] + vec[7];
    return accumulator + sum * 2 - sum / 3;
}
#else
static int32_t vector_fallback_reduction(int32x8_fallback vec, int32_t accumulator)
{
    int32_t sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += vec.data[i];
    }
    return accumulator + sum * 2 - sum / 3;
}
#endif

/* Volatile storage to prevent dead code elimination */
static volatile long double result_storage[10];
static volatile int storage_index = 0;

int main(int argc, char *argv[])
{
    int seed = 12345;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    srand(seed);
    
    /* Initialize base variables */
#if HAS_DFP
    _Decimal64 d64_a = 1.23456dd;
    _Decimal64 d64_b = 2.34567dd;
    _Decimal64 d64_c = 3.45678dd;
    _Decimal64 d64_d = 4.56789dd;
    _Decimal128 d128_e = 5.67890123456789dl;
    _Decimal128 d128_f = 6.78901234567890dl;
#else
    decimal64_fallback d64_a = { .f64 = 1.23456 };
    decimal64_fallback d64_b = { .f64 = 2.34567 };
    decimal64_fallback d64_c = { .f64 = 3.45678 };
    decimal64_fallback d64_d = { .f64 = 4.56789 };
    decimal128_fallback d128_e = { .ld = 5.67890123456789L };
    decimal128_fallback d128_f = { .ld = 6.78901234567890L };
#endif

#if HAS_COMPLEX
    long double _Complex ca = 1.0L + 2.0L * I;
    long double _Complex cb = 3.0L + 4.0L * I;
    long double _Complex cc = 5.0L + 6.0L * I;
    long double _Complex cd = 7.0L + 8.0L * I;
#else
    complex_fallback ca = { .real = 1.0L, .imag = 2.0L };
    complex_fallback cb = { .real = 3.0L, .imag = 4.0L };
    complex_fallback cc = { .real = 5.0L, .imag = 6.0L };
    complex_fallback cd = { .real = 7.0L, .imag = 8.0L };
#endif

#if HAS_VECTORS
    int32x8_t vec = { 1, 2, 3, 4, 5, 6, 7, 8 };
#else
    int32x8_fallback vec = { .data = { 1, 2, 3, 4, 5, 6, 7, 8 } };
#endif
    
    int accumulator = 0;
    long double checksum = 0.0L;
    
    /* Main computation loop - 4 iterations */
    for (int iter = 0; iter < 4; iter++) {
        volatile int condition = seed + iter;  /* Prevent constant folding */
        
        if (condition % 2 == 0) {
            /* Branch 1: Complex DFP operations */
#if HAS_DFP
            _Decimal128 dfp_result = dfp_complex_operation(
                d64_a, d64_b, d64_c, d64_d, d128_e, d128_f);
            long double ld_result = (long double)dfp_result;
#else
            long double ld_result = dfp_fallback_operation(
                d64_a, d64_b, d64_c, d64_d, d128_e, d128_f);
#endif
            
            /* Complex number operations */
#if HAS_COMPLEX
            long double _Complex complex_result = complex_high_precision(
                ca, cb, cc, cd);
            long double complex_magnitude = cabsl(complex_result);
#else
            complex_fallback complex_fb_result = complex_fallback_op(
                ca, cb, cc, cd);
            long double complex_magnitude = sqrtl(
                complex_fb_result.real * complex_fb_result.real +
                complex_fb_result.imag * complex_fb_result.imag);
#endif
            
            /* Vector reduction */
#if HAS_VECTORS
            accumulator = vector_reduction(vec, accumulator);
#else
            accumulator = vector_fallback_reduction(vec, accumulator);
#endif
            
            /* Call helper with many arguments */
            long double helper_result = helper_11_args(
                ld_result, complex_magnitude, (long double)accumulator,
                (long double)(seed + 1), (long double)(seed + 2),
                (long double)(seed + 3), (long double)(seed + 4),
                (long double)(seed + 5), (long double)(seed + 6),
                (long double)(seed + 7), (long double)(seed + 8));
            
            result_storage[storage_index++ % 10] = helper_result;
            checksum += helper_result;
        } else {
            /* Branch 2: Different mix of operations */
#if HAS_DFP
            _Decimal128 dfp_result2 = (_Decimal128)d64_a * (_Decimal128)d64_b +
                                     (_Decimal128)d64_c / (_Decimal128)d64_d;
            long double ld_result2 = (long double)dfp_result2;
#else
            long double ld_result2 = d64_a.f64 * d64_b.f64 + 
                                   d64_c.f64 / d64_d.f64;
#endif
            
            /* Another complex expression */
#if HAS_COMPLEX
            long double _Complex temp = ca * cb - cc * cd;
            long double complex_real2 = creall(temp);
            long double complex_imag2 = cimagl(temp);
#else
            long double complex_real2 = ca.real * cb.real - ca.imag * cb.imag -
                                       cc.real * cd.real + cc.imag * cd.imag;
            long double complex_imag2 = ca.real * cb.imag + ca.imag * cb.real -
                                       cc.real * cd.imag - cc.imag * cd.real;
#endif
            
            /* Vector operations with accumulation */
            int vec_temp = 0;
#if HAS_VECTORS
            for (int i = 0; i < 8; i++) {
                vec_temp += vec[i] * (i + 1);
            }
#else
            for (int i = 0; i < 8; i++) {
                vec_temp += vec.data[i] * (i + 1);
            }
#endif
            accumulator += vec_temp;
            
            /* Call 10-argument helper */
            long double helper_result2 = helper_10_args(
                ld_result2, complex_real2, complex_imag2,
                (long double)accumulator, (long double)(seed * 2),
                (long double)(seed * 3), (long double)(seed * 4),
                (long double)(seed * 5), (long double)(seed * 6),
                (long double)(seed * 7));
            
            result_storage[storage_index++ % 10] = helper_result2;
            checksum += helper_result2;
        }
        
        /* Modify some values for next iteration */
        seed = (seed * 1103515245 + 12345) & 0x7fffffff;
    }
    
    /* Final checksum and output */
    printf("Checksum: %Lf\n", checksum);
    printf("Accumulator: %d\n", accumulator);
    
    return 0;
}
