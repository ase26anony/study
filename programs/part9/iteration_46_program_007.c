#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* Enable complex.h if available */
#ifdef __STDC_IEC_559_COMPLEX__
#include <complex.h>
#endif

/* Feature detection and fallbacks */
#ifdef __DECIMAL_BID_FORMAT__
#define HAS_DFP 1
#else
#define HAS_DFP 0
#endif

/* Vector type definitions */
#if defined(__GNUC__) && (defined(__x86_64__) || defined(__i386__))
typedef int32_t v8si __attribute__((vector_size(32)));
typedef double v4df __attribute__((vector_size(32)));
#define HAS_VECTORS 1
#elif defined(__GNUC__) && defined(__aarch64__)
typedef int32_t v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
#define HAS_VECTORS 1
#else
#define HAS_VECTORS 0
#endif

/* DFP fallback using unions for portability */
#if !HAS_DFP
typedef union {
    unsigned long long words[2];
    double dbl;
} decimal64_fb;

typedef union {
    unsigned long long words[4];
    long double ldbl;
} decimal128_fb;

#define _Decimal64 decimal64_fb
#define _Decimal128 decimal128_fb
#endif

/* Helper function with 11 arguments - marked noinline to prevent optimization */
static __attribute__((noinline)) 
long double complex helper_11_args(
    long double a1, long double a2, long double a3,
    long double a4, long double a5, long double a6,
    long double a7, long double a8, long double a9,
    long double a10, long double a11)
{
    /* Complex arithmetic that may expand to many operands */
    long double complex c1 = a1 + a2 * I;
    long double complex c2 = a3 + a4 * I;
    long double complex c3 = a5 + a6 * I;
    
    /* Complex expression with many intermediate values */
    long double complex result = (c1 * c2) / (c3 + (a7 + a8 * I));
    result += (a9 + a10 * I) * (a11 + 1.0L * I);
    
    return result;
}

/* Another helper with 10 mixed-type arguments */
static __attribute__((noinline))
#if HAS_DFP
_Decimal128 helper_10_args_mixed(
    _Decimal64 d1, _Decimal64 d2,
    _Decimal128 d3, _Decimal128 d4,
    long double ld1, long double ld2,
    long double complex c1, long double complex c2,
    int64_t i1, int64_t i2)
#else
decimal128_fb helper_10_args_mixed(
    decimal64_fb d1, decimal64_fb d2,
    decimal128_fb d3, decimal128_fb d4,
    long double ld1, long double ld2,
    long double complex c1, long double complex c2,
    int64_t i1, int64_t i2)
#endif
{
#if HAS_DFP
    /* DFP arithmetic that may require many operands */
    _Decimal128 result = d3 * d4 + (_Decimal128)d1 / (_Decimal128)d2;
    result = result + (_Decimal128)(creal(c1) + creal(c2));
    result = result * (_Decimal128)(ld1 - ld2);
    result = result + (_Decimal128)(i1 * i2);
    return result;
#else
    /* Fallback using manual arithmetic */
    decimal128_fb result;
    result.ldbl = d3.ldbl * d4.ldbl + d1.dbl / d2.dbl;
    result.ldbl += creal(c1) + creal(c2);
    result.ldbl *= ld1 - ld2;
    result.ldbl += i1 * i2;
    return result;
#endif
}

/* Vector reduction helper */
#if HAS_VECTORS
static __attribute__((noinline))
double vector_reduce_accumulate(v4df vec, double accumulator)
{
    /* Horizontal reduction that may expand to many operations */
    double sum = vec[0] + vec[1] + vec[2] + vec[3];
    
    /* Complex expression with the accumulator */
    double result = sum * accumulator - (sum + accumulator) / (accumulator + 1.0);
    result = result + (sum * sum) / (accumulator * accumulator + 1.0);
    
    return result;
}
#endif

/* Global volatile to prevent optimization */
volatile long double global_accumulator = 0.0L;

int main(int argc, char *argv[])
{
    /* Use command line seed for deterministic behavior */
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
    
    _Decimal128 d128_a = 1.2345678901234567dl;
    _Decimal128 d128_b = 2.3456789012345678dl;
    _Decimal128 d128_c = 3.4567890123456789dl;
    _Decimal128 d128_d = 4.5678901234567890dl;
#else
    decimal64_fb d64_a = {.dbl = 1.23456};
    decimal64_fb d64_b = {.dbl = 2.34567};
    decimal64_fb d64_c = {.dbl = 3.45678};
    decimal64_fb d64_d = {.dbl = 4.56789};
    
    decimal128_fb d128_a = {.ldbl = 1.2345678901234567L};
    decimal128_fb d128_b = {.ldbl = 2.3456789012345678L};
    decimal128_fb d128_c = {.ldbl = 3.4567890123456789L};
    decimal128_fb d128_d = {.ldbl = 4.5678901234567890L};
#endif
    
    /* Complex numbers */
    long double complex ca = 1.0L + 2.0L * I;
    long double complex cb = 3.0L + 4.0L * I;
    long double complex cc = 5.0L + 6.0L * I;
    long double complex cd = 7.0L + 8.0L * I;
    
#if HAS_VECTORS
    /* Initialize vectors */
    v4df vec1 = {1.1, 2.2, 3.3, 4.4};
    v4df vec2 = {5.5, 6.6, 7.7, 8.8};
#endif
    
    /* Array to store results */
    long double results[5] = {0};
    int result_idx = 0;
    
    /* Volatile variable to prevent constant folding */
    volatile int condition = seed % 2;
    
    /* Main computation loop */
    for (int iter = 0; iter < 3; iter++) {
        long double iter_result = 0.0L;
        
        /* Conditional execution to prevent optimization */
        if (condition || iter > 0) {
            /* DFP arithmetic with many operands */
#if HAS_DFP
            _Decimal128 d128_result = d128_a * d128_b + d128_c / d128_d;
            d128_result = d128_result - (_Decimal128)d64_a + (_Decimal128)d64_b;
            d128_result = d128_result * (_Decimal128)(d64_c / d64_d);
            
            /* Convert to long double for accumulation */
            iter_result += (long double)d128_result;
#else
            decimal128_fb d128_result;
            d128_result.ldbl = d128_a.ldbl * d128_b.ldbl + d128_c.ldbl / d128_d.ldbl;
            d128_result.ldbl = d128_result.ldbl - d64_a.dbl + d64_b.dbl;
            d128_result.ldbl = d128_result.ldbl * (d64_c.dbl / d64_d.dbl);
            
            iter_result += d128_result.ldbl;
#endif
            
            /* Complex arithmetic with library functions */
            long double complex cl_result = (ca * cb) / (cc - cd);
            
            /* Complex power and sqrt operations */
#ifdef __STDC_IEC_559_COMPLEX__
            cl_result = cpow(cl_result, 2.0L + 0.5L * I);
            cl_result = cl_result + csqrt(ca * cb - cc / cd);
#endif
            
            iter_result += creal(cl_result) + cimag(cl_result);
            
#if HAS_VECTORS
            /* Vector reduction with accumulation */
            double vec_acc = 1.0 + iter * 0.1;
            double vec_result = vector_reduce_accumulate(vec1 + vec2, vec_acc);
            iter_result += vec_result;
#endif
            
            /* Call helper with 11 arguments */
            long double complex helper1_result = helper_11_args(
                iter_result, 2.0L, 3.0L, 4.0L, 5.0L,
                6.0L, 7.0L, 8.0L, 9.0L, 10.0L, 11.0L);
            
            iter_result += creal(helper1_result);
            
            /* Call helper with 10 mixed arguments */
#if HAS_DFP
            _Decimal128 helper2_result = helper_10_args_mixed(
                d64_a, d64_b, d128_c, d128_d,
                creal(ca), cimag(cb),
                cc, cd,
                seed + iter, seed - iter);
            
            iter_result += (long double)helper2_result;
#else
            decimal128_fb helper2_result = helper_10_args_mixed(
                d64_a, d64_b, d128_c, d128_d,
                creal(ca), cimag(cb),
                cc, cd,
                seed + iter, seed - iter);
            
            iter_result += helper2_result.ldbl;
#endif
        }
        
        /* Store result */
        results[result_idx++] = iter_result;
        global_accumulator += iter_result;
        
        /* Modify some values for next iteration */
        condition = !condition;
#if HAS_DFP
        d64_a += 0.1dd;
        d128_b *= 1.01dl;
#else
        d64_a.dbl += 0.1;
        d128_b.ldbl *= 1.01L;
#endif
        ca += 0.1L + 0.1L * I;
    }
    
    /* Compute checksum */
    long double checksum = 0.0L;
    for (int i = 0; i < result_idx; i++) {
        checksum += results[i];
    }
    
    /* Print result (using volatile to ensure computation happens) */
    printf("Checksum: %Lf\n", checksum + (long double)global_accumulator * 0.0L);
    
    return 0;
}
