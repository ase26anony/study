/*
 * This program is designed to stress GCC's RTL expander by creating
 * expressions that require 10-11 operands, specifically targeting the
 * uncovered lines 8254-8263 in optabs.cc.
 *
 * It uses Decimal Floating-Point (DFP), complex numbers, vector reductions,
 * and multi-argument functions to generate many operands during expansion.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Include complex.h if available */
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

/* ==================== DFP SECTION ==================== */
#if HAS_DFP
/* Use DFP types directly */
typedef _Decimal64  d64_t;
typedef _Decimal128 d128_t;

#else
/* Fallback: simulate DFP using integer arrays */
typedef struct { uint64_t lo, hi; } d128_t;
typedef uint64_t d64_t;

/* Simple emulation functions */
static d128_t d128_add(d128_t a, d128_t b) {
    d128_t r;
    r.lo = a.lo + b.lo;
    r.hi = a.hi + b.hi + (r.lo < a.lo);
    return r;
}

static d128_t d128_mul(d128_t a, d128_t b) {
    /* Simplified multiplication for demonstration */
    uint64_t a0 = a.lo & 0xFFFFFFFF;
    uint64_t a1 = a.lo >> 32;
    uint64_t b0 = b.lo & 0xFFFFFFFF;
    uint64_t b1 = b.lo >> 32;
    uint64_t p0 = a0 * b0;
    uint64_t p1 = a0 * b1 + a1 * b0;
    uint64_t p2 = a1 * b1;
    d128_t r;
    r.lo = p0 + (p1 << 32);
    r.hi = p2 + (p1 >> 32) + (r.lo < p0);
    return r;
}
#endif

/* ==================== COMPLEX SECTION ==================== */
#ifdef __STDC_IEC_559_COMPLEX__
typedef long double _Complex cl_t;
#else
/* Fallback: use struct */
typedef struct { long double re, im; } cl_t;
#endif

/* Complex multiplication */
static cl_t complex_mul(cl_t a, cl_t b) {
#ifdef __STDC_IEC_559_COMPLEX__
    return a * b;
#else
    cl_t r;
    r.re = a.re * b.re - a.im * b.im;
    r.im = a.re * b.im + a.im * b.re;
    return r;
#endif
}

/* Complex division */
static cl_t complex_div(cl_t a, cl_t b) {
#ifdef __STDC_IEC_559_COMPLEX__
    return a / b;
#else
    long double denom = b.re * b.re + b.im * b.im;
    cl_t r;
    r.re = (a.re * b.re + a.im * b.im) / denom;
    r.im = (a.im * b.re - a.re * b.im) / denom;
    return r;
#endif
}

/* ==================== VECTOR SECTION ==================== */
#if HAS_VECTOR
typedef int32_t int32x8_t __attribute__((vector_size(32)));
typedef double  float64x4_t __attribute__((vector_size(32)));
#else
/* Fallback: use arrays */
typedef struct { int32_t v[8]; } int32x8_t;
typedef struct { double v[4]; } float64x4_t;
#endif

/* Vector reduction sum */
static int32_t vector_sum_int(int32x8_t v) {
#if HAS_VECTOR
    return v[0] + v[1] + v[2] + v[3] + v[4] + v[5] + v[6] + v[7];
#else
    int32_t sum = 0;
    for (int i = 0; i < 8; i++) sum += v.v[i];
    return sum;
#endif
}

static double vector_sum_double(float64x4_t v) {
#if HAS_VECTOR
    return v[0] + v[1] + v[2] + v[3];
#else
    double sum = 0;
    for (int i = 0; i < 4; i++) sum += v.v[i];
    return sum;
#endif
}

/* ==================== MULTI-ARGUMENT HELPERS ==================== */
/* Helper with exactly 10 arguments - marked noinline to prevent simplification */
static d128_t __attribute__((noinline))
helper_10_args(d128_t a1, d128_t a2, d128_t a3, d128_t a4, d128_t a5,
               cl_t c1, cl_t c2, int32_t vi, double vd, int extra) {
#if HAS_DFP
    /* Use DFP builtins if available */
    d128_t dfp_sum = __builtin_dadd(a1, __builtin_dadd(a2, __builtin_dadd(a3, a4)));
    dfp_sum = __builtin_dadd(dfp_sum, a5);
#else
    d128_t dfp_sum = d128_add(a1, d128_add(a2, d128_add(a3, a4)));
    dfp_sum = d128_add(dfp_sum, a5);
#endif
    
    /* Convert complex to "decimal-like" representation */
    long double creal, cimag;
#ifdef __STDC_IEC_559_COMPLEX__
    creal = __real__(c1) + __real__(c2);
    cimag = __imag__(c1) + __imag__(c2);
#else
    creal = c1.re + c2.re;
    cimag = c1.im + c2.im;
#endif
    
    /* Combine everything */
#if HAS_DFP
    d128_t result = __builtin_dadd(dfp_sum, 
        __builtin_dadd(__builtin_dadd((d128_t){creal}, (d128_t){cimag}),
                      __builtin_dadd((d128_t){vi}, (d128_t){vd})));
#else
    d128_t temp = d128_add((d128_t){.lo = (uint64_t)creal, .hi = 0},
                          (d128_t){.lo = (uint64_t)cimag, .hi = 0});
    temp = d128_add(temp, (d128_t){.lo = vi, .hi = 0});
    temp = d128_add(temp, (d128_t){.lo = (uint64_t)vd, .hi = 0});
    d128_t result = d128_add(dfp_sum, temp);
#endif
    
    return result;
}

/* Helper with 11 arguments */
static d128_t __attribute__((noinline))
helper_11_args(d128_t a1, d128_t a2, d128_t a3, d128_t a4, d128_t a5, d128_t a6,
               cl_t c1, cl_t c2, cl_t c3, int32_t vi, double vd) {
    /* Similar to helper_10_args but with one more DFP and one more complex arg */
#if HAS_DFP
    d128_t dfp_sum = __builtin_dadd(a1, __builtin_dadd(a2, __builtin_dadd(a3, a4)));
    dfp_sum = __builtin_dadd(dfp_sum, __builtin_dadd(a5, a6));
#else
    d128_t dfp_sum = d128_add(a1, d128_add(a2, d128_add(a3, a4)));
    dfp_sum = d128_add(dfp_sum, d128_add(a5, a6));
#endif
    
    long double creal = 0, cimag = 0;
#ifdef __STDC_IEC_559_COMPLEX__
    creal = __real__(c1) + __real__(c2) + __real__(c3);
    cimag = __imag__(c1) + __imag__(c2) + __imag__(c3);
#else
    creal = c1.re + c2.re + c3.re;
    cimag = c1.im + c2.im + c3.im;
#endif
    
#if HAS_DFP
    d128_t result = __builtin_dadd(dfp_sum,
        __builtin_dadd(__builtin_dadd((d128_t){creal}, (d128_t){cimag}),
                      __builtin_dadd((d128_t){vi}, (d128_t){vd})));
#else
    d128_t temp = d128_add((d128_t){.lo = (uint64_t)creal, .hi = 0},
                          (d128_t){.lo = (uint64_t)cimag, .hi = 0});
    temp = d128_add(temp, (d128_t){.lo = vi, .hi = 0});
    temp = d128_add(temp, (d128_t){.lo = (uint64_t)vd, .hi = 0});
    d128_t result = d128_add(dfp_sum, temp);
#endif
    
    return result;
}

/* ==================== MAIN EXECUTION ==================== */
/* Volatile array to prevent dead code elimination */
static volatile uint64_t result_store[10];

int main(int argc, char *argv[]) {
    /* Use command-line seed for deterministic behavior */
    int seed = (argc > 1) ? atoi(argv[1]) : 42;
    srand(seed);
    
    /* Initialize DFP values */
    d128_t d128_a, d128_b, d128_c, d128_d, d128_e, d128_f;
#if HAS_DFP
    d128_a = 1.2345678901234567890123456789DL;
    d128_b = 9.8765432109876543210987654321DL;
    d128_c = 3.1415926535897932384626433833DL;
    d128_d = 2.7182818284590452353602874714DL;
    d128_e = 1.4142135623730950488016887242DL;
    d128_f = 1.6180339887498948482045868344DL;
#else
    d128_a = (d128_t){.lo = 0x123456789ABCDEF0, .hi = 0x0};
    d128_b = (d128_t){.lo = 0xFEDCBA9876543210, .hi = 0x1};
    d128_c = (d128_t){.lo = 0x3141592653589793, .hi = 0x2};
    d128_d = (d128_t){.lo = 0x2718281828459045, .hi = 0x3};
    d128_e = (d128_t){.lo = 0x1414213562373095, .hi = 0x4};
    d128_f = (d128_t){.lo = 0x1618033988749894, .hi = 0x5};
#endif
    
    /* Initialize complex values */
    cl_t ca, cb, cc, cd, ce;
#ifdef __STDC_IEC_559_COMPLEX__
    ca = 1.0 + 2.0i;
    cb = 3.0 - 4.0i;
    cc = -2.0 + 1.5i;
    cd = 0.5 - 3.0i;
    ce = 2.5 + 2.5i;
#else
    ca = (cl_t){.re = 1.0, .im = 2.0};
    cb = (cl_t){.re = 3.0, .im = -4.0};
    cc = (cl_t){.re = -2.0, .im = 1.5};
    cd = (cl_t){.re = 0.5, .im = -3.0};
    ce = (cl_t){.re = 2.5, .im = 2.5};
#endif
    
    /* Initialize vectors */
    int32x8_t vi;
    float64x4_t vd;
#if HAS_VECTOR
    vi = (int32x8_t){1, 2, 3, 4, 5, 6, 7, 8};
    vd = (float64x4_t){1.1, 2.2, 3.3, 4.4};
#else
    for (int i = 0; i < 8; i++) vi.v[i] = i + 1;
    for (int i = 0; i < 4; i++) vd.v[i] = (i + 1) * 1.1;
#endif
    
    /* Volatile condition to prevent constant folding */
    volatile int condition = seed;
    
    /* Main computation loop */
    for (int iter = 0; iter < 3; iter++) {
        d128_t d128_result;
        cl_t cl_result;
        int32_t vi_sum;
        double vd_sum;
        
        /* Conditional execution to prevent optimization */
        if (condition & (1 << iter)) {
            /* DFP arithmetic - may expand to multi-operand RTL */
#if HAS_DFP
            d128_result = __builtin_dadd(
                __builtin_dmul(d128_a, d128_b),
                __builtin_ddiv(d128_c, d128_d)
            );
#else
            d128_result = d128_add(d128_mul(d128_a, d128_b),
                                  d128_mul(d128_c, d128_d)); /* simplified */
#endif
            
            /* Complex arithmetic */
            cl_result = complex_div(
                complex_mul(ca, cb),
                complex_div(cc, cd)  /* Using division instead of subtraction
                                      to increase operation complexity */
            );
        } else {
            /* Alternative path with different operations */
#if HAS_DFP
            d128_result = __builtin_dadd(
                __builtin_dadd(__builtin_dmul(d128_e, d128_f),
                              __builtin_ddiv(d128_a, d128_b)),
                __builtin_dmul(d128_c, d128_d)
            );
#else
            d128_t t1 = d128_mul(d128_e, d128_f);
            d128_t t2 = d128_mul(d128_a, d128_b); /* simplified division */
            d128_t t3 = d128_mul(d128_c, d128_d);
            d128_result = d128_add(d128_add(t1, t2), t3);
#endif
            
            cl_result = complex_mul(
                complex_div(ca, ce),
                complex_mul(cb, cc)
            );
        }
        
        /* Vector reductions */
        vi_sum = vector_sum_int(vi);
        vd_sum = vector_sum_double(vd);
        
        /* Call helper functions with many arguments */
        d128_t h10_result = helper_10_args(
            d128_a, d128_b, d128_c, d128_d, d128_result,
            cl_result, ca, vi_sum, vd_sum, iter
        );
        
        d128_t h11_result = helper_11_args(
            d128_a, d128_b, d128_c, d128_d, d128_e, d128_f,
            ca, cb, cc, vi_sum, vd_sum
        );
        
        /* Store results to volatile memory */
#if HAS_DFP
        /* Convert DFP to integer for storage */
        uint64_t val1 = (uint64_t)((long double)h10_result);
        uint64_t val2 = (uint64_t)((long double)h11_result);
#else
        uint64_t val1 = h10_result.lo ^ h10_result.hi;
        uint64_t val2 = h11_result.lo ^ h11_result.hi;
#endif
        
        result_store[iter * 2] = val1;
        result_store[iter * 2 + 1] = val2;
        
        /* Modify inputs slightly for next iteration */
#if HAS_DFP
        d128_a = __builtin_dadd(d128_a, 1.0DL);
        d128_b = __builtin_dadd(d128_b, 2.0DL);
#else
        d128_a.lo += 1;
        d128_b.lo += 2;
#endif
        
#ifdef __STDC_IEC_559_COMPLEX__
        ca += 0.5 + 0.5i;
        cb += 0.3 - 0.3i;
#else
        ca.re += 0.5; ca.im += 0.5;
        cb.re += 0.3; cb.im -= 0.3;
#endif
        
#if HAS_VECTOR
        vi += 1;
        vd += 0.1;
#else
        for (int i = 0; i < 8; i++) vi.v[i] += 1;
        for (int i = 0; i < 4; i++) vd.v[i] += 0.1;
#endif
    }
    
    /* Compute checksum */
    uint64_t checksum = 0;
    for (int i = 0; i < 6; i++) {
        checksum += result_store[i];
    }
    
    printf("Checksum: %llu\n", (unsigned long long)checksum);
    return 0;
}
