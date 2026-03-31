#include <stdio.h>
#include <stdint.h>

// Force aggressive optimization on specific functions
__attribute__((optimize("O3")))
void test_short_fract_boundaries(void) {
    volatile short _Fract sf_min = 0x8000;  // Minimum: -1.0
    volatile short _Fract sf_max = 0x7FFF;  // Maximum: ~0.99997
    volatile short _Fract sf_zero = 0x0000; // Zero: 0.0
    volatile short _Fract sf_half = 0x4000; // 0.5
    
    // Mixed precision operations forcing range analysis
    _Sat unsigned long _Accum ula = 0.5uk;
    _Sat signed long _Accum sla = -0.5k;
    
    // Operations that approach boundaries
    for (int i = 0; i < 10; i++) {
        // Multiplication that can saturate
        _Sat unsigned short _Fract usf = (_Sat unsigned short _Fract)(0.9ur * (unsigned short _Fract)i / 10);
        
        // Division that can approach extremes
        _Sat signed short _Fract ssf = (_Sat signed short _Fract)((signed short _Fract)sf_half / (signed short _Fract)(i + 1));
        
        // Mixed-type arithmetic forcing conversions
        sla = (_Sat signed long _Accum)(sla * (_Sat signed long _Accum)ssf);
        ula = (_Sat unsigned long _Accum)(ula + (_Sat unsigned long _Accum)usf);
        
        // Boundary comparisons that should trigger the uncovered logic
        if (sla > (_Sat signed long _Accum)sf_max) {
            // Force saturation at upper bound
            sla = (_Sat signed long _Accum)sf_max;
        }
        
        if (ula < (_Sat unsigned long _Accum)sf_zero) {
            // Force saturation at lower bound
            ula = (_Sat unsigned long _Accum)sf_zero;
        }
    }
    
    // Final comparison that should exercise the range analysis
    if (sla == (_Sat signed long _Accum)sf_min || sla == (_Sat signed long _Accum)sf_max) {
        // Boundary case reached
        volatile int marker = 1;
        (void)marker;
    }
}

__attribute__((optimize("O3")))
void test_accum_overflow(void) {
    // Use builtins with fixed-point types
    _Sat signed long _Accum a = 0.5k;
    _Sat signed long _Accum b = 0.8k;
    _Sat signed long _Accum result;
    int overflow;
    
    // Multiplication overflow check
    overflow = __builtin_mul_overflow((long)a, (long)b, (long*)&result);
    if (overflow) {
        // Handle overflow - should trigger saturation logic
        result = (_Sat signed long _Accum)0x7FFFFFFFFFFFFFFFk; // Max positive
    }
    
    // Addition overflow check
    _Sat signed long _Accum c = 0.9k;
    overflow = __builtin_add_overflow((long)a, (long)c, (long*)&result);
    if (overflow) {
        result = (_Sat signed long _Accum)0x7FFFFFFFFFFFFFFFk;
    }
    
    // Division that can underflow
    _Sat signed short _Fract d = 0.0001r;
    _Sat signed short _Fract e = (_Sat signed short _Fract)(d / 10000.0r);
    
    // Comparison near boundaries
    if (result > 0.99k || e < 0.00001r) {
        volatile int boundary_flag = 1;
        (void)boundary_flag;
    }
}

// Struct containing fixed-point values
struct FixedPointContainer {
    _Sat unsigned short _Fract usf_array[4];
    _Sat signed long _Accum sla;
    short _Fract sf;
};

__attribute__((optimize("O3")))
void test_struct_operations(void) {
    struct FixedPointContainer container = {
        .usf_array = {0.1ur, 0.5ur, 0.9ur, 0.99ur},
        .sla = 0.0k,
        .sf = 0.0r
    };
    
    // Operations on array elements
    for (int i = 0; i < 4; i++) {
        // Multiplication that can saturate
        container.usf_array[i] = (_Sat unsigned short _Fract)
            (container.usf_array[i] * 1.1ur);
        
        // Accumulate into larger type
        container.sla = (_Sat signed long _Accum)
            (container.sla + (_Sat signed long _Accum)container.usf_array[i]);
        
        // Control flow dependent on fixed-point ranges
        if (container.usf_array[i] > 0.95ur) {
            // Near upper boundary
            container.sf = (_Sat short _Fract)(container.sf - 0.1r);
        } else if (container.usf_array[i] < 0.05ur) {
            // Near lower boundary
            container.sf = (_Sat short _Fract)(container.sf + 0.1r);
        }
    }
    
    // Final boundary check
    if (container.sla > 3.5k || container.sf < -0.9r) {
        volatile int struct_boundary = 1;
        (void)struct_boundary;
    }
}

__attribute__((optimize("O3")))
void test_mixed_precision_conversions(void) {
    // Initialize with boundary values
    _Sat unsigned long _Accum ula_max = 0xFFFFFFFFFFFFFFFFuk; // Max unsigned
    _Sat signed long _Accum sla_min = 0x8000000000000000k;    // Min signed
    _Sat signed long _Accum sla_max = 0x7FFFFFFFFFFFFFFFk;    // Max signed
    
    // Mixed precision operations
    short _Fract sf = 0.5r;
    _Sat unsigned short _Fract usf = 0.8ur;
    
    // Conversion chain that exercises range analysis
    _Sat signed long _Accum temp1 = (_Sat signed long _Accum)sf;
    _Sat unsigned long _Accum temp2 = (_Sat unsigned long _Accum)usf;
    
    // Operations that can overflow/underflow
    for (int i = 0; i < 5; i++) {
        temp1 = (_Sat signed long _Accum)(temp1 * 2.0k);
        temp2 = (_Sat unsigned long _Accum)(temp2 / 2.0uk);
        
        // Comparisons against boundaries
        if (temp1 >= sla_max || temp1 <= sla_min) {
            // At signed boundaries
            temp1 = (temp1 >= sla_max) ? sla_max : sla_min;
        }
        
        if (temp2 >= ula_max || temp2 == 0uk) {
            // At unsigned boundaries
            temp2 = (temp2 >= ula_max) ? ula_max : 0uk;
        }
    }
    
    // Complex comparison that should trigger the uncovered logic
    // This mimics a_high.sgt(max_r) || (a_high == max_r && a_low.ugt(max_s))
    _Sat signed long _Accum a_high = temp1;
    _Sat unsigned long _Accum a_low = temp2;
    
    if (a_high > 0.75k || (a_high == 0.75k && a_low > 0.5uk)) {
        volatile int complex_comparison = 1;
        (void)complex_comparison;
    }
}

int main(void) {
    volatile int result = 0;
    
    // Execute all test functions
    test_short_fract_boundaries();
    result += 1;
    
    test_accum_overflow();
    result += 2;
    
    test_struct_operations();
    result += 4;
    
    test_mixed_precision_conversions();
    result += 8;
    
    // Print result to prevent dead code elimination
    printf("Test result: %d\n", result);
    
    return 0;
}
