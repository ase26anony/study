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

/* Vector type fallback simulation */
typedef struct {
    long long data[8];
} int32x8_simd;

typedef struct {
    double data[4];
} float64x4_simd;

/* DFP fallback using integer representation */
typedef union {
    uint64_t bits;
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
    uint8_t bytes[16];
} decimal128_fallback;

/* Complex fallback */
typedef struct {
    long double real;
    long double imag;
} complex_fallback;

/* Helper functions with 10+ arguments (noinline to prevent simplification) */
static __attribute__((noinline)) 
long double helper_10_args(long double a1, long double a2, long double a3,
                          long double a4, long double a5, long double a6,
                          long double a7, long double a8, long double a9,
                          long double a10) {
    /* Complex expression that may expand to many operands */
    return ((a1 * a2) + (a3 / a4) - (a5 * a6) + (a7 - a8) * (a9 + a10)) /
           ((a1 + a2) * (a3 - a4) + (a5 / a6) - (a7 * a8) + (a9 - a10));
}

static __attribute__((noinline))
long double helper_11_args(long double a1, long double a2, long double a3,
                          long double a4, long double a5, long double a6,
                          long double a7, long double a8, long double a9,
                          long double a10, long double a11) {
    /* Even more complex expression */
    return (((a1 * a2) + (a3 * a4)) / ((a5 - a6) + (a7 * a8))) *
           ((a9 + a10) / (a11 + a1)) - ((a2 * a3) / (a4 + a5)) +
           ((a6 - a7) * (a8 + a9)) / (a10 * a11);
}

/* DFP operations with fallback */
#if HAS_DFP
static _Decimal128 dfp_complex_operation(_Decimal128 a, _Decimal128 b,
                                        _Decimal128 c, _Decimal128 d,
                                        _Decimal128 e, _Decimal128 f) {
    /* Complex DFP expression that may require many operands */
    return ((a * b) + (c / d) - (e * f)) / 
           ((a + b) * (c - d) + (e / f));
}
#else
static long double dfp_complex_operation_fallback(
    decimal128_fallback a, decimal128_fallback b,
    decimal128_fallback c, decimal128_fallback d,
    decimal128_fallback e, decimal128_fallback f) {
    
    /* Simulate DFP using integer arithmetic */
    long double da = (long double)a.words.low + (long double)a.words.high * 1.0e19;
    long double db = (long double)b.words.low + (long double)b.words.high * 1.0e19;
    long double dc = (long double)c.words.low + (long double)c.words.high * 1.0e19;
    long double dd = (long double)d.words.low + (long double)d.words.high * 1.0e19;
    long double de = (long double)e.words.low + (long double)e.words.high * 1.0e19;
    long double df = (long double)f.words.low + (long double)f.words.high * 1.0e19;
    
    return ((da * db) + (dc / dd) - (de * df)) / 
           ((da + db) * (dc - dd) + (de / df));
}
#endif

/* Complex number operations */
#if HAS_COMPLEX
static long double _Complex complex_high_precision(
    long double _Complex a, long double _Complex b,
    long double _Complex c, long double _Complex d) {
    
    /* Complex expression that may expand real/imag parts separately */
    return (a * b) / (c - d) + (a + b) * (c * d) - 
           (a / b) + (c + d) / (a - b);
}
#else
static complex_fallback complex_high_precision_fallback(
    complex_fallback a, complex_fallback b,
    complex_fallback c, complex_fallback d) {
    
    complex_fallback result;
    /* Manual complex arithmetic */
    long double t1_real = a.real * b.real - a.imag * b.imag;
    long double t1_imag = a.real * b.imag + a.imag * b.real;
    
    long double t2_real = c.real - d.real;
    long double t2_imag = c.imag - d.imag;
    
    long double denom = t2_real * t2_real + t2_imag * t2_imag;
    result.real = (t1_real * t2_real + t1_imag * t2_imag) / denom;
    result.imag = (t1_imag * t2_real - t1_real * t2_imag) / denom;
    
    return result;
}
#endif

/* Vector reduction */
static long long vector_reduction(int32x8_simd v) {
    long long sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += v.data[i];
    }
    return sum;
}

static double vector_fp_reduction(float64x4_simd v) {
    double sum = 0.0;
    for (int i = 0; i < 4; i++) {
        sum += v.data[i];
    }
    return sum;
}

/* Main computation with conditional execution */
static volatile long double global_accumulator = 0.0;

int main(int argc, char *argv[]) {
    int seed = 12345;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    srand(seed);
    
    /* Initialize base variables */
#if HAS_DFP
    _Decimal64 d64_a = 1.23456789e10dl;
    _Decimal64 d64_b = 9.87654321e9dl;
    _Decimal128 d128_a = 1.2345678901234567890123456789e30dl;
    _Decimal128 d128_b = 9.8765432109876543210987654321e29dl;
    _Decimal128 d128_c = 5.5555555555555555555555555555e28dl;
    _Decimal128 d128_d = 2.2222222222222222222222222222e27dl;
    _Decimal128 d128_e = 3.3333333333333333333333333333e26dl;
    _Decimal128 d128_f = 1.1111111111111111111111111111e25dl;
#else
    decimal64_fallback d64_a = {.parts = {.sign = 0, .exponent = 1023, .mantissa = 0x123456789ABCDULL}};
    decimal64_fallback d64_b = {.parts = {.sign = 0, .exponent = 1022, .mantissa = 0x9876543210FEDCULL}};
    decimal128_fallback d128_a = {.words = {.low = 0x123456789ABCDEF0ULL, .high = 0x0FEDCBA987654321ULL}};
    decimal128_fallback d128_b = {.words = {.low = 0x9876543210FEDCBAULL, .high = 0x0123456789ABCDEFULL}};
    decimal128_fallback d128_c = {.words = {.low = 0x5555555555555555ULL, .high = 0x5555555555555555ULL}};
    decimal128_fallback d128_d = {.words = {.low = 0x2222222222222222ULL, .high = 0x2222222222222222ULL}};
    decimal128_fallback d128_e = {.words = {.low = 0x3333333333333333ULL, .high = 0x3333333333333333ULL}};
    decimal128_fallback d128_f = {.words = {.low = 0x1111111111111111ULL, .high = 0x1111111111111111ULL}};
#endif

#if HAS_COMPLEX
    long double _Complex ca = 1.0L + 2.0L * I;
    long double _Complex cb = 3.0L - 4.0L * I;
    long double _Complex cc = 5.0L + 6.0L * I;
    long double _Complex cd = 7.0L - 8.0L * I;
#else
    complex_fallback ca = {1.0L, 2.0L};
    complex_fallback cb = {3.0L, -4.0L};
    complex_fallback cc = {5.0L, 6.0L};
    complex_fallback cd = {7.0L, -8.0L};
#endif

    /* Initialize vectors */
    int32x8_simd vec_int;
    float64x4_simd vec_fp;
    for (int i = 0; i < 8; i++) {
        vec_int.data[i] = seed + i * 1000;
        if (i < 4) {
            vec_fp.data[i] = (double)(seed + i * 100) / 7.0;
        }
    }
    
    /* Main computation loop */
    long double results[5] = {0};
    
    for (int iter = 0; iter < 3; iter++) {
        volatile int condition = seed + iter; /* Prevent constant folding */
        
        if (condition % 2 == 0) {
            /* Branch 1: DFP operations */
#if HAS_DFP
            _Decimal128 dfp_result = dfp_complex_operation(
                d128_a, d128_b, d128_c, d128_d, d128_e, d128_f);
            results[0] += (long double)dfp_result;
#else
            long double dfp_result = dfp_complex_operation_fallback(
                d128_a, d128_b, d128_c, d128_d, d128_e, d128_f);
            results[0] += dfp_result;
#endif
        } else {
            /* Branch 2: Complex operations */
#if HAS_COMPLEX
            long double _Complex c_result = complex_high_precision(ca, cb, cc, cd);
            results[1] += creall(c_result) + cimagl(c_result);
#else
            complex_fallback c_result = complex_high_precision_fallback(ca, cb, cc, cd);
            results[1] += c_result.real + c_result.imag;
#endif
        }
        
        /* Vector reductions (always executed) */
        long long vec_sum = vector_reduction(vec_int);
        double fp_sum = vector_fp_reduction(vec_fp);
        results[2] += (long double)vec_sum + (long double)fp_sum;
        
        /* Call helper functions with many arguments */
        long double base = (long double)(seed + iter) / 100.0L;
        if (condition % 3 == 0) {
            /* 10-argument call */
            long double h10 = helper_10_args(
                base + 1.0L, base + 2.0L, base + 3.0L,
                base + 4.0L, base + 5.0L, base + 6.0L,
                base + 7.0L, base + 8.0L, base + 9.0L,
                base + 10.0L);
            results[3] += h10;
        } else {
            /* 11-argument call */
            long double h11 = helper_11_args(
                base + 1.0L, base + 2.0L, base + 3.0L,
                base + 4.0L, base + 5.0L, base + 6.0L,
                base + 7.0L, base + 8.0L, base + 9.0L,
                base + 10.0L, base + 11.0L);
            results[4] += h11;
        }
        
        /* Modify inputs slightly for next iteration */
        for (int i = 0; i < 8; i++) {
            vec_int.data[i] += 1;
            if (i < 4) {
                vec_fp.data[i] += 0.1;
            }
        }
    }
    
    /* Aggregate results and compute checksum */
    long double checksum = 0.0L;
    for (int i = 0; i < 5; i++) {
        checksum += results[i];
        global_accumulator += results[i]; /* Prevent dead code elimination */
    }
    
    /* Print deterministic result */
    printf("Checksum: %.15Lf\n", checksum);
    printf("Accumulator: %.15Lf\n", (long double)global_accumulator);
    
    return 0;
}
