#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Feature detection and fallbacks */
#ifdef __DECIMAL_BID_FORMAT__
    #define DFP_SUPPORTED 1
    #include <decimalfp.h>
#else
    #define DFP_SUPPORTED 0
#endif

#ifdef __STDC_IEC_559_COMPLEX__
    #include <complex.h>
    #define COMPLEX_SUPPORTED 1
#else
    #define COMPLEX_SUPPORTED 0
#endif

/* Vector type definitions */
#if defined(__GNUC__) && (defined(__x86_64__) || defined(__i386__))
    typedef int32_t v8si __attribute__((vector_size(32)));
    typedef double v4df __attribute__((vector_size(32)));
    #define VECTOR_SUPPORTED 1
#elif defined(__GNUC__) && defined(__aarch64__)
    typedef int32_t v4si __attribute__((vector_size(16)));
    typedef float v4sf __attribute__((vector_size(16)));
    #define VECTOR_SUPPORTED 1
#else
    #define VECTOR_SUPPORTED 0
#endif

/* DFP fallback using integer arrays */
#if !DFP_SUPPORTED
typedef struct {
    uint64_t lo;
    uint64_t hi;
} decimal64_fb;

typedef struct {
    uint64_t w[2];
} decimal128_fb;
#endif

/* Complex fallback */
#if !COMPLEX_SUPPORTED
typedef struct {
    long double re;
    long double im;
} complex_fb;
#endif

/* Vector fallback */
#if !VECTOR_SUPPORTED
typedef struct {
    int32_t data[8];
} v8si_fb;

typedef struct {
    double data[4];
} v4df_fb;
#endif

/* Helper function with 11 arguments - marked noinline to prevent optimization */
static __attribute__((noinline)) 
long double helper_11_args(
    #if DFP_SUPPORTED
    _Decimal64 d64_1, _Decimal64 d64_2,
    _Decimal128 d128_1, _Decimal128 d128_2,
    #else
    decimal64_fb d64_1, decimal64_fb d64_2,
    decimal128_fb d128_1, decimal128_fb d128_2,
    #endif
    #if COMPLEX_SUPPORTED
    long double _Complex c1, long double _Complex c2,
    #else
    complex_fb c1, complex_fb c2,
    #endif
    #if VECTOR_SUPPORTED
    v8si v1, v4df v2,
    #else
    v8si_fb v1, v4df_fb v2,
    #endif
    int scalar1, long double scalar2
) {
    long double result = 0.0L;
    
    /* Combine all arguments in a way that uses many operands */
    #if DFP_SUPPORTED
    /* DFP operations that may expand to multi-operand RTL */
    _Decimal128 d128_tmp = d128_1 + d128_2;
    _Decimal64 d64_tmp = d64_1 * d64_2;
    result += (long double)d128_tmp + (long double)d64_tmp;
    #else
    /* Fallback: simple integer arithmetic */
    result += (long double)d64_1.lo + (long double)d64_2.hi;
    result += (long double)d128_1.w[0] + (long double)d128_2.w[1];
    #endif
    
    #if COMPLEX_SUPPORTED
    /* Complex operations */
    long double _Complex c_tmp = c1 * c2;
    result += creall(c_tmp) + cimagl(c_tmp);
    #else
    result += c1.re + c2.im;
    #endif
    
    #if VECTOR_SUPPORTED
    /* Vector reduction */
    for (int i = 0; i < 8; i++) result += v1[i];
    for (int i = 0; i < 4; i++) result += v2[i];
    #else
    for (int i = 0; i < 8; i++) result += v1.data[i];
    for (int i = 0; i < 4; i++) result += v2.data[i];
    #endif
    
    result += scalar1 + scalar2;
    return result;
}

/* Another helper with 10 arguments */
static __attribute__((noinline))
long double helper_10_args(
    #if DFP_SUPPORTED
    _Decimal128 a1, _Decimal128 a2, _Decimal128 a3,
    _Decimal64 b1, _Decimal64 b2,
    #else
    decimal128_fb a1, decimal128_fb a2, decimal128_fb a3,
    decimal64_fb b1, decimal64_fb b2,
    #endif
    #if COMPLEX_SUPPORTED
    long double _Complex c1, long double _Complex c2,
    #else
    complex_fb c1, complex_fb c2,
    #endif
    int i1, int i2
) {
    long double result = 0.0L;
    
    #if DFP_SUPPORTED
    /* Complex DFP expression that may need many operands */
    _Decimal128 d128_result = (a1 * a2) + (a3 / (_Decimal128)b1) - a2;
    _Decimal64 d64_result = b1 * b2 + b1 / b2;
    result = (long double)d128_result + (long double)d64_result;
    #else
    result = (long double)a1.w[0] + (long double)a2.w[1] + (long double)a3.w[0];
    result += (long double)b1.lo + (long double)b2.hi;
    #endif
    
    #if COMPLEX_SUPPORTED
    long double _Complex c_result = (c1 * c2) / (c1 + c2);
    result += creall(c_result) * cimagl(c_result);
    #else
    result += c1.re * c2.im;
    #endif
    
    result += i1 * i2;
    return result;
}

/* Volatile storage to prevent dead code elimination */
static volatile long double result_storage[10];
static volatile int storage_idx = 0;

int main(int argc, char *argv[]) {
    int seed = 42;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    srand(seed);
    
    /* Initialize base variables */
    #if DFP_SUPPORTED
    _Decimal64 d64_a = 1.23456789e10DL;
    _Decimal64 d64_b = 9.87654321e9DL;
    _Decimal64 d64_c = 5.55555555e8DL;
    _Decimal64 d64_d = 2.22222222e7DL;
    
    _Decimal128 d128_a = 1.2345678901234567890123456789e20DL;
    _Decimal128 d128_b = 9.8765432109876543210987654321e19DL;
    _Decimal128 d128_c = 5.5555555555555555555555555555e18DL;
    _Decimal128 d128_d = 2.2222222222222222222222222222e17DL;
    #else
    decimal64_fb d64_a = {12345678900ULL, 0};
    decimal64_fb d64_b = {9876543210ULL, 0};
    decimal64_fb d64_c = {555555555ULL, 0};
    decimal64_fb d64_d = {22222222ULL, 0};
    
    decimal128_fb d128_a = {{12345678901234567890ULL, 12345ULL}};
    decimal128_fb d128_b = {{98765432109876543210ULL, 9876ULL}};
    decimal128_fb d128_c = {{5555555555555555555ULL, 555ULL}};
    decimal128_fb d128_d = {{2222222222222222222ULL, 222ULL}};
    #endif
    
    #if COMPLEX_SUPPORTED
    long double _Complex ca = 1.0L + 2.0L * I;
    long double _Complex cb = 3.0L - 4.0L * I;
    long double _Complex cc = 5.0L + 6.0L * I;
    long double _Complex cd = 7.0L - 8.0L * I;
    #else
    complex_fb ca = {1.0L, 2.0L};
    complex_fb cb = {3.0L, -4.0L};
    complex_fb cc = {5.0L, 6.0L};
    complex_fb cd = {7.0L, -8.0L};
    #endif
    
    #if VECTOR_SUPPORTED
    v8si vec_int = {1, 2, 3, 4, 5, 6, 7, 8};
    v4df vec_double = {1.1, 2.2, 3.3, 4.4};
    #else
    v8si_fb vec_int = {{1, 2, 3, 4, 5, 6, 7, 8}};
    v4df_fb vec_double = {{1.1, 2.2, 3.3, 4.4}};
    #endif
    
    long double accumulator = 0.0L;
    
    /* Main computation loop - 4 iterations */
    for (int iter = 0; iter < 4; iter++) {
        long double iter_result = 0.0L;
        
        /* Conditional execution based on volatile variable */
        volatile int condition = iter % 2;
        
        if (condition) {
            /* DFP arithmetic with complex expressions */
            #if DFP_SUPPORTED
            _Decimal128 d128_result = d128_a * d128_b + d128_c / d128_d;
            _Decimal64 d64_result = d64_a * d64_b + d64_c / d64_d;
            iter_result += (long double)d128_result + (long double)d64_result;
            #else
            iter_result += (long double)d128_a.w[0] * (long double)d128_b.w[1] +
                          (long double)d128_c.w[0] / (long double)d128_d.w[1];
            iter_result += (long double)d64_a.lo * (long double)d64_b.hi +
                          (long double)d64_c.lo / (long double)d64_d.hi;
            #endif
        } else {
            /* Complex arithmetic */
            #if COMPLEX_SUPPORTED
            long double _Complex cl_result = (ca * cb) / (cc - cd);
            iter_result += creall(cl_result) + cimagl(cl_result);
            #else
            iter_result += ca.re * cb.re + cc.im - cd.im;
            #endif
        }
        
        /* Vector reduction with accumulation */
        #if VECTOR_SUPPORTED
        long double vec_sum = 0.0L;
        for (int i = 0; i < 8; i++) vec_sum += vec_int[i];
        for (int i = 0; i < 4; i++) vec_sum += vec_double[i];
        iter_result += vec_sum;
        #else
        for (int i = 0; i < 8; i++) iter_result += vec_int.data[i];
        for (int i = 0; i < 4; i++) iter_result += vec_double.data[i];
        #endif
        
        /* Call helper functions with many arguments */
        long double helper1_result = helper_11_args(
            #if DFP_SUPPORTED
            d64_a, d64_b, d128_a, d128_b,
            #else
            d64_a, d64_b, d128_a, d128_b,
            #endif
            #if COMPLEX_SUPPORTED
            ca, cb,
            #else
            ca, cb,
            #endif
            #if VECTOR_SUPPORTED
            vec_int, vec_double,
            #else
            vec_int, vec_double,
            #endif
            iter, iter_result
        );
        
        long double helper2_result = helper_10_args(
            #if DFP_SUPPORTED
            d128_a, d128_b, d128_c, d64_a, d64_b,
            #else
            d128_a, d128_b, d128_c, d64_a, d64_b,
            #endif
            #if COMPLEX_SUPPORTED
            cc, cd,
            #else
            cc, cd,
            #endif
            seed, iter
        );
        
        iter_result += helper1_result + helper2_result;
        
        /* Store result to volatile memory */
        result_storage[storage_idx++ % 10] = iter_result;
        accumulator += iter_result;
        
        /* Modify some inputs for next iteration */
        #if DFP_SUPPORTED
        d64_a += 1.0e5DL;
        d128_b -= 1.0e15DL;
        #else
        d64_a.lo += 100000;
        d128_b.w[0] -= 1000000000000000ULL;
        #endif
        
        #if COMPLEX_SUPPORTED
        ca += 0.1L + 0.2L * I;
        #else
        ca.re += 0.1L;
        ca.im += 0.2L;
        #endif
        
        #if VECTOR_SUPPORTED
        vec_int += (v8si){1, 1, 1, 1, 1, 1, 1, 1};
        #else
        for (int i = 0; i < 8; i++) vec_int.data[i] += 1;
        #endif
    }
    
    /* Compute checksum */
    long double checksum = accumulator;
    for (int i = 0; i < 10 && i < storage_idx; i++) {
        checksum += result_storage[i];
    }
    
    printf("Result checksum: %Lf\n", checksum);
    return 0;
}
