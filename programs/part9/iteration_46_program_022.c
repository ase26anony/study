#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* Feature detection and fallbacks */
#ifdef __DECIMAL_BID_FORMAT__
    #define DFP_SUPPORTED 1
    #include <decimal/decimal.h>
#else
    #define DFP_SUPPORTED 0
    /* Fallback DFP types using unions */
    typedef union {
        uint64_t u64[2];
        double   dbl[2];
    } decimal64_fb;
    
    typedef union {
        uint64_t u64[4];
        double   dbl[4];
    } decimal128_fb;
#endif

#ifdef __STDC_IEC_559_COMPLEX__
    #include <complex.h>
    #define COMPLEX_SUPPORTED 1
#else
    #define COMPLEX_SUPPORTED 0
#endif

/* Vector types if supported */
#ifdef __VECTOR_TYPES_SUPPORTED__
    typedef int32_t int32x8_t __attribute__((vector_size(32)));
    typedef double float64x4_t __attribute__((vector_size(32)));
#else
    /* Fallback using arrays */
    typedef struct { int32_t v[8]; } int32x8_t;
    typedef struct { double v[4]; } float64x4_t;
#endif

/* Global volatile to prevent optimization */
volatile uint64_t g_result_store[16];
static int g_store_idx = 0;

/* Helper functions with many arguments (10-11) */
static __attribute__((noinline)) 
#ifdef DFP_SUPPORTED
_Decimal128 helper_10_args_dfp(_Decimal64 a1, _Decimal64 a2, _Decimal128 a3, 
                               _Decimal128 a4, _Decimal64 a5, _Decimal64 a6,
                               _Decimal128 a7, _Decimal128 a8, _Decimal64 a9,
                               _Decimal64 a10) {
    /* Complex DFP expression that may expand to many operands */
    return a1 * a2 + a3 / a4 - a5 * a6 + a7 - a8 + a9 * a10;
}
#else
decimal128_fb helper_10_args_dfp(decimal64_fb a1, decimal64_fb a2, decimal128_fb a3,
                                 decimal128_fb a4, decimal64_fb a5, decimal64_fb a6,
                                 decimal128_fb a7, decimal128_fb a8, decimal64_fb a9,
                                 decimal64_fb a10) {
    /* Manual multi-precision arithmetic */
    decimal128_fb result;
    for (int i = 0; i < 4; i++) {
        result.u64[i] = a1.u64[i % 2] + a2.u64[i % 2] + a3.u64[i] 
                      - a4.u64[i] + a5.u64[i % 2] * a6.u64[i % 2];
    }
    return result;
}
#endif

static __attribute__((noinline))
#ifdef COMPLEX_SUPPORTED
long double _Complex helper_11_args_complex(long double _Complex c1,
                                           long double _Complex c2,
                                           long double _Complex c3,
                                           long double _Complex c4,
                                           long double _Complex c5,
                                           long double _Complex c6,
                                           long double _Complex c7,
                                           long double _Complex c8,
                                           long double _Complex c9,
                                           long double _Complex c10,
                                           long double _Complex c11) {
    /* Complex expression that may require many operands */
    return (c1 * c2 + c3 / c4 - c5 * c6 + c7 - c8 + c9 * c10) * c11;
}
#else
struct complex_fb { double real; double imag; };

struct complex_fb helper_11_args_complex(struct complex_fb c1, struct complex_fb c2,
                                        struct complex_fb c3, struct complex_fb c4,
                                        struct complex_fb c5, struct complex_fb c6,
                                        struct complex_fb c7, struct complex_fb c8,
                                        struct complex_fb c9, struct complex_fb c10,
                                        struct complex_fb c11) {
    struct complex_fb result;
    result.real = c1.real + c2.real + c3.real + c4.real + c5.real 
                + c6.real + c7.real + c8.real + c9.real + c10.real + c11.real;
    result.imag = c1.imag + c2.imag + c3.imag + c4.imag + c5.imag 
                + c6.imag + c7.imag + c8.imag + c9.imag + c10.imag + c11.imag;
    return result;
}
#endif

/* Vector reduction helper */
static __attribute__((noinline))
#ifdef __VECTOR_TYPES_SUPPORTED__
int32_t vector_reduce_sum(int32x8_t v) {
    /* Horizontal reduction - may expand to many operations */
    int32_t sum = 0;
    sum += v[0] + v[1] + v[2] + v[3] + v[4] + v[5] + v[6] + v[7];
    return sum;
}
#else
int32_t vector_reduce_sum(int32x8_t v) {
    int32_t sum = 0;
    for (int i = 0; i < 8; i++) sum += v.v[i];
    return sum;
}
#endif

/* Main computation with conditional execution */
static void perform_computations(int seed, int iteration) {
    /* Initialize with deterministic but complex patterns */
    uint64_t base = (seed * 1103515245 + 12345) & 0x7FFFFFFF;
    
#ifdef DFP_SUPPORTED
    /* DFP variables */
    _Decimal64 d64_a = (_Decimal64)base;
    _Decimal64 d64_b = (_Decimal64)(base + 1);
    _Decimal64 d64_c = (_Decimal64)(base + 2);
    _Decimal64 d64_d = (_Decimal64)(base + 3);
    _Decimal128 d128_a = (_Decimal128)(base * 1000);
    _Decimal128 d128_b = (_Decimal128)(base * 2000);
    _Decimal128 d128_c = (_Decimal128)(base * 3000);
    _Decimal128 d128_d = (_Decimal128)(base * 4000);
    
    /* Complex DFP expression with many operands */
    _Decimal128 d128_result;
    if (iteration % 2 == 0) {
        d128_result = d128_a * d128_b + d128_c / d128_d 
                    - d64_a * d64_b + d64_c * d64_d;
    } else {
        d128_result = d128_a / d128_b - d128_c * d128_d 
                    + d64_a / d64_b - d64_c / d64_d;
    }
    
    /* Call helper with 10 DFP arguments */
    _Decimal128 helper_result = helper_10_args_dfp(
        d64_a, d64_b, d128_a, d128_b, d64_c, d64_d,
        d128_c, d128_d, d64_a + d64_b, d64_c + d64_d
    );
    
    /* Store results */
    uint64_t *ptr = (uint64_t*)&helper_result;
    g_result_store[g_store_idx++] = ptr[0];
    g_result_store[g_store_idx++] = ptr[1];
#else
    /* Fallback DFP simulation */
    decimal64_fb d64_a, d64_b, d64_c, d64_d;
    decimal128_fb d128_a, d128_b, d128_c, d128_d;
    
    for (int i = 0; i < 2; i++) d64_a.u64[i] = base + i;
    for (int i = 0; i < 2; i++) d64_b.u64[i] = base + 10 + i;
    for (int i = 0; i < 2; i++) d64_c.u64[i] = base + 20 + i;
    for (int i = 0; i < 2; i++) d64_d.u64[i] = base + 30 + i;
    
    for (int i = 0; i < 4; i++) d128_a.u64[i] = base * 1000 + i;
    for (int i = 0; i < 4; i++) d128_b.u64[i] = base * 2000 + i;
    for (int i = 0; i < 4; i++) d128_c.u64[i] = base * 3000 + i;
    for (int i = 0; i < 4; i++) d128_d.u64[i] = base * 4000 + i;
    
    decimal128_fb helper_result = helper_10_args_dfp(
        d64_a, d64_b, d128_a, d128_b, d64_c, d64_d,
        d128_c, d128_d, d64_a, d64_b
    );
    
    for (int i = 0; i < 4; i++) {
        g_result_store[g_store_idx++] = helper_result.u64[i];
    }
#endif

#ifdef COMPLEX_SUPPORTED
    /* Complex number operations */
    long double _Complex ca = base + (base % 7) * I;
    long double _Complex cb = (base + 1) + ((base + 1) % 5) * I;
    long double _Complex cc = (base + 2) + ((base + 2) % 3) * I;
    long double _Complex cd = (base + 3) + ((base + 3) % 11) * I;
    long double _Complex ce = (base + 4) + ((base + 4) % 13) * I;
    long double _Complex cf = (base + 5) + ((base + 5) % 17) * I;
    
    /* Complex arithmetic with many intermediate values */
    long double _Complex cl_result;
    if (iteration % 3 == 0) {
        cl_result = (ca * cb) / (cc - cd) + (ce * cf);
    } else if (iteration % 3 == 1) {
        cl_result = csqrt(ca) * cpow(cb, cc) - cd / ce;
    } else {
        cl_result = (ca + cb) * (cc - cd) / (ce + cf);
    }
    
    /* Call helper with 11 complex arguments */
    long double _Complex complex_helper_result = helper_11_args_complex(
        ca, cb, cc, cd, ce, cf,
        ca * cb, cc / cd, ce - cf,
        ca + cc, cb + cd
    );
    
    /* Store results */
    long double *cptr = (long double*)&complex_helper_result;
    g_result_store[g_store_idx++] = (uint64_t)cptr[0];
    g_result_store[g_store_idx++] = (uint64_t)cptr[1];
#else
    /* Fallback complex simulation */
    struct complex_fb ca = {base, base % 7};
    struct complex_fb cb = {base + 1, (base + 1) % 5};
    struct complex_fb cc = {base + 2, (base + 2) % 3};
    struct complex_fb cd = {base + 3, (base + 3) % 11};
    struct complex_fb ce = {base + 4, (base + 4) % 13};
    struct complex_fb cf = {base + 5, (base + 5) % 17};
    
    struct complex_fb complex_helper_result = helper_11_args_complex(
        ca, cb, cc, cd, ce, cf,
        ca, cb, cc, cd, ce
    );
    
    g_result_store[g_store_idx++] = (uint64_t)complex_helper_result.real;
    g_result_store[g_store_idx++] = (uint64_t)complex_helper_result.imag;
#endif

    /* Vector operations */
#ifdef __VECTOR_TYPES_SUPPORTED__
    int32x8_t vec_int;
    for (int i = 0; i < 8; i++) {
        vec_int[i] = base + i * iteration;
    }
    
    /* Vector reduction with accumulation */
    int32_t vec_sum = 0;
    for (int i = 0; i < 3; i++) {  /* Small loop to repeat without optimization */
        vec_sum += vector_reduce_sum(vec_int);
        vec_int[0] += 1;  /* Prevent loop elimination */
    }
    
    g_result_store[g_store_idx++] = vec_sum;
#else
    /* Fallback vector simulation */
    int32x8_t vec_int;
    for (int i = 0; i < 8; i++) {
        vec_int.v[i] = base + i * iteration;
    }
    
    int32_t vec_sum = vector_reduce_sum(vec_int);
    g_result_store[g_store_idx++] = vec_sum;
#endif
}

int main(int argc, char *argv[]) {
    /* Use command line seed or default */
    int seed = 42;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    printf("Starting computation with seed: %d\n", seed);
    printf("DFP supported: %d, Complex supported: %d\n", 
           DFP_SUPPORTED, COMPLEX_SUPPORTED);
    
    /* Perform computations in a loop */
    for (int iter = 0; iter < 4; iter++) {
        perform_computations(seed + iter, iter);
    }
    
    /* Compute checksum of all results */
    uint64_t checksum = 0;
    for (int i = 0; i < g_store_idx; i++) {
        checksum += g_result_store[i];
    }
    
    printf("Checksum: %lu\n", (unsigned long)checksum);
    printf("Total values stored: %d\n", g_store_idx);
    
    return 0;
}
