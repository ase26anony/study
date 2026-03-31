#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Feature detection and fallbacks */
#ifdef __DECIMAL_BID_FORMAT__
    #define DFP_SUPPORTED 1
    #include <decimalfp.h>
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
    #define VECTOR_SUPPORTED 1
    typedef int32_t int32x8_t __attribute__((vector_size(32)));
    typedef float float64x4_t __attribute__((vector_size(32)));
#else
    #define VECTOR_SUPPORTED 0
#endif

/* DFP fallback using unions for 64-bit and 128-bit decimal */
#if !DFP_SUPPORTED
typedef union {
    uint64_t u64[2];
    uint32_t u32[4];
    uint16_t u16[8];
} decimal128_fallback;

typedef union {
    uint64_t u64;
    uint32_t u32[2];
} decimal64_fallback;

/* Software DFP emulation */
static decimal128_fallback dfp128_add(decimal128_fallback a, decimal128_fallback b) {
    decimal128_fallback result;
    uint64_t carry = 0;
    for (int i = 0; i < 2; i++) {
        uint64_t sum = a.u64[i] + b.u64[i] + carry;
        result.u64[i] = sum;
        carry = (sum < a.u64[i]) || (carry && sum == a.u64[i]);
    }
    return result;
}

static decimal128_fallback dfp128_mul(decimal128_fallback a, decimal128_fallback b) {
    /* Simplified multiplication for demonstration */
    decimal128_fallback result;
    result.u64[0] = a.u64[0] * b.u64[0];
    result.u64[1] = a.u64[1] * b.u64[1];
    return result;
}

#define DECIMAL64(x) ((decimal64_fallback){.u64 = (x)})
#define DECIMAL128(x1, x2) ((decimal128_fallback){.u64 = {(x1), (x2)}})
#define _Decimal64 decimal64_fallback
#define _Decimal128 decimal128_fallback
#endif

/* Complex number fallback */
#if !COMPLEX_SUPPORTED
typedef struct {
    long double real;
    long double imag;
} long_double_complex;

static long_double_complex complex_mul(long_double_complex a, long_double_complex b) {
    long_double_complex result;
    result.real = a.real * b.real - a.imag * b.imag;
    result.imag = a.real * b.imag + a.imag * b.real;
    return result;
}

static long_double_complex complex_add(long_double_complex a, long_double_complex b) {
    long_double_complex result;
    result.real = a.real + b.real;
    result.imag = a.imag + b.imag;
    return result;
}

#define _Complex 
#define I ((long_double_complex){0.0L, 1.0L})
#endif

/* Helper function with 11 arguments - marked noinline to prevent optimization */
static __attribute__((noinline)) 
long double helper_11_args(
    long double a1, long double a2, long double a3,
    long double a4, long double a5, long double a6,
    long double a7, long double a8, long double a9,
    long double a10, long double a11
) {
    /* Complex expression that may expand to many operands */
    volatile long double result = 0.0L;
    result = a1 * a2 + a3 / a4 - a5 * a6 + a7 - a8 / a9 + a10 * a11;
    result = result * a1 - a2 / a3 + a4 * a5 - a6 / a7 + a8 * a9 - a10 / a11;
    return result;
}

/* Another helper with 10 arguments for DFP operations */
static __attribute__((noinline))
#if DFP_SUPPORTED
_Decimal128 helper_dfp_10_args(
    _Decimal128 d1, _Decimal128 d2, _Decimal128 d3,
    _Decimal128 d4, _Decimal128 d5, _Decimal128 d6,
    _Decimal128 d7, _Decimal128 d8, _Decimal128 d9,
    _Decimal128 d10
) {
    /* Complex DFP expression */
    volatile _Decimal128 result;
#if DFP_SUPPORTED
    result = __builtin_dadd(__builtin_dmul(d1, d2), 
                           __builtin_ddiv(d3, d4));
    result = __builtin_dadd(result, __builtin_dmul(d5, d6));
    result = __builtin_dsub(result, __builtin_ddiv(d7, d8));
    result = __builtin_dadd(result, __builtin_dmul(d9, d10));
#else
    result = dfp128_add(dfp128_mul(d1, d2), dfp128_mul(d3, d4));
#endif
    return result;
}
#else
decimal128_fallback helper_dfp_10_args(
    decimal128_fallback d1, decimal128_fallback d2, decimal128_fallback d3,
    decimal128_fallback d4, decimal128_fallback d5, decimal128_fallback d6,
    decimal128_fallback d7, decimal128_fallback d8, decimal128_fallback d9,
    decimal128_fallback d10
) {
    decimal128_fallback result;
    result = dfp128_add(dfp128_mul(d1, d2), dfp128_mul(d3, d4));
    result = dfp128_add(result, dfp128_mul(d5, d6));
    result = dfp128_add(result, dfp128_mul(d7, d8));
    result = dfp128_add(result, dfp128_mul(d9, d10));
    return result;
}
#endif

/* Vector reduction helper */
#if VECTOR_SUPPORTED
static __attribute__((noinline))
float vector_reduction_accumulate(float64x4_t vec1, float64x4_t vec2, 
                                  float64x4_t vec3, float64x4_t vec4) {
    /* Horizontal reduction with accumulation */
    float64x4_t sum1 = vec1 + vec2;
    float64x4_t sum2 = vec3 + vec4;
    float64x4_t total = sum1 + sum2;
    
    /* Manual horizontal reduction */
    float result = 0.0f;
    for (int i = 0; i < 4; i++) {
        result += total[i];
    }
    
    /* Additional complex operations to increase operand count */
    result = result * result - result / 2.0f + result * 3.0f - result / 4.0f;
    return result;
}
#endif

int main(int argc, char *argv[]) {
    int seed = 12345;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    srand(seed);
    
    /* Initialize base variables */
    volatile int condition = seed % 2;
    
    /* DFP variables */
#if DFP_SUPPORTED
    _Decimal64 d64_a = 1.23456789e10DL;
    _Decimal64 d64_b = 9.87654321e9DL;
    _Decimal128 d128_a = 1.2345678901234567890123456789e30DL;
    _Decimal128 d128_b = 9.8765432109876543210987654321e29DL;
    _Decimal128 d128_c = 5.5555555555555555555555555555e28DL;
    _Decimal128 d128_d = 2.2222222222222222222222222222e27DL;
#else
    _Decimal64 d64_a = DECIMAL64(0x123456789ABCDEF0ULL);
    _Decimal64 d64_b = DECIMAL64(0xFEDCBA9876543210ULL);
    _Decimal128 d128_a = DECIMAL128(0x123456789ABCDEF0ULL, 0xFEDCBA9876543210ULL);
    _Decimal128 d128_b = DECIMAL128(0xFEDCBA9876543210ULL, 0x123456789ABCDEF0ULL);
    _Decimal128 d128_c = DECIMAL128(0x5555555555555555ULL, 0xAAAAAAAAAAAAAAAAULL);
    _Decimal128 d128_d = DECIMAL128(0x2222222222222222ULL, 0x4444444444444444ULL);
#endif
    
    /* Complex variables */
#if COMPLEX_SUPPORTED
    long double _Complex ca = 1.0L + 2.0L * I;
    long double _Complex cb = 3.0L - 4.0L * I;
    long double _Complex cc = 5.0L + 6.0L * I;
    long double _Complex cd = 7.0L - 8.0L * I;
#else
    long_double_complex ca = {1.0L, 2.0L};
    long_double_complex cb = {3.0L, -4.0L};
    long_double_complex cc = {5.0L, 6.0L};
    long_double_complex cd = {7.0L, -8.0L};
#endif
    
    /* Vector variables */
#if VECTOR_SUPPORTED
    float64x4_t vec1 = {1.0, 2.0, 3.0, 4.0};
    float64x4_t vec2 = {5.0, 6.0, 7.0, 8.0};
    float64x4_t vec3 = {9.0, 10.0, 11.0, 12.0};
    float64x4_t vec4 = {13.0, 14.0, 15.0, 16.0};
#endif
    
    /* Storage for results to prevent dead code elimination */
    volatile long double results[10];
    int result_idx = 0;
    
    /* Main computation loop */
    for (int iteration = 0; iteration < 3; iteration++) {
        /* Conditional execution to prevent constant folding */
        if (condition || iteration > 0) {
            /* Complex DFP arithmetic */
            _Decimal128 d128_result;
#if DFP_SUPPORTED
            d128_result = __builtin_dadd(__builtin_dmul(d128_a, d128_b),
                                        __builtin_ddiv(d128_c, d128_d));
            d128_result = __builtin_dmul(d128_result, d128_a);
            d128_result = __builtin_dsub(d128_result, __builtin_ddiv(d128_b, d128_c));
#else
            d128_result = dfp128_add(dfp128_mul(d128_a, d128_b),
                                    dfp128_mul(d128_c, d128_d));
#endif
            
            /* Complex number operations */
#if COMPLEX_SUPPORTED
            long double _Complex cl_result = (ca * cb) / (cc - cd);
            /* Additional complex operations */
            cl_result = cl_result * cl_result + ca / cb - cc * cd;
#else
            long_double_complex cl_result = complex_mul(ca, cb);
            long_double_complex temp = complex_add(cc, cd);
            cl_result.real = cl_result.real / temp.real;
            cl_result.imag = cl_result.imag / temp.imag;
#endif
            
            /* Vector reduction */
#if VECTOR_SUPPORTED
            float vec_result = vector_reduction_accumulate(vec1, vec2, vec3, vec4);
            /* Modify vectors for next iteration */
            for (int i = 0; i < 4; i++) {
                vec1[i] += 0.1;
                vec2[i] += 0.2;
                vec3[i] += 0.3;
                vec4[i] += 0.4;
            }
#endif
            
            /* Call helper with many arguments */
            long double helper_result = helper_11_args(
                1.0L + iteration, 2.0L + iteration, 3.0L + iteration,
                4.0L + iteration, 5.0L + iteration, 6.0L + iteration,
                7.0L + iteration, 8.0L + iteration, 9.0L + iteration,
                10.0L + iteration, 11.0L + iteration
            );
            
            /* Call DFP helper */
            _Decimal128 dfp_helper_result = helper_dfp_10_args(
                d128_a, d128_b, d128_c, d128_d, d128_result,
                d128_a, d128_b, d128_c, d128_d, d128_result
            );
            
            /* Store results */
            results[result_idx++] = helper_result;
#if DFP_SUPPORTED
            /* Convert DFP to long double for storage */
            results[result_idx++] = (long double)d128_result;
#else
            results[result_idx++] = (long double)d128_result.u64[0];
#endif
            
#if COMPLEX_SUPPORTED
            results[result_idx++] = creall(cl_result);
            results[result_idx++] = cimagl(cl_result);
#else
            results[result_idx++] = cl_result.real;
            results[result_idx++] = cl_result.imag;
#endif
            
#if VECTOR_SUPPORTED
            results[result_idx++] = vec_result;
#endif
            
            /* Update condition for next iteration */
            condition = !condition;
        }
    }
    
    /* Compute checksum */
    long double checksum = 0.0L;
    for (int i = 0; i < result_idx; i++) {
        checksum += results[i];
    }
    
    printf("Result count: %d\n", result_idx);
    printf("Checksum: %.15Lf\n", checksum);
    
    return 0;
}
