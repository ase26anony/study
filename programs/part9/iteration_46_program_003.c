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

/* Vector type definitions */
#if defined(__GNUC__) && (defined(__x86_64__) || defined(__i386__))
    typedef int32_t v8si __attribute__((vector_size(32)));
    typedef double v4df __attribute__((vector_size(32)));
    #define HAS_VECTORS 1
#elif defined(__GNUC__) && defined(__aarch64__)
    typedef int32_t v8si __attribute__((vector_size(32)));
    typedef double v4df __attribute__((vector_size(32)));
    #define HAS_VECTORS 1
#else
    #define HAS_VECTORS 0
#endif

/* DFP fallback using unions for software emulation */
#if !HAS_DFP
typedef union {
    unsigned long long parts[2];
    double dbl;
} decimal64_emu;

typedef union {
    unsigned long long parts[4];
    long double ldbl;
} decimal128_emu;

#define DEC64_ZERO {{0, 0}, 0.0}
#define DEC128_ZERO {{0, 0, 0, 0}, 0.0L}
#endif

/* Helper functions with many arguments - marked noinline to prevent optimization */
static __attribute__((noinline)) 
long double helper_10_args(long double a1, long double a2, long double a3,
                          long double a4, long double a5, long double a6,
                          long double a7, long double a8, long double a9,
                          long double a10) {
    /* Complex expression that may expand to many operands */
    return ((a1 * a2) + (a3 / a4) - (a5 * a6) + (a7 - a8) * (a9 + a10)) /
           (a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10);
}

static __attribute__((noinline))
long double helper_11_args(long double a1, long double a2, long double a3,
                          long double a4, long double a5, long double a6,
                          long double a7, long double a8, long double a9,
                          long double a10, long double a11) {
    /* Even more complex expression */
    return ((a1 * a2 * a3) + (a4 / a5 / a6) - (a7 * a8) + 
            (a9 - a10) * (a11 + a1)) /
           (a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10 + a11);
}

/* Vector reduction helper */
#if HAS_VECTORS
static __attribute__((noinline))
double vector_reduce_sum(v4df vec) {
    /* Horizontal sum that may expand to multiple operations */
    double sum = 0.0;
    sum += vec[0] + vec[1] + vec[2] + vec[3];
    return sum;
}

static __attribute__((noinline))
int32_t vector_reduce_sum_int(v8si vec) {
    /* Integer horizontal sum */
    int32_t sum = 0;
    sum += vec[0] + vec[1] + vec[2] + vec[3] + 
           vec[4] + vec[5] + vec[6] + vec[7];
    return sum;
}
#endif

/* Main computation function */
int main(int argc, char *argv[]) {
    int seed = 12345;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    srand(seed);
    
    volatile long double results[10] = {0};
    int result_idx = 0;
    
    /* Initialize base values */
#if HAS_DFP
    _Decimal64 d64_a = 1.23456789dd;
    _Decimal64 d64_b = 9.87654321dd;
    _Decimal64 d64_c = 3.14159265dd;
    _Decimal64 d64_d = 2.71828182dd;
    
    _Decimal128 d128_a = 1.2345678901234567890123456789dl;
    _Decimal128 d128_b = 9.8765432109876543210987654321dl;
    _Decimal128 d128_c = 3.1415926535897932384626433833dl;
    _Decimal128 d128_d = 2.7182818284590452353602874714dl;
#else
    /* DFP emulation using doubles */
    decimal64_emu d64_a = {{0x12345678, 0x9abcdef0}, 1.23456789};
    decimal64_emu d64_b = {{0xfedcba98, 0x76543210}, 9.87654321};
    decimal64_emu d64_c = {{0x31415926, 0x53589793}, 3.14159265};
    decimal64_emu d64_d = {{0x27182818, 0x28459045}, 2.71828182};
    
    decimal128_emu d128_a = {{0x12345678, 0x9abcdef0, 0x12345678, 0x9abcdef0}, 1.2345678901234567L};
    decimal128_emu d128_b = {{0xfedcba98, 0x76543210, 0xfedcba98, 0x76543210}, 9.8765432109876543L};
    decimal128_emu d128_c = {{0x31415926, 0x53589793, 0x23846264, 0x33832795}, 3.141592653589793L};
    decimal128_emu d128_d = {{0x27182818, 0x28459045, 0x23536028, 0x74713526}, 2.718281828459045L};
#endif

#if HAS_COMPLEX
    long double _Complex ca = 1.5L + 2.5L * I;
    long double _Complex cb = 3.5L + 4.5L * I;
    long double _Complex cc = 5.5L + 6.5L * I;
    long double _Complex cd = 7.5L + 8.5L * I;
#else
    /* Complex emulation using arrays */
    long double ca[2] = {1.5L, 2.5L};
    long double cb[2] = {3.5L, 4.5L};
    long double cc[2] = {5.5L, 6.5L};
    long double cd[2] = {7.5L, 8.5L};
#endif

#if HAS_VECTORS
    v4df vec_double = {1.1, 2.2, 3.3, 4.4};
    v8si vec_int = {1, 2, 3, 4, 5, 6, 7, 8};
#endif

    /* Main computation loop */
    for (int iter = 0; iter < 4; iter++) {
        volatile int condition = seed + iter;
        long double temp_result = 0.0L;
        
        /* Branch with complex DFP expressions */
        if (condition % 3 == 0) {
#if HAS_DFP
            /* DFP arithmetic that may expand to many operands */
            _Decimal128 d128_result = (d128_a * d128_b + d128_c / d128_d) *
                                     (d128_a - d128_b) / (d128_c + d128_d);
            temp_result = (long double)d128_result;
#else
            /* Emulated DFP using manual multi-precision */
            long double ld_a = d128_a.ldbl;
            long double ld_b = d128_b.ldbl;
            long double ld_c = d128_c.ldbl;
            long double ld_d = d128_d.ldbl;
            temp_result = (ld_a * ld_b + ld_c / ld_d) *
                         (ld_a - ld_b) / (ld_c + ld_d);
#endif
        } else if (condition % 3 == 1) {
#if HAS_COMPLEX
            /* Complex arithmetic with library calls */
            long double _Complex ctemp = (ca * cb) / (cc - cd);
            long double _Complex csqrt_val = csqrt(ctemp);
            long double _Complex cpow_val = cpow(ca, cb);
            
            /* Mix real and imaginary parts in complex expression */
            temp_result = creal(csqrt_val) * cimag(cpow_val) +
                         creal(cpow_val) * cimag(csqrt_val);
#else
            /* Manual complex arithmetic */
            long double real_temp = (ca[0] * cb[0] - ca[1] * cb[1]) / 
                                   (cc[0] - cd[0] - (cc[1] - cd[1]));
            long double imag_temp = (ca[0] * cb[1] + ca[1] * cb[0]) / 
                                   (cc[0] - cd[0] - (cc[1] - cd[1]));
            temp_result = real_temp * imag_temp;
#endif
        } else {
#if HAS_VECTORS
            /* Vector reduction with accumulation */
            double vec_sum = vector_reduce_sum(vec_double);
            int32_t int_sum = vector_reduce_sum_int(vec_int);
            temp_result = vec_sum + int_sum;
            
            /* Modify vectors for next iteration */
            vec_double += (v4df){0.1, 0.2, 0.3, 0.4};
            vec_int += (v8si){1, 1, 1, 1, 1, 1, 1, 1};
#else
            /* Manual vector-like computation */
            double manual_vec[4] = {1.1, 2.2, 3.3, 4.4};
            int32_t manual_int[8] = {1, 2, 3, 4, 5, 6, 7, 8};
            double manual_sum = 0.0;
            for (int i = 0; i < 4; i++) manual_sum += manual_vec[i];
            for (int i = 0; i < 8; i++) manual_sum += manual_int[i];
            temp_result = manual_sum;
#endif
        }
        
        /* Call helper functions with many arguments */
        long double arg_base = temp_result + iter;
        long double helper_result;
        
        if (condition % 2 == 0) {
            /* 10-argument call */
            helper_result = helper_10_args(
                arg_base * 1.0L, arg_base * 1.1L, arg_base * 1.2L,
                arg_base * 1.3L, arg_base * 1.4L, arg_base * 1.5L,
                arg_base * 1.6L, arg_base * 1.7L, arg_base * 1.8L,
                arg_base * 1.9L
            );
        } else {
            /* 11-argument call */
            helper_result = helper_11_args(
                arg_base * 1.0L, arg_base * 1.1L, arg_base * 1.2L,
                arg_base * 1.3L, arg_base * 1.4L, arg_base * 1.5L,
                arg_base * 1.6L, arg_base * 1.7L, arg_base * 1.8L,
                arg_base * 1.9L, arg_base * 2.0L
            );
        }
        
        /* Store result to prevent elimination */
        results[result_idx++ % 10] = helper_result;
        
        /* Update base values for next iteration */
#if HAS_DFP
        d128_a += (_Decimal128)0.123456789dl;
        d128_b -= (_Decimal128)0.098765432dl;
#elif HAS_COMPLEX
        ca += 0.1L + 0.2L * I;
        cb -= 0.2L + 0.1L * I;
#endif
    }
    
    /* Compute checksum */
    long double checksum = 0.0L;
    for (int i = 0; i < 10; i++) {
        checksum += results[i];
    }
    
    printf("Result checksum: %Lf\n", checksum);
    printf("Seed used: %d\n", seed);
    
    return 0;
}
