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

#ifdef __VECTOR_TYPES_SUPPORTED__
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
    uint64_t parts[4];
} decimal128_fb;

static decimal64_fb dfp64_add_fb(decimal64_fb a, decimal64_fb b) {
    decimal64_fb r;
    r.lo = a.lo + b.lo;
    r.hi = a.hi + b.hi + (r.lo < a.lo);
    return r;
}

static decimal128_fb dfp128_add_fb(decimal128_fb a, decimal128_fb b) {
    decimal128_fb r;
    uint64_t carry = 0;
    for (int i = 0; i < 4; i++) {
        uint64_t sum = a.parts[i] + b.parts[i] + carry;
        carry = (sum < a.parts[i]) || (carry && sum == a.parts[i]);
        r.parts[i] = sum;
    }
    return r;
}
#endif

/* Complex fallback */
#if !HAS_COMPLEX
typedef struct {
    long double re;
    long double im;
} complex_ld_fb;
#endif

/* Vector fallback */
#if !HAS_VECTORS
typedef struct {
    int32_t data[8];
} int32x8_fb;

typedef struct {
    double data[4];
} float64x4_fb;
#endif

/* Helper functions with many arguments - marked noinline to prevent optimization */
static __attribute__((noinline)) long double helper_10_args(
    long double a1, long double a2, long double a3, long double a4, long double a5,
    long double a6, long double a7, long double a8, long double a9, long double a10) {
    /* Complex expression to prevent constant folding */
    volatile long double v = 1.0;
    return (a1 * a2 + a3 * a4 - a5 * a6) / (a7 + a8 - a9 * a10) + v;
}

static __attribute__((noinline)) long double helper_11_args(
    long double a1, long double a2, long double a3, long double a4, long double a5,
    long double a6, long double a7, long double a8, long double a9, long double a10,
    long double a11) {
    volatile long double v = 2.0;
    return ((a1 + a2) * (a3 - a4) + (a5 * a6) / (a7 + a8)) * (a9 - a10 + a11) + v;
}

#if HAS_DFP
static __attribute__((noinline)) _Decimal128 dfp_helper_10_args(
    _Decimal128 d1, _Decimal128 d2, _Decimal128 d3, _Decimal128 d4, _Decimal128 d5,
    _Decimal128 d6, _Decimal128 d7, _Decimal128 d8, _Decimal128 d9, _Decimal128 d10) {
    /* Complex DFP expression */
    volatile _Decimal128 v = 1.0DL;
    return (d1 * d2 + d3 * d4 - d5 * d6) / (d7 + d8 - d9 * d10) + v;
}
#endif

/* Vector reduction helper */
#if HAS_VECTORS
typedef int32_t v8si __attribute__((vector_size(32)));
typedef double v4df __attribute__((vector_size(32)));

static __attribute__((noinline)) int32_t vector_reduce_sum(v8si vec) {
    /* Horizontal sum that may expand to many operations */
    int32_t sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += vec[i];
    }
    return sum;
}

static __attribute__((noinline)) double vector_reduce_product(v4df vec) {
    /* Horizontal product */
    double prod = 1.0;
    for (int i = 0; i < 4; i++) {
        prod *= vec[i];
    }
    return prod;
}
#endif

int main(int argc, char *argv[]) {
    int seed = (argc > 1) ? atoi(argv[1]) : 42;
    srand(seed);
    
    volatile long double global_accumulator = 0.0L;
    volatile int checksum = 0;
    
    /* Initialize base values */
#if HAS_DFP
    _Decimal64 d64_a = 1.23456789DD;
    _Decimal64 d64_b = 9.87654321DD;
    _Decimal64 d64_c = 3.14159265DD;
    _Decimal64 d64_d = 2.71828182DD;
    
    _Decimal128 d128_a = 1.2345678901234567890123456789DL;
    _Decimal128 d128_b = 9.8765432109876543210987654321DL;
    _Decimal128 d128_c = 3.1415926535897932384626433833DL;
    _Decimal128 d128_d = 2.7182818284590452353602874714DL;
#else
    decimal64_fb d64_a = {123456789ULL, 0};
    decimal64_fb d64_b = {987654321ULL, 0};
    decimal64_fb d64_c = {314159265ULL, 0};
    decimal64_fb d64_d = {271828182ULL, 0};
    
    decimal128_fb d128_a = {{123456789ULL, 0, 0, 0}};
    decimal128_fb d128_b = {{987654321ULL, 0, 0, 0}};
    decimal128_fb d128_c = {{314159265ULL, 0, 0, 0}};
    decimal128_fb d128_d = {{271828182ULL, 0, 0, 0}};
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
    v4df vec_double = {1.1, 2.2, 3.3, 4.4};
#endif

    /* Main computation loop */
    for (int iteration = 0; iteration < 3; iteration++) {
        volatile int condition = iteration + seed;
        long double result = 0.0L;
        
        /* Conditional execution to prevent constant folding */
        if (condition % 2 == 0) {
            /* DFP arithmetic - may expand to many operands */
#if HAS_DFP
            _Decimal128 d128_result = d128_a * d128_b + d128_c / d128_d;
            d128_result = d128_result - d128_a + d128_b * d128_c;
            
            /* Call helper with many DFP arguments */
            _Decimal128 dfp_helper_result = dfp_helper_10_args(
                d128_result, d128_a, d128_b, d128_c, d128_d,
                d128_result * d128_a, d128_result / d128_b,
                d128_c + d128_d, d128_a - d128_c, d128_b * d128_d);
            
            result += (long double)dfp_helper_result;
#else
            decimal128_fb d128_result = dfp128_add_fb(d128_a, d128_b);
            result += (long double)d128_result.parts[0];
#endif
        } else {
            /* Complex number operations */
#if HAS_COMPLEX
            long double _Complex cl_result = (ca * cb) / (cc - cd);
            cl_result = cl_result + ca * cd - cb / cc;
            
            /* Complex library function that may expand */
            long double _Complex cl_sqrt = csqrt(cl_result);
            long double _Complex cl_pow = cpow(cl_result, 2.0L + 0.0LI);
            
            result += creal(cl_sqrt) + cimag(cl_pow);
#else
            complex_ld_fb cl_result;
            cl_result.re = ca.re * cb.re - ca.im * cb.im;
            cl_result.im = ca.re * cb.im + ca.im * cb.re;
            result += cl_result.re + cl_result.im;
#endif
        }
        
        /* Vector reduction */
#if HAS_VECTORS
        int32_t vec_sum = vector_reduce_sum(vec_int);
        double vec_prod = vector_reduce_product(vec_double);
        
        result += vec_sum + vec_prod;
        
        /* Update vectors for next iteration */
        for (int i = 0; i < 8; i++) vec_int[i] += iteration;
        for (int i = 0; i < 4; i++) vec_double[i] *= 1.1;
#endif
        
        /* Call helper functions with many arguments */
        if (condition % 3 == 0) {
            result += helper_10_args(
                result, result * 2.0L, result / 3.0L,
                result + 1.0L, result - 2.0L,
                result * result, sqrtl(result + 1.0L),
                fabsl(result - 5.0L), expl(result * 0.1L),
                logl(result + 10.0L));
        } else {
            result += helper_11_args(
                result, result * 1.5L, result / 2.5L,
                result + 2.0L, result - 3.0L,
                result * 0.5L, sinl(result),
                cosl(result), tanl(result * 0.01L),
                asinl(result * 0.1L), acosl(result * 0.1L));
        }
        
        /* Store result to prevent elimination */
        global_accumulator += result;
        checksum += (int)(result * 1000.0L);
        
        /* Update base values for next iteration */
#if HAS_DFP
        d128_a = d128_a * 1.1DL;
        d128_b = d128_b / 1.1DL;
        d128_c = d128_c + 0.1DL;
        d128_d = d128_d - 0.1DL;
#endif
        
#if HAS_COMPLEX
        ca = ca * (1.0L + 0.1LI * iteration);
        cb = cb / (1.0L - 0.1LI * iteration);
#endif
    }
    
    printf("Checksum: %d\n", checksum);
    printf("Final accumulator: %Lf\n", (long double)global_accumulator);
    
    return 0;
}
