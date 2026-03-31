#include <stdio.h>
#include <stdint.h>

/* Force aggressive optimization on specific functions */
__attribute__((optimize("O3")))
static void test_boundary_comparisons(void) {
    /* Test various fixed-point types near their boundaries */
    volatile _Sat unsigned short _Fract usf1 = 0.9999r;
    volatile _Sat unsigned short _Fract usf2 = 0.0001r;
    volatile _Sat signed short _Fract ssf1 = -0.9999r;
    volatile _Sat signed short _Fract ssf2 = 0.9999r;
    
    /* Mixed precision operations */
    volatile _Sat unsigned long _Accum ula1 = 255.9999k;
    volatile _Sat signed long _Accum sla1 = -255.9999k;
    
    /* Force range analysis through arithmetic */
    _Sat unsigned short _Fract usf_prod = usf1 * usf2;
    _Sat signed short _Fract ssf_sum = ssf1 + ssf2;
    
    /* Mixed-type operations */
    _Sat signed long _Accum mixed_op = sla1 + (_Sat signed long _Accum)ssf1;
    
    /* Boundary comparisons that should trigger the uncovered logic */
    if (usf1 > usf2) {
        /* This comparison should force range analysis */
        volatile _Sat unsigned short _Fract usf_div = usf1 / usf2;
        (void)usf_div;
    }
    
    if (ssf1 < ssf2) {
        /* Another boundary comparison */
        volatile _Sat signed short _Fract ssf_mul = ssf1 * ssf2;
        (void)ssf_mul;
    }
    
    /* Test overflow builtins with fixed-point */
    _Sat signed short _Fract ssf3;
    int overflow = __builtin_mul_overflow(ssf1, ssf2, &ssf3);
    (void)overflow;
    
    /* Force evaluation of accumulator boundaries */
    if (ula1 > 255.0k) {
        volatile _Sat unsigned long _Accum ula_sat = ula1 + 0.0001k;
        (void)ula_sat;
    }
}

__attribute__((optimize("O3")))
static void test_mixed_precision_operations(void) {
    /* Create values at precise boundaries */
    volatile _Sat unsigned short _Fract max_usf = 0.9999r;
    volatile _Sat unsigned short _Fract min_usf = 0.0000r;
    volatile _Sat signed short _Fract max_ssf = 0.9999r;
    volatile _Sat signed short _Fract min_ssf = -0.9999r;
    
    /* Mixed precision chain */
    _Sat signed long _Accum acc = 0.0k;
    
    for (int i = 0; i < 10; i++) {
        /* Control flow dependent on fixed-point ranges */
        _Sat unsigned short _Fract temp = max_usf * min_usf;
        
        if (temp > 0.5r) {
            acc += (_Sat signed long _Accum)max_ssf;
        } else {
            acc += (_Sat signed long _Accum)min_ssf;
        }
        
        /* Modify values to approach boundaries */
        max_usf = max_usf * 0.9r;
        min_usf = min_usf + 0.1r;
    }
    
    /* Final boundary check */
    if (acc > 0.0k) {
        volatile _Sat signed long _Accum final_acc = acc * 1.1k;
        (void)final_acc;
    }
}

/* Struct containing fixed-point values */
struct FixedPointContainer {
    _Sat unsigned short _Fract usf_array[4];
    _Sat signed long _Accum sla;
    _Sat signed short _Fract ssf;
};

__attribute__((optimize("O3")))
static void test_struct_operations(void) {
    struct FixedPointContainer container = {
        .usf_array = {0.9999r, 0.5r, 0.25r, 0.125r},
        .sla = 127.9999k,
        .ssf = -0.5r
    };
    
    /* Operations on struct members */
    for (int i = 0; i < 4; i++) {
        container.usf_array[i] = container.usf_array[i] * 0.9999r;
        
        /* Boundary comparison in loop */
        if (container.usf_array[i] > 0.75r) {
            container.sla += (_Sat signed long _Accum)container.usf_array[i];
        }
    }
    
    /* Mixed operation with struct member */
    container.ssf = container.ssf * (_Sat signed short _Fract)container.sla;
    
    /* Overflow check with struct member */
    _Sat signed short _Fract result;
    int overflow = __builtin_add_overflow(container.ssf, 0.9999r, &result);
    (void)overflow;
    
    /* Final boundary comparison */
    if (container.sla > 128.0k || container.sla < -128.0k) {
        volatile _Sat signed long _Accum clamped = container.sla;
        (void)clamped;
    }
}

__attribute__((optimize("O3")))
static void test_extreme_boundaries(void) {
    /* Values at absolute boundaries */
    volatile _Sat unsigned short _Fract zero = 0.0r;
    volatile _Sat unsigned short _Fract near_one = 0.999999r;
    volatile _Sat signed short _Fract neg_one = -0.999999r;
    volatile _Sat signed short _Fract near_zero = 0.000001r;
    
    /* Operations that should saturate */
    _Sat unsigned short _Fract sat1 = near_one * near_one;  /* Should saturate near 1.0 */
    _Sat signed short _Fract sat2 = neg_one * neg_one;      /* Should be near 1.0 */
    
    /* Division near boundaries */
    _Sat unsigned short _Fract div1 = near_one / near_zero;
    _Sat signed short _Fract div2 = neg_one / near_zero;
    
    /* Comparisons that should trigger the uncovered range logic */
    if (sat1 > 0.999r) {
        volatile _Sat unsigned short _Fract adjusted = sat1 - 0.0001r;
        (void)adjusted;
    }
    
    if (div2 < -0.999r) {
        volatile _Sat signed short _Fract adjusted = div2 + 0.0001r;
        (void)adjusted;
    }
    
    /* Chain of operations to force complex range analysis */
    _Sat signed long _Accum chain = 0.0k;
    for (int i = 0; i < 8; i++) {
        chain = chain * 1.5k + (_Sat signed long _Accum)sat2;
        
        /* Conditional based on accumulated value */
        if (chain > 100.0k || chain < -100.0k) {
            chain = chain * 0.5k;
        }
    }
}

int main(void) {
    /* Prevent dead code elimination */
    volatile int result = 0;
    
    /* Execute all test functions */
    test_boundary_comparisons();
    test_mixed_precision_operations();
    test_struct_operations();
    test_extreme_boundaries();
    
    /* Aggregate results to prevent optimization */
    result = 1;
    
    printf("Fixed-point tests completed: %d\n", result);
    
    return 0;
}
