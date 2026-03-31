/* 
 * Test program targeting GCC optabs.cc lines 8254-8263
 * Designed to trigger internal function expansion with 10-11 operands
 * through DFP, complex, and vector operations.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Feature detection and fallbacks */
#ifdef __DECIMAL_BID_FORMAT__
    #define HAS_DFP 1
    #include <decimal.h>
#else
    #define HAS_DFP 0
#endif

#ifdef __STDC_IEC_559_COMPLEX__
    #include <complex.h>
    #define HAS_COMPLEX 1
#else
    #define HAS_COMPLEX 0
#endif

#ifdef __VECTOR_TYPES_SUPPORTED__
    #define HAS_VECTORS 1
#else
    #define HAS_VECTORS 0
#endif

/* DFP fallback structures */
#if !HAS_DFP
typedef struct {
    unsigned long long hi;
    unsigned long long lo;
} decimal64_fb;

typedef struct {
    unsigned long long w[2];
} decimal128_fb;
#endif

/* Complex fallback */
#if !HAS_COMPLEX
typedef struct {
    long double re;
    long double im;
} complex_fb;
#endif

/* Vector types */
#if HAS_VECTORS
    typedef int32_t int32x8_t __attribute__((vector_size(32)));
    typedef float float64x4_t __attribute__((vector_size(32)));
#else
    typedef struct {
        int32_t data[8];
    } int32x8_t;
    typedef struct {
        double data[4];
    } float64x4_t;
#endif

/* Global volatile to prevent optimization */
volatile uint64_t g_result = 0;

/* Helper function with 11 arguments - targeting case 11 */
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

/* Helper function with 10 arguments - targeting case 10 */
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
static _Decimal128 dfp_complex_expr(_Decimal64 a, _Decimal64 b, 
                                    _Decimal64 c, _Decimal64 d,
                                    _Decimal128 e, _Decimal128 f)
{
    /* Multi-operand DFP expression */
    _Decimal128 t1 = (_Decimal128)a * (_Decimal128)b;
    _Decimal128 t2 = (_Decimal128)c / (_Decimal128)d;
    _Decimal128 t3 = e + f;
    _Decimal128 t4 = t1 - t2;
    _Decimal128 t5 = t3 * t4;
    
    /* Use builtins that may expand to multi-operand patterns */
    _Decimal128 result = __builtin_dadd(t5, t1);
    result = __builtin_dmul(result, t2);
    result = __builtin_ddiv(result, t3);
    
    return result;
}
#else
/* Fallback DFP-like operations using integer arithmetic */
static uint64_t dfp_fallback(uint64_t a, uint64_t b, uint64_t c, 
                             uint64_t d, uint64_t e[2], uint64_t f[2])
{
    /* Simulate multi-precision arithmetic */
    unsigned __int128 t1 = (unsigned __int128)a * b;
    unsigned __int128 t2 = (unsigned __int128)c * 1000000000ULL / d;
    
    unsigned __int128 e128 = ((unsigned __int128)e[1] << 64) | e[0];
    unsigned __int128 f128 = ((unsigned __int128)f[1] << 64) | f[0];
    unsigned __int128 t3 = e128 + f128;
    
    unsigned __int128 t4 = t1 - t2;
    unsigned __int128 t5 = t3 * t4;
    
    return (uint64_t)(t5 >> 64) + (uint64_t)t5;
}
#endif

/* Complex number operations */
#if HAS_COMPLEX
static long double _Complex complex_high_precision(
    long double _Complex a, long double _Complex b,
    long double _Complex c, long double _Complex d)
{
    /* Complex expression that may expand to many real/imag operations */
    long double _Complex t1 = a * b;
    long double _Complex t2 = c / d;
    long double _Complex t3 = t1 + t2;
    long double _Complex t4 = t1 - t2;
    
    /* Library calls that may expand internally */
    long double _Complex sqrt_t3 = csqrt(t3);
    long double _Complex pow_t4 = cpow(t4, 2.0L);
    
    return sqrt_t3 * pow_t4;
}
#else
static complex_fb complex_fallback(complex_fb a, complex_fb b,
                                   complex_fb c, complex_fb d)
{
    complex_fb result;
    /* Manual complex arithmetic */
    result.re = (a.re * b.re - a.im * b.im) + 
                (c.re / d.re - c.im / d.im);
    result.im = (a.re * b.im + a.im * b.re) +
                (c.im / d.re + c.re / d.im);
    return result;
}
#endif

/* Vector reduction */
#if HAS_VECTORS
static int32_t vector_reduction(int32x8_t v, int32_t accum)
{
    /* Horizontal reduction that may expand to many operations */
    int32_t sum = v[0] + v[1] + v[2] + v[3] + 
                  v[4] + v[5] + v[6] + v[7];
    
    /* Additional accumulation operations */
    sum = sum * accum;
    sum = sum + (v[0] * v[1]);
    sum = sum - (v[2] * v[3]);
    sum = sum + (v[4] * v[5]);
    sum = sum - (v[6] * v[7]);
    
    return sum;
}

static double vector_fp_reduction(float64x4_t v, double accum)
{
    double sum = v[0] + v[1] + v[2] + v[3];
    sum = sum * accum;
    sum = sum + (v[0] * v[1]);
    sum = sum - (v[2] * v[3]);
    return sum;
}
#else
static int32_t vector_reduction_fb(int32x8_t v, int32_t accum)
{
    int32_t sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += v.data[i];
    }
    sum = sum * accum;
    for (int i = 0; i < 8; i += 2) {
        sum += v.data[i] * v.data[i+1];
    }
    return sum;
}
#endif

int main(int argc, char *argv[])
{
    /* Initialize with seed for deterministic behavior */
    int seed = (argc > 1) ? atoi(argv[1]) : 42;
    srand(seed);
    
    /* Storage for results to prevent dead code elimination */
    long double results[5] = {0};
    
    /* Base variables */
    long double base_vals[20];
    for (int i = 0; i < 20; i++) {
        base_vals[i] = (long double)(rand() % 1000) / 100.0L + 1.0L;
    }
    
    /* Initialize vectors */
#if HAS_VECTORS
    int32x8_t vec_int = {1, 2, 3, 4, 5, 6, 7, 8};
    float64x4_t vec_fp = {1.0, 2.0, 3.0, 4.0};
#else
    int32x8_t vec_int = {{1, 2, 3, 4, 5, 6, 7, 8}};
    float64x4_t vec_fp = {{1.0, 2.0, 3.0, 4.0}};
#endif
    
    /* Main computation loop */
    for (int iter = 0; iter < 3; iter++) {
        volatile int condition = (iter % 2) ? 1 : 0;
        long double result = 0;
        
        /* Conditional execution to prevent constant folding */
        if (condition) {
            /* DFP operations */
#if HAS_DFP
            _Decimal64 d64_a = 1.23456dd;
            _Decimal64 d64_b = 2.34567dd;
            _Decimal64 d64_c = 3.45678dd;
            _Decimal64 d64_d = 4.56789dd;
            _Decimal128 d128_e = 5.67890123456789dl;
            _Decimal128 d128_f = 6.78901234567890dl;
            
            _Decimal128 dfp_res = dfp_complex_expr(d64_a, d64_b, d64_c, 
                                                   d64_d, d128_e, d128_f);
            result += (long double)dfp_res;
#else
            uint64_t dfp_fb_res = dfp_fallback(123456, 234567, 345678, 456789,
                                              (uint64_t[2]){567890, 0},
                                              (uint64_t[2]){678901, 0});
            result += (long double)dfp_fb_res;
#endif
            
            /* Complex operations */
#if HAS_COMPLEX
            long double _Complex ca = base_vals[0] + base_vals[1] * I;
            long double _Complex cb = base_vals[2] + base_vals[3] * I;
            long double _Complex cc = base_vals[4] + base_vals[5] * I;
            long double _Complex cd = base_vals[6] + base_vals[7] * I;
            
            long double _Complex cplx_res = complex_high_precision(ca, cb, cc, cd);
            result += creal(cplx_res) + cimag(cplx_res);
#else
            complex_fb cfa = {base_vals[0], base_vals[1]};
            complex_fb cfb = {base_vals[2], base_vals[3]};
            complex_fb cfc = {base_vals[4], base_vals[5]};
            complex_fb cfd = {base_vals[6], base_vals[7]};
            
            complex_fb cplx_fb_res = complex_fallback(cfa, cfb, cfc, cfd);
            result += cplx_fb_res.re + cplx_fb_res.im;
#endif
        } else {
            /* Vector operations in the else branch */
#if HAS_VECTORS
            int vec_res = vector_reduction(vec_int, iter + 1);
            double vec_fp_res = vector_fp_reduction(vec_fp, (double)(iter + 1));
            result += (long double)vec_res + (long double)vec_fp_res;
#else
            int vec_res = vector_reduction_fb(vec_int, iter + 1);
            result += (long double)vec_res;
#endif
        }
        
        /* Call helper functions with many arguments */
        long double helper_arg = base_vals[iter];
        long double h1 = helper_10_args(
            helper_arg * 1.0L, helper_arg * 2.0L,
            helper_arg * 3.0L, helper_arg * 4.0L,
            helper_arg * 5.0L, helper_arg * 6.0L,
            helper_arg * 7.0L, helper_arg * 8.0L,
            helper_arg * 9.0L, helper_arg * 10.0L);
        
        long double h2 = helper_11_args(
            helper_arg * 1.1L, helper_arg * 2.2L,
            helper_arg * 3.3L, helper_arg * 4.4L,
            helper_arg * 5.5L, helper_arg * 6.6L,
            helper_arg * 7.7L, helper_arg * 8.8L,
            helper_arg * 9.9L, helper_arg * 10.1L,
            helper_arg * 11.1L);
        
        results[iter] = result + h1 + h2;
        
        /* Store to volatile global to prevent elimination */
        g_result += (uint64_t)results[iter];
    }
    
    /* Compute checksum */
    uint64_t checksum = g_result;
    for (int i = 0; i < 3; i++) {
        checksum += (uint64_t)(results[i] * 1000.0L);
    }
    
    printf("Checksum: %llu\n", (unsigned long long)checksum);
    return 0;
}
