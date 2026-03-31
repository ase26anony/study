#include <stdint.h>
#include <stdio.h>
#include <string.h>

// Prevent inlining to maintain function boundaries for coverage
#define NOINLINE __attribute__((noinline))

// Fixed-point types if available
#ifdef __STDC_IEC_559__
#include <stdfix.h>
#endif

// Checksum function to prevent dead code elimination
static uint32_t checksum = 0;

NOINLINE static void update_checksum(uint64_t value) {
    checksum = (checksum * 31) ^ (uint32_t)value;
    checksum = (checksum * 31) ^ (uint32_t)(value >> 32);
}

// 1. Integer Range Boundary Tests
NOINLINE static void test_narrowing_conversions(void) {
    uint64_t wide_values[] = {
        0xFFFFFFFFFFFFFFFFULL,  // Max uint64
        0x8000000000000000ULL,  // INT64_MIN as unsigned
        0x7FFFFFFFFFFFFFFFULL,  // INT64_MAX
        0x00000000FFFFFFFFULL,  // Max uint32
        0x0000000080000000ULL,  // INT32_MIN as unsigned
        0x000000007FFFFFFFULL,  // INT32_MAX
    };
    
    for (size_t i = 0; i < sizeof(wide_values)/sizeof(wide_values[0]); i++) {
        // Narrowing conversions that require range analysis
        uint32_t narrow1 = (uint32_t)wide_values[i];
        int32_t narrow2 = (int32_t)wide_values[i];
        int16_t narrow3 = (int16_t)wide_values[i];
        uint8_t narrow4 = (uint8_t)wide_values[i];
        
        // Comparisons at type boundaries
        if (narrow1 > 0xFFFFFF00UL) {
            update_checksum(narrow1);
        }
        if (narrow2 < -1000 || narrow2 > 1000) {
            update_checksum(narrow2);
        }
        if ((int64_t)narrow3 * 2 > INT16_MAX) {
            update_checksum(narrow3);
        }
        
        // Shift operations with potential overflow
        uint32_t shifted = narrow1 << (narrow4 & 0x1F);
        update_checksum(shifted);
    }
}

// 2. Loop Bound Analysis with Complex Conditions
NOINLINE static void test_loop_range_analysis(void) {
    int32_t results = 0;
    
    // Outer loop with bitmasked bounds
    for (int32_t i = 1000 & 0xFFF; i < (2000 | 0x7FF); i += 3) {
        // Inner loop with dependent bounds
        for (int32_t j = (i & 0xFF); j < ((i ^ 0x7F) & 0x1FF); j += 5) {
            // Complex condition using bitwise ops
            if ((j & 0xF) == 0 || (j | 0x1) > 100) {
                results += j * i;
            }
        }
        
        // Another loop with shifting bounds
        int32_t k = i >> 2;
        while (k < (i << 1)) {
            results ^= k;
            k += (i & 0x3F) + 1;
        }
    }
    
    update_checksum(results);
    
    // Loop with modulo-based bounds
    uint64_t counter = 0;
    for (int32_t x = 0; x < 1000; x++) {
        int32_t bound = (x * 37) % 256;
        for (int32_t y = 0; y < bound; y++) {
            counter += x * y;
            if (counter > 0xFFFFFFFFULL) {
                counter &= 0xFFFFFFFFULL;
            }
        }
    }
    update_checksum(counter);
}

// 3. Fixed-Point and Saturation Arithmetic
NOINLINE static void test_saturation_arithmetic(void) {
    // Manual saturation arithmetic
    int32_t sat_min = -1000;
    int32_t sat_max = 1000;
    
    int32_t values[] = {500, 1500, -500, -1500, 0, 999, -999};
    
    for (size_t i = 0; i < sizeof(values)/sizeof(values[0]); i++) {
        int32_t val = values[i];
        
        // Saturation clamp
        int32_t saturated;
        if (val > sat_max) {
            saturated = sat_max;
        } else if (val < sat_min) {
            saturated = sat_min;
        } else {
            saturated = val;
        }
        
        // Operations that might overflow
        int32_t doubled = saturated * 2;
        if (doubled > sat_max) doubled = sat_max;
        if (doubled < sat_min) doubled = sat_min;
        
        update_checksum(saturated + doubled);
    }
    
#ifdef __STDC_IEC_559__
    // GCC fixed-point types if available
    _Accum acc = 0.5k;
    _Fract frac = 0.25r;
    
    for (int i = 0; i < 10; i++) {
        acc += 0.1k;
        frac += 0.05r;
        
        // Comparisons that require range analysis
        if (acc > 1.0k || frac < 0.0r) {
            update_checksum((uint64_t)(acc * 1000));
        }
    }
#endif
}

// 4. Conditional Code with Value-Dependent Dead Branches
NOINLINE static void test_conditional_ranges(void) {
    uint32_t x = 0xFFFFFF00;
    uint32_t y = 0x00000100;
    
    // Comparisons at extreme edges
    if (x > UINT32_MAX - 255) {
        update_checksum(x);
    }
    
    if (y < 256) {
        update_checksum(y);
    }
    
    // Chain of conditions that constrain ranges
    int32_t a = 500;
    if (a > 0 && a < 1000) {
        int32_t b = a * 2;
        if (b > 750 && b < 1500) {
            int32_t c = b + a;
            if (c > 1000 && c < 2000) {
                update_checksum(c);
            }
        }
    }
    
    // Modulo-constrained value
    uint16_t mod_val = 30000;
    uint16_t constrained = mod_val % 256;
    
    // This comparison should be analyzable
    if (constrained > 200 && constrained < 255) {
        update_checksum(constrained);
    }
}

// 5. Structs with Bit-Fields and Unions
NOINLINE static void test_bitfield_ranges(void) {
    struct BitFieldStruct {
        unsigned int small : 4;    // 0-15
        signed int signed_small : 5; // -16 to 15
        unsigned int medium : 10;  // 0-1023
        unsigned int large : 18;   // 0-262143
    } bfs;
    
    union BitFieldUnion {
        struct BitFieldStruct bits;
        uint32_t raw;
    } u;
    
    // Assign values at boundaries
    bfs.small = 15;      // Max for 4 bits
    bfs.signed_small = -8; // Middle of range
    bfs.medium = 1023;   // Max for 10 bits
    bfs.large = 131072;  // Middle of 18-bit range
    
    u.bits = bfs;
    update_checksum(u.raw);
    
    // Comparisons against bit-field capacities
    if (bfs.small == 15) {
        update_checksum(1);
    }
    
    if (bfs.signed_small > -16 && bfs.signed_small < 15) {
        update_checksum(2);
    }
    
    if (bfs.medium >= 0 && bfs.medium <= 1023) {
        update_checksum(3);
    }
    
    // Overflow in bit-field assignment
    unsigned int overflow_test = 20; // > 4-bit capacity
    bfs.small = overflow_test; // Should be truncated
    update_checksum(bfs.small);
}

// 6. Compiler Builtins for Overflow Detection
NOINLINE static void test_overflow_builtins(void) {
    int32_t of_results = 0;
    
    // Test cases with partially known ranges
    int32_t vals[] = {100, 1000, 10000, 100000};
    
    for (size_t i = 0; i < sizeof(vals)/sizeof(vals[0]); i++) {
        int32_t a = vals[i];
        int32_t b = vals[i] * 2;
        
        int32_t result;
        // Overflow checks that depend on range analysis
        if (!__builtin_add_overflow(a, b, &result)) {
            of_results += result;
        }
        
        if (!__builtin_mul_overflow(a, 3, &result)) {
            of_results ^= result;
        }
        
        // Conditional overflow test
        if (a > 0 && a < 1000) {
            // Range is known to be safe for this multiplication
            if (!__builtin_mul_overflow(a, 100, &result)) {
                of_results |= result;
            }
        }
    }
    
    update_checksum(of_results);
    
    // Loop with overflow checks
    uint64_t large_sum = 0;
    for (uint32_t i = 0; i < 10000; i++) {
        uint64_t temp;
        if (!__builtin_add_overflow(large_sum, i * 1000ULL, &temp)) {
            large_sum = temp;
        }
    }
    update_checksum(large_sum);
}

int main(void) {
    checksum = 0;
    
    test_narrowing_conversions();
    test_loop_range_analysis();
    test_saturation_arithmetic();
    test_conditional_ranges();
    test_bitfield_ranges();
    test_overflow_builtins();
    
    // Print checksum to ensure all code executes
    printf("Checksum: %u\n", checksum);
    
    return 0;
}
