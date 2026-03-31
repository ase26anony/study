/* 
 * Test program designed to trigger 10-11 operand internal function expansion
 * in GCC's optabs.cc, specifically targeting decimal floating-point (DFP),
 * complex numbers, and vector operations.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Include complex.h if available */
#ifdef __STDC_IEC_559_COMPLEX__
#include <complex.h>
#endif

/* Feature detection and fallback definitions */
#if defined(__DECIMAL_BID_FORMAT__) || defined(__DECIMAL_DPD_FORMAT__)
#define HAS_DFP 1
#else
#define HAS_DFP 0
#endif

/* GCC vector extension support check */
#ifdef __GNUC__
#define HAS_VECTOR_EXT 1
#else
#define HAS_VECTOR_EXT 0
#endif

/* Complex type support check */
#ifdef __STDC_IEC_559_COMPLEX__
#define HAS_COMPLEX 1
#else
#define HAS_COMPLEX 0
#endif

/* ==================== DFP Types and Operations ==================== */
#if HAS_DFP
/* Use native DFP types */
typedef _Decimal64  dfp64_t;
typedef _Decimal128 dfp128_t;

/* DFP arithmetic using builtins */
static dfp128_t dfp_add(dfp128_t a, dfp128_t b) {
    return __builtin_dadd(a, b);
}

static dfp128_t dfp_mul(dfp128_t a, dfp128_t b) {
    return __builtin_dmul(a, b);
}

static dfp128_t dfp_div(dfp128_t a, dfp128_t b) {
    return __builtin_ddiv(a, b);
}

static dfp128_t dfp_sub(dfp128_t a, dfp128_t b) {
    return __builtin_dsub(a, b);
}

#else
/* Fallback: software DFP emulation using integer arrays */
typedef struct {
    uint64_t lo;
    uint64_t hi;
} dfp128_t;

typedef uint64_t dfp64_t;

/* Simple emulation functions */
static dfp128_t dfp_add(dfp128_t a, dfp128_t b) {
    dfp128_t r;
    r.lo = a.lo + b.lo;
    r.hi = a.hi + b.hi + (r.lo < a.lo);
    return r;
}

static dfp128_t dfp_mul(dfp128_t a, dfp128_t b) {
    /* Simplified multiplication for demonstration */
    dfp128_t r;
    r.lo = a.lo * b.lo;
    r.hi = a.hi * b.hi;
    return r;
}

static dfp128_t dfp_div(dfp128_t a, dfp128_t b) {
    dfp128_t r;
    if (b.lo != 0 || b.hi != 0) {
        r.lo = a.lo / (b.lo ? b.lo : 1);
        r.hi = a.hi / (b.hi ? b.hi : 1);
    } else {
        r.lo = 0;
        r.hi = 0;
    }
    return r;
}

static dfp128_t dfp_sub(dfp128_t a, dfp128_t b) {
    dfp128_t r;
    r.lo = a.lo - b.lo;
    r.hi = a.hi - b.hi - (r.lo > a.lo);
    return r;
}
#endif

/* ==================== Complex Number Operations ==================== */
#if HAS_COMPLEX
typedef long double _Complex clong_t;
typedef double _Complex cdouble_t;

/* Complex operations that may expand to multi-operand patterns */
static clong_t complex_mul(clong_t a, clong_t b) {
    return a * b;
}

static clong_t complex_div(clong_t a, clong_t b) {
    return a / b;
}

static clong_t complex_add(clong_t a, clong_t b) {
    return a + b;
}

static clong_t complex_sqrt(clong_t a) {
    /* May expand to internal function with many operands */
    return csqrtl(a);
}

#else
/* Fallback: manual complex arithmetic */
typedef struct {
    long double re;
    long double im;
} clong_t;

static clong_t complex_mul(clong_t a, clong_t b) {
    clong_t r;
    r.re = a.re * b.re - a.im * b.im;
    r.im = a.re * b.im + a.im * b.re;
    return r;
}

static clong_t complex_div(clong_t a, clong_t b) {
    long double denom = b.re * b.re + b.im * b.im;
    clong_t r;
    if (denom != 0.0L) {
        r.re = (a.re * b.re + a.im * b.im) / denom;
        r.im = (a.im * b.re - a.re * b.im) / denom;
    } else {
        r.re = 0.0L;
        r.im = 0.0L;
    }
    return r;
}

static clong_t complex_add(clong_t a, clong_t b) {
    clong_t r;
    r.re = a.re + b.re;
    r.im = a.im + b.im;
    return r;
}
#endif

/* ==================== Vector Operations ==================== */
#if HAS_VECTOR_EXT
/* Define wide vector types */
typedef int32_t __attribute__((vector_size(32))) v8si_t;
typedef float __attribute__((vector_size(32))) v8sf_t;
typedef double __attribute__((vector_size(32))) v4df_t;

/* Vector reduction with accumulation */
static int32_t vector_reduce_sum(v8si_t v) {
    /* Horizontal sum that may expand to multiple operations */
    int32_t sum = 0;
    sum += v[0] + v[1] + v[2] + v[3];
    sum += v[4] + v[5] + v[6] + v[7];
    return sum;
}

static double vector_reduce_product(v4df_t v) {
    /* Horizontal product */
    double prod = 1.0;
    prod *= v[0] * v[1] * v[2] * v[3];
    return prod;
}

#else
/* Fallback: use arrays */
typedef struct {
    int32_t data[8];
} v8si_t;

typedef struct {
    double data[4];
} v4df_t;

static int32_t vector_reduce_sum(v8si_t v) {
    int32_t sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += v.data[i];
    }
    return sum;
}

static double vector_reduce_product(v4df_t v) {
    double prod = 1.0;
    for (int i = 0; i < 4; i++) {
        prod *= v.data[i];
    }
    return prod;
}
#endif

/* ==================== Multi-Operand Helper Functions ==================== */
/* Helper with exactly 10 arguments - may trigger case 10 in optabs.cc */
static __attribute__((noinline)) 
long double helper_10_args(
    dfp128_t a1, dfp128_t a2, clong_t a3, clong_t a4,
    v8si_t a5, v4df_t a6, int32_t a7, double a8,
    long double a9, uint64_t a10
) {
    /* Combine arguments in a way that uses all of them */
    long double result = 0.0L;
    
#if HAS_DFP
    /* Convert DFP to long double for mixing */
    result += (long double)a1 + (long double)a2;
#else
    result += (long double)a1.lo + (long double)a2.hi;
#endif
    
#if HAS_COMPLEX
    result += creall(a3) + cimagl(a4);
#else
    result += a3.re + a4.im;
#endif
    
    result += (long double)vector_reduce_sum(a5);
    result += (long double)vector_reduce_product(a6);
    result += (long double)a7 + a8 + a9 + (long double)a10;
    
    return result;
}

/* Helper with exactly 11 arguments - may trigger case 11 in optabs.cc */
static __attribute__((noinline))
long double helper_11_args(
    dfp128_t a1, dfp128_t a2, dfp128_t a3, clong_t a4,
    clong_t a5, v8si_t a6, v4df_t a7, int32_t a8,
    double a9, long double a10, uint64_t a11
) {
    /* More complex combination */
    long double result = 1.0L;
    
#if HAS_DFP
    result *= (long double)dfp_mul(a1, a2);
    result += (long double)a3;
#else
    result *= (long double)dfp_mul(a1, a2).lo;
    result += (long double)a3.lo;
#endif
    
#if HAS_COMPLEX
    result *= creall(a4) + cimagl(a5);
#else
    result *= a4.re + a5.im;
#endif
    
    result *= (long double)vector_reduce_sum(a6);
    result += (long double)vector_reduce_product(a7);
    result += (long double)a8 * a9 * a10 * (long double)a11;
    
    return result;
}

/* ==================== Main Computation Loop ==================== */
/* Global volatile to prevent optimization */
volatile long double global_accumulator = 0.0L;

int main(int argc, char *argv[]) {
    /* Use command-line seed for deterministic behavior */
    int seed = 12345;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    srand(seed);
    
    /* Initialize base variables */
#if HAS_DFP
    dfp128_t d128_a = 1.23456789e10DL;
    dfp128_t d128_b = 9.87654321e5DL;
    dfp128_t d128_c = 3.14159265e15DL;
    dfp128_t d128_d = 2.71828182e10DL;
#else
    dfp128_t d128_a = { .lo = 123456789ULL, .hi = 0 };
    dfp128_t d128_b = { .lo = 987654321ULL, .hi = 0 };
    dfp128_t d128_c = { .lo = 314159265ULL, .hi = 1 };
    dfp128_t d128_d = { .lo = 271828182ULL, .hi = 0 };
#endif
    
#if HAS_COMPLEX
    clong_t ca = 1.0L + 2.0LI;
    clong_t cb = 3.0L - 4.0LI;
    clong_t cc = -2.0L + 1.5LI;
    clong_t cd = 0.5L - 3.0LI;
#else
    clong_t ca = { .re = 1.0L, .im = 2.0L };
    clong_t cb = { .re = 3.0L, .im = -4.0L };
    clong_t cc = { .re = -2.0L, .im = 1.5L };
    clong_t cd = { .re = 0.5L, .im = -3.0L };
#endif
    
    /* Initialize vectors */
#if HAS_VECTOR_EXT
    v8si_t vi = { 1, 2, 3, 4, 5, 6, 7, 8 };
    v4df_t vd = { 1.1, 2.2, 3.3, 4.4 };
#else
    v8si_t vi = { .data = {1, 2, 3, 4, 5, 6, 7, 8} };
    v4df_t vd = { .data = {1.1, 2.2, 3.3, 4.4} };
#endif
    
    /* Storage for loop results */
    long double results[5] = {0};
    
    /* Main computation loop - 5 iterations */
    for (int iter = 0; iter < 5; iter++) {
        /* Conditional execution based on volatile-like condition */
        int condition = iter % 2;
        
        if (condition) {
            /* Branch 1: Complex DFP arithmetic */
            dfp128_t dfp_result = dfp_add(
                dfp_mul(d128_a, d128_b),
                dfp_div(d128_c, d128_d)
            );
            
#if HAS_COMPLEX
            clong_t cl_result = complex_div(
                complex_mul(ca, cb),
                complex_add(cc, cd)
            );
#else
            clong_t cl_result = complex_div(
                complex_mul(ca, cb),
                complex_add(cc, cd)
            );
#endif
            
            /* Vector reduction with accumulation */
            int32_t vec_sum = vector_reduce_sum(vi);
            double vec_prod = vector_reduce_product(vd);
            
            /* Call helper with 10 arguments */
            results[iter] = helper_10_args(
                dfp_result, d128_a,
                cl_result, ca,
                vi, vd,
                vec_sum, vec_prod,
                (long double)iter, (uint64_t)seed
            );
        } else {
            /* Branch 2: Different combination */
            dfp128_t dfp_result2 = dfp_sub(
                dfp_mul(d128_c, d128_d),
                dfp_div(d128_a, d128_b)
            );
            
#if HAS_COMPLEX
            clong_t cl_result2 = complex_add(
                complex_div(cb, cc),
                complex_mul(cd, ca)
            );
#else
            clong_t cl_result2 = complex_add(
                complex_div(cb, cc),
                complex_mul(cd, ca)
            );
#endif
            
            /* Modify vectors slightly */
#if HAS_VECTOR_EXT
            vi[iter % 8] += iter;
            vd[iter % 4] += 0.1 * iter;
#else
            vi.data[iter % 8] += iter;
            vd.data[iter % 4] += 0.1 * iter;
#endif
            
            int32_t vec_sum2 = vector_reduce_sum(vi);
            double vec_prod2 = vector_reduce_product(vd);
            
            /* Call helper with 11 arguments */
            results[iter] = helper_11_args(
                dfp_result2, d128_b, d128_c,
                cl_result2, cb,
                vi, vd,
                vec_sum2, vec_prod2,
                (long double)(iter * 2), (uint64_t)(seed + iter)
            );
        }
        
        /* Update volatile global to prevent dead code elimination */
        global_accumulator += results[iter];
        
        /* Small modification to inputs for next iteration */
#if HAS_DFP
        d128_a = dfp_add(d128_a, 1.0e5DL);
        d128_b = dfp_sub(d128_b, 1.0e3DL);
#else
        d128_a.lo += 100000;
        d128_b.lo -= 1000;
#endif
    }
    
    /* Compute checksum */
    long double checksum = 0.0L;
    for (int i = 0; i < 5; i++) {
        checksum += results[i];
    }
    
    /* Add global accumulator */
    checksum += global_accumulator;
    
    /* Print deterministic result */
    printf("Result checksum: %Lf\n", checksum);
    printf("Seed used: %d\n", seed);
    
    return 0;
}
