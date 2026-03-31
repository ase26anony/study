#include <stdio.h>
#include <stdint.h>

// Force aggressive optimization on specific functions
__attribute__((optimize("O3")))
void test_signed_fract_range(void) {
    // Use _Fract types to exercise fractional bit calculations
    signed short _Fract sf1 = 0.5r;
    signed short _Fract sf2 = -0.25r;
    unsigned short _Fract usf1 = 0.75ur;
    
    // Mixed precision operations forcing range analysis
    signed _Accum sa1 = 100.0k;
    signed _Accum sa2 = -50.0k;
    
    // Operations that approach boundaries
    for (int i = 0; i < 10; i++) {
        sf1 = sf1 * sf2;  // Multiplication changes range
        sa1 = sa1 + sa2;  // Addition moves toward negative
        
        // Boundary comparison - should trigger range analysis
        if (sf1 > 0.8r) {
            usf1 = usf1 / 2.0ur;
        }
        
        // Force comparison with extreme values
        if (sa1 < -100.0k) {
            sa1 = -100.0k;  // Clamp manually
        }
    }
    
    // Final comparison that might trigger the uncovered code
    volatile signed _Accum result = sf1 * sa1;
    (void)result;
}

__attribute__((optimize("O3")))
void test_saturation_arithmetic(void) {
    // Saturation types to force boundary clamping
    _Sat signed long _Accum ssa1 = 500.0lk;
    _Sat signed long _Accum ssa2 = 300.0lk;
    _Sat unsigned long _Accum usa1 = 1000.0ulk;
    
    // Operations that would overflow without saturation
    for (int i = 0; i < 5; i++) {
        // These multiplications approach max/min boundaries
        ssa1 = ssa1 * 2.0lk;
        ssa2 = ssa2 * -2.0lk;
        usa1 = usa1 * 1.5ulk;
        
        // Built-in overflow checks with fixed-point
        signed long _Accum tmp;
        if (__builtin_mul_overflow(ssa1, ssa2, &tmp)) {
            // Overflow occurred - boundary reached
            ssa1 = ssa1 / 2.0lk;
        }
        
        // Boundary comparisons
        if (ssa1 > 1000.0lk || ssa2 < -1000.0lk) {
            // Reset to middle range
            ssa1 = 0.0lk;
            ssa2 = 0.0lk;
        }
    }
    
    // Final operation that might saturate
    _Sat signed long _Accum final = ssa1 + ssa2;
    (void)final;
}

// Struct containing fixed-point values
struct FixedPointContainer {
    signed _Fract sf_array[4];
    unsigned _Accum ua_array[2];
    _Sat signed short _Accum ssa_array[3];
};

__attribute__((optimize("O3")))
void test_struct_operations(void) {
    struct FixedPointContainer container = {
        .sf_array = {0.1r, 0.2r, 0.3r, 0.4r},
        .ua_array = {100.0k, 200.0k},
        .ssa_array = {50.0hk, -50.0hk, 0.0hk}
    };
    
    // Operations on array elements
    for (int i = 0; i < 4; i++) {
        container.sf_array[i] = container.sf_array[i] * 2.0r;
        
        // Boundary check
        if (container.sf_array[i] > 0.9r) {
            container.sf_array[i] = 0.9r;
        }
    }
    
    // Mixed operations between array elements
    for (int i = 0; i < 2; i++) {
        container.ua_array[i] = container.ua_array[i] + 
                               (container.sf_array[i] * 100.0k);
        
        // Force range analysis with comparison
        if (container.ua_array[i] > 300.0k) {
            container.ua_array[i] = 300.0k;
        }
    }
    
    // Saturation arithmetic in struct
    for (int i = 0; i < 3; i++) {
        container.ssa_array[i] = container.ssa_array[i] * 3.0hk;
        
        // This should trigger saturation at boundaries
        if (__builtin_add_overflow(container.ssa_array[i], 
                                   container.ssa_array[(i+1)%3],
                                   &container.ssa_array[i])) {
            // Handle overflow
            container.ssa_array[i] = (i % 2) ? 127.0hk : -128.0hk;
        }
    }
}

__attribute__((optimize("O3")))
void test_boundary_comparisons(void) {
    // Values at or near boundaries
    signed _Accum near_max = 127.999k;  // Close to maximum
    signed _Accum near_min = -127.999k; // Close to minimum
    unsigned _Accum near_umax = 255.999uk; // Near unsigned max
    
    // Division operations that can produce extreme values
    signed _Accum div_result = near_max / 0.1k;
    
    // This comparison should trigger the uncovered range analysis
    // when a_high.sgt(max_r) || (a_high == max_r && a_low.ugt(max_s))
    if (div_result > 1000.0k || div_result < -1000.0k) {
        // Reset to safe range
        div_result = 0.0k;
    }
    
    // More boundary testing with different operations
    for (int i = 0; i < 8; i++) {
        near_max = near_max * 1.1k;
        near_min = near_min * 1.1k;
        near_umax = near_umax * 1.05uk;
        
        // Multiple comparisons to exercise the uncovered condition
        if (near_max > 100.0k && near_min < -100.0k) {
            // Cross-boundary operation
            signed _Accum diff = near_max - near_min;
            (void)diff;
        }
        
        if (near_umax > 200.0uk) {
            near_umax = near_umax / 2.0uk;
        }
    }
    
    volatile signed _Accum final_check = div_result + near_max + near_min;
    (void)final_check;
}

__attribute__((optimize("O3")))
void test_mixed_precision_conversions(void) {
    // Different fixed-point types
    signed short _Fract ssf = 0.5hr;
    signed _Accum sa = 50.0k;
    signed long _Accum sla = 1000.0lk;
    
    // Mixed precision operations requiring conversions
    for (int i = 0; i < 6; i++) {
        // These operations force precision conversions
        sa = sa + (signed _Accum)ssf;
        sla = sla * (signed long _Accum)sa;
        ssf = ssf * (signed short _Fract)(sa / 100.0k);
        
        // Boundary checks after conversions
        if (sa > 75.0k || sa < 25.0k) {
            sa = 50.0k;  // Reset to middle
        }
        
        if (sla > 5000.0lk) {
            sla = sla / 2.0lk;
        }
        
        // This comparison with converted values might trigger
        // the double-int range comparison logic
        if ((signed long _Accum)sa > sla / 10.0lk) {
            ssf = ssf * 0.9hr;
        }
    }
    
    volatile signed long _Accum mixed_result = sla + (signed long _Accum)sa;
    (void)mixed_result;
}

int main(void) {
    printf("Starting fixed-point range analysis tests...\n");
    
    // Call all test functions to exercise different code paths
    test_signed_fract_range();
    test_saturation_arithmetic();
    test_struct_operations();
    test_boundary_comparisons();
    test_mixed_precision_conversions();
    
    // Aggregate results in volatile variables to prevent optimization
    volatile int dummy = 0;
    
    printf("Tests completed.\n");
    
    return dummy;
}
