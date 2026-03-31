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

/* Vector types if supported */
#ifdef __VECTOR_TYPES_SUPPORTED__
    typedef int32_t int32x4_t __attribute__((vector_size(16)));
    typedef float float32x4_t __attribute__((vector_size(16)));
    #define HAS_VECTORS 1
#else
    #define HAS_VECTORS 0
#endif

/* Global volatile to prevent optimization */
volatile uint64_t g_result = 0;

/* DFP fallback structures */
typedef struct {
    uint64_t lo;
    uint64_t hi;
} dfp128_fallback_t;

/* Helper function with 11 arguments - marked noinline */
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
double helper_10_args(
    double b1, double b2, double b3, double b4, double b5,
    double b6, double b7, double b8, double b9, double b10)
{
    /* Another complex expression */
    return (b1 + b2) * (b3 - b4) / (b5 * b6) + 
           (b7 / b8) - (b9 * b10);
}

#if HAS_DFP
/* DFP operations that may generate many operands */
static __attribute__((noinline))
_Decimal128 dfp_complex_op(
    _Decimal128 d1, _Decimal128 d2, _Decimal128 d3,
    _Decimal128 d4, _Decimal128 d5, _Decimal128 d6)
{
    /* Complex DFP expression that may expand to many RTL operands */
    return ((d1 * d2) + (d3 / d4) - (d5 * d6)) / 
           (d1 + d2 - d3 + d4 - d5 + d6);
}
#endif

#if HAS_COMPLEX
/* Complex number operations */
static __attribute__((noinline))
long double _Complex complex_op(
    long double _Complex c1, long double _Complex c2,
    long double _Complex c3, long double _Complex c4)
{
    /* Complex expression that handles real/imag parts separately */
    return (c1 * c2) / (c3 - c4) + (c1 + c2) * (c3 * c4);
}
#endif

#if HAS_VECTORS
/* Vector reduction with accumulation */
static __attribute__((noinline))
float vector_reduce(float32x4_t v1, float32x4_t v2, 
                    float32x4_t v3, float32x4_t v4)
{
    /* Horizontal reduction that may create many intermediate operands */
    float32x4_t sum1 = v1 + v2;
    float32x4_t sum2 = v3 + v4;
    float32x4_t prod = sum1 * sum2;
    
    /* Manual horizontal sum */
    float result = 0.0f;
    for (int i = 0; i < 4; i++) {
        result += prod[i];
    }
    
    return result;
}
#endif

/* Fallback multi-precision arithmetic for targets without DFP */
static __attribute__((noinline))
dfp128_fallback_t manual_dfp_op(
    dfp128_fallback_t a, dfp128_fallback_t b,
    dfp128_fallback_t c, dfp128_fallback_t d)
{
    /* Simulate DFP operations using integer arithmetic */
    dfp128_fallback_t result;
    
    /* Complex expression with many intermediate values */
    uint64_t t1_lo = a.lo + b.lo;
    uint64_t t1_hi = a.hi + b.hi + (t1_lo < a.lo);
    
    uint64_t t2_lo = c.lo - d.lo;
    uint64_t t2_hi = c.hi - d.hi - (c.lo < d.lo);
    
    uint64_t t3_lo = a.lo * b.lo;
    uint64_t t3_hi = a.hi * b.hi;
    
    uint64_t t4_lo = c.lo / (d.lo | 1);  /* Avoid division by zero */
    uint64_t t4_hi = c.hi / (d.hi | 1);
    
    result.lo = (t1_lo + t2_lo) * (t3_lo - t4_lo);
    result.hi = (t1_hi + t2_hi) * (t3_hi - t4_hi);
    
    return result;
}

int main(int argc, char *argv[])
{
    /* Use command-line seed for deterministic behavior */
    int seed = 12345;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    srand(seed);
    
    /* Initialize base variables */
    double base_doubles[20];
    long double base_long_doubles[20];
    
    for (int i = 0; i < 20; i++) {
        base_doubles[i] = (double)(rand() % 1000) / 100.0 + 1.0;
        base_long_doubles[i] = (long double)(rand() % 1000) / 100.0 + 1.0;
    }
    
    /* Array to store results */
    double results[10] = {0};
    int result_idx = 0;
    
    /* Main computation loop */
    for (int iteration = 0; iteration < 5; iteration++) {
        double loop_result = 0.0;
        
        /* 1. Call helper with 11 arguments */
        loop_result += helper_11_args(
            base_long_doubles[0] + iteration,
            base_long_doubles[1] * 2.0L,
            base_long_doubles[2] / 3.0L,
            base_long_doubles[3] - 4.0L,
            base_long_doubles[4] + 5.0L,
            base_long_doubles[5] * 6.0L,
            base_long_doubles[6] / 7.0L,
            base_long_doubles[7] - 8.0L,
            base_long_doubles[8] + 9.0L,
            base_long_doubles[9] * 10.0L,
            base_long_doubles[10] / 11.0L
        );
        
        /* 2. Call helper with 10 arguments */
        loop_result += helper_10_args(
            base_doubles[0] * iteration,
            base_doubles[1] + 1.0,
            base_doubles[2] - 2.0,
            base_doubles[3] * 3.0,
            base_doubles[4] / 4.0,
            base_doubles[5] + 5.0,
            base_doubles[6] - 6.0,
            base_doubles[7] * 7.0,
            base_doubles[8] / 8.0,
            base_doubles[9] + 9.0
        );
        
        /* 3. Complex number operations if available */
        #if HAS_COMPLEX
        {
            long double _Complex c1 = base_long_doubles[0] + 
                                     base_long_doubles[1] * I;
            long double _Complex c2 = base_long_doubles[2] + 
                                     base_long_doubles[3] * I;
            long double _Complex c3 = base_long_doubles[4] + 
                                     base_long_doubles[5] * I;
            long double _Complex c4 = base_long_doubles[6] + 
                                     base_long_doubles[7] * I;
            
            long double _Complex c_result = complex_op(c1, c2, c3, c4);
            loop_result += creal(c_result) + cimag(c_result);
        }
        #endif
        
        /* 4. Vector operations if available */
        #if HAS_VECTORS
        {
            float32x4_t v1 = {base_doubles[0], base_doubles[1], 
                              base_doubles[2], base_doubles[3]};
            float32x4_t v2 = {base_doubles[4], base_doubles[5], 
                              base_doubles[6], base_doubles[7]};
            float32x4_t v3 = {base_doubles[8], base_doubles[9], 
                              base_doubles[10], base_doubles[11]};
            float32x4_t v4 = {base_doubles[12], base_doubles[13], 
                              base_doubles[14], base_doubles[15]};
            
            loop_result += vector_reduce(v1, v2, v3, v4);
        }
        #endif
        
        /* 5. DFP operations or fallback */
        #if HAS_DFP
        {
            _Decimal128 d1 = (_Decimal128)base_long_doubles[0];
            _Decimal128 d2 = (_Decimal128)base_long_doubles[1];
            _Decimal128 d3 = (_Decimal128)base_long_doubles[2];
            _Decimal128 d4 = (_Decimal128)base_long_doubles[3];
            _Decimal128 d5 = (_Decimal128)base_long_doubles[4];
            _Decimal128 d6 = (_Decimal128)base_long_doubles[5];
            
            _Decimal128 d_result = dfp_complex_op(d1, d2, d3, d4, d5, d6);
            loop_result += (double)d_result;
        }
        #else
        {
            /* Use manual multi-precision fallback */
            dfp128_fallback_t a = {.lo = (uint64_t)base_doubles[0], 
                                   .hi = (uint64_t)base_doubles[1]};
            dfp128_fallback_t b = {.lo = (uint64_t)base_doubles[2], 
                                   .hi = (uint64_t)base_doubles[3]};
            dfp128_fallback_t c = {.lo = (uint64_t)base_doubles[4], 
                                   .hi = (uint64_t)base_doubles[5]};
            dfp128_fallback_t d = {.lo = (uint64_t)base_doubles[6], 
                                   .hi = (uint64_t)base_doubles[7]};
            
            dfp128_fallback_t fb_result = manual_dfp_op(a, b, c, d);
            loop_result += (double)fb_result.lo + (double)fb_result.hi;
        }
        #endif
        
        /* Store result to prevent optimization */
        if (result_idx < 10) {
            results[result_idx++] = loop_result;
        }
        
        /* Update volatile global to prevent dead code elimination */
        g_result += (uint64_t)loop_result;
    }
    
    /* Compute checksum */
    double checksum = 0.0;
    for (int i = 0; i < result_idx; i++) {
        checksum += results[i];
    }
    
    /* Also use volatile to ensure computation isn't optimized away */
    volatile double final_checksum = checksum + (double)g_result;
    
    printf("Result checksum: %f\n", (double)final_checksum);
    return 0;
}
