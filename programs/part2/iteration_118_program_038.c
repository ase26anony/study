/* fixed-point-test.c
 * Tests GCC's fixed-point arithmetic range calculation logic
 * Specifically targets uncovered lines in fixed-value.cc (lines 264-277)
 */

#include <stdio.h>

/* Force compile-time evaluation with constexpr-style patterns */
#define EVAL_CONST(expr) (__builtin_constant_p(expr) ? (expr) : (expr))

/* Test different fixed-point types and saturation behaviors */
int main(void) {
    volatile int result = 0; /* Prevent dead code elimination */
    
    /* 1. Basic fixed-point types with boundary values */
    unsigned short _Fract usf_max = 0.999999r;  /* Max unsigned short fract */
    signed _Fract sf_min = -0.999999r;          /* Min signed fract */
    unsigned _Accum ua_max = 255.999999999k;    /* Max 8.8 unsigned accum */
    signed _Accum sa_min = -128.000000000k;     /* Min 8.8 signed accum */
    
    /* 2. Saturation types for overflow/underflow testing */
    unsigned _Sat _Fract usf_sat = 0.5r;
    signed _Sat _Accum sa_sat = 0.0k;
    unsigned _Sat long _Accum ula_sat = 0.0lk;
    
    /* 3. Compile-time constant expressions that trigger range checks */
    static const unsigned _Fract const_uf = 0.75r;
    static const signed long _Accum const_sla = -0.0000000000000001lk;
    
    /* 4. Mixed precision types */
    short _Fract sf1 = 0.5hr;
    long _Accum la1 = 0.0000000000000000lk;
    
    /* Test 1: Operations that approach maximum representable values */
    printf("Test 1: Boundary value operations\n");
    
    /* This multiplication should approach but not exceed max for unsigned fract */
    usf_sat = usf_max * usf_max;  /* 0.999999r * 0.999999r */
    
    /* This should trigger saturation check for signed accum */
    sa_sat = sa_min * 2.0k;  /* Underflow for signed accum */
    
    /* Test 2: Complex compile-time expressions with ternary operator */
    printf("Test 2: Compile-time constant folding\n");
    
    /* Force constant folding with ternary operator */
    const unsigned _Fract cf_test = EVAL_CONST(
        (const_uf > 0.5r) ? const_uf * 1.5r : const_uf * 0.5r
    );
    
    /* Another constant expression that uses different fixed-point types */
    const signed _Accum ca_test = EVAL_CONST(
        (const_sla < 0.0lk) ? (signed _Accum)(const_sla * 2.0lk) : 0.0k
    );
    
    /* Test 3: Loop with fixed iterations for constant propagation */
    printf("Test 3: Loop-based constant propagation\n");
    
    unsigned _Fract loop_accum = 0.0r;
    for (int i = 0; i < 4; i++) {  /* Small, fixed count for unrolling */
        /* Conditional that depends on fixed-point comparison */
        if (loop_accum > 0.5r) {
            loop_accum = loop_accum * 0.75r;  /* Scale down */
        } else {
            loop_accum = loop_accum + 0.25r;  /* Add quarter */
        }
        
        /* Mix with integer arithmetic */
        int int_val = (int)(loop_accum * 100);
        result += int_val;
    }
    
    /* Test 4: Explicit casts that trigger range checking */
    printf("Test 4: Type conversion range checks\n");
    
    /* Cast from fixed-point to integer (truncation) */
    int int_from_fract = (int)(usf_max * 2);  /* May overflow */
    int int_from_accum = (int)ua_max;         /* Direct cast */
    
    /* Cast from fixed-point to floating-point */
    float float_from_fract = (float)sf_min;
    double double_from_accum = (double)sa_min;
    
    /* Cast from integer to fixed-point */
    unsigned _Fract fract_from_int = (unsigned _Fract)255;  /* Max for 8-bit */
    signed _Accum accum_from_int = (signed _Accum)(-129);   /* May underflow */
    
    /* Test 5: Saturation arithmetic with overflow/underflow */
    printf("Test 5: Saturation arithmetic\n");
    
    /* These operations should trigger saturation logic */
    unsigned _Sat _Fract usf_sat_result = usf_sat + usf_sat;  /* Overflow */
    signed _Sat _Accum sa_sat_result = sa_sat - 256.0k;       /* Underflow */
    
    /* Test 6: Shift operations (for accum types) */
    printf("Test 6: Shift operations\n");
    
    /* Simulate left shift through multiplication */
    signed _Accum sa_shifted = sa_min * 4.0k;  /* Effectively left shift 2 bits */
    
    /* Test 7: Aggregate initializers with fixed-point */
    printf("Test 7: Aggregate initialization\n");
    
    struct MixedFixed {
        unsigned short _Fract usf;
        signed _Accum sa;
        unsigned _Sat _Fract usf_sat;
    };
    
    /* Initialize with boundary values */
    struct MixedFixed agg = {
        .usf = 0.999999hr,      /* Max for short fract */
        .sa = -128.000000000k,  /* Min for 8.8 accum */
        .usf_sat = 0.999999r    /* Will saturate in operations */
    };
    
    /* Array with fixed-point values */
    unsigned _Fract fract_array[4] = {
        0.0r, 0.333333r, 0.666666r, 0.999999r
    };
    
    /* Test 8: Complex expression with multiple fixed-point types */
    printf("Test 8: Complex mixed-type expressions\n");
    
    /* Expression that mixes different fixed-point types */
    long _Accum complex_expr = (long _Accum)usf_max * 100.0lk + 
                               (long _Accum)sf_min * 50.0lk +
                               (long _Accum)const_uf * 25.0lk;
    
    /* Test 9: Conditional compilation based on fixed-point constants */
    printf("Test 9: Conditional compilation tests\n");
    
#if 1  /* Always true, but creates context for constant evaluation */
    /* Use fixed-point value as array index (converted to int) */
    int idx = (int)(const_uf * 4);  /* Range 0-3 */
    if (idx >= 0 && idx < 4) {
        result += fract_array[idx];
    }
#endif
    
    /* Test 10: Operations that specifically target the uncovered comparison */
    printf("Test 10: Direct boundary comparison tests\n");
    
    /* Create values that should trigger the a_high.sgt(max_r) comparison */
    unsigned long _Accum ula_boundary = 0.9999999999999999lk;  /* Very close to 1 */
    
    /* Operations that might exceed max_r/max_s or min_r/min_s */
    ula_sat = ula_boundary * 2.0lk;  /* Should saturate for unsigned long accum */
    
    /* For signed types, test negative boundary */
    signed long _Accum sla_boundary = -0.9999999999999999lk;  /* Very close to -1 */
    signed long _Accum sla_test = sla_boundary * 2.0lk;  /* Should approach -2 */
    
    /* Prevent compiler from optimizing everything away */
    result += (int)(usf_max * 1000) + 
              (int)(sf_min * 1000) + 
              (int)(complex_expr) +
              int_from_fract + 
              int_from_accum +
              (int)(ula_sat * 100) +
              (int)(sla_test * 100);
    
    /* Use volatile to ensure computations aren't optimized out */
    volatile int final_result = result;
    
    printf("Final accumulated result: %d\n", final_result);
    
    return 0;
}
