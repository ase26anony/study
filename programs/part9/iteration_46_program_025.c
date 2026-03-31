#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Enable complex.h if available */
#ifdef __STDC_IEC_559_COMPLEX__
#include <complex.h>
#endif

/* Feature detection and fallbacks */
#if defined(__DECIMAL_BID_FORMAT__) || defined(__DECIMAL_DPD_FORMAT__)
#define HAS_DFP 1
#else
#define HAS_DFP 0
#endif

/* Vector type definitions */
#if defined(__GNUC__) && (__GNUC__ >= 4)
#define HAS_VECTOR_TYPES 1
typedef int32_t int32x4_t __attribute__((vector_size(16)));
typedef float float32x4_t __attribute__((vector_size(16)));
typedef double float64x2_t __attribute__((vector_size(16)));
#else
#define HAS_VECTOR_TYPES 0
#endif

/* DFP fallback using unions */
#if !HAS_DFP
typedef union {
    unsigned long long ull[2];
    double d;
} decimal64_fb;

typedef union {
    unsigned long long ull[4];
    long double ld;
} decimal128_fb;

#define decimal64_fb_add(a, b) ((decimal64_fb){{a.ull[0] + b.ull[0], a.ull[1] + b.ull[1]}})
#define decimal64_fb_mul(a, b) ((decimal64_fb){{a.ull[0] * b.ull[0], a.ull[1] * b.ull[1]}})
#endif

/* Helper functions with many arguments (10-11) */
static __attribute__((noinline)) 
long double helper_10_args(
    long double a1, long double a2, long double a3, long double a4, long double a5,
    long double a6, long double a7, long double a8, long double a9, long double a10)
{
    /* Simple combination to prevent optimization */
    return a1 + a2 - a3 + a4 - a5 + a6 - a7 + a8 - a9 + a10;
}

static __attribute__((noinline))
long double helper_11_args(
    long double a1, long double a2, long double a3, long double a4, long double a5,
    long double a6, long double a7, long double a8, long double a9, long double a10,
    long double a11)
{
    return a1 - a2 + a3 - a4 + a5 - a6 + a7 - a8 + a9 - a10 + a11;
}

/* Vector reduction helper */
#if HAS_VECTOR_TYPES
static __attribute__((noinline))
double vector_reduce_sum(float64x2_t v1, float64x2_t v2, float64x2_t v3, 
                         float64x2_t v4, float64x2_t v5)
{
    /* Horizontal reduction with accumulation */
    double sum = 0.0;
    sum += v1[0] + v1[1];
    sum += v2[0] + v2[1];
    sum += v3[0] + v3[1];
    sum += v4[0] + v4[1];
    sum += v5[0] + v5[1];
    return sum;
}
#endif

/* Complex number operations */
#ifdef __STDC_IEC_559_COMPLEX__
static __attribute__((noinline))
long double complex complex_operation(
    long double complex c1, long double complex c2,
    long double complex c3, long double complex c4,
    long double complex c5, long double complex c6)
{
    /* Complex expression that may expand to many operands */
    return (c1 * c2 + c3 * c4) / (c5 - c6) * (c1 + c2) - (c3 * c4);
}
#endif

/* Main computation with conditional execution */
static volatile int g_condition = 1;

int main(int argc, char *argv[])
{
    int seed = 1;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    /* Prevent constant folding */
    volatile int loop_count = 3;
    
    /* Results accumulator */
    volatile long double results[5] = {0};
    int result_idx = 0;
    
    for (int iter = 0; iter < loop_count; iter++) {
        /* Initialize base values with seed-dependent values */
        long double base = (long double)(seed + iter) * 1.23456789L;
        
        /* DFP operations if available */
#if HAS_DFP
        _Decimal64 d64_a = (_Decimal64)(base);
        _Decimal64 d64_b = (_Decimal64)(base * 2.0L);
        _Decimal64 d64_c = (_Decimal64)(base * 3.0L);
        _Decimal64 d64_d = (_Decimal64)(base * 4.0L);
        
        _Decimal128 d128_a = (_Decimal128)(base * 10.0L);
        _Decimal128 d128_b = (_Decimal128)(base * 20.0L);
        _Decimal128 d128_c = (_Decimal128)(base * 30.0L);
        _Decimal128 d128_d = (_Decimal128)(base * 40.0L);
        
        /* Complex DFP expression - may expand to many operands */
        if (g_condition) {
            /* This complex expression with DFP may trigger multi-operand expansion */
            _Decimal128 d128_result = d128_a * d128_b + d128_c / d128_d;
            d128_result = d128_result - d128_a + d128_b * d128_c;
            results[result_idx++] = (long double)d128_result;
        }
#else
        /* Fallback: use arrays to simulate multi-precision */
        decimal64_fb fb_a = {{seed + iter, seed + iter + 1}};
        decimal64_fb fb_b = {{seed + iter + 2, seed + iter + 3}};
        decimal64_fb fb_c = {{seed + iter + 4, seed + iter + 5}};
        decimal64_fb fb_d = {{seed + iter + 6, seed + iter + 7}};
        
        if (g_condition) {
            decimal64_fb fb_result = decimal64_fb_add(
                decimal64_fb_mul(fb_a, fb_b),
                decimal64_fb_mul(fb_c, fb_d)
            );
            results[result_idx++] = (long double)fb_result.ull[0];
        }
#endif
        
        /* Complex number operations */
#ifdef __STDC_IEC_559_COMPLEX__
        long double complex ca = base + (base * 0.5L) * I;
        long double complex cb = base * 2.0L + (base * 1.5L) * I;
        long double complex cc = base * 3.0L + (base * 2.5L) * I;
        long double complex cd = base * 4.0L + (base * 3.5L) * I;
        long double complex ce = base * 5.0L + (base * 4.5L) * I;
        long double complex cf = base * 6.0L + (base * 5.5L) * I;
        
        if (g_condition || iter % 2 == 0) {
            long double complex cl_result = complex_operation(ca, cb, cc, cd, ce, cf);
            results[result_idx++] = creall(cl_result) + cimagl(cl_result);
        }
#endif
        
        /* Vector operations */
#if HAS_VECTOR_TYPES
        float64x2_t vec1 = {base, base * 1.1};
        float64x2_t vec2 = {base * 1.2, base * 1.3};
        float64x2_t vec3 = {base * 1.4, base * 1.5};
        float64x2_t vec4 = {base * 1.6, base * 1.7};
        float64x2_t vec5 = {base * 1.8, base * 1.9};
        
        if (g_condition) {
            double vec_sum = vector_reduce_sum(vec1, vec2, vec3, vec4, vec5);
            results[result_idx++] = (long double)vec_sum;
        }
#endif
        
        /* Call helper functions with many arguments */
        if (g_condition || iter < 2) {
            long double arg1 = base * 1.0L;
            long double arg2 = base * 2.0L;
            long double arg3 = base * 3.0L;
            long double arg4 = base * 4.0L;
            long double arg5 = base * 5.0L;
            long double arg6 = base * 6.0L;
            long double arg7 = base * 7.0L;
            long double arg8 = base * 8.0L;
            long double arg9 = base * 9.0L;
            long double arg10 = base * 10.0L;
            long double arg11 = base * 11.0L;
            
            /* Mix both 10 and 11 argument calls */
            long double res10 = helper_10_args(arg1, arg2, arg3, arg4, arg5,
                                              arg6, arg7, arg8, arg9, arg10);
            
            long double res11 = helper_11_args(arg1, arg2, arg3, arg4, arg5,
                                              arg6, arg7, arg8, arg9, arg10,
                                              arg11);
            
            results[result_idx++] = res10 + res11;
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    long double checksum = 0.0L;
    for (int i = 0; i < result_idx; i++) {
        checksum += results[i];
    }
    
    /* Print deterministic result */
    printf("Result checksum: %Lf\n", checksum);
    
    return 0;
}
