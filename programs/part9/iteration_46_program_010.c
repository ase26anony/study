#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Feature detection and fallbacks */
#ifdef __DECIMAL_BID_FORMAT__
    #define DFP_SUPPORTED 1
    #include <decimal/decimal.h>
#else
    #define DFP_SUPPORTED 0
#endif

#ifdef __STDC_IEC_559_COMPLEX__
    #include <complex.h>
    #define COMPLEX_SUPPORTED 1
#else
    #define COMPLEX_SUPPORTED 0
#endif

/* Vector type fallbacks */
#if defined(__VECTOR_TYPES_SUPPORTED__) || defined(__GNUC__)
    #define VECTOR_SUPPORTED 1
    typedef int32_t int32x8_t __attribute__((vector_size(32)));
    typedef double float64x4_t __attribute__((vector_size(32)));
#else
    #define VECTOR_SUPPORTED 0
    typedef struct { int32_t data[8]; } int32x8_t;
    typedef struct { double data[4]; } float64x4_t;
#endif

/* DFP fallback using unions for software emulation */
#if !DFP_SUPPORTED
typedef union {
    unsigned long long ull[2];
    double dbl[2];
} decimal64_fallback;

typedef union {
    unsigned long long ull[4];
    double dbl[4];
} decimal128_fallback;

#define DECIMAL64(x) ((decimal64_fallback){.ull = {x, 0}})
#define DECIMAL128(x) ((decimal128_fallback){.ull = {x, 0, 0, 0}})
#define DFP_ADD(a, b) add_fallback(&a, &b)
#define DFP_MUL(a, b) mul_fallback(&a, &b)
#define DFP_DIV(a, b) div_fallback(&a, &b)

static decimal128_fallback add_fallback(const decimal128_fallback *a, 
                                       const decimal128_fallback *b) {
    decimal128_fallback result;
    for (int i = 0; i < 4; i++) {
        result.ull[i] = a->ull[i] + b->ull[i];
    }
    return result;
}

static decimal128_fallback mul_fallback(const decimal128_fallback *a,
                                       const decimal128_fallback *b) {
    decimal128_fallback result;
    result.ull[0] = a->ull[0] * b->ull[0];
    result.ull[1] = a->ull[1] * b->ull[1];
    result.ull[2] = a->ull[2] * b->ull[2];
    result.ull[3] = a->ull[3] * b->ull[3];
    return result;
}

static decimal128_fallback div_fallback(const decimal128_fallback *a,
                                       const decimal128_fallback *b) {
    decimal128_fallback result;
    for (int i = 0; i < 4; i++) {
        if (b->ull[i] != 0) {
            result.ull[i] = a->ull[i] / b->ull[i];
        } else {
            result.ull[i] = a->ull[i];
        }
    }
    return result;
}
#else
    #define DECIMAL64(x) (x##dl)
    #define DECIMAL128(x) (x##dl)
    #define DFP_ADD(a, b) __builtin_dadd(a, b)
    #define DFP_MUL(a, b) __builtin_dmul(a, b)
    #define DFP_DIV(a, b) __builtin_ddiv(a, b)
#endif

/* Complex number fallback */
#if !COMPLEX_SUPPORTED
typedef struct {
    double real;
    double imag;
} complex_fallback;

#define COMPLEX(r, i) ((complex_fallback){r, i})
#define C_ADD(a, b) ((complex_fallback){a.real + b.real, a.imag + b.imag})
#define C_MUL(a, b) ((complex_fallback){ \
    a.real * b.real - a.imag * b.imag, \
    a.real * b.imag + a.imag * b.real})
#define C_DIV(a, b) { \
    double denom = b.real * b.real + b.imag * b.imag; \
    if (denom != 0.0) { \
        a.real = (a.real * b.real + a.imag * b.imag) / denom; \
        a.imag = (a.imag * b.real - a.real * b.imag) / denom; \
    }}
#else
    #define COMPLEX(r, i) ((r) + (i) * I)
    #define C_ADD(a, b) ((a) + (b))
    #define C_MUL(a, b) ((a) * (b))
    #define C_DIV(a, b) ((a) / (b))
#endif

/* Vector operations fallback */
#if !VECTOR_SUPPORTED
static int32_t vector_reduce_sum_int32(int32x8_t v) {
    int32_t sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += v.data[i];
    }
    return sum;
}

static double vector_reduce_sum_float64(float64x4_t v) {
    double sum = 0.0;
    for (int i = 0; i < 4; i++) {
        sum += v.data[i];
    }
    return sum;
}
#else
static int32_t vector_reduce_sum_int32(int32x8_t v) {
    int32_t result = 0;
    for (int i = 0; i < 8; i++) {
        result += v[i];
    }
    return result;
}

static double vector_reduce_sum_float64(float64x4_t v) {
    double result = 0.0;
    for (int i = 0; i < 4; i++) {
        result += v[i];
    }
    return result;
}
#endif

/* Helper function with 11 arguments - marked noinline to prevent optimization */
static __attribute__((noinline)) 
long double helper_11_args(
    #if DFP_SUPPORTED
    _Decimal128 d1, _Decimal128 d2, _Decimal128 d3,
    #else
    decimal128_fallback d1, decimal128_fallback d2, decimal128_fallback d3,
    #endif
    #if COMPLEX_SUPPORTED
    long double _Complex c1, long double _Complex c2,
    #else
    complex_fallback c1, complex_fallback c2,
    #endif
    int32x8_t v1, float64x4_t v2,
    int a, int b, int c) {
    
    /* Combine all arguments into a single result */
    long double result = 0.0L;
    
    /* Process DFP arguments */
    #if DFP_SUPPORTED
    result += (long double)__builtin_dadd(d1, d2);
    result += (long double)__builtin_dmul(d2, d3);
    #else
    result += (long double)d1.ull[0];
    result += (long double)d2.ull[0];
    result += (long double)d3.ull[0];
    #endif
    
    /* Process complex arguments */
    #if COMPLEX_SUPPORTED
    result += creall(c1) + cimagl(c1);
    result += creall(c2) + cimagl(c2);
    #else
    result += c1.real + c1.imag;
    result += c2.real + c2.imag;
    #endif
    
    /* Process vector arguments */
    result += vector_reduce_sum_int32(v1);
    result += vector_reduce_sum_float64(v2);
    
    /* Process integer arguments */
    result += a + b + c;
    
    return result;
}

/* Another helper with 10 arguments */
static __attribute__((noinline))
double helper_10_args(
    double a1, double a2, double a3, double a4, double a5,
    double a6, double a7, double a8, double a9, double a10) {
    
    /* Complex expression that may expand to many operands */
    return ((((a1 * a2) + (a3 / a4)) - (a5 * a6)) + 
            ((a7 - a8) * (a9 + a10))) / 
           ((a1 + a2 + a3 + a4 + a5) * 0.5);
}

/* Volatile global to prevent dead code elimination */
static volatile double global_accumulator[10];

int main(int argc, char *argv[]) {
    /* Use command line seed for deterministic behavior */
    int seed = 42;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    srand(seed);
    
    /* Initialize base variables */
    #if DFP_SUPPORTED
    _Decimal128 d128_a = DECIMAL128(1.23456789);
    _Decimal128 d128_b = DECIMAL128(2.34567891);
    _Decimal128 d128_c = DECIMAL128(3.45678912);
    _Decimal128 d128_d = DECIMAL128(4.56789123);
    #else
    decimal128_fallback d128_a = DECIMAL128(123456789ULL);
    decimal128_fallback d128_b = DECIMAL128(234567891ULL);
    decimal128_fallback d128_c = DECIMAL128(345678912ULL);
    decimal128_fallback d128_d = DECIMAL128(456789123ULL);
    #endif
    
    #if COMPLEX_SUPPORTED
    long double _Complex ca = COMPLEX(1.5L, 2.5L);
    long double _Complex cb = COMPLEX(3.5L, 4.5L);
    long double _Complex cc = COMPLEX(5.5L, 6.5L);
    long double _Complex cd = COMPLEX(7.5L, 8.5L);
    #else
    complex_fallback ca = COMPLEX(1.5, 2.5);
    complex_fallback cb = COMPLEX(3.5, 4.5);
    complex_fallback cc = COMPLEX(5.5, 6.5);
    complex_fallback cd = COMPLEX(7.5, 8.5);
    #endif
    
    /* Initialize vectors */
    int32x8_t vec_int;
    float64x4_t vec_double;
    
    #if VECTOR_SUPPORTED
    for (int i = 0; i < 8; i++) vec_int[i] = seed + i;
    for (int i = 0; i < 4; i++) vec_double[i] = seed * 0.1 + i;
    #else
    for (int i = 0; i < 8; i++) vec_int.data[i] = seed + i;
    for (int i = 0; i < 4; i++) vec_double.data[i] = seed * 0.1 + i;
    #endif
    
    /* Main computation loop */
    double checksum = 0.0;
    volatile int condition = seed % 2; /* Prevent constant folding */
    
    for (int iter = 0; iter < 4; iter++) {
        double loop_result = 0.0;
        
        /* Conditional execution to prevent optimization */
        if (condition || iter % 2 == 0) {
            /* DFP arithmetic with complex expressions */
            #if DFP_SUPPORTED
            _Decimal128 d128_result = __builtin_dadd(
                __builtin_dmul(d128_a, d128_b),
                __builtin_ddiv(d128_c, d128_d));
            loop_result += (double)d128_result;
            #else
            decimal128_fallback d128_result = DFP_ADD(
                DFP_MUL(d128_a, d128_b),
                DFP_DIV(d128_c, d128_d));
            loop_result += (double)d128_result.ull[0];
            #endif
            
            /* Complex arithmetic */
            #if COMPLEX_SUPPORTED
            long double _Complex cl_result = C_DIV(
                C_MUL(ca, cb),
                C_ADD(cc, cd));
            loop_result += creall(cl_result) + cimagl(cl_result);
            #else
            complex_fallback cl_result = C_MUL(ca, cb);
            C_DIV(cl_result, C_ADD(cc, cd));
            loop_result += cl_result.real + cl_result.imag;
            #endif
            
            /* Vector reduction with accumulation */
            int32_t vec_sum_int = vector_reduce_sum_int32(vec_int);
            double vec_sum_double = vector_reduce_sum_float64(vec_double);
            loop_result += vec_sum_int + vec_sum_double;
            
            /* Call helper with 11 arguments */
            long double helper1_result = helper_11_args(
                #if DFP_SUPPORTED
                d128_a, d128_b, d128_result,
                #else
                d128_a, d128_b, d128_result,
                #endif
                #if COMPLEX_SUPPORTED
                ca, cl_result,
                #else
                ca, cl_result,
                #endif
                vec_int, vec_double,
                seed, iter, vec_sum_int);
            loop_result += (double)helper1_result;
        } else {
            /* Alternative path with different operations */
            double temp[10];
            for (int i = 0; i < 10; i++) {
                temp[i] = seed * 0.5 + i + iter;
            }
            
            /* Call helper with 10 arguments */
            double helper2_result = helper_10_args(
                temp[0], temp[1], temp[2], temp[3], temp[4],
                temp[5], temp[6], temp[7], temp[8], temp[9]);
            loop_result += helper2_result;
        }
        
        /* Store result to prevent elimination */
        global_accumulator[iter % 10] = loop_result;
        checksum += loop_result;
        
        /* Modify variables for next iteration */
        #if DFP_SUPPORTED
        d128_a = __builtin_dadd(d128_a, DECIMAL128(0.1));
        #else
        d128_a.ull[0] += 100000000ULL;
        #endif
        
        #if COMPLEX_SUPPORTED
        ca += 0.1L + 0.1L * I;
        #else
        ca.real += 0.1;
        ca.imag += 0.1;
        #endif
        
        #if VECTOR_SUPPORTED
        for (int i = 0; i < 8; i++) vec_int[i] += iter;
        for (int i = 0; i < 4; i++) vec_double[i] += iter * 0.01;
        #else
        for (int i = 0; i < 8; i++) vec_int.data[i] += iter;
        for (int i = 0; i < 4; i++) vec_double.data[i] += iter * 0.01;
        #endif
    }
    
    /* Final checksum computation and output */
    printf("Checksum: %f\n", checksum);
    
    /* Additional verification */
    double final_sum = 0.0;
    for (int i = 0; i < 10; i++) {
        final_sum += global_accumulator[i];
    }
    printf("Accumulator sum: %f\n", final_sum);
    
    return 0;
}
