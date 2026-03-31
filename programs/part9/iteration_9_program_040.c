#include <stdint.h>
#include <stdio.h>
#include <string.h>

// Prevent inlining to maintain function boundaries for coverage
#define NOINLINE __attribute__((noinline))

// Fixed-point types if available
#ifdef __STDC_IEC_559__
typedef _Fract fract_t;
typedef _Accum accum_t;
#else
typedef int32_t fract_t;
typedef int64_t accum_t;
#endif

// Checksum/hash to prevent dead code elimination
static uint32_t checksum = 0;

NOINLINE static void update_checksum(uint64_t value) {
    checksum = (checksum * 31) + (value & 0xFFFFFFFF);
    checksum ^= (value >> 32) & 0xFFFFFFFF;
}

// 1. Integer Range Boundary Tests
NOINLINE static uint32_t test_narrowing_conversions(void) {
    uint32_t local_sum = 0;
    
    // Constants at type boundaries
    int64_t large_positive = 0x7FFFFFFFFFFFFFFFLL;
    int64_t large_negative = 0x8000000000000000LL;
    uint64_t large_unsigned = 0xFFFFFFFFFFFFFFFFULL;
    
    // Narrowing conversions that require range analysis
    int32_t narrow1 = (int32_t)large_positive;  // May overflow
    int32_t narrow2 = (int32_t)large_negative;  // Sign extension
    uint32_t narrow3 = (uint32_t)large_unsigned; // Truncation
    
    // Shifts that may overflow
    int32_t shift1 = narrow1 << 3;
    int32_t shift2 = narrow2 >> 2;
    uint32_t shift3 = narrow3 << 1;
    
    // Comparisons against type limits
    if (narrow1 > INT32_MAX - 100) {
        local_sum += 1;
    }
    if (narrow2 < INT32_MIN + 50) {
        local_sum += 2;
    }
    if (narrow3 > UINT32_MAX - 1000) {
        local_sum += 4;
    }
    
    // Complex boundary comparisons
    int64_t val = (int64_t)narrow1 * 100;
    if (val > INT64_MAX / 2 || val < INT64_MIN / 2) {
        local_sum += 8;
    }
    
    update_checksum(local_sum + narrow1 + narrow2 + narrow3);
    return local_sum;
}

// 2. Loop Bound Analysis with Complex Conditions
NOINLINE static uint32_t test_loop_range_analysis(void) {
    uint32_t local_sum = 0;
    
    // Variables with constrained ranges
    uint32_t a = 1000;
    uint32_t b = 2000;
    uint32_t c = 7;
    
    // Loop with bitwise operations in bounds
    for (uint32_t i = a & 0xFFF; i < (b | 0x7FF); i += c) {
        // Nested loop with dependent bounds
        for (uint32_t j = (i & 0x3F); j < 100; j += (c & 0x3)) {
            local_sum += i * j;
            
            // Additional range-restricting condition
            if (j > 50 && j < 75) {
                uint32_t k = (i ^ j) & 0xFF;
                local_sum += k;
            }
        }
        
        // Complex exit condition involving shifted values
        if ((i << 2) > 0x3000) {
            break;
        }
    }
    
    // Another loop with signed arithmetic
    int32_t start = -100;
    int32_t end = 100;
    int32_t step = 3;
    
    for (int32_t i = start; i < end; i += step) {
        // Condition that depends on i's range
        if (i > 0 && i < 50) {
            local_sum += i * 2;
        } else if (i < 0 && i > -50) {
            local_sum += i * 3;
        }
        
        // Shift with sign extension consideration
        int32_t shifted = i << 1;
        if (shifted > 100 || shifted < -100) {
            local_sum += 5;
        }
    }
    
    update_checksum(local_sum);
    return local_sum;
}

// 3. Fixed-Point and Saturation Arithmetic
NOINLINE static uint32_t test_saturation_arithmetic(void) {
    uint32_t local_sum = 0;
    
    // Saturation clamping functions
    static int32_t sat_add(int32_t a, int32_t b) {
        int64_t result = (int64_t)a + b;
        if (result > INT32_MAX) return INT32_MAX;
        if (result < INT32_MIN) return INT32_MIN;
        return (int32_t)result;
    }
    
    static int32_t sat_mul(int32_t a, int32_t b) {
        int64_t result = (int64_t)a * b;
        if (result > INT32_MAX) return INT32_MAX;
        if (result < INT32_MIN) return INT32_MIN;
        return (int32_t)result;
    }
    
    // Test saturation with boundary values
    int32_t max_val = INT32_MAX;
    int32_t min_val = INT32_MIN;
    
    local_sum += sat_add(max_val, 1);      // Should saturate to INT32_MAX
    local_sum += sat_add(min_val, -1);     // Should saturate to INT32_MIN
    local_sum += sat_mul(max_val, 2);      // Should saturate to INT32_MAX
    local_sum += sat_mul(min_val, 2);      // Should saturate to INT32_MIN
    
    // Test with values near boundaries
    for (int i = -5; i <= 5; i++) {
        int32_t val = INT32_MAX + i;
        local_sum += sat_add(val, 100);
    }
    
#ifdef __STDC_IEC_559__
    // Fixed-point arithmetic if supported
    fract_t f1 = 0.5r;
    fract_t f2 = 0.75r;
    accum_t acc = 100.0k;
    
    // Operations that may saturate
    fract_t f3 = f1 + f2;
    accum_t acc2 = acc * 2.0k;
    
    local_sum += (uint32_t)(f3 * 1000);
    local_sum += (uint32_t)acc2;
#endif
    
    update_checksum(local_sum);
    return local_sum;
}

// 4. Structs with Bit-Fields and Unions
NOINLINE static uint32_t test_bitfield_ranges(void) {
    uint32_t local_sum = 0;
    
    // Struct with various bit-fields
    struct BitFields {
        unsigned int small : 3;    // 0-7
        signed int signed_small : 4; // -8 to 7
        unsigned int medium : 10;  // 0-1023
        signed int signed_medium : 12; // -2048 to 2047
        unsigned int large : 20;   // 0-1048575
    } bf;
    
    // Union to test type punning
    union ValuePun {
        uint32_t full;
        struct {
            uint16_t low;
            uint16_t high;
        } parts;
    } pun;
    
    // Assign values to bit-fields
    bf.small = 5;           // Within range
    bf.signed_small = -3;   // Within range
    bf.medium = 500;        // Within range
    bf.signed_medium = 1500; // Within range
    bf.large = 500000;      // Within range
    
    // Comparisons that test bit-field range understanding
    if (bf.small > 3 && bf.small < 7) {
        local_sum += 1;
    }
    
    if (bf.signed_small >= -4 && bf.signed_small <= 4) {
        local_sum += 2;
    }
    
    // Test near upper bound
    bf.medium = 1020;
    if (bf.medium > 1000) {
        local_sum += 4;
    }
    
    // Test with union type punning
    pun.full = 0x12345678;
    if (pun.parts.high > 0x1000 && pun.parts.low < 0x5678) {
        local_sum += 8;
    }
    
    // Complex condition with multiple bit-fields
    if ((bf.small | bf.medium) > 200) {
        local_sum += 16;
    }
    
    update_checksum(local_sum + bf.small + bf.medium + bf.large);
    return local_sum;
}

// 5. Compiler Builtins for Overflow Detection
NOINLINE static uint32_t test_overflow_builtins(void) {
    uint32_t local_sum = 0;
    
    // Variables with partially known ranges
    int32_t a = 100;
    int32_t b = 200;
    int32_t c = INT32_MAX - 100;
    
    int overflow;
    
    // Basic overflow checks
    if (__builtin_add_overflow(a, b, &overflow)) {
        local_sum += 1;
    }
    
    if (__builtin_mul_overflow(a, b, &overflow)) {
        local_sum += 2;
    }
    
    // Near overflow boundary
    if (__builtin_add_overflow(c, 150, &overflow)) {
        local_sum += 4;
    }
    
    // In loops with range-constrained variables
    for (int i = 0; i < 100; i++) {
        int32_t x = i * 1000;
        int32_t y = 50000;
        
        if (__builtin_add_overflow(x, y, &overflow)) {
            local_sum += i;
        }
    }
    
    // Multiple operations in sequence
    int32_t result = 1;
    for (int i = 1; i <= 20; i++) {
        if (__builtin_mul_overflow(result, i, &result)) {
            local_sum += 1000;
            break;
        }
    }
    
    // Unsigned overflow checks
    uint32_t u1 = UINT32_MAX - 10;
    uint32_t u2 = 20;
    uint32_t u_result;
    
    if (__builtin_uadd_overflow(u1, u2, &u_result)) {
        local_sum += 8;
    }
    
    // Complex condition with overflow check
    int32_t x = 1000000;
    int32_t y = 2000000;
    int32_t z;
    
    if (!__builtin_add_overflow(x, y, &z)) {
        if (z > 1500000 && z < 4000000) {
            local_sum += 16;
        }
    }
    
    update_checksum(local_sum + result + u_result);
    return local_sum;
}

int main(void) {
    uint32_t total = 0;
    
    // Run all tests
    total += test_narrowing_conversions();
    total += test_loop_range_analysis();
    total += test_saturation_arithmetic();
    total += test_bitfield_ranges();
    total += test_overflow_builtins();
    
    // Use results to prevent optimization
    printf("Result: %u (checksum: %u)\n", total, checksum);
    
    return (total > 0) ? 0 : 1;
}
