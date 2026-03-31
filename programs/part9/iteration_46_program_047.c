#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* Feature detection and fallbacks */
#ifdef __DECIMAL_BID_FORMAT__
    #define HAS_DFP 1
    #ifdef __STDC_IEC_559_COMPLEX__
        #include <complex.h>
        #define HAS_COMPLEX 1
    #endif
#else
    #define HAS_DFP 0
#endif

#ifdef __VECTOR_TYPES_SUPPORTED__
    #define HAS_VECTOR 1
    typedef int32_t int32x8_t __attribute__((vector_size(32)));
    typedef double float64x4_t __attribute__((vector_size(32)));
#else
    #define HAS_VECTOR 0
    typedef struct { int32_t v[8]; } int32x8_t;
    typedef struct { double v[4]; } float64x4_t;
#endif

/* DFP fallback structures if not supported */
#if !HAS_DFP
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
#if !HAS_COMPLEX
typedef struct {
    long double real;
    long double imag;
} long_double_complex;
#define _Complex long_double_complex
#endif

/* Helper function with 11 arguments - marked noinline to prevent simplification */
static __attribute__((noinline)) long double 
complex_multi_op_helper(
    long double a1, long double a2, long double a3,
    long double a4, long double a5, long double a6,
    long double a7, long double a8, long double a9,
    long double a10, long double a11)
{
    /* Complex expression that may expand to many operands */
    return ((a1 * a2) + (a3 / a4) - (a5 * a6) + 
            (a7 - a8) * (a9 + a10) / a11);
}

/* Another helper with 10 arguments for DFP operations */
static __attribute__((noinline)) long double
dfp_multi_op_helper(
    long double d1, long double d2, long double d3,
    long double d4, long double d5, long double d6,
    long double d7, long double d8, long double d9,
    long double d10)
{
    /* DFP-style computation that may require many temporaries */
    return (((d1 * d2) + (d3 * d4)) / ((d5 - d6) * (d7 + d8))) * (d9 / d10);
}

/* Vector reduction helper */
static __attribute__((noinline)) double
vector_reduce_helper(float64x4_t vec)
{
    #if HAS_VECTOR
    double sum = 0.0;
    for (int i = 0; i < 4; i++) {
        sum += vec[i];
    }
    return sum;
    #else
    double sum = 0.0;
    for (int i = 0; i < 4; i++) {
        sum += ((float64x4_t*)&vec)->v[i];
    }
    return sum;
    #endif
}

/* Main computation function */
static volatile double global_accumulator[10];

int main(int argc, char *argv[])
{
    int seed = 12345;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    srand(seed);
    
    /* Initialize base variables with deterministic but non-constant values */
    long double base_values[20];
    for (int i = 0; i < 20; i++) {
        base_values[i] = (long double)(rand() % 1000) / 100.0 + 1.0;
    }
    
    /* DFP-like variables (or fallbacks) */
    _Decimal64 dfp64_a, dfp64_b, dfp64_c;
    _Decimal128 dfp128_a, dfp128_b, dfp128_c, dfp128_d;
    
    #if HAS_DFP
    /* Use actual DFP if available */
    dfp64_a = (_Decimal64)base_values[0];
    dfp64_b = (_Decimal64)base_values[1];
    dfp64_c = (_Decimal64)base_values[2];
    
    dfp128_a = (_Decimal128)base_values[3];
    dfp128_b = (_Decimal128)base_values[4];
    dfp128_c = (_Decimal128)base_values[5];
    dfp128_d = (_Decimal128)base_values[6];
    #else
    /* Fallback initialization */
    dfp64_a.ull[0] = (unsigned long long)(base_values[0] * 1000);
    dfp64_b.ull[0] = (unsigned long long)(base_values[1] * 1000);
    dfp64_c.ull[0] = (unsigned long long)(base_values[2] * 1000);
    
    for (int i = 0; i < 4; i++) {
        dfp128_a.ull[i] = (unsigned long long)(base_values[3 + i] * 1000);
        dfp128_b.ull[i] = (unsigned long long)(base_values[4 + i] * 1000);
        dfp128_c.ull[i] = (unsigned long long)(base_values[5 + i] * 1000);
        dfp128_d.ull[i] = (unsigned long long)(base_values[6 + i] * 1000);
    }
    #endif
    
    /* Complex variables */
    #if HAS_COMPLEX
    long double _Complex ca, cb, cc, cd;
    ca = base_values[7] + base_values[8] * I;
    cb = base_values[9] + base_values[10] * I;
    cc = base_values[11] + base_values[12] * I;
    cd = base_values[13] + base_values[14] * I;
    #else
    long_double_complex ca, cb, cc, cd;
    ca.real = base_values[7]; ca.imag = base_values[8];
    cb.real = base_values[9]; cb.imag = base_values[10];
    cc.real = base_values[11]; cc.imag = base_values[12];
    cd.real = base_values[13]; cd.imag = base_values[14];
    #endif
    
    /* Vector variables */
    float64x4_t vec1, vec2;
    #if HAS_VECTOR
    for (int i = 0; i < 4; i++) {
        vec1[i] = base_values[15 + i];
        vec2[i] = base_values[16 + i];
    }
    #else
    for (int i = 0; i < 4; i++) {
        ((float64x4_t*)&vec1)->v[i] = base_values[15 + i];
        ((float64x4_t*)&vec2)->v[i] = base_values[16 + i];
    }
    #endif
    
    volatile int condition = seed % 2;
    double accumulator = 0.0;
    
    /* Main computation loop - designed to create many operands */
    for (int iter = 0; iter < 3; iter++) {
        long double result1, result2, result3, result4;
        
        /* Conditional execution to prevent constant folding */
        if (condition) {
            /* Complex DFP-like computation that may expand to many operands */
            #if HAS_DFP
            /* Use DFP builtins if available */
            result1 = (long double)__builtin_dadd(
                __builtin_dmul(dfp128_a, dfp128_b),
                __builtin_ddiv(dfp128_c, dfp128_d));
            #else
            /* Manual multi-precision emulation */
            result1 = (base_values[3] * base_values[4]) + 
                     (base_values[5] / base_values[6]);
            #endif
            
            /* Complex number operations */
            #if HAS_COMPLEX
            long double _Complex temp = (ca * cb) / (cc - cd);
            result2 = creal(temp) + cimag(temp);
            #else
            result2 = (ca.real * cb.real - ca.imag * cb.imag) /
                     (cc.real - cd.real + cc.imag - cd.imag);
            #endif
        } else {
            /* Alternative path with different computations */
            #if HAS_DFP
            result1 = (long double)__builtin_dsub(
                __builtin_dmul(dfp128_b, dfp128_c),
                __builtin_ddiv(dfp128_a, dfp128_d));
            #else
            result1 = (base_values[4] * base_values[5]) - 
                     (base_values[3] / base_values[6]);
            #endif
            
            #if HAS_COMPLEX
            long double _Complex temp = csqrt(ca) * cpow(cb, cc);
            result2 = creal(temp) - cimag(temp);
            #else
            result2 = sqrt(ca.real * ca.real + ca.imag * ca.imag) *
                     pow(cb.real * cb.real + cb.imag * cb.imag, 
                         cc.real * cc.real + cc.imag * cc.imag);
            #endif
        }
        
        /* Vector reduction with accumulation */
        result3 = vector_reduction_helper(vec1);
        float64x4_t vec_result;
        #if HAS_VECTOR
        vec_result = vec1 + vec2;
        for (int i = 0; i < 2; i++) {  /* Small loop to repeat operations */
            vec_result = vec_result * vec1 - vec2;
        }
        result4 = vector_reduction_helper(vec_result);
        #else
        for (int i = 0; i < 4; i++) {
            ((float64x4_t*)&vec_result)->v[i] = 
                ((float64x4_t*)&vec1)->v[i] + ((float64x4_t*)&vec2)->v[i];
        }
        for (int i = 0; i < 2; i++) {
            for (int j = 0; j < 4; j++) {
                ((float64x4_t*)&vec_result)->v[j] = 
                    ((float64x4_t*)&vec_result)->v[j] * 
                    ((float64x4_t*)&vec1)->v[j] - 
                    ((float64x4_t*)&vec2)->v[j];
            }
        }
        result4 = vector_reduction_helper(vec_result);
        #endif
        
        /* Call helper functions with many arguments */
        long double helper_result1 = complex_multi_op_helper(
            result1, result2, result3, result4,
            base_values[0], base_values[1], base_values[2],
            base_values[3], base_values[4], base_values[5],
            base_values[6]);
        
        long double helper_result2 = dfp_multi_op_helper(
            result1, result2, result3, result4,
            base_values[7], base_values[8], base_values[9],
            base_values[10], base_values[11], base_values[12]);
        
        /* Aggregate results to prevent elimination */
        global_accumulator[iter * 2] = helper_result1;
        global_accumulator[iter * 2 + 1] = helper_result2;
        accumulator += helper_result1 + helper_result2;
        
        /* Modify condition for next iteration */
        condition = !condition;
    }
    
    /* Compute final checksum */
    double checksum = accumulator;
    for (int i = 0; i < 6; i++) {
        checksum += global_accumulator[i];
    }
    
    printf("Result checksum: %f\n", checksum);
    return 0;
}
