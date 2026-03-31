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
        } u128;
        long double ld128;
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

/* Vector types if supported */
#ifdef __VECTOR_TYPES_SUPPORTED__
    typedef int32_t int32x8_t __attribute__((vector_size(32)));
    typedef double float64x4_t __attribute__((vector_size(32)));
#else
    /* Fallback using arrays */
    typedef int32_t int32x8_t[8];
    typedef double float64x4_t[4];
#endif

/* Global volatile to prevent optimization */
volatile uint64_t g_result_store[16];
volatile int g_store_idx = 0;

/* Helper functions with many arguments */
static __attribute__((noinline)) 
long double helper_10_args(
    long double a1, long double a2, long double a3,
    long double a4, long double a5, long double a6,
    long double a7, long double a8, long double a9,
    long double a10)
{
    /* Complex expression that may expand to many operands */
    return ((a1 * a2) + (a3 / a4) - (a5 * a6) + 
            (a7 - a8) * (a9 + a10)) / (a1 + a2 + a3);
}

static __attribute__((noinline))
long double helper_11_args(
    long double a1, long double a2, long double a3,
    long double a4, long double a5, long double a6,
    long double a7, long double a8, long double a9,
    long double a10, long double a11)
{
    /* Even more complex expression */
    return (((a1 * a2 * a3) + (a4 * a5 * a6)) / 
            ((a7 + a8 + a9) * (a10 - a11))) * 
           (a1 + a2 - a3 + a4 - a5);
}

/* DFP operations with fallback */
#if HAS_DFP
static _Decimal128 dfp_complex_expr(_Decimal64 a, _Decimal64 b, 
                                    _Decimal64 c, _Decimal64 d,
                                    _Decimal128 e, _Decimal128 f)
{
    /* Complex DFP expression that may generate many operands */
    _Decimal128 t1 = (_Decimal128)a * (_Decimal128)b;
    _Decimal128 t2 = (_Decimal128)c / (_Decimal128)d;
    _Decimal128 t3 = e * f;
    _Decimal128 t4 = t1 + t2 - t3;
    _Decimal128 t5 = e / f + (_Decimal128)a;
    
    return t4 * t5 + (_Decimal128)b / (_Decimal128)c;
}
#else
static long double dfp_complex_expr_fallback(
    decimal64_fallback a, decimal64_fallback b,
    decimal64_fallback c, decimal64_fallback d,
    decimal128_fallback e, decimal128_fallback f)
{
    /* Simulate DFP with manual precision */
    long double t1 = (long double)a.f64 * (long double)b.f64;
    long double t2 = (long double)c.f64 / (long double)d.f64;
    long double t3 = e.ld128 * f.ld128;
    long double t4 = t1 + t2 - t3;
    long double t5 = e.ld128 / f.ld128 + (long double)a.f64;
    
    return t4 * t5 + (long double)b.f64 / (long double)c.f64;
}
#endif

/* Complex number operations */
#if HAS_COMPLEX
static long double _Complex complex_power_expr(
    long double _Complex a, long double _Complex b,
    long double _Complex c, long double _Complex d)
{
    /* Complex expression that may expand to many real/imag operations */
    long double _Complex t1 = a * b;
    long double _Complex t2 = c / d;
    long double _Complex t3 = t1 + t2;
    long double _Complex t4 = csqrt(t3);
    long double _Complex t5 = cpow(a, b);
    
    return (t4 * t5) / (a - b + c - d);
}
#else
static complex_fallback complex_power_expr_fallback(
    complex_fallback a, complex_fallback b,
    complex_fallback c, complex_fallback d)
{
    /* Manual complex arithmetic */
    complex_fallback t1, t2, t3, t4, t5, result;
    
    /* t1 = a * b */
    t1.real = a.real * b.real - a.imag * b.imag;
    t1.imag = a.real * b.imag + a.imag * b.real;
    
    /* t2 = c / d */
    long double denom = d.real * d.real + d.imag * d.imag;
    t2.real = (c.real * d.real + c.imag * d.imag) / denom;
    t2.imag = (c.imag * d.real - c.real * d.imag) / denom;
    
    /* t3 = t1 + t2 */
    t3.real = t1.real + t2.real;
    t3.imag = t1.imag + t2.imag;
    
    /* t4 = sqrt(t3) - manual sqrt */
    long double r = sqrt(t3.real * t3.real + t3.imag * t3.imag);
    t4.real = sqrt((r + t3.real) / 2.0);
    t4.imag = copysign(sqrt((r - t3.real) / 2.0), t3.imag);
    
    /* t5 = a^b - approximated */
    long double log_r = log(sqrt(a.real * a.real + a.imag * a.imag));
    long double theta = atan2(a.imag, a.real);
    long double exp_real = exp(b.real * log_r - b.imag * theta);
    long double exp_imag = b.real * theta + b.imag * log_r;
    t5.real = exp_real * cos(exp_imag);
    t5.imag = exp_real * sin(exp_imag);
    
    /* result = (t4 * t5) / (a - b + c - d) */
    complex_fallback num, den;
    
    /* num = t4 * t5 */
    num.real = t4.real * t5.real - t4.imag * t5.imag;
    num.imag = t4.real * t5.imag + t4.imag * t5.real;
    
    /* den = a - b + c - d */
    den.real = a.real - b.real + c.real - d.real;
    den.imag = a.imag - b.imag + c.imag - d.imag;
    
    /* result = num / den */
    long double denom2 = den.real * den.real + den.imag * den.imag;
    result.real = (num.real * den.real + num.imag * den.imag) / denom2;
    result.imag = (num.imag * den.real - num.real * den.imag) / denom2;
    
    return result;
}
#endif

/* Vector reduction */
#ifdef __VECTOR_TYPES_SUPPORTED__
static double vector_reduction(float64x4_t v1, float64x4_t v2,
                               float64x4_t v3, float64x4_t v4)
{
    /* Complex vector operations that may expand */
    float64x4_t sum1 = v1 + v2;
    float64x4_t sum2 = v3 + v4;
    float64x4_t prod = sum1 * sum2;
    
    /* Horizontal reduction */
    double result = 0.0;
    for (int i = 0; i < 4; i++) {
        result += prod[i];
    }
    
    /* Additional accumulation */
    float64x4_t t1 = v1 * v2 - v3 / v4;
    for (int i = 0; i < 4; i++) {
        result += t1[i];
    }
    
    return result;
}
#else
static double vector_reduction_fallback(float64x4_t v1, float64x4_t v2,
                                        float64x4_t v3, float64x4_t v4)
{
    double result = 0.0;
    
    /* Manual vector operations */
    for (int i = 0; i < 4; i++) {
        double sum1 = v1[i] + v2[i];
        double sum2 = v3[i] + v4[i];
        double prod = sum1 * sum2;
        result += prod;
    }
    
    for (int i = 0; i < 4; i++) {
        double t1 = v1[i] * v2[i] - v3[i] / v4[i];
        result += t1;
    }
    
    return result;
}
#endif

int main(int argc, char *argv[])
{
    /* Use command line seed for deterministic behavior */
    int seed = 42;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    srand(seed);
    
    /* Initialize base variables */
#if HAS_DFP
    _Decimal64 d64_a = 1.23456789dd;
    _Decimal64 d64_b = 9.87654321dd;
    _Decimal64 d64_c = 3.14159265dd;
    _Decimal64 d64_d = 2.71828182dd;
    _Decimal128 d128_e = 1.2345678901234567dl;
    _Decimal128 d128_f = 9.8765432109876543dl;
#else
    decimal64_fallback d64_a = {.f64 = 1.23456789};
    decimal64_fallback d64_b = {.f64 = 9.87654321};
    decimal64_fallback d64_c = {.f64 = 3.14159265};
    decimal64_fallback d64_d = {.f64 = 2.71828182};
    decimal128_fallback d128_e = {.ld128 = 1.2345678901234567L};
    decimal128_fallback d128_f = {.ld128 = 9.8765432109876543L};
#endif

#if HAS_COMPLEX
    long double _Complex ca = 1.0 + 2.0i;
    long double _Complex cb = 3.0 - 4.0i;
    long double _Complex cc = -2.0 + 1.5i;
    long double _Complex cd = 0.5 - 3.0i;
#else
    complex_fallback ca = {1.0, 2.0};
    complex_fallback cb = {3.0, -4.0};
    complex_fallback cc = {-2.0, 1.5};
    complex_fallback cd = {0.5, -3.0};
#endif

    /* Initialize vectors */
#ifdef __VECTOR_TYPES_SUPPORTED__
    float64x4_t v1 = {1.0, 2.0, 3.0, 4.0};
    float64x4_t v2 = {5.0, 6.0, 7.0, 8.0};
    float64x4_t v3 = {9.0, 10.0, 11.0, 12.0};
    float64x4_t v4 = {13.0, 14.0, 15.0, 16.0};
#else
    float64x4_t v1 = {1.0, 2.0, 3.0, 4.0};
    float64x4_t v2 = {5.0, 6.0, 7.0, 8.0};
    float64x4_t v3 = {9.0, 10.0, 11.0, 12.0};
    float64x4_t v4 = {13.0, 14.0, 15.0, 16.0};
#endif

    /* Volatile condition to prevent constant folding */
    volatile int condition = seed;
    
    /* Main computation loop */
    uint64_t checksum = 0;
    for (int iter = 0; iter < 4; iter++) {
        long double result = 0.0;
        
        /* Conditional execution to force expansion */
        if (condition & (1 << iter)) {
            /* DFP operations */
#if HAS_DFP
            _Decimal128 dfp_result = dfp_complex_expr(
                d64_a, d64_b, d64_c, d64_d, d128_e, d128_f);
            result += (long double)dfp_result;
#else
            long double dfp_result = dfp_complex_expr_fallback(
                d64_a, d64_b, d64_c, d64_d, d128_e, d128_f);
            result += dfp_result;
#endif
            
            /* Complex operations */
#if HAS_COMPLEX
            long double _Complex c_result = complex_power_expr(ca, cb, cc, cd);
            result += creal(c_result) + cimag(c_result);
#else
            complex_fallback c_result = complex_power_expr_fallback(ca, cb, cc, cd);
            result += c_result.real + c_result.imag;
#endif
        } else {
            /* Vector reduction */
#ifdef __VECTOR_TYPES_SUPPORTED__
            double vec_result = vector_reduction(v1, v2, v3, v4);
#else
            double vec_result = vector_reduction_fallback(v1, v2, v3, v4);
#endif
            result += vec_result;
            
            /* Call helper with many arguments */
            if (iter % 2 == 0) {
                result += helper_10_args(
                    result, result * 2, result / 3,
                    result + 1, result - 1, result * 1.5,
                    result / 1.5, result + 2, result - 2,
                    result * 0.5);
            } else {
                result += helper_11_args(
                    result, result * 2, result / 3,
                    result + 1, result - 1, result * 1.5,
                    result / 1.5, result + 2, result - 2,
                    result * 0.5, result + 3);
            }
        }
        
        /* Store result to prevent elimination */
        uint64_t stored = *(uint64_t*)&result;
        g_result_store[g_store_idx++ % 16] = stored;
        checksum += stored;
        
        /* Modify variables for next iteration */
#if HAS_DFP
        d64_a += 0.1dd;
        d64_b -= 0.1dd;
        d128_e *= 1.1dl;
#else
        d64_a.f64 += 0.1;
        d64_b.f64 -= 0.1;
        d128_e.ld128 *= 1.1L;
#endif
        
#if HAS_COMPLEX
        ca *= 0.9 + 0.1i;
        cb /= 1.1 - 0.05i;
#else
        ca.real *= 0.9;
        ca.imag *= 1.1;
        cb.real /= 1.1;
        cb.imag /= 0.95;
#endif
        
        /* Modify vectors */
        for (int i = 0; i < 4; i++) {
#ifdef __VECTOR_TYPES_SUPPORTED__
            v1[i] += 0.1;
            v2[i] -= 0.1;
#else
            v1[i] += 0.1;
            v2[i] -= 0.1;
#endif
        }
    }
    
    /* Final checksum and output */
    printf("Checksum: %llu\n", (unsigned long long)checksum);
    
    /* Additional verification */
    uint64_t final_check = 0;
    for (int i = 0; i < 16 && i < g_store_idx; i++) {
        final_check ^= g_result_store[i];
    }
    printf("Final XOR: %llu\n", (unsigned long long)final_check);
    
    return 0;
}
