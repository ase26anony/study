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

/* Global volatile to prevent optimization */
volatile uint64_t g_result_store[10];
volatile int g_store_idx = 0;

/* Helper function with 11 arguments - marked noinline */
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

/* Another helper with 10 arguments */
static __attribute__((noinline))
double helper_10_args(
    double b1, double b2, double b3, double b4, double b5,
    double b6, double b7, double b8, double b9, double b10)
{
    /* Mix of operations to create operand pressure */
    return (((b1 + b2) * (b3 - b4)) / 
            ((b5 * b6) + (b7 / b8) - (b9 * b10)));
}

/* DFP fallback using integer arrays for 128-bit emulation */
typedef struct {
    uint64_t lo;
    uint64_t hi;
} dfp128_emu;

static dfp128_emu dfp128_add(dfp128_emu a, dfp128_emu b) {
    dfp128_emu result;
    result.lo = a.lo + b.lo;
    result.hi = a.hi + b.hi + (result.lo < a.lo);
    return result;
}

static dfp128_emu dfp128_mul(dfp128_emu a, dfp128_emu b) {
    /* Simplified multiplication for demonstration */
    dfp128_emu result;
    uint64_t a0 = a.lo & 0xFFFFFFFF;
    uint64_t a1 = a.lo >> 32;
    uint64_t a2 = a.hi & 0xFFFFFFFF;
    uint64_t a3 = a.hi >> 32;
    
    uint64_t b0 = b.lo & 0xFFFFFFFF;
    uint64_t b1 = b.lo >> 32;
    uint64_t b2 = b.hi & 0xFFFFFFFF;
    uint64_t b3 = b.hi >> 32;
    
    /* Multi-precision multiplication (simplified) */
    uint64_t p0 = a0 * b0;
    uint64_t p1 = a0 * b1 + a1 * b0;
    uint64_t p2 = a0 * b2 + a1 * b1 + a2 * b0;
    uint64_t p3 = a0 * b3 + a1 * b2 + a2 * b1 + a3 * b0;
    
    result.lo = p0 + (p1 << 32);
    result.hi = (p1 >> 32) + p2 + (p3 << 32);
    return result;
}

/* Vector reduction with accumulation */
#if HAS_VECTORS
static double vector_reduce_sum(v4df vec) {
    double sum = 0.0;
    for (int i = 0; i < 4; i++) {
        sum += vec[i];
    }
    return sum;
}

static int32_t vector_reduce_sum_int(v8si vec) {
    int32_t sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += vec[i];
    }
    return sum;
}
#endif

int main(int argc, char *argv[]) {
    int seed = 12345;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    srand(seed);
    
    /* Initialize base variables */
    double base_doubles[20];
    for (int i = 0; i < 20; i++) {
        base_doubles[i] = (double)rand() / RAND_MAX * 100.0;
    }
    
    long double base_long_doubles[10];
    for (int i = 0; i < 10; i++) {
        base_long_doubles[i] = (long double)rand() / RAND_MAX * 100.0L;
    }
    
    /* DFP or emulated DFP variables */
#if HAS_DFP
    _Decimal64 d64_a = 1.23456dd;
    _Decimal64 d64_b = 7.89012dd;
    _Decimal128 d128_a = 1.234567890123456dl;
    _Decimal128 d128_b = 9.876543210987654dl;
    _Decimal128 d128_c = 5.555555555555555dl;
    _Decimal128 d128_d = 2.222222222222222dl;
#else
    /* Use emulated DFP */
    dfp128_emu d128_a = { .lo = 0x123456789ABCDEF0ULL, .hi = 0x0FEDCBA987654321ULL };
    dfp128_emu d128_b = { .lo = 0x1111111111111111ULL, .hi = 0x2222222222222222ULL };
    dfp128_emu d128_c = { .lo = 0x3333333333333333ULL, .hi = 0x4444444444444444ULL };
    dfp128_emu d128_d = { .lo = 0x5555555555555555ULL, .hi = 0x6666666666666666ULL };
#endif
    
    /* Complex numbers */
#if HAS_COMPLEX
    long double _Complex cl_a = base_long_doubles[0] + base_long_doubles[1] * I;
    long double _Complex cl_b = base_long_doubles[2] + base_long_doubles[3] * I;
    long double _Complex cl_c = base_long_doubles[4] + base_long_doubles[5] * I;
    long double _Complex cl_d = base_long_doubles[6] + base_long_doubles[7] * I;
#endif
    
    /* Vector variables */
#if HAS_VECTORS
    v4df vec_double = { base_doubles[0], base_doubles[1], 
                        base_doubles[2], base_doubles[3] };
    v8si vec_int = { (int32_t)seed, (int32_t)seed+1, (int32_t)seed+2, 
                     (int32_t)seed+3, (int32_t)seed+4, (int32_t)seed+5,
                     (int32_t)seed+6, (int32_t)seed+7 };
#endif
    
    /* Main computation loop */
    for (int iter = 0; iter < 3; iter++) {
        volatile int condition = seed % 2;
        long double result = 0.0L;
        
        /* Conditional execution to prevent constant folding */
        if (condition) {
            /* DFP arithmetic - may expand to many operands */
#if HAS_DFP
            /* Use DFP builtins that may expand to multi-operand patterns */
            _Decimal128 d128_result = d128_a * d128_b + d128_c / d128_d;
            /* Convert to double for further use */
            result += (long double)d128_result;
#else
            /* Emulated DFP operations */
            dfp128_emu temp1 = dfp128_mul(d128_a, d128_b);
            dfp128_emu temp2 = dfp128_add(temp1, d128_c);
            result += (long double)(temp2.lo + temp2.hi);
#endif
            
            /* Complex arithmetic */
#if HAS_COMPLEX
            long double _Complex cl_result = (cl_a * cl_b) / (cl_c - cl_d);
            /* Mix with library function call */
            long double _Complex cl_sqrt = csqrt(cl_result);
            result += creall(cl_sqrt) + cimagl(cl_sqrt);
#endif
        } else {
            /* Alternative path with different operations */
#if HAS_DFP
            _Decimal128 d128_result = d128_c * d128_d - d128_a / d128_b;
            result += (long double)d128_result * 2.0L;
#else
            dfp128_emu temp1 = dfp128_mul(d128_c, d128_d);
            result += (long double)(temp1.lo + temp1.hi) * 2.0L;
#endif
            
#if HAS_COMPLEX
            long double _Complex cl_result = (cl_c + cl_d) * (cl_a - cl_b);
            long double _Complex cl_pow = cpow(cl_result, 0.5L + 0.5L * I);
            result += creall(cl_pow) * cimagl(cl_pow);
#endif
        }
        
        /* Vector reduction */
#if HAS_VECTORS
        double vec_sum = vector_reduce_sum(vec_double);
        int32_t int_sum = vector_reduce_sum_int(vec_int);
        result += vec_sum + int_sum;
        
        /* Update vectors for next iteration */
        for (int i = 0; i < 4; i++) {
            vec_double[i] += 0.1 * i;
        }
        for (int i = 0; i < 8; i++) {
            vec_int[i] += iter;
        }
#endif
        
        /* Call helper functions with many arguments */
        long double helper1_result = helper_11_args(
            base_long_doubles[0] + iter,
            base_long_doubles[1] - iter,
            base_long_doubles[2] * (1.0L + iter * 0.1L),
            base_long_doubles[3] / (1.0L + iter * 0.05L),
            base_long_doubles[4],
            base_long_doubles[5],
            base_long_doubles[6],
            base_long_doubles[7],
            base_long_doubles[8],
            base_long_doubles[9],
            result
        );
        
        double helper2_result = helper_10_args(
            base_doubles[0] + iter,
            base_doubles[1] - iter,
            base_doubles[2] * (1.0 + iter * 0.1),
            base_doubles[3] / (1.0 + iter * 0.05),
            base_doubles[4],
            base_doubles[5],
            base_doubles[6],
            base_doubles[7],
            base_doubles[8],
            base_doubles[9]
        );
        
        /* Combine results */
        long double final_result = result + helper1_result + helper2_result;
        
        /* Store to volatile global to prevent elimination */
        if (g_store_idx < 10) {
            g_result_store[g_store_idx++] = (uint64_t)final_result;
        }
    }
    
    /* Compute checksum */
    uint64_t checksum = 0;
    for (int i = 0; i < g_store_idx; i++) {
        checksum += g_result_store[i];
    }
    
    printf("Checksum: %llu\n", (unsigned long long)checksum);
    return 0;
}
