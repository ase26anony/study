/* fixed-point-test.c - Targeting GCC's fixed-value range analysis */
#include <stdio.h>
#include <stdint.h>

/* Use volatile to prevent optimization from eliminating computations */
volatile int result = 0;

/* Function with aggressive optimization to trigger range analysis */
__attribute__((optimize("O3"))) 
void test_boundary_comparisons(void) {
    /* Test 1: Signed fixed-point types approaching boundaries */
    signed short _Fract sf_min = -0.9999r;  /* Near minimum */
    signed short _Fract sf_max = 0.9999r;   /* Near maximum */
    signed short _Fract sf_mid = 0.5r;
    
    /* Test 2: Unsigned saturated accumulators */
    _Sat unsigned long _Accum usa_min = 0.0uk;
    _Sat unsigned long _Accum usa_max = 65535.9999847412109375uk; /* ULACCUM_MAX */
    _Sat unsigned long _Accum usa_mid = 32768.0uk;
    
    /* Test 3: Mixed precision operations */
    unsigned short _Fract usf_val = 0.75ur;
    signed long _Accum sla_val = -10000.0lk;
    
    /* Boundary value comparisons that should trigger range analysis */
    if (sf_min > -1.0r) {  /* Always true for short _Fract */
        result += 1;
    }
    
    if (sf_max < 1.0r) {   /* Always true for short _Fract */
        result += 2;
    }
    
    /* Mixed-type comparison forcing precision conversion */
    if ((signed long _Accum)sf_mid > sla_val) {
        result += 4;
    }
    
    /* Saturation arithmetic at boundaries */
    _Sat unsigned long _Accum usa_sum = usa_max + 1.0uk;  /* Should saturate */
    if (usa_sum == usa_max) {  /* Testing saturation behavior */
        result += 8;
    }
    
    /* Multiplication near boundaries */
    _Sat unsigned long _Accum usa_prod = usa_mid * 2.0uk;
    if (usa_prod > usa_max) {  /* Should trigger range comparison */
        result += 16;
    }
}

/* Function with overflow checks using builtins */
__attribute__((optimize("O2")))
void test_overflow_checks(void) {
    signed long _Accum sla1 = 10000.0lk;
    signed long _Accum sla2 = 20000.0lk;
    signed long _Accum sla_result;
    
    /* Use builtin overflow check with fixed-point */
    int overflow = __builtin_mul_overflow((long)sla1, (long)sla2, (long*)&sla_result);
    if (overflow) {
        result += 32;
    }
    
    /* Division with boundary values */
    signed short _Fract sf_div = 0.5r / 0.25r;
    if (sf_div > 1.9r) {  /* Should be exactly 2.0 but clamped for _Fract */
        result += 64;
    }
}

/* Struct containing fixed-point values */
struct FixedPointContainer {
    _Sat unsigned short _Accum usa_values[4];
    signed short _Fract sf_values[4];
    unsigned short _Fract usf_values[4];
};

/* Function operating on struct arrays */
__attribute__((optimize("O3")))
void test_struct_operations(void) {
    struct FixedPointContainer container;
    
    /* Initialize with boundary values */
    container.usa_values[0] = 0.0uhk;           /* Minimum */
    container.usa_values[1] = 255.9999847412109375uhk; /* USACCUM_MAX */
    container.usa_values[2] = 127.5uhk;         /* Midpoint */
    container.usa_values[3] = 0.0uhk;
    
    container.sf_values[0] = -1.0r;             /* SFRACT_MIN */
    container.sf_values[1] = 0.9999r;           /* Near SFRACT_MAX */
    container.sf_values[2] = 0.0r;
    container.sf_values[3] = 0.5r;
    
    container.usf_values[0] = 0.0ur;            /* Minimum */
    container.usf_values[1] = 0.9999ur;         /* Near UFRACT_MAX */
    container.usf_values[2] = 0.25ur;
    container.usf_values[3] = 0.75ur;
    
    /* Perform operations that should trigger range analysis */
    for (int i = 0; i < 4; i++) {
        /* Multiplication pushing values to boundaries */
        container.usa_values[i] = container.usa_values[i] * 2.0uhk;
        
        /* Division creating extreme values */
        if (container.sf_values[i] != 0.0r) {
            container.sf_values[i] = 1.0r / container.sf_values[i];
        }
        
        /* Comparisons that depend on computed ranges */
        if (container.usa_values[i] > 200.0uhk) {
            result += 128;
        }
        
        if (container.sf_values[i] < -0.5r) {
            result += 256;
        }
    }
    
    /* Cross-type operations forcing conversions */
    for (int i = 0; i < 3; i++) {
        signed short _Fract mixed_result = 
            container.sf_values[i] * (signed short _Fract)container.usf_values[i+1];
        
        if (mixed_result > 0.5r || mixed_result < -0.5r) {
            result += 512;
        }
    }
}

/* Complex function with nested conditionals */
__attribute__((optimize("O2")))
void test_nested_range_analysis(void) {
    /* Create values that will exercise the comparison logic */
    _Sat unsigned long _Accum high_range = 4294967295.9999847412109375uk; /* Near max */
    _Sat unsigned long _Accum low_range = 0.0000152587890625uk; /* Near min */
    
    /* Operations that create boundary conditions */
    for (int i = 0; i < 10; i++) {
        high_range = high_range / 1.5uk;
        low_range = low_range * 2.0uk;
        
        /* Nested comparisons that should trigger the uncovered logic */
        if (high_range < 1000000.0uk) {
            if (low_range > 0.1uk) {
                if (high_range + low_range > 1000000.1uk) {
                    result += 1024;
                }
            }
        }
        
        /* Force saturation at boundaries */
        _Sat unsigned long _Accum test_sat = high_range * 1000.0uk;
        if (test_sat == 4294967295.9999847412109375uk) { /* ULACCUM_MAX */
            result += 2048;
        }
    }
}

/* Main function that orchestrates all tests */
int main(void) {
    printf("Starting fixed-point range analysis tests...\n");
    
    /* Run all test functions */
    test_boundary_comparisons();
    test_overflow_checks();
    test_struct_operations();
    test_nested_range_analysis();
    
    /* Print result to prevent dead code elimination */
    printf("Accumulated result: %d\n", result);
    
    /* Additional volatile operations to ensure all code executes */
    volatile int final_check = result;
    if (final_check > 0) {
        printf("Tests completed successfully.\n");
    }
    
    return 0;
}
