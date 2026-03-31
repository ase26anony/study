/* 
 * Test program targeting GCC's optabs.cc uncovered lines 8254-8263
 * Generates multi-operand internal function expansions for DFP, complex,
 * and vector operations.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

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

#ifdef __VECTOR_TYPES_SUPPORTED__
#define HAS_VECTOR 1
#else
#define HAS_VECTOR 0
#endif

/* DFP types with fallbacks */
#if HAS_DFP
typedef _Decimal64  d64_t;
typedef _Decimal128 d128_t;
#else
/* Software emulation using unions for 64-bit and 128-bit decimal */
typedef union {
    uint64_t i;
    double   f;
} d64_t;

typedef struct {
    uint64_t lo;
    uint64_t hi;
} d128_t;
#endif

/* Complex types */
#ifdef __STDC_IEC_559_COMPLEX__
typedef long double _Complex cl_t;
#else
typedef struct {
    long double real;
    long double imag;
} cl_t;
#endif

/* Vector types */
#if HAS_VECTOR
typedef int32_t int32x8_t __attribute__((vector_size(32)));
typedef double  float64x4_t __attribute__((vector_size(32)));
#else
typedef struct {
    int32_t data[8];
} int32x8_t;
typedef struct {
    double data[4];
} float64x4_t;
#endif

/* Volatile global to prevent optimization */
volatile uint64_t g_result = 0;

/* Helper function with 11 arguments - targeting case 11 */
static d128_t __attribute__((noinline))
helper_11_args(d64_t a0, d64_t a1, d128_t a2, d128_t a3, 
               cl_t a4, cl_t a5, cl_t a6,
               int32x8_t a7, float64x4_t a8,
               uint64_t a9, uint64_t a10)
{
    d128_t result;
    
#if HAS_DFP
    /* DFP arithmetic that may expand to multi-operand pattern */
    result = (d128_t)a0 * (d128_t)a1 + a2 - a3;
#else
    /* Fallback: combine using integer arithmetic */
    result.lo = a0.i + a1.i + a2.lo + a3.lo;
    result.hi = a2.hi + a3.hi;
#endif
    
    /* Use complex arguments */
#ifdef __STDC_IEC_559_COMPLEX__
    cl_t csum = a4 + a5 + a6;
    result = result + (d128_t)(creal(csum) + cimag(csum));
#endif
    
    /* Use vector arguments */
#if HAS_VECTOR
    /* Horizontal reduction */
    double vsum = a8[0] + a8[1] + a8[2] + a8[3];
    result = result + (d128_t)vsum;
#endif
    
    /* Use integer arguments */
    result = result + (d128_t)(a9 + a10);
    
    return result;
}

/* Helper function with 10 arguments - targeting case 10 */
static cl_t __attribute__((noinline))
helper_10_args(cl_t a0, cl_t a1, cl_t a2, cl_t a3,
               d128_t a4, d128_t a5, d128_t a6,
               float64x4_t a7, int32x8_t a8,
               uint64_t a9)
{
    cl_t result;
    
#ifdef __STDC_IEC_559_COMPLEX__
    /* Complex arithmetic that may require many operands */
    result = (a0 * a1) / (a2 - a3);
    
    /* Add DFP values */
    result += (cl_t)((double)a4 + (double)a5 + (double)a6);
#else
    result.real = a0.real * a1.real - a0.imag * a1.imag;
    result.imag = a0.real * a1.imag + a0.imag * a1.real;
#endif
    
    /* Add vector contribution */
#if HAS_VECTOR
    double vsum = a7[0] + a7[1] + a7[2] + a7[3];
    result += (cl_t)vsum;
#endif
    
    /* Add integer */
    result += (cl_t)(double)a9;
    
    return result;
}

/* Initialize DFP values */
static void init_dfp(d64_t *d64, d128_t *d128, int seed)
{
#if HAS_DFP
    *d64 = seed * 1.5DL;
    *d128 = seed * 2.7DL;
#else
    d64->i = seed * 3;
    d128->lo = seed * 5;
    d128->hi = seed * 7;
#endif
}

/* Initialize complex values */
static void init_complex(cl_t *c, int seed)
{
#ifdef __STDC_IEC_559_COMPLEX__
    *c = seed * 1.1L + I * seed * 0.9L;
#else
    c->real = seed * 1.1L;
    c->imag = seed * 0.9L;
#endif
}

/* Initialize vector values */
static void init_vectors(int32x8_t *vi, float64x4_t *vf, int seed)
{
#if HAS_VECTOR
    for (int i = 0; i < 8; i++) vi[i] = seed * (i + 1);
    for (int i = 0; i < 4; i++) vf[i] = seed * (i + 1) * 0.5;
#else
    for (int i = 0; i < 8; i++) vi->data[i] = seed * (i + 1);
    for (int i = 0; i < 4; i++) vf->data[i] = seed * (i + 1) * 0.5;
#endif
}

/* Vector reduction with accumulation */
static double vector_reduction(float64x4_t v, double accum)
{
#if HAS_VECTOR
    /* This may expand to multiple operations */
    return accum + v[0] + v[1] + v[2] + v[3];
#else
    return accum + v.data[0] + v.data[1] + v.data[2] + v.data[3];
#endif
}

int main(int argc, char *argv[])
{
    int seed = 1;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    /* Initialize base variables */
    d64_t d64_a, d64_b;
    d128_t d128_c, d128_d, d128_e;
    cl_t ca, cb, cc, cd;
    int32x8_t vec_i;
    float64x4_t vec_f;
    
    init_dfp(&d64_a, &d128_c, seed);
    init_dfp(&d64_b, &d128_d, seed + 1);
    init_dfp(&d64_b, &d128_e, seed + 2); /* Reuse d64_b for init */
    
    init_complex(&ca, seed);
    init_complex(&cb, seed + 1);
    init_complex(&cc, seed + 2);
    init_complex(&cd, seed + 3);
    
    init_vectors(&vec_i, &vec_f, seed);
    
    /* Storage for results to prevent elimination */
    d128_t results_d128[3];
    cl_t results_cl[3];
    double accum = 0.0;
    
    /* Main computation loop - 3 iterations */
    for (int iter = 0; iter < 3; iter++) {
        volatile int condition = seed + iter;
        
        /* Conditional execution to prevent constant folding */
        if (condition > 0) {
            /* DFP arithmetic that may expand to many operands */
#if HAS_DFP
            d128_t d128_result = d128_c * d128_d + d128_e / (d128_t)d64_a;
#else
            d128_t d128_result;
            d128_result.lo = d128_c.lo * d128_d.lo + d128_e.lo / d64_a.i;
            d128_result.hi = d128_c.hi * d128_d.hi;
#endif
            results_d128[iter] = d128_result;
            
            /* Complex arithmetic */
#ifdef __STDC_IEC_559_COMPLEX__
            cl_t cl_result = (ca * cb) / (cc - cd);
#else
            cl_t cl_result;
            double denom_real = cc.real - cd.real;
            double denom_imag = cc.imag - cd.imag;
            double denom_norm = denom_real * denom_real + denom_imag * denom_imag;
            cl_result.real = (ca.real * cb.real - ca.imag * cb.imag) / denom_norm;
            cl_result.imag = (ca.real * cb.imag + ca.imag * cb.real) / denom_norm;
#endif
            results_cl[iter] = cl_result;
            
            /* Vector reduction with accumulation */
            accum = vector_reduction(vec_f, accum);
            
            /* Call helper with 11 arguments */
            d128_t h11_result = helper_11_args(
                d64_a, d64_b, d128_c, d128_d,
                ca, cb, cc,
                vec_i, vec_f,
                (uint64_t)seed, (uint64_t)iter
            );
            
            /* Call helper with 10 arguments */
            cl_t h10_result = helper_10_args(
                ca, cb, cc, cd,
                d128_c, d128_d, d128_e,
                vec_f, vec_i,
                (uint64_t)iter
            );
            
            /* Aggregate results to volatile global */
#ifdef __STDC_IEC_559_COMPLEX__
            g_result += (uint64_t)creal(h10_result) + (uint64_t)cimag(h10_result);
#else
            g_result += (uint64_t)h10_result.real + (uint64_t)h10_result.imag;
#endif
            
#if HAS_DFP
            g_result += (uint64_t)(double)h11_result;
#else
            g_result += h11_result.lo + h11_result.hi;
#endif
        }
        
        /* Modify values slightly for next iteration */
#if HAS_DFP
        d64_a += 0.1DL;
        d128_c += 0.2DL;
#else
        d64_a.i += 1;
        d128_c.lo += 2;
#endif
        
#ifdef __STDC_IEC_559_COMPLEX__
        ca += 0.1L + I * 0.05L;
#else
        ca.real += 0.1L;
        ca.imag += 0.05L;
#endif
    }
    
    /* Compute final checksum */
    uint64_t checksum = g_result;
    
    /* Add contributions from stored results */
    for (int i = 0; i < 3; i++) {
#if HAS_DFP
        checksum += (uint64_t)(double)results_d128[i];
#else
        checksum += results_d128[i].lo + results_d128[i].hi;
#endif
        
#ifdef __STDC_IEC_559_COMPLEX__
        checksum += (uint64_t)creal(results_cl[i]) + (uint64_t)cimag(results_cl[i]);
#else
        checksum += (uint64_t)results_cl[i].real + (uint64_t)results_cl[i].imag;
#endif
    }
    
    checksum += (uint64_t)accum;
    
    printf("Checksum: %llu\n", (unsigned long long)checksum);
    return 0;
}
