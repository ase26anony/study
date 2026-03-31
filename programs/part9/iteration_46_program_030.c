#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

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

/* Vector type definitions */
#if defined(__GNUC__) && (defined(__x86_64__) || defined(__i386__) || defined(__aarch64__))
    #define VECTOR_SIZE 8
    typedef int32_t int32x8_t __attribute__((vector_size(32)));
    typedef float float32x8_t __attribute__((vector_size(32)));
    #define HAS_VECTORS 1
#else
    #define VECTOR_SIZE 8
    #define HAS_VECTORS 0
#endif

/* DFP fallback using integer arrays */
#if !HAS_DFP
typedef struct {
    uint64_t lo;
    uint64_t hi;
} decimal64_fb;

typedef struct {
    uint64_t parts[2];
} decimal128_fb;

static decimal64_fb dfp64_add(decimal64_fb a, decimal64_fb b) {
    decimal64_fb r;
    r.lo = a.lo + b.lo;
    r.hi = a.hi + b.hi + (r.lo < a.lo);
    return r;
}

static decimal64_fb dfp64_mul(decimal64_fb a, decimal64_fb b) {
    /* Simplified multiplication for coverage only */
    decimal64_fb r;
    r.lo = a.lo * b.lo;
    r.hi = a.hi * b.hi;
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

/* Helper function with 11 arguments - marked noinline */
static __attribute__((noinline)) 
long double helper_11_args(
    long double a1, long double a2, long double a3,
    long double a4, long double a5, long double a6,
    long double a7, long double a8, long double a9,
    long double a10, long double a11)
{
    /* Complex expression to prevent optimization */
    return ((a1 * a2) + (a3 * a4) - (a5 * a6)) / 
           ((a7 + a8 + a9) * (a10 - a11 + 1.0L));
}

/* Helper function with 10 arguments for DFP operations */
#if HAS_DFP
static __attribute__((noinline))
_Decimal128 helper_10_args_dfp(
    _Decimal128 d1, _Decimal128 d2, _Decimal128 d3,
    _Decimal128 d4, _Decimal128 d5, _Decimal128 d6,
    _Decimal128 d7, _Decimal128 d8, _Decimal128 d9,
    _Decimal128 d10)
{
    /* Complex DFP expression */
    return ((d1 * d2) + (d3 * d4) - (d5 * d6)) / 
           ((d7 + d8) * (d9 - d10));
}
#endif

/* Vector reduction helper */
#if HAS_VECTORS
static __attribute__((noinline))
int32_t vector_reduce_sum(int32x8_t v) {
    /* Force horizontal reduction */
    int32_t sum = 0;
    for (int i = 0; i < VECTOR_SIZE; i++) {
        sum += v[i];
    }
    return sum;
}
#endif

/* Global volatile to prevent optimization */
volatile long double global_accumulator = 0.0L;

int main(int argc, char *argv[]) {
    int seed = 12345;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    srand(seed);
    
    /* Initialize base variables */
    long double ld_vals[12];
    for (int i = 0; i < 12; i++) {
        ld_vals[i] = (long double)(rand() % 1000) / 100.0L;
    }
    
    /* DFP initialization */
#if HAS_DFP
    _Decimal64 d64_a = 1.23456789dd;
    _Decimal64 d64_b = 9.87654321dd;
    _Decimal64 d64_c = 5.55555555dd;
    _Decimal64 d64_d = 3.33333333dd;
    
    _Decimal128 d128_a = 1.2345678901234567dl;
    _Decimal128 d128_b = 9.8765432109876543dl;
    _Decimal128 d128_c = 5.5555555555555555dl;
    _Decimal128 d128_d = 3.3333333333333333dl;
#else
    /* Fallback DFP-like values */
    decimal64_fb d64_a = {123456789ULL, 0};
    decimal64_fb d64_b = {987654321ULL, 0};
    decimal64_fb d64_c = {555555555ULL, 0};
    decimal64_fb d64_d = {333333333ULL, 0};
#endif
    
    /* Complex initialization */
#if HAS_COMPLEX
    long double _Complex ca = ld_vals[0] + ld_vals[1] * I;
    long double _Complex cb = ld_vals[2] + ld_vals[3] * I;
    long double _Complex cc = ld_vals[4] + ld_vals[5] * I;
    long double _Complex cd = ld_vals[6] + ld_vals[7] * I;
#else
    complex_ld_fb ca = {ld_vals[0], ld_vals[1]};
    complex_ld_fb cb = {ld_vals[2], ld_vals[3]};
    complex_ld_fb cc = {ld_vals[4], ld_vals[5]};
    complex_ld_fb cd = {ld_vals[6], ld_vals[7]};
#endif
    
    /* Vector initialization */
#if HAS_VECTORS
    int32x8_t vec_a = {1, 2, 3, 4, 5, 6, 7, 8};
    int32x8_t vec_b = {8, 7, 6, 5, 4, 3, 2, 1};
    float32x8_t fvec_a = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    float32x8_t fvec_b = {8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f};
#endif
    
    /* Storage for results to prevent DCE */
    long double results[5] = {0};
    int result_idx = 0;
    
    /* Main computation loop */
    for (int iter = 0; iter < 3; iter++) {
        volatile int condition = rand() % 2;
        
        if (condition) {
            /* Branch 1: Complex DFP operations */
#if HAS_DFP
            /* Complex DFP expression that may expand to many operands */
            _Decimal128 d128_result = ((d128_a * d128_b) + (d128_c / d128_d)) *
                                      ((d128_a + d128_b) - (d128_c * d128_d)) /
                                      ((d128_b - d128_a) + (d128_d / d128_c));
            
            /* Call helper with 10 DFP arguments */
            _Decimal128 helper_result = helper_10_args_dfp(
                d128_result, d128_a, d128_b, d128_c, d128_d,
                d128_result * 2.0dl, d128_a / 3.0dl,
                d128_b + d128_c, d128_c - d128_d,
                d128_d * d128_a);
            
            results[result_idx++] = (long double)helper_result;
#endif
            
            /* Complex number operations */
#if HAS_COMPLEX
            long double _Complex cl_result = (ca * cb) / (cc - cd);
            cl_result = cl_result * cl_result + ca / cb - cc * cd;
            
            /* Use real and imaginary parts in expressions */
            long double re = creall(cl_result);
            long double im = cimagl(cl_result);
            results[result_idx++] = re * re + im * im;
#endif
        } else {
            /* Branch 2: Vector and mixed operations */
#if HAS_VECTORS
            /* Vector operations that may expand */
            int32x8_t vec_result = vec_a * vec_b + vec_a - vec_b;
            float32x8_t fvec_result = fvec_a * fvec_b / (fvec_a + fvec_b);
            
            /* Horizontal reduction with accumulation */
            int32_t vec_sum = 0;
            for (int i = 0; i < VECTOR_SIZE; i++) {
                vec_sum += vec_result[i];
            }
            
            float vec_fsum = 0.0f;
            for (int i = 0; i < VECTOR_SIZE; i++) {
                vec_fsum += fvec_result[i];
            }
            
            results[result_idx++] = vec_sum + vec_fsum;
#endif
            
            /* Mixed-type helper call with 11 arguments */
            long double helper_res = helper_11_args(
                ld_vals[0] + iter, ld_vals[1] - iter,
                ld_vals[2] * iter, ld_vals[3] / (iter + 1.0L),
                ld_vals[4], ld_vals[5],
                ld_vals[6] * ld_vals[7],
                ld_vals[8] - ld_vals[9],
                ld_vals[10] + ld_vals[11],
                (long double)(iter * 2),
                (long double)(iter + 1));
            
            results[result_idx++] = helper_res;
        }
        
        /* Update DFP values for next iteration */
#if HAS_DFP
        d128_a = d128_a * 1.1dl;
        d128_b = d128_b / 1.1dl;
        d128_c = d128_c + 0.5dl;
        d128_d = d128_d - 0.25dl;
#endif
        
        /* Update complex values */
#if HAS_COMPLEX
        ca = ca * (1.0L + 0.1L * I);
        cb = cb / (1.0L - 0.1L * I);
#endif
    }
    
    /* Aggregate results into volatile global */
    for (int i = 0; i < result_idx; i++) {
        global_accumulator += results[i];
    }
    
    /* Simple checksum and output */
    long double checksum = 0.0L;
    for (int i = 0; i < result_idx; i++) {
        checksum += results[i];
    }
    
    printf("Result count: %d\n", result_idx);
    printf("Checksum: %Lf\n", checksum);
    
    return 0;
}
