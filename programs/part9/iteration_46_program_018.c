/* 
 * Test program targeting GCC's optabs.cc lines 8254-8263
 * Designed to trigger internal function expansion with 10-11 operands
 * through DFP arithmetic, complex operations, and vector reductions.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Feature detection and fallbacks */
#ifdef __DECIMAL_BID_FORMAT__
#include <decimal/decimal.h>
#define HAS_DFP 1
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
    unsigned long long lo;
    unsigned long long hi;
} decimal64_fb;

typedef struct {
    unsigned long long lo;
    unsigned long long hi;
    unsigned long long extra[2];
} decimal128_fb;
#endif

/* Vector types */
#if HAS_VECTORS
typedef int32_t int32x8_t __attribute__((vector_size(32)));
typedef double float64x4_t __attribute__((vector_size(32)));
#else
typedef struct { int32_t v[8]; } int32x8_t;
typedef struct { double v[4]; } float64x4_t;
#endif

/* Complex fallback */
#if !HAS_COMPLEX
typedef struct {
    long double re;
    long double im;
} long_double_complex;
#endif

/* Global volatile to prevent optimization */
volatile uint64_t g_result = 0;

/* Helper function with 11 arguments - targeting case 11 */
static __attribute__((noinline)) 
long double helper_11(
#if HAS_DFP
    _Decimal64 d1, _Decimal64 d2, _Decimal64 d3,
    _Decimal128 d4, _Decimal128 d5,
#else
    decimal64_fb d1, decimal64_fb d2, decimal64_fb d3,
    decimal128_fb d4, decimal128_fb d5,
#endif
#if HAS_COMPLEX
    long double _Complex c1, long double _Complex c2,
#else
    long_double_complex c1, long_double_complex c2,
#endif
    float64x4_t v1, float64x4_t v2,
    int32x8_t vi, double scalar)
{
    /* Simple combination to use all arguments */
    long double result = 0;
    
#if HAS_DFP
    /* DFP operations that may expand to multi-operand patterns */
    result += (long double)d1 * 0.1L;
    result += (long double)d2 * 0.2L;
    result += (long double)d3 * 0.3L;
    result += (long double)d4 * 0.4L;
    result += (long double)d5 * 0.5L;
#else
    /* Fallback DFP simulation */
    result += (long double)d1.lo * 0.1L;
    result += (long double)d2.lo * 0.2L;
    result += (long double)d3.lo * 0.3L;
    result += (long double)d4.lo * 0.4L;
    result += (long double)d5.lo * 0.5L;
#endif

#if HAS_COMPLEX
    /* Complex operations */
    result += creall(c1) * 0.6L;
    result += cimagl(c1) * 0.7L;
    result += creall(c2) * 0.8L;
    result += cimagl(c2) * 0.9L;
#else
    result += c1.re * 0.6L;
    result += c1.im * 0.7L;
    result += c2.re * 0.8L;
    result += c2.im * 0.9L;
#endif

#if HAS_VECTORS
    /* Vector reduction */
    double vsum = 0;
    for (int i = 0; i < 4; i++) {
        vsum += v1[i] + v2[i];
    }
    result += vsum;
    
    int32_t visum = 0;
    for (int i = 0; i < 8; i++) {
        visum += vi[i];
    }
    result += visum;
#else
    result += v1.v[0] + v2.v[0];
    result += vi.v[0];
#endif

    result += scalar;
    return result;
}

/* Helper function with 10 arguments - targeting case 10 */
static __attribute__((noinline))
double helper_10(
    double a1, double a2, double a3, double a4, double a5,
    double a6, double a7, double a8, double a9, double a10)
{
    /* Complex expression that may expand to many operands */
    return ((a1 * a2) + (a3 * a4) - (a5 * a6)) / 
           ((a7 + a8) * (a9 - a10)) + 
           (a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10) * 0.1;
}

int main(int argc, char *argv[]) {
    int seed = 42;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    srand(seed);
    
    /* Initialize variables */
#if HAS_DFP
    _Decimal64 d64_a = 1.23456dd;
    _Decimal64 d64_b = 2.34567dd;
    _Decimal64 d64_c = 3.45678dd;
    _Decimal128 d128_a = 1.2345678901234567dl;
    _Decimal128 d128_b = 2.3456789012345678dl;
#else
    decimal64_fb d64_a = {123456ULL, 0};
    decimal64_fb d64_b = {234567ULL, 0};
    decimal64_fb d64_c = {345678ULL, 0};
    decimal128_fb d128_a = {12345678901234567ULL, 0, {0, 0}};
    decimal128_fb d128_b = {23456789012345678ULL, 0, {0, 0}};
#endif

#if HAS_COMPLEX
    long double _Complex ca = 1.0L + 2.0L * I;
    long double _Complex cb = 3.0L + 4.0L * I;
    long double _Complex cc = 5.0L + 6.0L * I;
    long double _Complex cd = 7.0L + 8.0L * I;
#else
    long_double_complex ca = {1.0L, 2.0L};
    long_double_complex cb = {3.0L, 4.0L};
    long_double_complex cc = {5.0L, 6.0L};
    long_double_complex cd = {7.0L, 8.0L};
#endif

#if HAS_VECTORS
    float64x4_t vec1 = {1.0, 2.0, 3.0, 4.0};
    float64x4_t vec2 = {5.0, 6.0, 7.0, 8.0};
    int32x8_t ivec = {1, 2, 3, 4, 5, 6, 7, 8};
#else
    float64x4_t vec1 = {{1.0, 2.0, 3.0, 4.0}};
    float64x4_t vec2 = {{5.0, 6.0, 7.0, 8.0}};
    int32x8_t ivec = {{1, 2, 3, 4, 5, 6, 7, 8}};
#endif

    double accumulator = 0.0;
    volatile int condition = seed; /* Prevent constant folding */
    
    /* Main computation loop */
    for (int iter = 0; iter < 3; iter++) {
        double loop_result = 0.0;
        
        /* Conditional execution to prevent optimization */
        if (condition & (1 << iter)) {
            /* DFP arithmetic - may expand to multi-operand patterns */
#if HAS_DFP
            _Decimal128 d128_temp = d128_a * d128_b + d128_a / d128_b;
            _Decimal64 d64_temp = d64_a * d64_b + d64_c / d64_a;
#else
            decimal128_fb d128_temp = {d128_a.lo * d128_b.lo, 0, {0, 0}};
            decimal64_fb d64_temp = {d64_a.lo * d64_b.lo, 0};
#endif
            
            /* Complex arithmetic */
#if HAS_COMPLEX
            long double _Complex ctemp = (ca * cb) / (cc - cd);
            /* Complex function that may expand */
            long double _Complex csqrt_temp = csqrt(ctemp);
            long double _Complex cpow_temp = cpow(ca, cb);
#else
            long_double_complex ctemp = {
                (ca.re * cb.re - ca.im * cb.im) / (cc.re - cd.re),
                (ca.re * cb.im + ca.im * cb.re) / (cc.im - cd.im)
            };
            long_double_complex csqrt_temp = {
                sqrtl(ctemp.re * ctemp.re + ctemp.im * ctemp.im),
                0.0L
            };
            long_double_complex cpow_temp = {
                powl(ca.re, cb.re),
                0.0L
            };
#endif
            
            /* Vector reduction with accumulation */
            double vec_sum = 0.0;
#if HAS_VECTORS
            for (int i = 0; i < 4; i++) {
                vec_sum += vec1[i] + vec2[i];
            }
            /* Horizontal reduction pattern */
            float64x4_t vec3 = vec1 + vec2;
            vec_sum += vec3[0] + vec3[1] + vec3[2] + vec3[3];
#else
            for (int i = 0; i < 4; i++) {
                vec_sum += vec1.v[i] + vec2.v[i];
            }
#endif
            
            /* Call helper with 11 arguments - targeting case 11 */
            loop_result += helper_11(
#if HAS_DFP
                d64_a, d64_b, d64_temp,
                d128_a, d128_temp,
#else
                d64_a, d64_b, d64_temp,
                d128_a, d128_temp,
#endif
#if HAS_COMPLEX
                ctemp, csqrt_temp,
#else
                ctemp, csqrt_temp,
#endif
                vec1, vec2, ivec, vec_sum);
        } else {
            /* Alternative path with different operations */
            double temp_array[10];
            for (int i = 0; i < 10; i++) {
                temp_array[i] = (double)(i + iter) * 1.234;
            }
            
            /* Call helper with 10 arguments - targeting case 10 */
            loop_result += helper_10(
                temp_array[0], temp_array[1], temp_array[2], temp_array[3],
                temp_array[4], temp_array[5], temp_array[6], temp_array[7],
                temp_array[8], temp_array[9]);
        }
        
        /* Accumulate results to prevent dead code elimination */
        accumulator += loop_result;
        g_result += (uint64_t)loop_result;
    }
    
    /* Final checksum */
    uint64_t checksum = g_result + (uint64_t)accumulator;
    printf("Result checksum: %llu\n", (unsigned long long)checksum);
    
    return 0;
}
