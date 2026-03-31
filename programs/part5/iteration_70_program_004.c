#include <stdio.h>
#include <stdint.h>

// Enable fixed-point extensions
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"

// Test function with aggressive optimization
__attribute__((optimize("O3")))
void test_boundary_comparisons(void) {
    // Initialize fixed-point variables at boundaries
    volatile _Sat unsigned short _Fract usf_min = 0.0ur;
    volatile _Sat unsigned short _Fract usf_max = 0.9999ur;
    volatile _Sat signed short _Fract ssf_min = -1.0r;
    volatile _Sat signed short _Fract ssf_max = 0.9999r;
    
    // Mixed precision operations
    volatile _Sat unsigned long _Accum ula_min = 0.0ulk;
    volatile _Sat unsigned long _Accum ula_max = 255.999999999ulk;
    volatile _Sat signed long _Accum sla_min = -256.0lk;
    volatile _Sat signed long _Accum sla_max = 255.999999999lk;
    
    // Force range analysis through arithmetic operations
    for (int i = 0; i < 10; i++) {
        // Operations that approach boundaries
        usf_max = usf_max * 0.9999ur;
        ssf_min = ssf_min * 0.9999r;
        
        // Mixed-type operations forcing conversions
        ula_max = ula_max * (_Sat unsigned long _Accum)usf_max;
        sla_min = sla_min * (_Sat signed long _Accum)ssf_min;
        
        // Boundary comparisons that should trigger the uncovered code
        if (ula_max > ula_min * 2.0ulk) {
            // This comparison should exercise a_high.sgt(max_r) logic
            volatile _Sat unsigned short _Fract temp = usf_max;
            usf_max = usf_max / 1.0001ur;
        }
        
        if (sla_min < sla_max * 0.5lk) {
            // This should trigger min_s/min_r initialization
            volatile _Sat signed short _Fract temp = ssf_min;
            ssf_min = ssf_min / 1.0001r;
        }
    }
}

// Function with overflow checks
__attribute__((optimize("O2")))
void test_overflow_operations(void) {
    _Sat unsigned long _Accum ula1 = 200.0ulk;
    _Sat unsigned long _Accum ula2 = 200.0ulk;
    _Sat signed long _Accum sla1 = 200.0lk;
    _Sat signed long _Accum sla2 = -200.0lk;
    
    // Use builtins to force overflow analysis
    _Sat unsigned long _Accum mul_result;
    int overflow = __builtin_mul_overflow(ula1, ula2, &mul_result);
    
    _Sat signed long _Accum add_result;
    int overflow2 = __builtin_add_overflow(sla1, sla2, &add_result);
    
    // Comparisons that should trigger range analysis
    if (mul_result > 100.0ulk && !overflow) {
        volatile _Sat unsigned short _Fract usf = 0.5ur;
        usf = usf * 0.9999ur;
    }
    
    if (add_result < 0.0lk && !overflow2) {
        volatile _Sat signed short _Fract ssf = -0.5r;
        ssf = ssf / 1.0001r;
    }
}

// Struct with fixed-point members
struct FixedPointContainer {
    _Sat unsigned short _Fract usf_array[4];
    _Sat signed long _Accum sla_array[4];
    _Sat unsigned long _Accum ula_value;
};

__attribute__((optimize("O3")))
void test_struct_operations(void) {
    struct FixedPointContainer container;
    
    // Initialize with boundary values
    for (int i = 0; i < 4; i++) {
        container.usf_array[i] = (_Sat unsigned short _Fract)(i * 0.25ur);
        container.sla_array[i] = (_Sat signed long _Accum)((i - 2) * 100.0lk);
    }
    container.ula_value = 255.999999999ulk;
    
    // Operations on struct members
    for (int i = 0; i < 3; i++) {
        // Mixed operations that should force range analysis
        container.sla_array[i] = container.sla_array[i] * 
                                (_Sat signed long _Accum)container.usf_array[i];
        
        // Comparison that should trigger a_high == max_r && a_low.ugt(max_s)
        if (container.sla_array[i] > container.sla_array[i+1]) {
            container.usf_array[i] = container.usf_array[i] * 0.9ur;
        }
        
        // Force min_s shift/extend operations
        if (container.ula_value < 128.0ulk) {
            container.ula_value = container.ula_value * 2.0ulk;
        }
    }
}

// Complex mixed precision operations
__attribute__((optimize("O2")))
_Sat signed long _Accum test_mixed_precision(_Sat unsigned short _Fract usf, 
                                           _Sat signed long _Accum sla) {
    // Multiple conversions and operations
    _Sat signed short _Fract ssf = (_Sat signed short _Fract)usf;
    _Sat unsigned long _Accum ula = (_Sat unsigned long _Accum)ssf;
    
    // Operations that should exercise i_f_bits calculations
    _Sat signed long _Accum result = sla * (_Sat signed long _Accum)ula;
    result = result / (_Sat signed long _Accum)usf;
    
    // Boundary comparison
    if (result > 0.0lk && result < 100.0lk) {
        // This should trigger the uncovered comparison logic
        volatile _Sat unsigned short _Fract temp = usf;
        result = result * (_Sat signed long _Accum)(temp * 0.5ur);
    }
    
    return result;
}

int main(void) {
    // Prevent dead code elimination
    volatile int result = 0;
    
    // Execute all test functions
    test_boundary_comparisons();
    test_overflow_operations();
    test_struct_operations();
    
    // Test mixed precision with boundary values
    _Sat unsigned short _Fract usf_test = 0.9999ur;
    _Sat signed long _Accum sla_test = -255.999999999lk;
    
    _Sat signed long _Accum mixed_result = test_mixed_precision(usf_test, sla_test);
    
    // Force evaluation and prevent optimization
    result = (mixed_result != 0.0lk);
    
    printf("Fixed-point test result: %d\n", result);
    
    return 0;
}

#pragma GCC diagnostic pop
