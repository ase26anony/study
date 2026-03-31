#include <stdio.h>
#include <stdint.h>

// Use volatile to prevent optimization from removing computations
volatile int result = 0;

// Function with aggressive optimization
__attribute__((optimize("O3")))
void test_boundary_comparisons() {
    // Initialize fixed-point types with boundary values
    _Sat unsigned short _Fract usf_max = 0.9999r;  // Near max for unsigned short _Fract
    _Sat unsigned short _Fract usf_min = 0.0001r;  // Near min for unsigned short _Fract
    _Sat signed long _Accum sla_max = 0.999999999999999999lk;  // Near max
    _Sat signed long _Accum sla_min = -0.999999999999999999lk; // Near min
    
    // Mixed precision operations to force range analysis
    signed short _Fract ssf1 = 0.5r;
    signed short _Fract ssf2 = -0.25r;
    
    // Operations that approach boundaries
    for (int i = 0; i < 10; i++) {
        // Multiplication that can approach limits
        usf_max = usf_max * 1.1r;
        usf_min = usf_min * 0.9r;
        
        // Mixed type operations
        sla_max = sla_max + (_Accum)usf_max;
        sla_min = sla_min - (_Accum)usf_min;
        
        // Control flow dependent on fixed-point ranges
        if (usf_max > 0.8r && sla_max < 0.9lk) {
            ssf1 = ssf1 * 0.8r;
        }
        
        if (usf_min < 0.2r || sla_min > -0.8lk) {
            ssf2 = ssf2 / 0.7r;
        }
    }
    
    // Final boundary comparisons
    if (usf_max > 0.95r) {
        result += 1;
    }
    
    if (sla_min < -0.95lk) {
        result += 2;
    }
}

__attribute__((optimize("O3")))
void test_overflow_checks() {
    // Test overflow builtins with fixed-point
    _Sat unsigned _Accum ua1 = 0.8uk;
    _Sat unsigned _Accum ua2 = 0.7uk;
    _Sat unsigned _Accum ua_result;
    
    // Use builtins to check for overflow
    int overflow;
    
    // Multiplication overflow check
    overflow = __builtin_mul_overflow((unsigned long)ua1, (unsigned long)ua2, 
                                      (unsigned long*)&ua_result);
    if (overflow) {
        result += 4;
    }
    
    // Addition overflow check
    _Sat signed _Accum sa1 = 0.9k;
    _Sat signed _Accum sa2 = 0.8k;
    overflow = __builtin_add_overflow((long)sa1, (long)sa2, (long*)&sa1);
    if (overflow) {
        result += 8;
    }
    
    // Division near boundaries
    _Sat signed short _Accum ssa = 0.999k;
    for (int i = 0; i < 5; i++) {
        ssa = ssa / 0.5k;
        
        // Boundary comparison
        if (ssa > 1.5k || ssa < -1.5k) {
            result += 16;
            break;
        }
    }
}

// Struct containing fixed-point values
struct FixedPointContainer {
    _Sat unsigned long _Fract ulf;
    _Sat signed short _Accum ssa;
    _Fract f;
    unsigned _Accum ua;
};

__attribute__((optimize("O3")))
void test_struct_operations() {
    struct FixedPointContainer container[3];
    
    // Initialize with boundary values
    for (int i = 0; i < 3; i++) {
        container[i].ulf = (i == 0) ? 0.999999999999999999ulr : 0.5ulr;
        container[i].ssa = (i == 1) ? -0.999k : 0.75k;
        container[i].f = (i == 2) ? 0.99r : 0.25r;
        container[i].ua = 0.8uk;
    }
    
    // Perform operations on struct members
    for (int i = 0; i < 3; i++) {
        // Mixed operations
        container[i].ssa = container[i].ssa * (_Accum)container[i].f;
        container[i].ua = container[i].ua + (_Accum)container[i].ulf;
        
        // Boundary comparisons
        if (container[i].ssa > 0.9k && container[i].ua < 0.9uk) {
            container[i].f = container[i].f * 1.1r;
        }
        
        if (container[i].ulf < 0.1ulr || container[i].ssa < -0.9k) {
            container[i].ua = container[i].ua / 1.1uk;
        }
    }
    
    // Final comparison that should trigger the uncovered logic
    if (container[0].ssa > container[1].ssa && 
        container[0].ua == container[2].ua) {
        result += 32;
    }
}

__attribute__((optimize("O3")))
void test_extreme_boundaries() {
    // Test with values at exact boundaries
    _Sat unsigned _Fract uf_max = 0.999999r;  // Very close to 1.0
    _Sat unsigned _Fract uf_min = 0.000001r;  // Very close to 0.0
    _Sat signed _Accum sa_max = 0.999999k;    // Very close to 1.0
    _Sat signed _Accum sa_min = -0.999999k;   // Very close to -1.0
    
    // Operations designed to push boundaries
    for (int i = 0; i < 100; i++) {
        // These operations should saturate at boundaries
        uf_max = uf_max * 1.0001r;
        uf_min = uf_min * 0.9999r;
        sa_max = sa_max + 0.0001k;
        sa_min = sa_min - 0.0001k;
        
        // Comparisons that should exercise the uncovered code
        // by comparing high/low parts of double-int representations
        if (uf_max > 0.99999r) {
            // This should trigger comparisons with max_r/max_s
            result += 64;
        }
        
        if (sa_min < -0.99999k) {
            // This should trigger comparisons with min_r/min_s
            result += 128;
        }
        
        // Mixed precision comparison
        if ((_Accum)uf_max > sa_max || (_Accum)uf_min < sa_min) {
            result += 256;
        }
    }
}

int main() {
    // Run all tests to exercise the fixed-value range analysis
    test_boundary_comparisons();
    test_overflow_checks();
    test_struct_operations();
    test_extreme_boundaries();
    
    // Print result to prevent dead code elimination
    printf("Result: %d\n", result);
    
    return 0;
}
