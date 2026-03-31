#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* Feature detection and fallbacks */
#ifdef __STDC_IEC_559_COMPLEX__
#include <complex.h>
#define HAS_COMPLEX 1
#else
#define HAS_COMPLEX 0
#endif

#ifdef __DECIMAL_BID_FORMAT__
#define HAS_DFP 1
#else
#define HAS_DFP 0
#endif

/* Vector type definitions */
typedef int32_t int32x4_t __attribute__((vector_size(16)));
typedef float float32x4_t __attribute__((vector_size(16)));
typedef double float64x2_t __attribute__((vector_size(16)));

/* Fallback DFP types for targets without native support */
#if !HAS_DFP
typedef struct {
    uint64_t lo;
    uint64_t hi;
} decimal64_fb;

typedef struct {
    uint64_t w[2];
} decimal128_fb;

#define DEC64(x) ((decimal64_fb){.lo = (x), .hi = 0})
#define DEC128(x) ((decimal128_fb){.w[0] = (x), .w[1] = 0})
#else
typedef _Decimal64 decimal64_fb;
typedef _Decimal128 decimal128_fb;
#define DEC64(x) ((_Decimal64)(x))
#define DEC128(x) ((_Decimal128)(x))
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
    return a1 * a2 + a3 / a4 - a5 * a6 + a7 / a8 - a9 * a10 + a11;
}

/* Another helper with 10 arguments for DFP operations */
static __attribute__((noinline))
#if HAS_DFP
_Decimal128 helper_10_args_dfp(
    _Decimal128 d1, _Decimal128 d2, _Decimal128 d3,
    _Decimal128 d4, _Decimal128 d5, _Decimal128 d6,
    _Decimal128 d7, _Decimal128 d8, _Decimal128 d9,
    _Decimal128 d10)
{
    /* DFP arithmetic that may require many operands during expansion */
    return d1 * d2 + d3 * d4 - d5 * d6 + d7 * d8 - d9 * d10;
}
#else
decimal128_fb helper_10_args_dfp(
    decimal128_fb d1, decimal128_fb d2, decimal128_fb d3,
    decimal128_fb d4, decimal128_fb d5, decimal128_fb d6,
    decimal128_fb d7, decimal128_fb d8, decimal128_fb d9,
    decimal128_fb d10)
{
    /* Fallback using integer arithmetic */
    decimal128_fb result;
    result.w[0] = d1.w[0] + d2.w[0] + d3.w[0] + d4.w[0] + d5.w[0] + 
                  d6.w[0] + d7.w[0] + d8.w[0] + d9.w[0] + d10.w[0];
    result.w[1] = 0;
    return result;
}
#endif

/* Complex number helper with mixed operations */
#if HAS_COMPLEX
static __attribute__((noinline))
long double _Complex helper_complex_mix(
    long double _Complex c1, long double _Complex c2,
    long double _Complex c3, long double _Complex c4,
    long double _Complex c5, long double _Complex c6,
    long double _Complex c7, long double _Complex c8,
    long double _Complex c9, long double _Complex c10)
{
    /* Complex arithmetic that expands to many real/imaginary operations */
    return (c1 * c2) / (c3 - c4) + (c5 * c6) / (c7 - c8) + c9 * c10;
}
#endif

/* Vector reduction with accumulation */
static float vector_reduce_sum(float32x4_t v1, float32x4_t v2, 
                               float32x4_t v3, float32x4_t v4)
{
    /* Horizontal reduction that may expand to many operations */
    float32x4_t sum = v1 + v2 + v3 + v4;
    float result = 0.0f;
    
    /* Manual reduction to force operand expansion */
    result += sum[0] + sum[1] + sum[2] + sum[3];
    
    return result;
}

/* Global volatile to prevent optimization */
volatile long double global_accumulator = 0.0L;

int main(int argc, char *argv[])
{
    int seed = 12345;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    srand(seed);
    
    /* Initialize DFP values */
#if HAS_DFP
    _Decimal64 d64_a = DEC64(1.23456789);
    _Decimal64 d64_b = DEC64(9.87654321);
    _Decimal128 d128_a = DEC128(1.234567890123456789);
    _Decimal128 d128_b = DEC128(9.876543210987654321);
    _Decimal128 d128_c = DEC128(3.141592653589793238);
    _Decimal128 d128_d = DEC128(2.718281828459045235);
#else
    decimal64_fb d64_a = DEC64(123456789);
    decimal64_fb d64_b = DEC64(987654321);
    decimal128_fb d128_a = DEC128(1234567890123456789ULL);
    decimal128_fb d128_b = DEC128(9876543210987654321ULL);
    decimal128_fb d128_c = DEC128(3141592653589793238ULL);
    decimal128_fb d128_d = DEC128(2718281828459045235ULL);
#endif
    
    /* Initialize complex numbers */
#if HAS_COMPLEX
    long double _Complex ca = 1.0L + 2.0L * I;
    long double _Complex cb = 3.0L - 4.0L * I;
    long double _Complex cc = 5.0L + 6.0L * I;
    long double _Complex cd = 7.0L - 8.0L * I;
    long double _Complex ce = 9.0L + 10.0L * I;
    long double _Complex cf = 11.0L - 12.0L * I;
#endif
    
    /* Initialize vectors */
    float32x4_t vec1 = {1.0f, 2.0f, 3.0f, 4.0f};
    float32x4_t vec2 = {5.0f, 6.0f, 7.0f, 8.0f};
    float32x4_t vec3 = {9.0f, 10.0f, 11.0f, 12.0f};
    float32x4_t vec4 = {13.0f, 14.0f, 15.0f, 16.0f};
    
    /* Storage for results to prevent dead code elimination */
    long double results[5] = {0};
    int result_idx = 0;
    
    /* Main computation loop - 3 iterations */
    for (int iter = 0; iter < 3; iter++) {
        volatile int condition = seed + iter; /* Prevent constant folding */
        
        if (condition & 1) {
            /* Branch 1: Complex DFP arithmetic */
#if HAS_DFP
            _Decimal128 d128_result = d128_a * d128_b + d128_c / d128_d;
            /* More complex expression with many operands */
            d128_result = d128_result * d128_a - d128_b / d128_c + 
                         d128_d * d128_a - d128_b / d128_c + d128_d;
#else
            decimal128_fb d128_result;
            /* Simulate DFP operations */
            d128_result.w[0] = d128_a.w[0] + d128_b.w[0] + d128_c.w[0] + 
                              d128_d.w[0] + d128_a.w[0] + d128_b.w[0];
            d128_result.w[1] = 0;
#endif
            
            /* Call helper with many DFP arguments */
#if HAS_DFP
            _Decimal128 helper_result = helper_10_args_dfp(
                d128_a, d128_b, d128_c, d128_d, d128_result,
                d128_a, d128_b, d128_c, d128_d, d128_result);
#else
            decimal128_fb helper_result = helper_10_args_dfp(
                d128_a, d128_b, d128_c, d128_d, d128_result,
                d128_a, d128_b, d128_c, d128_d, d128_result);
#endif
            
            results[result_idx++] = (long double)helper_result.w[0];
        } else {
            /* Branch 2: Complex number operations */
#if HAS_COMPLEX
            long double _Complex cl_result = (ca * cb) / (cc - cd);
            /* More complex expressions */
            cl_result = cl_result + (ce * cf) / (ca - cb) + 
                       (cc * cd) / (ce - cf) + (ca * ce) / (cb - cf);
            
            /* Call complex helper */
            long double _Complex complex_helper_result = helper_complex_mix(
                ca, cb, cc, cd, ce, cf, ca, cb, cc, cd);
            
            results[result_idx++] = creall(complex_helper_result) + 
                                   cimagl(complex_helper_result);
#endif
        }
        
        /* Always execute: Vector reduction */
        float vec_sum = vector_reduce_sum(vec1, vec2, vec3, vec4);
        results[result_idx++] = vec_sum;
        
        /* Call helper with 11 arguments using mixed computations */
        long double ld1 = (long double)(iter + 1);
        long double ld2 = (long double)(seed % 100);
        long double helper_11_result = helper_11_args(
            ld1, ld2, ld1 * 2, ld2 / 2, ld1 + ld2,
            ld1 - ld2, ld1 * ld2, ld2 / ld1, sqrtl(ld1),
            logl(ld2 + 1), expl(ld1 / 10));
        
        results[result_idx++] = helper_11_result;
        
        /* Update global accumulator to prevent optimization */
        for (int i = 0; i < result_idx; i++) {
            global_accumulator += results[i];
        }
    }
    
    /* Compute checksum */
    long double checksum = 0.0L;
    for (int i = 0; i < result_idx; i++) {
        checksum += results[i];
    }
    
    /* Also include global accumulator */
    checksum += global_accumulator;
    
    printf("Checksum: %Lf\n", checksum);
    printf("Result count: %d\n", result_idx);
    
    return 0;
}
