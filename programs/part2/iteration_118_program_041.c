/* Test program to exercise fixed-point range calculations in GCC */
#include <stdio.h>

/* Force compile-time evaluation with static const */
static const unsigned short _Fract max_uf = 0.99999r;
static const signed _Fract min_sf = -0.99999r;
static const unsigned _Sat _Fract sat_uf = 0.5r;
static const signed _Sat long _Accum sat_sla = -0.5lk;
static const long _Accum max_la = 0.999999999999999999lk;

/* Struct with mixed fixed-point types */
struct fixed_mix {
    unsigned _Fract f1;
    signed _Accum a1;
    unsigned _Sat short _Fract sat_f2;
    long _Fract f3;
};

/* Array initialized with fixed-point constants */
static const struct fixed_mix mixes[] = {
    {0.75r, 0.5k, 0.999r, 0.999999999999999lr},
    {0.25r, -0.25k, 0.0r, -0.5lr},
    {0.999r, 0.999k, 0.5r, 0.0lr}
};

/* Helper to force constant folding */
#define EVAL_CONST(expr) (__builtin_constant_p(expr) ? (expr) : (expr))

int main(void) {
    volatile int result = 0; /* Prevent elimination */
    
    /* Test 1: Direct overflow with multiplication */
    unsigned _Fract uf1 = 0.8r;
    unsigned _Fract uf2 = 0.9r;
    unsigned _Fract uf_prod = uf1 * uf2; /* May overflow in some representations */
    
    /* Test 2: Saturation arithmetic that should hit bounds */
    unsigned _Sat _Fract sat1 = 0.8r;
    unsigned _Sat _Fract sat2 = 0.9r;
    unsigned _Sat _Fract sat_sum = sat1 + sat2; /* Should saturate to max */
    
    /* Test 3: Mixed precision with explicit casts */
    signed _Accum acc1 = 0.123456789k;
    signed short _Fract sf1 = (signed short _Fract)acc1; /* Range check */
    float float_from_fixed = (float)max_uf; /* Conversion */
    
    /* Test 4: Compile-time conditional with fixed-point */
    #if __FLT_EVAL_METHOD__ == 0
    const signed _Fract ct_fract = 0.7r;
    #else
    const signed _Fract ct_fract = 0.3r;
    #endif
    
    /* Test 5: Loop with fixed-point operations (will unroll) */
    signed _Fract accum = 0.0r;
    for (int i = 0; i < 4; i++) {
        /* Conditional based on fixed-point comparison */
        if (accum > 0.5r) {
            accum = accum * 0.8r; /* May underflow */
        } else {
            accum = accum + 0.25r; /* May overflow */
        }
        
        /* Mix with integer */
        int int_val = (int)(accum * 100); /* Conversion triggers range check */
        result += int_val;
    }
    
    /* Test 6: Extreme values near bounds */
    unsigned _Fract near_max = 0.999999r; /* Very close to 1.0 */
    signed _Fract near_min = -0.999999r; /* Very close to -1.0 */
    
    /* Operations that should trigger range calculations */
    unsigned _Fract test1 = near_max * near_max; /* Should be ~0.999998 */
    signed _Fract test2 = near_min + near_min;   /* Should be ~-1.999998 -> may saturate */
    
    /* Test 7: Use __builtin_constant_p to force constant evaluation */
    if (__builtin_constant_p(max_uf * max_uf)) {
        /* This expression must be evaluated at compile-time */
        const unsigned _Fract const_prod = max_uf * max_uf;
        result += (int)(const_prod * 1000);
    }
    
    /* Test 8: Shift-like behavior through multiplication */
    long _Accum la1 = 0.5lk;
    long _Accum la2 = la1 * 2.0lk; /* Effectively left shift */
    
    /* Test 9: Ternary with fixed-point constants */
    signed _Fract ternary_result = (result > 0) ? 0.9r : -0.9r;
    
    /* Test 10: Array indexing with fixed-point derived index */
    int idx = (int)(ternary_result * 10 + 10); /* Map [-0.9,0.9] to [~1,~19] */
    if (idx >= 0 && idx < 3) {
        result += mixes[idx].f1 * 100;
    }
    
    /* Prevent dead code elimination */
    result += (int)(uf_prod * 100);
    result += (int)(sat_sum * 100);
    result += (int)(float_from_fixed * 100);
    result += (int)(test1 * 100);
    result += (int)(test2 * 100);
    result += (int)(la2 * 100);
    
    printf("Result: %d\n", result);
    return 0;
}
