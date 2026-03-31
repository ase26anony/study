/* Test program to exercise fixed-point range calculation logic in GCC */
/* Compile with: gcc -O2 -std=c23 -Wno-psabi -fdump-tree-original test_fixed.c -o test_fixed */

#include <stdio.h>
#include <stdint.h>

/* Force compile-time evaluation */
#define EVAL_CONST(expr) (__builtin_constant_p(expr) ? (expr) : (expr))

/* Struct with mixed fixed-point types */
struct FixedPointData {
    unsigned short _Fract usf;
    signed _Fract sf;
    unsigned _Accum ua;
    signed long _Accum sla;
    _Sat signed _Fract ssf;
    _Sat unsigned _Accum sua;
};

/* Array initialization with fixed-point constants */
static const struct FixedPointData init_data[] = {
    {0.99999hr, -0.99999r, 255.99999uk, -32767.99999lk, 0.5r, 127.5uk},
    {0.00001hr, -0.00001r, 0.00001uk, -0.00001lk, -0.5r, 0.5uk},
    {0.5hr, -0.5r, 127.5uk, -16383.5lk, 0.99999r, 255.99999uk}
};

/* Function to trigger range checks through conversions */
static int_fast32_t convert_fixed_to_int(signed _Accum val) {
    /* This conversion should trigger range checking */
    return (int_fast32_t)val;
}

/* Function using ternary operator with constant folding */
static unsigned _Sat _Fract saturate_multiply(unsigned _Fract a, unsigned _Fract b) {
    /* Force constant folding with ternary */
    const unsigned _Fract limit = 0.75r;
    return (a > limit && b > limit) ? 0.99999r : (a * b);
}

int main(void) {
    volatile int result = 0; /* Prevent dead code elimination */
    
    /* Test 1: Direct extreme value initialization */
    printf("Test 1: Extreme value initialization\n");
    
    /* These should trigger max/min range calculations */
    const unsigned short _Fract max_usf = 0.99999hr;  /* Near max */
    const unsigned short _Fract min_usf = 0.00001hr;  /* Near min */
    const signed long _Accum max_sla = 32767.99999lk; /* Near max */
    const signed long _Accum min_sla = -32768.00000lk; /* Near min */
    
    /* Force evaluation through conversion */
    result += convert_fixed_to_int(255.99999k);
    result += convert_fixed_to_int(-255.99999k);
    
    /* Test 2: Saturation arithmetic that should overflow/underflow */
    printf("Test 2: Saturation arithmetic\n");
    
    _Sat unsigned _Fract suf1 = 0.9r;
    _Sat unsigned _Fract suf2 = 0.8r;
    _Sat signed _Accum ssa1 = 127.9k;
    _Sat signed _Accum ssa2 = 127.8k;
    
    /* These additions should saturate */
    _Sat unsigned _Fract sum_uf = suf1 + suf2;      /* Should saturate to ~1.0 */
    _Sat signed _Accum sum_sa = ssa1 + ssa2;        /* Should saturate */
    
    /* Test 3: Compile-time constant folding with ternary */
    printf("Test 3: Constant folding\n");
    
    /* Force compiler to evaluate at compile time */
    static const unsigned _Fract cf1 = 0.99999r;
    static const unsigned _Fract cf2 = 0.99998r;
    
    /* This ternary should be evaluated at compile time, triggering range checks */
    const unsigned _Sat _Fract folded = EVAL_CONST(
        (cf1 > 0.9r && cf2 > 0.9r) ? 
        (cf1 * cf2) : 
        0.5r
    );
    
    /* Test 4: Mixed-type expressions and conversions */
    printf("Test 4: Mixed-type conversions\n");
    
    signed _Accum acc_val = 127.999k;
    float float_val = (float)acc_val;  /* Conversion to float */
    int int_val = (int)acc_val;        /* Conversion to int - should trigger range check */
    
    /* Test 5: Loop with fixed-point operations */
    printf("Test 5: Loop operations\n");
    
    unsigned _Fract accum = 0.0r;
    signed _Accum signed_accum = 0.0k;
    
    /* Small fixed loop for potential unrolling */
    for (int i = 0; i < 4; i++) {
        /* Conditional based on fixed-point comparison */
        if (accum > 0.5r) {
            signed_accum -= 64.5k;
        } else {
            signed_accum += 64.5k;
        }
        
        /* Multiplication that could overflow */
        accum = accum * 1.1r;
        
        /* Cast to prevent dead code elimination */
        result += (int)(accum * 1000r);
    }
    
    /* Test 6: Array indexing with fixed-point derived index */
    printf("Test 6: Array indexing\n");
    
    /* Use fixed-point to compute array index */
    signed _Fract index_frac = 0.5r;
    int array_index = (int)(index_frac * 4r);  /* Should be 2 */
    
    int test_array[4] = {10, 20, 30, 40};
    result += test_array[array_index];
    
    /* Test 7: Preprocessor conditional with fixed-point constant */
    printf("Test 7: Preprocessor conditionals\n");
    
#if (0.99999r > 0.9r)  /* Always true, but tests fixed-point in preprocessor */
    const signed _Fract pp_val = -0.99999r;
#else
    const signed _Fract pp_val = 0.0r;
#endif
    
    /* Test 8: Complex struct initialization */
    printf("Test 8: Struct operations\n");
    
    struct FixedPointData data = {
        .usf = 0.75hr,
        .sf = -0.25r,
        .ua = 128.25uk,
        .sla = -16384.5lk,
        .ssf = 0.9r,
        .sua = 200.75uk
    };
    
    /* Perform operations on struct members */
    data.ssf = data.ssf + 0.2r;  /* Should saturate */
    data.sua = data.sua * 2.0uk; /* Should saturate */
    
    /* Test 9: Shift-like operations through multiplication */
    printf("Test 9: Shift simulation\n");
    
    /* Simulate left shift by multiplying by powers of 2 */
    unsigned _Accum shift_test = 1.0uk;
    for (int i = 0; i < 8; i++) {
        shift_test = shift_test * 2.0uk;  /* Each multiply doubles the value */
    }
    /* Last multiplication might trigger overflow check */
    shift_test = shift_test * 2.0uk;
    
    /* Test 10: Boundary value comparisons */
    printf("Test 10: Boundary comparisons\n");
    
    /* Create values near boundaries */
    const signed _Accum near_max = 127.9999k;
    const signed _Accum near_min = -127.9999k;
    
    /* Comparisons that should trigger range checking */
    int is_near_max = (near_max > 127.9k);
    int is_near_min = (near_min < -127.9k);
    
    /* Use results to prevent optimization */
    result += is_near_max + is_near_min;
    result += (int)(sum_uf * 1000r);
    result += (int)(folded * 1000r);
    result += int_val;
    result += (int)(data.ssf * 1000r);
    result += (int)(shift_test);
    
    printf("Final result (prevent optimization): %d\n", result);
    
    return 0;
}
