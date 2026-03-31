/* fixed-point-test.c - Targeting GCC's fixed-value range analysis */
#include <stdio.h>
#include <stdint.h>

/* Use volatile to prevent optimization of results */
volatile int global_result = 0;

/* Function with aggressive optimization */
__attribute__((optimize("O3")))
void test_boundary_comparisons(void) {
    /* Test 1: Signed fixed-point with boundary values */
    signed short _Fract sf_min = -1.0r;
    signed short _Fract sf_max = 0.9999r;
    signed short _Fract sf_mid = 0.5r;
    
    /* Test 2: Unsigned saturated accumulators */
    _Sat unsigned long _Accum usa_min = 0.0uk;
    _Sat unsigned long _Accum usa_max = 0.999999999999999999uk; /* Near max */
    
    /* Test 3: Mixed precision operations */
    signed long _Accum sla = 0.5k;
    unsigned short _Fract usf = 0.8r;
    
    /* Force range analysis through arithmetic */
    signed long _Accum mixed_result = sla * (signed long _Accum)usf;
    
    /* Boundary comparisons that should trigger the uncovered code */
    if (sf_max > sf_mid) {
        /* This should set max_r.high = 0, max_r.low = 0 */
        global_result += 1;
    }
    
    if (sf_min < sf_mid) {
        /* This should set min_r.high = -1, min_r.low = -1 */
        global_result += 2;
    }
    
    /* Mixed comparison with conversion */
    if ((signed long _Accum)usf > mixed_result) {
        global_result += 4;
    }
}

__attribute__((optimize("O2")))
void test_saturation_overflow(void) {
    /* Test saturation at boundaries */
    _Sat signed short _Fract ssf1 = 0.9r;
    _Sat signed short _Fract ssf2 = 0.8r;
    
    /* These operations should saturate */
    _Sat signed short _Fract sat_sum = ssf1 + ssf2;  /* Should saturate near 1.0 */
    _Sat signed short _Fract sat_prod = ssf1 * ssf2; /* Should be < 1.0 */
    
    /* Use builtins for overflow detection */
    _Sat signed short _Fract of_result;
    int overflow = __builtin_mul_overflow(ssf1, ssf2, &of_result);
    
    /* Comparisons that should use the uncovered range logic */
    if (sat_sum > 0.95r) {
        /* Should trigger a_high.sgt(max_r) comparison */
        global_result += 8;
    }
    
    if (sat_prod < 0.8r && !overflow) {
        global_result += 16;
    }
    
    /* Test near-zero boundary */
    _Sat signed short _Fract near_zero = 0.0001r;
    if (near_zero > 0.0r && near_zero < 0.001r) {
        /* Should exercise min_s initialization with low=1 */
        global_result += 32;
    }
}

/* Struct with fixed-point members */
struct FixedPointStruct {
    _Sat unsigned short _Fract usf;
    signed long _Accum sla;
    signed short _Fract ssf;
};

__attribute__((optimize("O3")))
void test_struct_operations(void) {
    struct FixedPointStruct fps[3];
    
    /* Initialize with boundary values */
    fps[0].usf = 0.0r;
    fps[0].sla = -1.0k;
    fps[0].ssf = -1.0r;
    
    fps[1].usf = 0.9999r;  /* Near max */
    fps[1].sla = 0.0k;
    fps[1].ssf = 0.0r;
    
    fps[2].usf = 0.5r;
    fps[2].sla = 0.5k;
    fps[2].ssf = 0.5r;
    
    /* Perform operations that require range analysis */
    for (int i = 0; i < 2; i++) {
        /* Mixed operations between array elements */
        signed long _Accum temp = fps[i].sla * (signed long _Accum)fps[i+1].usf;
        
        /* Comparison that should use the double-int range logic */
        if (temp > (signed long _Accum)fps[i].ssf) {
            global_result += 64 * (i + 1);
        }
        
        /* Test the specific condition pattern from uncovered code */
        if (fps[i].sla > fps[i+1].sla || 
            (fps[i].sla == fps[i+1].sla && fps[i].usf > fps[i+1].usf)) {
            /* This should trigger: a_high.sgt(max_r) || (a_high == max_r && a_low.ugt(max_s)) */
            global_result += 128 * (i + 1);
        }
    }
}

/* Test precision conversions and shifts */
__attribute__((optimize("O2")))
void test_precision_conversions(void) {
    /* Start with small fractional value */
    unsigned short _Fract small_frac = 0.0001r;
    
    /* Convert through different precisions */
    signed long _Accum converted = (signed long _Accum)small_frac;
    
    /* Multiply to shift bits - similar to min_s.alshift() in uncovered code */
    signed long _Accum shifted = converted * 256.0k;  /* Shift left 8 bits */
    
    /* Comparisons that should exercise zext and sext operations */
    if (shifted > 0.0k) {
        /* Should trigger range initialization for positive values */
        global_result += 256;
    }
    
    if (converted < shifted) {
        /* Should exercise both min and max range comparisons */
        global_result += 512;
    }
    
    /* Test with builtin overflow */
    _Sat signed short _Fract a = 0.9r;
    _Sat signed short _Fract b = 0.95r;
    _Sat signed short _Fract c;
    
    if (__builtin_add_overflow(a, b, &c)) {
        /* Overflow occurred - should saturate */
        global_result += 1024;
    }
    
    /* Compare saturated value */
    if (c >= 0.9999r) {
        /* Should be at or near maximum */
        global_result += 2048;
    }
}

int main(void) {
    printf("Starting fixed-point range analysis tests...\n");
    
    /* Reset global result */
    global_result = 0;
    
    /* Execute all test functions */
    test_boundary_comparisons();
    test_saturation_overflow();
    test_struct_operations();
    test_precision_conversions();
    
    /* Print result to prevent dead code elimination */
    printf("Test result: %d\n", global_result);
    
    /* Additional volatile store to ensure all computations are kept */
    volatile int final_check = global_result;
    
    return (final_check > 0) ? 0 : 1;
}
