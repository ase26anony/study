/* fixed-point-test.c */
#include <stdio.h>
#include <stdint.h>

/* Force aggressive optimization on specific functions */
__attribute__((optimize("O3")))
static void test_short_fract_range(void) {
    /* Test short _Fract with boundary values */
    volatile short _Fract sf_min = 0x8000;  /* Minimum: -1.0 */
    volatile short _Fract sf_max = 0x7FFF;  /* Maximum: ~0.9999 */
    volatile short _Fract sf_zero = 0x0000; /* Zero */
    
    /* Operations that force range analysis */
    short _Fract a = sf_max * sf_max;      /* ~0.9998 */
    short _Fract b = sf_min * sf_zero;     /* 0 */
    short _Fract c = a / b;                /* Division by zero - special case */
    
    /* Boundary comparisons that should trigger the uncovered code */
    if (a > sf_max) {
        /* This should never execute but forces comparison analysis */
        printf("Unexpected: a > sf_max\n");
    }
    
    if (b == sf_zero) {
        /* This will execute, testing equality comparison */
        short _Fract d = sf_max / (_Fract)2.0r;
        if (d > (_Fract)0.4r && d < (_Fract)0.6r) {
            /* Nested condition with fixed-point range */
            volatile short _Fract e = d * d;
        }
    }
}

__attribute__((optimize("O3")))
static void test_sat_accum_range(void) {
    /* Test saturated unsigned long accum with overflow scenarios */
    _Sat unsigned long _Accum ula_min = 0x00000000;  /* 0.0 */
    _Sat unsigned long _Accum ula_max = 0xFFFFFFFFFFFFFFFF;  /* Max */
    
    /* Mixed precision operations */
    unsigned short _Fract usf = 0x7FFF;  /* ~0.9999 */
    _Sat unsigned long _Accum ula1 = ula_max * usf;
    _Sat unsigned long _Accum ula2 = ula_min / usf;
    
    /* Built-in overflow checks with fixed-point */
    _Sat unsigned long _Accum result;
    int overflow = __builtin_mul_overflow(ula_max, (_Accum)2.0k, &result);
    
    /* Control flow dependent on saturated ranges */
    for (_Sat unsigned long _Accum i = ula_min; i < ula_max; i += 0x1000000000000000k) {
        /* Loop with fixed-point increment */
        if (i > ula_max / (_Accum)2.0k) {
            _Sat unsigned long _Accum temp = i * i;
            if (temp > ula_max) {
                /* Saturation should occur here */
                temp = ula_max;
            }
        }
    }
    
    /* Boundary value comparison */
    if (ula1 == ula_max || ula2 == ula_min) {
        /* Triggers equality comparison in range analysis */
        volatile _Sat unsigned long _Accum diff = ula_max - ula_min;
    }
}

__attribute__((optimize("O3")))
static void test_mixed_precision_ops(void) {
    /* Mixed signedness and precision */
    signed long _Accum sla = -0.5k;
    unsigned short _Fract usf = 0x4000;  /* 0.5 */
    
    /* Precision conversion forcing zext/alshift operations */
    signed long _Accum result1 = sla * (_Accum)usf;  /* -0.25 */
    signed short _Fract result2 = (_Fract)sla;       /* Precision loss */
    
    /* Array of fixed-point values */
    _Fract fract_array[4] = {0.1r, 0.2r, 0.3r, 0.4r};
    _Accum accum_array[4] = {0.1k, 0.2k, 0.3k, 0.4k};
    
    /* Operations on arrays */
    for (int i = 0; i < 4; i++) {
        fract_array[i] = fract_array[i] * 2.0r;
        accum_array[i] = accum_array[i] / (_Accum)fract_array[i];
        
        /* Conditional with mixed-type comparison */
        if ((_Accum)fract_array[i] > accum_array[i]) {
            accum_array[i] = (_Accum)fract_array[i];
        }
    }
    
    /* Struct with fixed-point members */
    struct FixedPointStruct {
        _Fract f;
        _Accum a;
        _Sat unsigned short _Fract sat_f;
    } fps;
    
    fps.f = 0.75r;
    fps.a = 100.5k;
    fps.sat_f = 0x7FFF;
    
    /* Struct operations */
    fps.a = fps.a * (_Accum)fps.f;
    if (fps.sat_f > (_Fract)0.5r) {
        fps.sat_f = fps.sat_f * 2.0r;  /* Should saturate */
    }
}

__attribute__((optimize("O3")))
static void test_extreme_boundaries(void) {
    /* Test values at extreme boundaries */
    _Sat signed long _Accum sat_min = -1.0k;
    _Sat signed long _Accum sat_max = 1.0k - 0.00000000023283064365386962890625k; /* 1-ε */
    
    /* Operations that should trigger max_r/min_r comparisons */
    _Sat signed long _Accum a = sat_max * sat_max;  /* Approaches 1.0 */
    _Sat signed long _Accum b = sat_min * sat_max;  /* Approaches -1.0 */
    
    /* Built-in overflow with saturation */
    _Sat signed long _Accum sum;
    int overflow_add = __builtin_add_overflow(sat_max, 0.0000000001k, &sum);
    
    /* Complex condition similar to uncovered code */
    if (a > sat_max || (a == sat_max && b < sat_min)) {
        /* This should trigger the specific comparison pattern */
        volatile _Sat signed long _Accum diff = a - b;
    }
    
    /* Division near boundaries */
    _Sat signed long _Accum c = sat_max / (_Accum)0.5k;  /* Approaches 2.0 but saturates */
    _Sat signed long _Accum d = sat_min / (_Accum)0.5k;  /* Approaches -2.0 but saturates */
    
    /* Nested comparisons */
    if (c == sat_max && d == sat_min) {
        /* Both saturated at extremes */
        for (_Sat signed long _Accum i = sat_min; i < sat_max; i += 0.0000001k) {
            _Sat signed long _Accum square = i * i;
            if (square > 0.9k && square < 1.0k) {
                volatile _Sat signed long _Accum root = square;
            }
        }
    }
}

int main(void) {
    volatile int result = 0;
    
    /* Execute all test functions */
    test_short_fract_range();
    test_sat_accum_range();
    test_mixed_precision_ops();
    test_extreme_boundaries();
    
    /* Prevent dead code elimination */
    printf("Fixed-point tests completed. Result: %d\n", result);
    
    return 0;
}
