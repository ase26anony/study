/* Compile with: gcc -O2 -std=c23 -Wno-psabi -fdump-tree-original fixed-value-test.c -o fixed-value-test */

#include <stdio.h>

/* Fixed-point type definitions */
typedef _Fract fract_t;
typedef _Accum accum_t;
typedef unsigned _Fract ufract_t;
typedef unsigned _Accum uaccum_t;
typedef _Sat _Fract sat_fract_t;
typedef _Sat _Accum sat_accum_t;
typedef unsigned _Sat _Fract sat_ufract_t;
typedef unsigned _Sat _Accum sat_uaccum_t;

/* Struct with mixed fixed-point types */
struct fixed_mix {
    short _Fract sf;
    long _Accum la;
    unsigned _Sat _Fract usf;
    _Sat long long _Accum slla;
};

/* Array initialized with fixed-point constants at boundaries */
static const fract_t fract_array[] = {
    0.999999r,  /* Near max for signed fract */
    -0.999999r, /* Near min for signed fract */
    0.5r,
    -0.5r,
    0.0r
};

/* Compile-time constant expressions that should trigger range checks */
#define MAX_UFRACT ((ufract_t)0.999999r)
#define MIN_SFRACT ((short _Fract)-0.999999r)
#define MAX_ACCUM ((accum_t)32767.999999k)
#define MIN_ACCUM ((accum_t)-32768.000000k)

/* Force constant folding with ternary operator */
static const accum_t folded_const = 
    (__builtin_constant_p(MAX_ACCUM) && __builtin_constant_p(MIN_ACCUM)) ? 
    (MAX_ACCUM + MIN_ACCUM) : 0k;

/* Function to test overflow with saturation */
static sat_accum_t test_saturation(sat_accum_t a, sat_accum_t b) {
    /* These operations should trigger saturation logic */
    sat_accum_t sum = a + b;
    sat_accum_t prod = a * b;
    sat_accum_t shifted = a << 2;  /* Left shift may overflow */
    
    return sum + prod + shifted;
}

/* Function to test range boundaries */
static int test_range_boundaries(void) {
    int results = 0;
    
    /* Test 1: Operations near maximum representable values */
    uaccum_t max_uaccum = (uaccum_t)4294967295.999999lk;
    uaccum_t near_max = max_uaccum - (uaccum_t)0.000001lk;
    
    /* This addition should approach/overflow boundary */
    uaccum_t test1 = near_max + (uaccum_t)0.000002lk;
    
    /* Test 2: Operations near minimum representable values */
    accum_t min_accum = (accum_t)-32768.000000k;
    accum_t near_min = min_accum + (accum_t)0.000001k;
    
    /* This subtraction should approach/underflow boundary */
    accum_t test2 = near_min - (accum_t)0.000002k;
    
    /* Test 3: Mixed-type conversions at boundaries */
    fract_t max_fract = (fract_t)0.999999r;
    int as_int = (int)(max_fract * 1000);  /* Conversion should check range */
    
    /* Test 4: Saturation arithmetic with overflow */
    sat_uaccum_t sat_max = (sat_uaccum_t)4294967295.999999lk;
    sat_uaccum_t sat_add = sat_max + (sat_uaccum_t)1.0lk;  /* Should saturate */
    
    /* Test 5: Compile-time constant expression evaluation */
    #ifdef __OPTIMIZE__
    static const long _Accum compile_time_check = 
        (MAX_ACCUM > MIN_ACCUM) ? MAX_ACCUM : MIN_ACCUM;
    #endif
    
    /* Use results to prevent dead code elimination */
    results += (int)(test1 * 0.000001lk);
    results += (int)(test2 * 0.000001k);
    results += as_int;
    results += (int)(sat_add * 0.000001lk);
    
    return results;
}

int main(void) {
    volatile int dummy = 0;  /* Prevent optimization */
    int total_results = 0;
    
    /* Initialize struct with boundary values */
    struct fixed_mix fm = {
        .sf = MIN_SFRACT,
        .la = (long _Accum)9223372036854775807.999999llk,
        .usf = MAX_UFRACT,
        .slla = (sat long long _Accum)-9223372036854775808.000000llk
    };
    
    /* Test array access with fixed-point derived indices */
    for (int i = 0; i < 3; i++) {  /* Small loop for unrolling */
        /* Create value flow with conditional assignments */
        fract_t x = fract_array[i];
        
        if (x > (fract_t)0.0r) {
            x = x * (fract_t)1.5r;  /* May overflow for values > 0.666r */
        } else {
            x = x * (fract_t)-1.5r; /* May overflow for values < -0.666r */
        }
        
        /* Convert to integer with range check */
        int idx = (int)(x * 100);
        if (idx < 0) idx = 0;
        if (idx > 99) idx = 99;
        
        dummy += idx;  /* Use result */
        
        /* Test saturation arithmetic in loop */
        sat_ufract_t sat_val = (sat_ufract_t)0.8r;
        for (int j = 0; j < 2; j++) {
            sat_val = sat_val + (sat_ufract_t)0.3r;  /* Should saturate at 0.999999r */
        }
        dummy += (int)(sat_val * 1000);
    }
    
    /* Test conversions that should trigger range calculations */
    accum_t large_accum = (accum_t)30000.5k;
    fract_t as_fract = (fract_t)large_accum;  /* Conversion with potential overflow */
    
    /* Test shift operations that may overflow */
    uaccum_t shift_test = (uaccum_t)2147483647.999999lk;
    shift_test = shift_test << 1;  /* Likely overflow */
    
    /* Force evaluation of compile-time constant */
    #ifdef __OPTIMIZE__
    if (__builtin_constant_p(folded_const)) {
        dummy += (int)(folded_const * 0.001k);
    }
    #endif
    
    /* Test saturation function with boundary values */
    sat_accum_t sat_result = test_saturation(
        (sat_accum_t)32767.0k,
        (sat_accum_t)32767.0k
    );
    dummy += (int)(sat_result * 0.001k);
    
    /* Run range boundary tests */
    total_results = test_range_boundaries();
    
    /* Print something to prevent complete optimization */
    printf("Result: %d (dummy: %d)\n", total_results, dummy);
    
    return 0;
}
