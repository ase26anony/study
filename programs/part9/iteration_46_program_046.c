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
#if defined(__GNUC__) && (defined(__x86_64__) || defined(__i386__) || defined(__aarch64__))
    #define VECTOR_SUPPORTED 1
    typedef int32_t int32x8_t __attribute__((vector_size(32)));
    typedef float float64x4_t __attribute__((vector_size(32)));
#else
    #define VECTOR_SUPPORTED 0
    typedef struct { int32_t data[8]; } int32x8_t;
    typedef struct { double data[4]; } float64x4_t;
#endif

/* DFP fallback using integer representation */
#if !DFP_SUPPORTED
typedef union {
    uint64_t u64;
    struct {
        uint64_t mantissa : 52;
        uint64_t exponent : 11;
        uint64_t sign : 1;
    } parts;
} decimal64_fallback;

typedef union {
    struct {
        uint64_t low;
        uint64_t high;
    } words;
    struct {
        uint64_t mantissa_low : 64;
        uint64_t mantissa_high : 49;
        uint64_t exponent : 15;
        uint64_t sign : 1;
    } parts;
} decimal128_fallback;
#endif

/* Helper function with 11 arguments - marked noinline to prevent simplification */
static __attribute__((noinline)) 
long double helper_11_args(
    #if DFP_SUPPORTED
    _Decimal64 d1, _Decimal64 d2, _Decimal128 d3, _Decimal128 d4,
    #else
    decimal64_fallback d1, decimal64_fallback d2, 
    decimal128_fallback d3, decimal128_fallback d4,
    #endif
    #if COMPLEX_SUPPORTED
    long double _Complex c1, long double _Complex c2,
    #else
    struct { long double re, im; } c1, struct { long double re, im; } c2,
    #endif
    #if VECTOR_SUPPORTED
    int32x8_t v1, float64x4_t v2,
    #else
    int32x8_t v1, float64x4_t v2,
    #endif
    int extra1, int extra2, int extra3)
{
    long double result = 0.0L;
    
    /* Combine all arguments in a way that uses all of them */
    #if DFP_SUPPORTED
    /* DFP operations */
    _Decimal64 d64_sum = d1 + d2;
    _Decimal128 d128_sum = d3 + d4;
    result += (long double)d64_sum + (long double)d128_sum;
    #else
    /* Fallback: treat as integers */
    result += (long double)d1.u64 + (long double)d2.u64;
    result += (long double)d3.words.low + (long double)d3.words.high;
    result += (long double)d4.words.low + (long double)d4.words.high;
    #endif
    
    #if COMPLEX_SUPPORTED
    /* Complex operations */
    long double _Complex cprod = c1 * c2;
    result += creal(cprod) + cimag(cprod);
    #else
    /* Complex fallback */
    result += c1.re * c2.re - c1.im * c2.im;  /* real part of product */
    result += c1.re * c2.im + c1.im * c2.re;  /* imag part of product */
    #endif
    
    #if VECTOR_SUPPORTED
    /* Vector operations */
    for (int i = 0; i < 8; i++) result += v1[i];
    for (int i = 0; i < 4; i++) result += v2[i];
    #else
    for (int i = 0; i < 8; i++) result += v1.data[i];
    for (int i = 0; i < 4; i++) result += v2.data[i];
    #endif
    
    result += extra1 + extra2 + extra3;
    return result;
}

/* Another helper with 10 arguments */
static __attribute__((noinline))
long double helper_10_args(
    #if DFP_SUPPORTED
    _Decimal64 d1, _Decimal64 d2, _Decimal64 d3, _Decimal128 d4,
    #else
    decimal64_fallback d1, decimal64_fallback d2, decimal64_fallback d3,
    decimal128_fallback d4,
    #endif
    #if COMPLEX_SUPPORTED
    long double _Complex c1, long double _Complex c2, long double _Complex c3,
    #else
    struct { long double re, im; } c1, struct { long double re, im; } c2,
    struct { long double re, im; } c3,
    #endif
    int extra1, int extra2)
{
    long double result = 0.0L;
    
    #if DFP_SUPPORTED
    /* Complex DFP expression that may expand to many operands */
    _Decimal64 d64_result = d1 * d2 + d3;
    result += (long double)d64_result + (long double)d4;
    #else
    result += (long double)d1.u64 * (long double)d2.u64 + (long double)d3.u64;
    result += (long double)d4.words.low + (long double)d4.words.high;
    #endif
    
    #if COMPLEX_SUPPORTED
    /* Complex expression with multiple operations */
    long double _Complex c_result = (c1 * c2) / (c3 + 1.0L);
    result += creal(c_result) * cimag(c_result);
    #else
    /* Manual complex arithmetic */
    long double denom_re = c3.re + 1.0L;
    long double denom_im = c3.im;
    long double denom_sq = denom_re * denom_re + denom_im * denom_im;
    
    long double prod_re = c1.re * c2.re - c1.im * c2.im;
    long double prod_im = c1.re * c2.im + c1.im * c2.re;
    
    long double res_re = (prod_re * denom_re + prod_im * denom_im) / denom_sq;
    long double res_im = (prod_im * denom_re - prod_re * denom_im) / denom_sq;
    
    result += res_re * res_im;
    #endif
    
    result += extra1 * extra2;
    return result;
}

/* Vector reduction with accumulation */
static long double vector_reduction_accumulate(
    #if VECTOR_SUPPORTED
    int32x8_t vi, float64x4_t vf,
    #else
    int32x8_t vi, float64x4_t vf,
    #endif
    long double accumulator)
{
    long double result = accumulator;
    
    #if VECTOR_SUPPORTED
    /* Horizontal reduction - may expand to many operations */
    for (int i = 0; i < 8; i++) {
        result += vi[i];
    }
    for (int i = 0; i < 4; i++) {
        result *= 1.0 + vf[i];
    }
    #else
    for (int i = 0; i < 8; i++) {
        result += vi.data[i];
    }
    for (int i = 0; i < 4; i++) {
        result *= 1.0 + vf.data[i];
    }
    #endif
    
    return result;
}

int main(int argc, char *argv[]) {
    /* Use command line seed for deterministic behavior */
    int seed = 42;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    srand(seed);
    
    /* Prevent optimization */
    volatile long double checksum = 0.0L;
    long double results[5];
    
    /* Initialize base variables */
    #if DFP_SUPPORTED
    _Decimal64 d64_a = 1.23456789e10DL;
    _Decimal64 d64_b = 9.87654321e9DL;
    _Decimal64 d64_c = 5.55555555e8DL;
    _Decimal128 d128_a = 1.2345678901234567890123456789e20DL;
    _Decimal128 d128_b = 9.8765432109876543210987654321e19DL;
    #else
    decimal64_fallback d64_a = { .u64 = 0x123456789ABCDEF0ULL };
    decimal64_fallback d64_b = { .u64 = 0xFEDCBA9876543210ULL };
    decimal64_fallback d64_c = { .u64 = 0x5555555555555555ULL };
    decimal128_fallback d128_a = { .words = { 0x123456789ABCDEF0ULL, 0x0FEDCBA987654321ULL } };
    decimal128_fallback d128_b = { .words = { 0xFEDCBA9876543210ULL, 0x0123456789ABCDEFULL } };
    #endif
    
    #if COMPLEX_SUPPORTED
    long double _Complex ca = 1.0L + 2.0LI;
    long double _Complex cb = 3.0L - 4.0LI;
    long double _Complex cc = -2.0L + 1.5LI;
    long double _Complex cd = 0.5L - 3.0LI;
    #else
    struct { long double re, im; } ca = { 1.0L, 2.0L };
    struct { long double re, im; } cb = { 3.0L, -4.0L };
    struct { long double re, im; } cc = { -2.0L, 1.5L };
    struct { long double re, im; } cd = { 0.5L, -3.0L };
    #endif
    
    #if VECTOR_SUPPORTED
    int32x8_t vi = { 1, 2, 3, 4, 5, 6, 7, 8 };
    float64x4_t vf = { 1.1, 2.2, 3.3, 4.4 };
    #else
    int32x8_t vi = { .data = { 1, 2, 3, 4, 5, 6, 7, 8 } };
    float64x4_t vf = { .data = { 1.1, 2.2, 3.3, 4.4 } };
    #endif
    
    /* Main computation loop - 3 iterations */
    for (int iter = 0; iter < 3; iter++) {
        long double iter_result = 0.0L;
        
        /* Conditional execution based on volatile variable */
        volatile int condition = iter % 2;
        
        if (condition) {
            /* DFP arithmetic with multiple operations */
            #if DFP_SUPPORTED
            _Decimal64 d64_result = d64_a * d64_b + d64_c;
            _Decimal128 d128_result = d128_a * d128_b / (_Decimal128)d64_a;
            iter_result += (long double)d64_result + (long double)d128_result;
            #else
            iter_result += (long double)d64_a.u64 * (long double)d64_b.u64 + (long double)d64_c.u64;
            iter_result += (long double)d128_a.words.low * (long double)d128_b.words.low / 
                          (long double)d64_a.u64;
            #endif
        } else {
            /* Complex arithmetic */
            #if COMPLEX_SUPPORTED
            long double _Complex c_result = (ca * cb) / (cc - cd);
            iter_result += creal(c_result) + cimag(c_result);
            
            /* Complex power - may expand to many operations */
            c_result = cpow(ca, cb);
            iter_result += creal(c_result) * cimag(c_result);
            #else
            /* Manual complex operations */
            long double diff_re = cc.re - cd.re;
            long double diff_im = cc.im - cd.im;
            long double denom_sq = diff_re * diff_re + diff_im * diff_im;
            
            long double prod_re = ca.re * cb.re - ca.im * cb.im;
            long double prod_im = ca.re * cb.im + ca.im * cb.re;
            
            long double res_re = (prod_re * diff_re + prod_im * diff_im) / denom_sq;
            long double res_im = (prod_im * diff_re - prod_re * diff_im) / denom_sq;
            
            iter_result += res_re + res_im;
            #endif
        }
        
        /* Vector reduction with accumulation */
        iter_result = vector_reduction_accumulate(vi, vf, iter_result);
        
        /* Call helper with 11 arguments */
        long double helper1_result = helper_11_args(
            #if DFP_SUPPORTED
            d64_a, d64_b, d128_a, d128_b,
            #else
            d64_a, d64_b, d128_a, d128_b,
            #endif
            #if COMPLEX_SUPPORTED
            ca, cb,
            #else
            ca, cb,
            #endif
            vi, vf,
            iter, seed, iter * 2);
        
        /* Call helper with 10 arguments */
        long double helper2_result = helper_10_args(
            #if DFP_SUPPORTED
            d64_a, d64_b, d64_c, d128_a,
            #else
            d64_a, d64_b, d64_c, d128_a,
            #endif
            #if COMPLEX_SUPPORTED
            ca, cb, cc,
            #else
            ca, cb, cc,
            #endif
            iter, seed);
        
        /* Combine results */
        results[iter] = iter_result + helper1_result + helper2_result;
        checksum += results[iter];
        
        /* Modify inputs slightly for next iteration */
        #if DFP_SUPPORTED
        d64_a += 1.0e5DL;
        d128_b *= 1.1DL;
        #else
        d64_a.u64 += 1000;
        d128_b.words.low += 2000;
        #endif
        
        #if COMPLEX_SUPPORTED
        ca += 0.1L + 0.2LI;
        #else
        ca.re += 0.1L;
        ca.im += 0.2L;
        #endif
        
        #if VECTOR_SUPPORTED
        for (int i = 0; i < 8; i++) vi[i] += 1;
        for (int i = 0; i < 4; i++) vf[i] *= 1.01;
        #else
        for (int i = 0; i < 8; i++) vi.data[i] += 1;
        for (int i = 0; i < 4; i++) vf.data[i] *= 1.01;
        #endif
    }
    
    /* Final checksum computation */
    long double final_checksum = 0.0L;
    for (int i = 0; i < 3; i++) {
        final_checksum += results[i];
    }
    
    printf("Checksum: %Lf\n", final_checksum);
    printf("Volatile checksum: %Lf\n", (long double)checksum);
    
    return 0;
}
