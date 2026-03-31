#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Feature detection and fallbacks */
#ifdef __DECIMAL_BID_FORMAT__
    #define HAS_DFP 1
    #include <decimalfp.h>
#else
    #define HAS_DFP 0
#endif

#ifdef __STDC_IEC_559_COMPLEX__
    #include <complex.h>
    #define HAS_COMPLEX 1
#else
    #define HAS_COMPLEX 0
#endif

/* Vector type definitions */
typedef int32_t int32x4_t __attribute__((vector_size(16)));
typedef float float32x4_t __attribute__((vector_size(16)));
typedef double float64x2_t __attribute__((vector_size(16)));

/* Global volatile to prevent optimization */
volatile uint64_t g_result_accumulator = 0;

/* Helper function with 11 arguments - marked noinline to prevent simplification */
static __attribute__((noinline)) 
long double helper_11_args(
    long double a1, long double a2, long double a3,
    long double a4, long double a5, long double a6,
    long double a7, long double a8, long double a9,
    long double a10, long double a11)
{
    /* Complex expression that might expand to many operands */
    return ((a1 * a2) + (a3 / a4) - (a5 * a6) + 
            (a7 - a8) * (a9 + a10) / a11);
}

/* Helper with 10 arguments for DFP operations */
static __attribute__((noinline))
#if HAS_DFP
_Decimal128 helper_10_args_dfp(
    _Decimal64 d1, _Decimal64 d2, _Decimal64 d3,
    _Decimal64 d4, _Decimal64 d5, _Decimal64 d6,
    _Decimal128 d7, _Decimal128 d8, _Decimal128 d9,
    _Decimal128 d10)
{
    /* Complex DFP expression likely to need many operands */
    return ((_Decimal128)d1 * (_Decimal128)d2 + 
            (_Decimal128)d3 / (_Decimal128)d4 -
            (_Decimal128)d5 * (_Decimal128)d6 +
            d7 * d8 - d9 / d10);
}
#else
/* Fallback using integer arrays to simulate DFP */
uint64_t helper_10_args_dfp_fallback(
    uint64_t d1, uint64_t d2, uint64_t d3,
    uint64_t d4, uint64_t d5, uint64_t d6,
    uint64_t d7[2], uint64_t d8[2], uint64_t d9[2],
    uint64_t d10[2])
{
    /* Manual multi-precision arithmetic */
    uint64_t result[2] = {0, 0};
    
    /* Simulate: d1*d2 + d3/d4 - d5*d6 + d7*d8 - d9/d10 */
    uint64_t t1 = d1 * d2;
    uint64_t t2 = d3 / (d4 ? d4 : 1);
    uint64_t t3 = d5 * d6;
    
    /* 128-bit multiplication simulation */
    uint64_t t4_lo, t4_hi;
    __asm__("" : "=r"(t4_lo), "=r"(t4_hi) : "0"(d7[0]), "1"(d7[1]));
    
    uint64_t t5_lo = d9[0] / (d10[0] ? d10[0] : 1);
    
    result[0] = t1 + t2 - t3 + t4_lo - t5_lo;
    result[1] = t4_hi; /* Carry from 128-bit operations */
    
    return result[0] + result[1];
}
#endif

/* Complex number helper with mixed operations */
#if HAS_COMPLEX
static __attribute__((noinline))
long double _Complex helper_complex_ops(
    long double _Complex c1, long double _Complex c2,
    long double _Complex c3, long double _Complex c4,
    long double _Complex c5, long double _Complex c6,
    long double _Complex c7, long double _Complex c8,
    long double _Complex c9, long double _Complex c10)
{
    /* Complex expression that may expand to many real/imag operations */
    long double _Complex t1 = c1 * c2;
    long double _Complex t2 = c3 / c4;
    long double _Complex t3 = c5 - c6;
    long double _Complex t4 = c7 + c8;
    long double _Complex t5 = c9 * c10;
    
    return (t1 + t2) * (t3 - t4) / t5;
}
#endif

/* Vector reduction with accumulation */
static float vector_reduce_sum(float32x4_t v) {
    /* Horizontal sum that may expand to multiple operations */
    float32x4_t t = __builtin_shufflevector(v, v, 2, 3, 0, 1);
    v = v + t;
    t = __builtin_shufflevector(v, v, 1, 0, 3, 2);
    v = v + t;
    return v[0];
}

int main(int argc, char *argv[]) {
    int seed = 12345;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    srand(seed);
    
    /* Initialize base values */
    long double ld_vals[20];
    for (int i = 0; i < 20; i++) {
        ld_vals[i] = (long double)(rand() % 1000) / 100.0 + 1.0;
    }
    
#if HAS_DFP
    /* DFP initialization */
    _Decimal64 d64_vals[10];
    _Decimal128 d128_vals[10];
    for (int i = 0; i < 10; i++) {
        d64_vals[i] = (_Decimal64)(i + 1) * 1.5DD;
        d128_vals[i] = (_Decimal128)(i + 1) * 2.5DL;
    }
#else
    /* Fallback DFP simulation */
    uint64_t d64_fallback[10];
    uint64_t d128_fallback[10][2];
    for (int i = 0; i < 10; i++) {
        d64_fallback[i] = (i + 1) * 150;
        d128_fallback[i][0] = (i + 1) * 250;
        d128_fallback[i][1] = 0;
    }
#endif
    
#if HAS_COMPLEX
    /* Complex initialization */
    long double _Complex complex_vals[10];
    for (int i = 0; i < 10; i++) {
        complex_vals[i] = (long double)(i + 1) + 
                         (long double)(i * 2) * I;
    }
#endif
    
    /* Vector initialization */
    float32x4_t vec_vals[5];
    for (int i = 0; i < 5; i++) {
        vec_vals[i] = (float32x4_t){
            (float)(i*4 + 1), (float)(i*4 + 2),
            (float)(i*4 + 3), (float)(i*4 + 4)
        };
    }
    
    /* Main computation loop */
    volatile long double loop_results[5] = {0};
    
    for (int iter = 0; iter < 3; iter++) {
        long double result = 0.0;
        
        /* 1. Call helper with 11 arguments */
        long double h1 = helper_11_args(
            ld_vals[0], ld_vals[1], ld_vals[2],
            ld_vals[3], ld_vals[4], ld_vals[5],
            ld_vals[6], ld_vals[7], ld_vals[8],
            ld_vals[9], ld_vals[10]
        );
        
        /* 2. DFP operations with conditional execution */
        volatile int condition = seed % 2;
        if (condition) {
#if HAS_DFP
            _Decimal128 dfp_result = helper_10_args_dfp(
                d64_vals[0], d64_vals[1], d64_vals[2],
                d64_vals[3], d64_vals[4], d64_vals[5],
                d128_vals[0], d128_vals[1], d128_vals[2],
                d128_vals[3]
            );
            /* Convert DFP to long double for accumulation */
            result += (long double)dfp_result;
#else
            uint64_t dfp_result = helper_10_args_dfp_fallback(
                d64_fallback[0], d64_fallback[1], d64_fallback[2],
                d64_fallback[3], d64_fallback[4], d64_fallback[5],
                d128_fallback[0], d128_fallback[1], d128_fallback[2],
                d128_fallback[3]
            );
            result += (long double)dfp_result;
#endif
        }
        
        /* 3. Complex number operations */
#if HAS_COMPLEX
        long double _Complex cresult = helper_complex_ops(
            complex_vals[0], complex_vals[1], complex_vals[2],
            complex_vals[3], complex_vals[4], complex_vals[5],
            complex_vals[6], complex_vals[7], complex_vals[8],
            complex_vals[9]
        );
        result += creall(cresult) + cimagl(cresult);
#endif
        
        /* 4. Vector reduction with accumulation */
        float vec_sum = 0.0f;
        for (int i = 0; i < 5; i++) {
            vec_sum += vector_reduce_sum(vec_vals[i]);
            /* Modify vector slightly each iteration */
            vec_vals[i][0] += 0.1f;
        }
        result += (long double)vec_sum;
        
        /* 5. Additional complex expression inline */
#if HAS_COMPLEX
        if (condition) {
            /* Complex power operation that may expand */
            long double _Complex cp1 = complex_vals[0] * complex_vals[1];
            long double _Complex cp2 = complex_vals[2] / complex_vals[3];
            long double _Complex cp3 = cp1 + cp2;
            long double _Complex cp4 = complex_vals[4] - complex_vals[5];
            long double _Complex cp5 = cp3 * cp4;
            long double _Complex cp6 = complex_vals[6] + complex_vals[7];
            long double _Complex cp7 = cp5 / cp6;
            
            result += creall(cp7) * cimagl(cp7);
        }
#endif
        
        /* Combine all results */
        result += h1;
        loop_results[iter] = result;
        
        /* Update global accumulator to prevent elimination */
        g_result_accumulator += (uint64_t)result;
    }
    
    /* Compute final checksum */
    uint64_t checksum = g_result_accumulator;
    for (int i = 0; i < 3; i++) {
        checksum += (uint64_t)loop_results[i];
    }
    
    printf("Result checksum: %llu\n", (unsigned long long)checksum);
    return 0;
}
