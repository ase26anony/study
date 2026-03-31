/* fixed-point-test.c - Targeting GCC's fixed-value.cc uncovered lines 264-277 */
#include <stdio.h>
#include <stdint.h>

/* Force aggressive optimization on specific functions */
__attribute__((optimize("O3")))
static void test_boundary_comparisons(void) {
    /* Mixed fixed-point types to trigger range analysis */
    volatile _Sat unsigned short _Fract usf1 = 0.9999r;  /* Near max */
    volatile _Sat signed short _Fract sf1 = -0.9999r;    /* Near min */
    volatile _Sat unsigned long _Accum ula1 = 255.9999k; /* Boundary value */
    volatile _Sat signed long _Accum sla1 = -255.9999k;
    
    /* Operations that force range computations */
    _Sat unsigned short _Fract usf2 = usf1 * 0.5r;
    _Sat signed short _Fract sf2 = sf1 / 0.25r;
    
    /* Boundary comparisons - should trigger the uncovered condition */
    if (usf1 > usf2) {
        /* This comparison forces range analysis */
        volatile _Sat unsigned long _Accum ula2 = ula1 + 0.0001k;
        if (ula2 > ula1) {
            /* Nested comparisons for complex range tracking */
            _Sat signed long _Accum sla2 = sla1 - 0.0001k;
            if (sla2 < sla1) {
                /* Force evaluation of boundary conditions */
                volatile int marker = 1;
                (void)marker;
            }
        }
    }
}

__attribute__((optimize("O3")))
static void test_mixed_precision_ops(void) {
    /* Different fractional bit configurations */
    _Sat unsigned short _Fract usf = 0.75r;
    _Sat signed long _Accum sla = 100.5k;
    _Sat unsigned long _Accum ula = 200.25k;
    
    /* Mixed precision operations */
    _Sat signed long _Accum result1 = sla * (_Sat signed long _Accum)usf;
    _Sat unsigned long _Accum result2 = ula / (_Sat unsigned long _Accum)usf;
    
    /* Comparisons near boundaries */
    if (result1 > 50.0k && result1 < 150.0k) {
        /* Range-dependent control flow */
        volatile _Sat signed short _Fract sf_bound = 0.9999r;
        if (sf_bound == 0.9999r) {
            /* Exact boundary comparison */
            volatile int trigger = 1;
            (void)trigger;
        }
    }
    
    if (result2 > 250.0k || result2 < 300.0k) {
        /* OR condition for range analysis */
        volatile _Sat unsigned short _Fract usf_min = 0.0r;
        volatile _Sat unsigned short _Fract usf_max = 1.0r;
        
        /* Force min/max range initialization */
        if (usf_min == 0.0r && usf_max == 1.0r) {
            volatile int range_check = 1;
            (void)range_check;
        }
    }
}

__attribute__((optimize("O3")))
static void test_overflow_checks(void) {
    /* Use builtins with fixed-point types */
    _Sat unsigned short _Fract usf_a = 0.9r;
    _Sat unsigned short _Fract usf_b = 0.9r;
    _Sat unsigned short _Fract usf_result;
    int overflow;
    
    /* Multiplication overflow check */
    overflow = __builtin_mul_overflow(usf_a, usf_b, &usf_result);
    
    if (overflow) {
        /* Saturation occurred - at boundary */
        volatile _Sat signed long _Accum sla_max = 255.9999k;
        volatile _Sat signed long _Accum sla_min = -255.9999k;
        
        /* Comparisons that should trigger the uncovered code */
        if (sla_max > 0.0k && sla_min < 0.0k) {
            volatile int sat_marker = 1;
            (void)sat_marker;
        }
    }
    
    /* Addition with potential saturation */
    _Sat signed long _Accum sla1 = 200.0k;
    _Sat signed long _Accum sla2 = 100.0k;
    _Sat signed long _Accum sla_sum;
    
    overflow = __builtin_add_overflow(sla1, sla2, &sla_sum);
    
    /* Complex boundary condition */
    if (sla_sum > 250.0k || (sla_sum == 300.0k && sla1 > 150.0k)) {
        /* This mirrors the uncovered condition structure */
        volatile int complex_check = 1;
        (void)complex_check;
    }
}

/* Struct with fixed-point members */
struct fixed_point_container {
    _Sat unsigned short _Fract usf;
    _Sat signed long _Accum sla;
    _Sat unsigned long _Accum ula;
};

__attribute__((optimize("O3")))
static void test_struct_operations(void) {
    struct fixed_point_container container[3];
    
    /* Initialize with boundary values */
    container[0].usf = 0.0r;
    container[0].sla = -255.9999k;
    container[0].ula = 0.0k;
    
    container[1].usf = 0.5r;
    container[1].sla = 0.0k;
    container[1].ula = 127.5k;
    
    container[2].usf = 1.0r;
    container[2].sla = 255.9999k;
    container[2].ula = 255.9999k;
    
    /* Array operations that force range analysis through memory */
    for (int i = 0; i < 3; i++) {
        /* Operations on struct members */
        container[i].sla = container[i].sla * 1.1k;
        container[i].ula = container[i].ula / 1.1k;
        
        /* Boundary comparisons in loop */
        if (container[i].usf > 0.75r || 
            (container[i].usf == 0.75r && container[i].sla > 100.0k)) {
            /* Direct match to uncovered condition pattern */
            volatile int struct_check = i;
            (void)struct_check;
        }
    }
}

__attribute__((optimize("O3")))
static void test_extreme_values(void) {
    /* Minimum and maximum values for each type */
    volatile _Sat unsigned short _Fract usf_min = 0.0r;
    volatile _Sat unsigned short _Fract usf_max = 1.0r;
    volatile _Sat signed short _Fract sf_min = -1.0r;
    volatile _Sat signed short _Fract sf_max = 1.0r;
    volatile _Sat unsigned long _Accum ula_min = 0.0k;
    volatile _Sat unsigned long _Accum ula_max = 255.9999k;
    volatile _Sat signed long _Accum sla_min = -255.9999k;
    volatile _Sat signed long _Accum sla_max = 255.9999k;
    
    /* Operations that push to boundaries */
    _Sat unsigned short _Fract usf_test = usf_max * usf_max;  /* Should saturate */
    _Sat signed long _Accum sla_test = sla_max + 0.0001k;     /* Should saturate */
    
    /* Complex boundary condition matching uncovered pattern */
    if (sla_test == sla_max || 
        (sla_test > sla_min && sla_test < sla_max)) {
        /* Nested comparisons for range analysis */
        if (usf_test == usf_max && ula_min == 0.0k) {
            volatile int extreme_marker = 1;
            (void)extreme_marker;
        }
    }
    
    /* Division near zero */
    _Sat signed long _Accum sla_near_zero = 0.0001k;
    _Sat signed long _Accum sla_div = sla_max / sla_near_zero;
    
    /* Should trigger large value range */
    if (sla_div > 1000000.0k || sla_div < -1000000.0k) {
        volatile int large_range = 1;
        (void)large_range;
    }
}

int main(void) {
    volatile int result = 0;
    
    /* Execute all test functions */
    test_boundary_comparisons();
    result += 1;
    
    test_mixed_precision_ops();
    result += 2;
    
    test_overflow_checks();
    result += 3;
    
    test_struct_operations();
    result += 4;
    
    test_extreme_values();
    result += 5;
    
    /* Print to prevent dead code elimination */
    printf("Fixed-point test result: %d\n", result);
    
    return 0;
}
