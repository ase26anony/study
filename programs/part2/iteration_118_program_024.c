/* fixed-point-coverage.c */
/* Compile with: gcc -O3 -std=c23 -fdump-tree-optimized fixed-point-coverage.c */

#include <stdio.h>

/* Force compile-time evaluation with constexpr */
#define EVAL_CONST(expr) (__builtin_constant_p(expr) ? (expr) : (expr))

/* Struct with mixed fixed-point types */
struct FixedPointData {
    unsigned short _Fract usf;
    signed _Fract sf;
    unsigned _Sat _Fract usatf;
    long _Accum la;
    unsigned long _Sat _Accum ulsata;
};

/* Array initialization with fixed-point constants */
static const struct FixedPointData init_data[] = {
    {0.99999hr, -0.99999r, 0.99999ur, 9223372036854775.5807lk, 18446744073709551.1615ulk},
    {0.00001hr, -0.00001r, 0.00001ur, -9223372036854775.5807lk, 0.00000ulk},
    {0.5hr, -0.5r, 0.5ur, 0.0lk, 0.5ulk}
};

/* Function to trigger range checks through conversions */
static inline long test_conversion(_Accum a) {
    /* This conversion should trigger range checking */
    return (long)a;
}

/* Function using ternary operator for constant folding */
static unsigned _Sat _Fract saturate_add(unsigned _Sat _Fract a, unsigned _Sat _Fract b) {
    /* Force constant folding with ternary */
    return EVAL_CONST(a + b > 0.99999ur ? 0.99999ur : a + b);
}

int main(void) {
    volatile int result = 0; /* Prevent dead code elimination */
    
    /* 1. Fixed-point types with boundary values */
    const unsigned short _Fract max_usf = 0.99999hr;
    const signed _Fract min_sf = -0.99999r;
    const unsigned _Sat _Fract sat_max = 0.99999ur;
    const long _Accum max_la = 9223372036854775.5807lk;  /* Near max */
    const long _Accum min_la = -9223372036854775.5807lk; /* Near min */
    
    /* 2. Arithmetic that may overflow */
    unsigned _Sat _Fract sat_result = sat_max + sat_max;  /* Should saturate */
    long _Accum la_result = max_la * 2.0lk;  /* Should trigger overflow check */
    
    /* 3. Conversions at boundaries */
    long int_val1 = test_conversion(max_la);
    long int_val2 = test_conversion(min_la);
    float float_val = (float)max_usf;
    
    /* 4. Loop with fixed-point operations (small count for unrolling) */
    signed _Fract accum = 0.0r;
    for (int i = 0; i < 3; i++) {
        /* Conditional based on fixed-point comparison */
        if (accum > 0.5r) {
            accum = accum * 0.75r;  /* Multiplication may underflow */
        } else {
            accum = accum + 0.25r;  /* Addition may overflow */
        }
        
        /* Mix with array access */
        accum = accum + init_data[i].sf;
        
        /* Shift-like operation through multiplication */
        if (i == 1) {
            accum = accum * 4.0r;  /* Effectively left shift for overflow */
        }
    }
    
    /* 5. Complex expression with multiple conversions */
    result = (int)((long)((max_la + min_la) * 0.5lk) + 
                   (int)(float_val * 100.0f) + 
                   (int)(sat_result * 1000.0r));
    
    /* 6. Compile-time conditional using preprocessor */
#if defined(__STDC_IEC_60559_DFP__)
    /* Use extreme values in array indexing */
    const int idx = (int)(max_usf * 2);  /* Conversion may overflow */
    if (idx < 3) {
        result += (int)(init_data[idx].usf * 1000);
    }
#endif
    
    /* 7. Additional saturation tests */
    unsigned _Sat _Fract sat1 = 0.75ur;
    unsigned _Sat _Fract sat2 = 0.5ur;
    unsigned _Sat _Fract sat3 = sat1 + sat2;  /* 1.25 -> should saturate to 0.99999 */
    
    /* 8. Boundary checks through comparisons */
    int overflow_detected = 0;
    if (la_result > max_la || la_result < min_la) {
        overflow_detected = 1;
    }
    
    /* 9. Use __builtin_constant_p to force constant evaluation */
    if (__builtin_constant_p(max_la * min_la)) {
        /* This expression should be evaluated at compile-time */
        const long _Accum product = max_la * min_la;
        result += (int)(product * 0.000001lk);
    }
    
    /* Print to prevent optimization */
    printf("Results: %d %ld %ld %f %d\n", 
           result, int_val1, int_val2, float_val, overflow_detected);
    
    /* Assign to volatile to ensure all computations are kept */
    volatile signed _Fract keep_alive = accum;
    volatile unsigned _Sat _Fract keep_sat = sat3;
    
    return 0;
}

/* Additional compile-time tests */
#ifdef __cplusplus
/* C++ version with constexpr */
constexpr unsigned _Fract compile_time_test() {
    constexpr unsigned _Fract a = 0.75r;
    constexpr unsigned _Fract b = 0.5r;
    constexpr unsigned _Fract c = a + b;  /* Should be 1.25 but clamped */
    return c > 1.0r ? 1.0r : c;
}

static constexpr unsigned _Fract ct_result = compile_time_test();
#endif
