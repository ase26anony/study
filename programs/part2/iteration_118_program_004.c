/* fixed-point-coverage.c
 * Targets uncovered lines 264-277 in fixed-value.cc
 * Compile with: gcc -O2 -std=c23 -Wno-psabi fixed-point-coverage.c -o fixed-point-test
 */

#include <stdio.h>

/* Force compile-time evaluation with constexpr-style constants */
#define EVAL_CONST(expr) __builtin_constant_p(expr) ? (expr) : (expr)

/* Test different fixed-point types and saturations */
int main(void) {
    /* Requirement 1: Use various fixed-point types with extreme values */
    
    /* Unsigned fract types at boundaries */
    unsigned short _Fract usf_min = 0.0hr;
    unsigned short _Fract usf_max = 0.999999hr;  /* Pushes against max representable */
    
    /* Signed accum types with saturation */
    _Sat signed _Accum ssa_mid = 0.5k;
    _Sat signed _Accum ssa_max = 0.999999999k;  /* Close to max */
    _Sat signed _Accum ssa_min = -0.999999999k; /* Close to min */
    
    /* Long accum for wider range */
    long _Accum la_large = 123456.123456789lk;
    long _Accum la_small = -123456.123456789lk;
    
    /* Requirement 2: Force constant folding with static const */
    static const unsigned _Fract uf_const = 0.999999r;
    static const signed _Accum sa_const = -0.999999999k;
    
    /* Requirement 3: Saturation arithmetic that will overflow/underflow */
    _Sat unsigned short _Fract usf_sat1 = 0.8hr;
    _Sat unsigned short _Fract usf_sat2 = 0.9hr;
    
    /* This addition should saturate */
    _Sat unsigned short _Fract usf_sum = usf_sat1 + usf_sat2;
    
    /* Requirement 4: Mixed-type conversions */
    volatile int int_from_fract;
    volatile float float_from_accum;
    
    /* Requirement 5: Compile-time conditional with __builtin_constant_p */
#if defined(__GNUC__)
    if (__builtin_constant_p(uf_const > 0.5r)) {
        /* This block only compiled if expression is constant */
        int_from_fract = (int)(uf_const * 100);
    }
#endif
    
    /* Requirement 6: Aggregate initializers with fixed-point */
    struct {
        unsigned short _Fract f1;
        signed _Accum a1;
        _Sat long _Accum sa1;
    } fp_struct = {
        .f1 = 0.75hr,
        .a1 = -0.25k,
        .sa1 = 0.999999999999lk
    };
    
    /* Array with fixed-point initializers */
    signed _Accum fp_array[4] = {
        0.0k,
        0.25k,
        0.5k,
        0.999999999k  /* Edge case */
    };
    
    /* Execution flow: Loop with fixed-point operations */
    signed _Accum accumulator = 0.0k;
    
    /* Small fixed loop that can be unrolled */
    for (int i = 0; i < 4; i++) {
        /* Requirement: Conditional assignments based on comparisons */
        if (fp_array[i] > 0.75k) {
            /* This comparison may trigger range checking */
            accumulator = accumulator + 0.1k;
        } else if (fp_array[i] < -0.75k) {
            accumulator = accumulator - 0.1k;
        } else {
            /* Multiplication that could overflow */
            accumulator = accumulator * 1.1k;
        }
        
        /* Mix with integer arithmetic */
        int_from_fract = (int)(accumulator * 10);
    }
    
    /* Force evaluation of saturation arithmetic */
    _Sat signed _Accum test_sat = ssa_max + ssa_max;  /* Should saturate to max */
    _Sat signed _Accum test_sat2 = ssa_min - ssa_max; /* Should saturate to min */
    
    /* More boundary tests */
    _Sat unsigned _Fract uf_boundary = uf_const + uf_const;  /* Should saturate */
    
    /* Conversions that require range checking */
    float_from_accum = (float)la_large;
    int_from_fract = (int)(usf_max * 1000);
    
    /* Complex expression that should trigger constant folding */
    static const signed _Accum complex_expr = 
        (sa_const > 0.0k) ? (sa_const * 2.0k) : (sa_const / 2.0k);
    
    /* Print results to prevent dead code elimination */
    printf("Results (compiler-dependent):\n");
    printf("usf_sum: %d (as int)\n", (int)(usf_sum * 1000));
    printf("accumulator: %d (as int)\n", (int)(accumulator * 1000));
    printf("test_sat: %d (as int)\n", (int)(test_sat * 1000));
    printf("complex_expr: %d (as int)\n", (int)(complex_expr * 1000));
    
    /* Use volatile to ensure all computations happen */
    volatile signed _Accum vol_accum = accumulator;
    volatile _Sat unsigned _Fract vol_sat = uf_boundary;
    
    return (int)(vol_accum * 100) + (int)(vol_sat * 100);
}

/* Additional compile-time tests using macros */
#ifdef __GNUC__
/* This function uses only compile-time constants */
static inline long _Accum compile_time_test(void) {
    /* Extreme values that should trigger range checks */
    const long _Accum max_val = 0.999999999999999lk;
    const long _Accum min_val = -0.999999999999999lk;
    
    /* Operations that might overflow */
    const long _Accum product = max_val * max_val;
    const long _Accum sum = max_val + max_val;
    
    /* Conditional with constant comparison */
    return __builtin_constant_p(product > sum) ? product : sum;
}

/* Force evaluation at compile time */
static const long _Accum ct_result = compile_time_test();
#endif
