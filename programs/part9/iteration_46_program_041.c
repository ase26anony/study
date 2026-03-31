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

/* DFP fallback using integer arrays */
#if !HAS_DFP
typedef struct {
    uint64_t lo;
    uint64_t hi;
} decimal64_fb;

typedef struct {
    uint64_t w[2];
} decimal128_fb;

static decimal64_fb dfp64_add(decimal64_fb a, decimal64_fb b) {
    decimal64_fb r;
    r.lo = a.lo + b.lo;
    r.hi = a.hi + b.hi + (r.lo < a.lo);
    return r;
}

static decimal128_fb dfp128_mul(decimal128_fb a, decimal128_fb b) {
    decimal128_fb r;
    /* Simplified multiplication for demonstration */
    r.w[0] = a.w[0] * b.w[0];
    r.w[1] = a.w[1] * b.w[1];
    return r;
}
#endif

/* Complex fallback */
#if !HAS_COMPLEX
typedef struct {
    long double re;
    long double im;
} complex_ld_fb;

static complex_ld_fb complex_mul(complex_ld_fb a, complex_ld_fb b) {
    complex_ld_fb r;
    r.re = a.re * b.re - a.im * b.im;
    r.im = a.re * b.im + a.im * b.re;
    return r;
}
#endif

/* Helper function with 11 arguments - marked noinline to prevent simplification */
static __attribute__((noinline)) long double
helper_11_args(
#if HAS_DFP
    _Decimal64 d1, _Decimal64 d2, _Decimal128 d3,
#else
    decimal64_fb d1, decimal64_fb d2, decimal128_fb d3,
#endif
#if HAS_COMPLEX
    long double _Complex c1, long double _Complex c2,
#else
    complex_ld_fb c1, complex_ld_fb c2,
#endif
#if HAS_VECTORS
    v8si v1, v4df v2,
#else
    int32_t v1[8], double v2[4],
#endif
    int i1, int i2, long double ld1)
{
    long double result = 0.0L;
    
    /* Combine all arguments in a way that uses many operands */
#if HAS_DFP
    /* DFP operations that may expand to multi-operand RTL */
    result += (long double)d1 + (long double)d2;
    /* Access d3 components separately to increase operand count */
    {
        _Decimal64 d3_low = (_Decimal64)d3;
        _Decimal64 d3_high = (_Decimal64)(d3 >> 64);
        result += (long double)d3_low * 0.5L + (long double)d3_high * 0.25L;
    }
#else
    result += (long double)d1.lo + (long double)d1.hi * 0.5L;
    result += (long double)d2.lo * 0.25L + (long double)d2.hi * 0.125L;
    result += (long double)d3.w[0] * 0.0625L + (long double)d3.w[1] * 0.03125L;
#endif

#if HAS_COMPLEX
    /* Complex operations */
    result += creall(c1) * 2.0L - cimagl(c1);
    result += creall(c2) * 3.0L + cimagl(c2) * 4.0L;
#else
    result += c1.re * 2.0L - c1.im;
    result += c2.re * 3.0L + c2.im * 4.0L;
#endif

#if HAS_VECTORS
    /* Vector reduction */
    v8si v1_sum = v1[0] + v1[1] + v1[2] + v1[3] + v1[4] + v1[5] + v1[6] + v1[7];
    v4df v2_sum = v2[0] + v2[1] + v2[2] + v2[3];
    result += (long double)v1_sum + (long double)v2_sum;
#else
    for (int i = 0; i < 8; i++) result += (long double)v1[i];
    for (int i = 0; i < 4; i++) result += (long double)v2[i];
#endif

    result += (long double)i1 * 5.0L + (long double)i2 * 6.0L + ld1 * 7.0L;
    
    return result;
}

/* Another helper with 10 arguments */
static __attribute__((noinline)) long double
helper_10_args(
    long double a1, long double a2, long double a3,
    long double a4, long double a5, long double a6,
    long double a7, long double a8, long double a9,
    long double a10)
{
    /* Complex expression that may expand to many operands */
    return ((a1 * a2) + (a3 * a4) - (a5 * a6)) / 
           ((a7 + a8) * (a9 - a10) + 1.0L);
}

/* Volatile storage to prevent optimization */
volatile long double g_result_store[10];
static int store_idx = 0;

int main(int argc, char *argv[]) {
    int seed = 12345;
    if (argc > 1) seed = atoi(argv[1]);
    
    /* Initialize deterministic values */
    srand(seed);
    
    /* Base variables */
#if HAS_DFP
    _Decimal64 d64_a = 1.234567dd;
    _Decimal64 d64_b = 9.876543dd;
    _Decimal64 d64_c = 5.555555dd;
    _Decimal64 d64_d = 3.333333dd;
    _Decimal128 d128_a = 1.2345678901234567dl;
    _Decimal128 d128_b = 9.8765432109876543dl;
#else
    decimal64_fb d64_a = {1234567ULL, 0ULL};
    decimal64_fb d64_b = {9876543ULL, 0ULL};
    decimal64_fb d64_c = {5555555ULL, 0ULL};
    decimal64_fb d64_d = {3333333ULL, 0ULL};
    decimal128_fb d128_a = {{12345678901234567ULL, 0ULL}};
    decimal128_fb d128_b = {{98765432109876543ULL, 0ULL}};
#endif

#if HAS_COMPLEX
    long double _Complex ca = 1.0L + 2.0LI;
    long double _Complex cb = 3.0L - 4.0LI;
    long double _Complex cc = 5.0L + 6.0LI;
    long double _Complex cd = 7.0L - 8.0LI;
#else
    complex_ld_fb ca = {1.0L, 2.0L};
    complex_ld_fb cb = {3.0L, -4.0L};
    complex_ld_fb cc = {5.0L, 6.0L};
    complex_ld_fb cd = {7.0L, -8.0L};
#endif

#if HAS_VECTORS
    v8si vec_int = {1, 2, 3, 4, 5, 6, 7, 8};
    v4df vec_dbl = {1.1, 2.2, 3.3, 4.4};
#else
    int32_t vec_int[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    double vec_dbl[4] = {1.1, 2.2, 3.3, 4.4};
#endif

    volatile int condition = seed & 1; /* Prevent constant folding */
    
    /* Main computation loop */
    for (int iter = 0; iter < 3; iter++) {
        long double result = 0.0L;
        
        /* Conditional execution to prevent optimization */
        if (condition) {
            /* DFP arithmetic - may expand to multi-operand patterns */
#if HAS_DFP
            _Decimal128 d128_result;
            /* Complex DFP expression */
            d128_result = d128_a * d128_b + d128_a / d128_b - 
                         d128_b * d128_a + d128_b / d128_a;
            result += (long double)d128_result;
            
            /* More DFP operations */
            _Decimal64 d64_result = d64_a * d64_b + d64_c / d64_d;
            result += (long double)d64_result * 0.1L;
#else
            decimal128_fb d128_result = dfp128_mul(d128_a, d128_b);
            result += (long double)d128_result.w[0] + (long double)d128_result.w[1];
            
            decimal64_fb d64_result = dfp64_add(d64_a, d64_b);
            result += (long double)d64_result.lo + (long double)d64_result.hi;
#endif

            /* Complex number operations */
#if HAS_COMPLEX
            long double _Complex cl_result;
            cl_result = (ca * cb) / (cc - cd);
            result += creall(cl_result) + cimagl(cl_result);
            
            /* Complex power/sqrt operations */
            cl_result = csqrt(ca) + cpow(cb, cc);
            result += creall(cl_result) * 2.0L - cimagl(cl_result);
#else
            complex_ld_fb cl_result = complex_mul(ca, cb);
            result += cl_result.re + cl_result.im;
#endif
        } else {
            /* Alternative path with different operations */
#if HAS_DFP
            _Decimal128 d128_tmp = d128_a * d128_b * d128_a / d128_b;
            result += (long double)d128_tmp;
#else
            decimal128_fb d128_tmp = dfp128_mul(d128_a, d128_b);
            result += (long double)d128_tmp.w[0] * (long double)d128_tmp.w[1];
#endif
        }
        
        /* Vector reduction with accumulation */
#if HAS_VECTORS
        int32_t vec_sum = 0;
        /* Manual unrolled reduction to increase operand count */
        vec_sum += vec_int[0] + vec_int[1] + vec_int[2] + vec_int[3] +
                   vec_int[4] + vec_int[5] + vec_int[6] + vec_int[7];
        
        double dbl_sum = vec_dbl[0] + vec_dbl[1] + vec_dbl[2] + vec_dbl[3];
        
        result += (long double)vec_sum + (long double)dbl_sum;
        
        /* Modify vectors slightly */
        vec_int[iter % 8] += iter;
        vec_dbl[iter % 4] += (double)iter * 0.1;
#else
        int32_t vec_sum = 0;
        for (int i = 0; i < 8; i++) vec_sum += vec_int[i];
        double dbl_sum = 0.0;
        for (int i = 0; i < 4; i++) dbl_sum += vec_dbl[i];
        result += (long double)vec_sum + (long double)dbl_sum;
        
        vec_int[iter % 8] += iter;
        vec_dbl[iter % 4] += (double)iter * 0.1;
#endif
        
        /* Call helper with many arguments */
        long double helper_result;
        if (iter % 2 == 0) {
            helper_result = helper_11_args(
#if HAS_DFP
                d64_a, d64_b, d128_a,
#else
                d64_a, d64_b, d128_a,
#endif
#if HAS_COMPLEX
                ca, cb,
#else
                ca, cb,
#endif
#if HAS_VECTORS
                vec_int, vec_dbl,
#else
                vec_int, vec_dbl,
#endif
                seed + iter, iter * 2, result);
        } else {
            /* Create many intermediate values for 10-argument helper */
            long double t1 = result * 1.1L;
            long double t2 = result * 0.9L;
            long double t3 = result + 1.0L;
            long double t4 = result - 1.0L;
            long double t5 = result * 2.0L;
            long double t6 = result / 2.0L;
            long double t7 = result + 3.0L;
            long double t8 = result - 3.0L;
            long double t9 = result * 1.5L;
            long double t10 = result * 0.5L;
            
            helper_result = helper_10_args(t1, t2, t3, t4, t5, 
                                          t6, t7, t8, t9, t10);
        }
        
        /* Store result to prevent dead code elimination */
        g_result_store[store_idx++ % 10] = result + helper_result;
        
        /* Toggle condition */
        condition = !condition;
    }
    
    /* Compute checksum */
    long double checksum = 0.0L;
    for (int i = 0; i < 10 && i < store_idx; i++) {
        checksum += g_result_store[i];
    }
    
    printf("Result checksum: %Lf\n", checksum);
    return 0;
}
