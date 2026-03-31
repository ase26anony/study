/* 
 * Test program targeting GCC optabs.cc lines 8254-8263
 * Generates multi-operand expressions for DFP, complex, and vector operations
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Enable complex.h if available */
#ifdef __STDC_IEC_559_COMPLEX__
#include <complex.h>
#endif

/* Feature detection and fallbacks */
#if defined(__DECIMAL_BID_FORMAT__) || defined(__DPD_FORMAT__)
#define HAS_DFP 1
#else
#define HAS_DFP 0
#endif

/* GCC vector extensions */
#if defined(__GNUC__) && (__GNUC__ >= 4)
#define HAS_VECTOR 1
#else
#define HAS_VECTOR 0
#endif

/* DFP types with fallbacks */
#if HAS_DFP
typedef _Decimal64  dfp64_t;
typedef _Decimal128 dfp128_t;
#else
/* Software emulation using unions for 64-bit and 128-bit values */
typedef union {
    uint64_t u[2];
    double   f[2];
} dfp128_t;

typedef union {
    uint64_t u;
    double   f;
} dfp64_t;
#endif

/* Complex types */
#ifdef __STDC_IEC_559_COMPLEX__
typedef long double _Complex clong_t;
typedef double _Complex cdouble_t;
#else
typedef struct { long double re, im; } clong_t;
typedef struct { double re, im; } cdouble_t;
#endif

/* Vector types */
#if HAS_VECTOR
typedef int32_t int32x8_t __attribute__((vector_size(32)));
typedef float float64x4_t __attribute__((vector_size(32)));
#else
typedef struct { int32_t v[8]; } int32x8_t;
typedef struct { double v[4]; } float64x4_t;
#endif

/* Global volatile to prevent optimization */
volatile uint64_t g_result = 0;

/* Helper function with 10 arguments - marked noinline */
static uint64_t __attribute__((noinline))
helper_10_args(dfp64_t a1, dfp128_t a2, clong_t a3, cdouble_t a4,
               int32x8_t a5, float64x4_t a6, uint64_t a7, 
               uint64_t a8, uint64_t a9, uint64_t a10)
{
    /* Combine all arguments into a single result */
    uint64_t result = 0;
    
#if HAS_DFP
    /* DFP operations */
    result += (uint64_t)((double)a1 * 1.5);
#else
    result += a1.u;
#endif
    
    /* Complex operations */
#ifdef __STDC_IEC_559_COMPLEX__
    result += (uint64_t)creall(a3) + (uint64_t)cimagl(a3);
#else
    result += (uint64_t)a3.re + (uint64_t)a3.im;
#endif
    
    /* Vector reduction */
#if HAS_VECTOR
    for (int i = 0; i < 8; i++) result += a5[i];
    for (int i = 0; i < 4; i++) result += (uint64_t)a6[i];
#else
    for (int i = 0; i < 8; i++) result += a5.v[i];
    for (int i = 0; i < 4; i++) result += (uint64_t)a6.v[i];
#endif
    
    result += a7 + a8 + a9 + a10;
    return result;
}

/* Helper function with 11 arguments */
static uint64_t __attribute__((noinline))
helper_11_args(dfp64_t a1, dfp128_t a2, clong_t a3, cdouble_t a4,
               int32x8_t a5, float64x4_t a6, uint64_t a7,
               uint64_t a8, uint64_t a9, uint64_t a10, uint64_t a11)
{
    uint64_t base = helper_10_args(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
    return base + a11;
}

/* Initialize DFP values */
static void init_dfp(dfp64_t *d64, dfp128_t *d128)
{
#if HAS_DFP
    *d64 = 1.5dd;
    *d128 = 3.1415926535897932384626433832795028841971dl;
#else
    d64->u = 0x3FF8000000000000ULL;  /* 1.5 in double */
    d128->u[0] = 0x400921FB54442D18ULL;
    d128->u[1] = 0x0000000000000000ULL;
#endif
}

/* Initialize complex values */
static void init_complex(clong_t *cl, cdouble_t *cd)
{
#ifdef __STDC_IEC_559_COMPLEX__
    *cl = 1.0L + 2.0LI;
    *cd = 3.0 + 4.0I;
#else
    cl->re = 1.0L;
    cl->im = 2.0L;
    cd->re = 3.0;
    cd->im = 4.0;
#endif
}

/* Initialize vector values */
static void init_vectors(int32x8_t *vi, float64x4_t *vf)
{
#if HAS_VECTOR
    *vi = (int32x8_t){1, 2, 3, 4, 5, 6, 7, 8};
    *vf = (float64x4_t){1.1, 2.2, 3.3, 4.4};
#else
    for (int i = 0; i < 8; i++) vi->v[i] = i + 1;
    for (int i = 0; i < 4; i++) vf->v[i] = (i + 1) * 1.1;
#endif
}

/* Complex multiplication */
static clong_t complex_mul(clong_t a, clong_t b)
{
#ifdef __STDC_IEC_559_COMPLEX__
    return a * b;
#else
    clong_t result;
    result.re = a.re * b.re - a.im * b.im;
    result.im = a.re * b.im + a.im * b.re;
    return result;
#endif
}

/* Complex division */
static clong_t complex_div(clong_t a, clong_t b)
{
#ifdef __STDC_IEC_559_COMPLEX__
    return a / b;
#else
    clong_t result;
    long double denom = b.re * b.re + b.im * b.im;
    result.re = (a.re * b.re + a.im * b.im) / denom;
    result.im = (a.im * b.re - a.re * b.im) / denom;
    return result;
#endif
}

/* Vector reduction with accumulation */
static uint64_t vector_reduce(int32x8_t v, uint64_t acc)
{
    uint64_t sum = acc;
#if HAS_VECTOR
    for (int i = 0; i < 8; i++) sum += v[i];
#else
    for (int i = 0; i < 8; i++) sum += v.v[i];
#endif
    return sum;
}

int main(int argc, char *argv[])
{
    /* Use command line seed for deterministic behavior */
    int seed = 42;
    if (argc > 1) seed = atoi(argv[1]);
    srand(seed);
    
    /* Base variables */
    dfp64_t d64_a, d64_b, d64_c, d64_d;
    dfp128_t d128_a, d128_b, d128_c, d128_d;
    clong_t cl_a, cl_b, cl_c, cl_d;
    cdouble_t cd_a, cd_b;
    int32x8_t vec_i;
    float64x4_t vec_f;
    
    /* Initialize values */
    init_dfp(&d64_a, &d128_a);
    init_dfp(&d64_b, &d128_b);
    init_dfp(&d64_c, &d128_c);
    init_dfp(&d64_d, &d128_d);
    init_complex(&cl_a, &cd_a);
    init_complex(&cl_b, &cd_b);
    init_complex(&cl_c, &cl_d);
    init_vectors(&vec_i, &vec_f);
    
    /* Volatile condition to prevent constant folding */
    volatile int condition = seed;
    
    /* Result storage */
    uint64_t results[5] = {0};
    
    /* Main computation loop */
    for (int iter = 0; iter < 3; iter++) {
        uint64_t iter_result = 0;
        
        /* Conditional execution block */
        if (condition & (1 << iter)) {
            /* DFP arithmetic - may expand to multi-operand RTL */
#if HAS_DFP
            dfp128_t d128_result;
            /* Complex expression with many operands */
            d128_result = d128_a * d128_b + d128_c / d128_d;
            iter_result += (uint64_t)((double)d128_result * 100.0);
#else
            /* Software emulation */
            double temp = d128_a.f[0] * d128_b.f[0] + d128_c.f[0] / d128_d.f[0];
            iter_result += (uint64_t)(temp * 100.0);
#endif
            
            /* Complex arithmetic with library-like operations */
            clong_t cl_result;
            cl_result = complex_mul(cl_a, cl_b);
            cl_result = complex_div(cl_result, cl_c);
            
#ifdef __STDC_IEC_559_COMPLEX__
            iter_result += (uint64_t)creall(cl_result) + (uint64_t)cimagl(cl_result);
#else
            iter_result += (uint64_t)cl_result.re + (uint64_t)cl_result.im;
#endif
        } else {
            /* Alternative path with different operations */
            /* Vector reduction with accumulation */
            uint64_t vec_acc = 0;
            for (int i = 0; i < 2; i++) {  /* Small fixed loop */
                vec_acc = vector_reduce(vec_i, vec_acc);
                
#if HAS_VECTOR
                /* Vector operations */
                vec_i += (int32x8_t){1, 1, 1, 1, 1, 1, 1, 1};
#endif
            }
            iter_result += vec_acc;
        }
        
        /* Call helper functions with many arguments */
        if (iter % 2 == 0) {
            results[iter] = helper_10_args(d64_a, d128_a, cl_a, cd_a,
                                          vec_i, vec_f, iter_result,
                                          seed, iter, condition);
        } else {
            results[iter] = helper_11_args(d64_b, d128_b, cl_b, cd_b,
                                          vec_i, vec_f, iter_result,
                                          seed, iter, condition, results[iter-1]);
        }
        
        /* Store to volatile global to prevent elimination */
        g_result += results[iter];
    }
    
    /* Compute checksum */
    uint64_t checksum = 0;
    for (int i = 0; i < 3; i++) {
        checksum += results[i];
    }
    
    printf("Checksum: %llu\n", (unsigned long long)checksum);
    printf("Global result: %llu\n", (unsigned long long)g_result);
    
    return 0;
}
