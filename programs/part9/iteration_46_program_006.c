#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* Feature detection and fallbacks */
#ifdef __DECIMAL_BID_FORMAT__
    #define HAS_DFP 1
    #include <decimal.h>
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
    #define HAS_VECTORS 1
    typedef int32_t v8si __attribute__((vector_size(32)));
    typedef double v4df __attribute__((vector_size(32)));
#else
    #define HAS_VECTORS 0
#endif

/* DFP fallback structures */
#if !HAS_DFP
typedef struct {
    uint64_t lo;
    uint64_t hi;
} decimal64_fb;

typedef struct {
    uint64_t w[2];
} decimal128_fb;
#endif

/* Helper function with 11 arguments - marked noinline to prevent simplification */
static __attribute__((noinline)) 
long double helper_11_args(
    long double a1, long double a2, long double a3,
    long double a4, long double a5, long double a6,
    long double a7, long double a8, long double a9,
    long double a10, long double a11)
{
    /* Complex expression that may expand to many operands */
    return ((a1 * a2) + (a3 / a4) - (a5 * a6) + 
            (a7 - a8) * (a9 + a10) / a11);
}

/* Another helper with exactly 10 arguments */
static __attribute__((noinline))
double helper_10_args(
    double b1, double b2, double b3, double b4, double b5,
    double b6, double b7, double b8, double b9, double b10)
{
    /* Multi-step computation that may require many temporaries */
    double t1 = b1 * b2 + b3;
    double t2 = b4 / b5 - b6;
    double t3 = b7 * b8 + b9;
    return (t1 * t2) / (t3 + b10);
}

#if HAS_VECTORS
/* Vector reduction with accumulation */
static double vector_reduce_sum(v4df vec) {
    /* Horizontal sum that may expand to multiple operations */
    double sum = 0.0;
    sum += vec[0] + vec[1] + vec[2] + vec[3];
    return sum;
}

static int32_t vector_reduce_product(v8si vec) {
    /* Horizontal product */
    int32_t prod = 1;
    prod *= vec[0] * vec[1] * vec[2] * vec[3];
    prod *= vec[4] * vec[5] * vec[6] * vec[7];
    return prod;
}
#endif

/* Main computation function */
int main(int argc, char *argv[]) {
    int seed = (argc > 1) ? atoi(argv[1]) : 42;
    srand(seed);
    
    volatile double results[10] = {0};
    int result_idx = 0;
    
    /* Initialize base values */
    double base_dbl = 1.5 + (rand() % 100) * 0.01;
    long double base_ld = 2.5L + (rand() % 100) * 0.01L;
    
#if HAS_DFP
    /* Decimal floating point values */
    _Decimal64 d64_a = (_Decimal64)base_dbl;
    _Decimal64 d64_b = (_Decimal64)(base_dbl * 0.75);
    _Decimal64 d64_c = (_Decimal64)(base_dbl * 1.25);
    _Decimal64 d64_d = (_Decimal64)(base_dbl * 0.5);
    
    _Decimal128 d128_a = (_Decimal128)base_ld;
    _Decimal128 d128_b = (_Decimal128)(base_ld * 0.8L);
    _Decimal128 d128_c = (_Decimal128)(base_ld * 1.2L);
    _Decimal128 d128_d = (_Decimal128)(base_ld * 0.3L);
#else
    /* Fallback: use arrays to simulate multi-word arithmetic */
    decimal64_fb d64_a = {.lo = (uint64_t)(base_dbl * 1e9), .hi = 0};
    decimal64_fb d64_b = {.lo = (uint64_t)(base_dbl * 0.75 * 1e9), .hi = 0};
    decimal64_fb d64_c = {.lo = (uint64_t)(base_dbl * 1.25 * 1e9), .hi = 0};
    decimal64_fb d64_d = {.lo = (uint64_t)(base_dbl * 0.5 * 1e9), .hi = 0};
    
    decimal128_fb d128_a = {.w = {(uint64_t)(base_ld * 1e9), 0}};
    decimal128_fb d128_b = {.w = {(uint64_t)(base_ld * 0.8L * 1e9), 0}};
    decimal128_fb d128_c = {.w = {(uint64_t)(base_ld * 1.2L * 1e9), 0}};
    decimal128_fb d128_d = {.w = {(uint64_t)(base_ld * 0.3L * 1e9), 0}};
#endif

#if HAS_COMPLEX
    /* Complex numbers with high precision */
    long double _Complex ca = base_ld + base_ld * 0.5L * I;
    long double _Complex cb = base_ld * 0.75L + base_ld * 0.25L * I;
    long double _Complex cc = base_ld * 1.1L + base_ld * 0.9L * I;
    long double _Complex cd = base_ld * 0.4L + base_ld * 0.6L * I;
#endif

#if HAS_VECTORS
    /* Vector initialization */
    v4df vec_double = {base_dbl, base_dbl * 0.5, base_dbl * 0.25, base_dbl * 0.125};
    v8si vec_int = {1, 2, 3, 4, 5, 6, 7, 8};
#endif

    /* Main computation loop */
    for (int iter = 0; iter < 4; iter++) {
        volatile int condition = (iter % 2 == 0) ? 1 : 0;
        
        if (condition) {
            /* Branch 1: Complex DFP operations */
#if HAS_DFP
            /* Multi-step DFP computation that may expand to many operands */
            _Decimal128 d128_result;
            /* This complex expression may require many temporary operands */
            d128_result = d128_a * d128_b + d128_c / d128_d;
            d128_result = d128_result * d128_a - d128_b / d128_c;
            
            /* Convert to double for storage */
            results[result_idx++] = (double)d128_result;
#else
            /* Fallback: manual multi-precision arithmetic */
            uint64_t temp_lo = d128_a.w[0] * d128_b.w[0] / 1000000000ULL;
            uint64_t temp_hi = d128_c.w[0] / d128_d.w[0];
            results[result_idx++] = (double)(temp_lo + temp_hi) / 1e9;
#endif
        } else {
            /* Branch 2: Complex number operations */
#if HAS_COMPLEX
            /* Complex arithmetic that may expand real/imag parts separately */
            long double _Complex cl_result;
            cl_result = (ca * cb) / (cc - cd);
            /* Additional operations to increase operand count */
            cl_result = cl_result + ca * cd - cb / cc;
            
            /* Store real part */
            results[result_idx++] = creall(cl_result);
#else
            /* Fallback: manual complex arithmetic */
            double re = base_dbl * 0.75 - base_dbl * 0.25;
            double im = base_dbl * 0.5 + base_dbl * 0.125;
            results[result_idx++] = re + im;
#endif
        }
        
        /* Vector reduction (always executed) */
#if HAS_VECTORS
        double vec_sum = vector_reduce_sum(vec_double);
        int vec_prod = vector_reduce_product(vec_int);
        results[result_idx++] = vec_sum + vec_prod;
        
        /* Modify vectors for next iteration */
        for (int i = 0; i < 4; i++) vec_double[i] *= 1.1;
        for (int i = 0; i < 8; i++) vec_int[i] += 1;
#endif
        
        /* Call helper functions with many arguments */
        /* These calls create sites with 10-11 arguments at the RTL level */
        double helper_result_10 = helper_10_args(
            base_dbl * 1.0, base_dbl * 1.1, base_dbl * 1.2,
            base_dbl * 1.3, base_dbl * 1.4, base_dbl * 1.5,
            base_dbl * 1.6, base_dbl * 1.7, base_dbl * 1.8,
            base_dbl * 1.9);
        
        long double helper_result_11 = helper_11_args(
            base_ld * 1.0L, base_ld * 1.1L, base_ld * 1.2L,
            base_ld * 1.3L, base_ld * 1.4L, base_ld * 1.5L,
            base_ld * 1.6L, base_ld * 1.7L, base_ld * 1.8L,
            base_ld * 1.9L, base_ld * 2.0L);
        
        results[result_idx++] = helper_result_10;
        results[result_idx++] = (double)helper_result_11;
        
        /* Update base values for next iteration */
        base_dbl *= 0.95;
        base_ld *= 0.97L;
    }
    
    /* Compute checksum */
    double checksum = 0.0;
    for (int i = 0; i < result_idx; i++) {
        checksum += results[i];
    }
    
    printf("Result count: %d\n", result_idx);
    printf("Checksum: %.15f\n", checksum);
    
    return 0;
}
