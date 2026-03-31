#include <stdint.h>
#include <stdio.h>
#include <string.h>

// Prevent inlining to maintain function boundaries for coverage
#define NOINLINE __attribute__((noinline))

// Checksum function to prevent dead code elimination
static inline uint32_t checksum(uint32_t *data, size_t len) {
    uint32_t sum = 0;
    for (size_t i = 0; i < len; i++) {
        sum = (sum << 3) ^ (sum >> 29) ^ data[i];
    }
    return sum;
}

// 1. Integer Range Boundary Tests
NOINLINE uint32_t test_narrowing_conversions(void) {
    uint32_t results[20];
    int idx = 0;
    
    // Constants at type boundaries
    int64_t large_positive = 0x7FFFFFFFFFFFFFFFLL;
    int64_t large_negative = 0x8000000000000000LL;
    uint64_t large_unsigned = 0xFFFFFFFFFFFFFFFFULL;
    
    // Narrowing conversions that require range analysis
    int32_t narrow1 = (int32_t)large_positive;  // May overflow
    int32_t narrow2 = (int32_t)large_negative;  // Sign extension
    uint32_t narrow3 = (uint32_t)large_unsigned; // Truncation
    
    results[idx++] = narrow1;
    results[idx++] = narrow2;
    results[idx++] = narrow3;
    
    // Shifts that may overflow
    uint32_t x = 0x80000000U;
    uint32_t y = x >> 1;
    uint32_t z = x << 1;  // Overflow for signed interpretation
    
    results[idx++] = y;
    results[idx++] = z;
    
    // Comparisons at boundaries
    int boundary_tests = 0;
    if (narrow1 > 0x7FFFFFFF - 100) boundary_tests |= 1;
    if (narrow2 < -0x7FFFFFFF + 100) boundary_tests |= 2;
    if (narrow3 > 0xFFFFFFFFU - 1000) boundary_tests |= 4;
    
    results[idx++] = boundary_tests;
    
    // Mixed-width operations
    int64_t mixed = (int64_t)narrow1 * (int64_t)narrow2;
    int32_t mixed_narrow = (int32_t)mixed;
    
    results[idx++] = (uint32_t)(mixed >> 32);
    results[idx++] = (uint32_t)mixed;
    results[idx++] = mixed_narrow;
    
    return checksum(results, idx);
}

// 2. Loop Bound Analysis with Complex Conditions
NOINLINE uint32_t test_loop_range_analysis(void) {
    uint32_t results[50];
    int idx = 0;
    uint32_t sum = 0;
    
    // Outer loop with bitmasked bound
    uint32_t outer_bound = 1000;
    for (uint32_t i = (outer_bound & 0x3FF); i < (outer_bound | 0x7FF); i += 17) {
        // Inner loop with dependent bounds
        uint32_t inner_start = i & 0xFF;
        uint32_t inner_end = (i | 0x7F) + 50;
        
        for (uint32_t j = inner_start; j < inner_end && j < 200; j += 5) {
            // Complex condition using bitwise ops
            if (((j ^ i) & 0x3F) == 0) {
                sum += j * i;
            }
            results[idx++ % 50] = sum;
        }
        
        // Loop with shifting bound
        uint32_t shift_bound = 1 << ((i >> 3) & 0x7);
        for (uint32_t k = 0; k < shift_bound && k < 64; k++) {
            sum += (k << (i & 0x7));
            results[idx++ % 50] = sum;
        }
    }
    
    // Nested loops with wrap-around conditions
    int32_t signed_i, signed_j;
    for (signed_i = -100; signed_i < 100; signed_i += 7) {
        int32_t bound = (signed_i < 0) ? -signed_i : signed_i;
        bound = (bound & 0x3F) + 10;
        
        for (signed_j = -bound; signed_j < bound; signed_j += 3) {
            int32_t product = signed_i * signed_j;
            // Condition that depends on range analysis
            if (product > 0x3FFFFFFF || product < -0x3FFFFFFF) {
                sum += 1;
            }
            results[idx++ % 50] = sum;
        }
    }
    
    results[49] = sum;
    return checksum(results, 50);
}

// 3. Fixed-Point and Saturation Arithmetic
NOINLINE uint32_t test_saturation_arithmetic(void) {
    uint32_t results[30];
    int idx = 0;
    
    // Manual saturation arithmetic
    int32_t sat_add(int32_t a, int32_t b) {
        int64_t result = (int64_t)a + (int64_t)b;
        if (result > 0x7FFFFFFF) return 0x7FFFFFFF;
        if (result < -0x80000000) return -0x80000000;
        return (int32_t)result;
    }
    
    int32_t sat_mul(int32_t a, int32_t b) {
        int64_t result = (int64_t)a * (int64_t)b;
        if (result > 0x7FFFFFFF) return 0x7FFFFFFF;
        if (result < -0x80000000) return -0x80000000;
        return (int32_t)result;
    }
    
    // Test saturation at boundaries
    int32_t max_val = 0x7FFFFFFF;
    int32_t min_val = 0x80000000;
    int32_t large_val = 0x70000000;
    
    results[idx++] = sat_add(max_val, 1);
    results[idx++] = sat_add(min_val, -1);
    results[idx++] = sat_add(large_val, large_val);
    results[idx++] = sat_mul(max_val, 2);
    results[idx++] = sat_mul(min_val, 2);
    
    // Progressive saturation in loop
    int32_t accum = 0;
    for (int i = 0; i < 100; i += 3) {
        accum = sat_add(accum, 0x10000000);
        results[idx++ % 30] = accum;
    }
    
    // GCC fixed-point types if available
    #ifdef __FRACT_FBIT__
    _Fract f1 = 0.5r;
    _Fract f2 = 0.7r;
    _Accum a1 = 100.5k;
    _Accum a2 = 200.3k;
    
    // Fixed-point operations that may saturate
    _Fract f3 = f1 + f2;
    _Accum a3 = a1 * a2;
    
    results[idx++ % 30] = *(uint32_t*)&f3;
    results[idx++ % 30] = *(uint32_t*)&a3;
    #endif
    
    return checksum(results, 30);
}

// 4. Structs with Bit-Fields and Unions
NOINLINE uint32_t test_bitfield_ranges(void) {
    uint32_t results[20];
    int idx = 0;
    
    // Struct with various bit-fields
    struct BitFieldStruct {
        unsigned int small : 3;    // 0-7
        signed int signed_small : 4; // -8 to 7
        unsigned int medium : 10;  // 0-1023
        signed int signed_medium : 12; // -2048 to 2047
        unsigned int large : 31;   // 0-2147483647
    } bfs;
    
    // Union to test type punning
    union BitFieldUnion {
        struct BitFieldStruct bf;
        uint64_t raw;
    } u;
    
    // Assign values at bit-field boundaries
    bfs.small = 7;          // Max for 3 bits
    bfs.signed_small = -8;  // Min for 4-bit signed
    bfs.medium = 1023;      // Max for 10 bits
    bfs.signed_medium = 2047; // Max for 12-bit signed
    bfs.large = 0x7FFFFFFF; // Max for 31 bits
    
    results[idx++] = bfs.small;
    results[idx++] = bfs.signed_small;
    results[idx++] = bfs.medium;
    results[idx++] = bfs.signed_medium;
    results[idx++] = bfs.large;
    
    // Test overflow/wrap-around in bit-fields
    bfs.small = 8;          // Should wrap to 0
    bfs.signed_small = 8;   // Should wrap to -8 for signed
    bfs.medium = 1024;      // Should wrap to 0
    bfs.signed_medium = 2048; // Should wrap to -2048
    
    results[idx++] = bfs.small;
    results[idx++] = bfs.signed_small;
    results[idx++] = bfs.medium;
    results[idx++] = bfs.signed_medium;
    
    // Comparisons that depend on bit-field range knowledge
    int cmp_results = 0;
    if (bfs.small <= 7) cmp_results |= 1;      // Always true
    if (bfs.signed_small >= -8) cmp_results |= 2; // Always true
    if (bfs.medium < 1024) cmp_results |= 4;   // Always true
    if (bfs.signed_medium > -2049) cmp_results |= 8; // Always true
    
    results[idx++] = cmp_results;
    
    // Union access
    u.bf = bfs;
    results[idx++] = (uint32_t)u.raw;
    results[idx++] = (uint32_t)(u.raw >> 32);
    
    return checksum(results, idx);
}

// 5. Compiler Builtins for Overflow Detection
NOINLINE uint32_t test_overflow_builtins(void) {
    uint32_t results[40];
    int idx = 0;
    uint32_t overflow_count = 0;
    
    // Test overflow builtins with various inputs
    int32_t a = 0x70000000;
    int32_t b = 0x10000000;
    int32_t result;
    
    // Range-constrained values
    int32_t constrained_a = a & 0x7FFFFFFF;  // Always positive
    int32_t constrained_b = b | 0x80000000;  // Has top bit set
    
    // Addition overflow tests
    if (__builtin_add_overflow(constrained_a, constrained_b, &result)) {
        overflow_count++;
    }
    results[idx++] = result;
    
    if (__builtin_add_overflow(a, b, &result)) {
        overflow_count++;
    }
    results[idx++] = result;
    
    // Multiplication overflow tests
    int32_t c = 0x10000;
    int32_t d = 0x10000;
    
    if (__builtin_mul_overflow(c, d, &result)) {
        overflow_count++;
    }
    results[idx++] = result;
    
    // Subtraction overflow tests
    int32_t e = 0x80000000;
    int32_t f = 1;
    
    if (__builtin_sub_overflow(e, f, &result)) {
        overflow_count++;
    }
    results[idx++] = result;
    
    // Loop with overflow checks
    for (int i = 0; i < 20; i++) {
        int32_t x = 0x40000000 + i * 0x08000000;
        int32_t y = 0x40000000;
        
        if (__builtin_add_overflow(x, y, &result)) {
            overflow_count++;
            result = 0x7FFFFFFF;  // Saturate on overflow
        }
        results[idx++ % 40] = result;
        
        // Chain operations
        if (!__builtin_add_overflow(result, i * 100, &result)) {
            if (__builtin_mul_overflow(result, 2, &result)) {
                overflow_count++;
            }
        }
        results[idx++ % 40] = result;
    }
    
    // Unsigned overflow tests
    uint32_t u1 = 0xFFFFFFFF;
    uint32_t u2 = 1;
    uint32_t uresult;
    
    if (__builtin_add_overflow(u1, u2, &uresult)) {
        overflow_count++;
    }
    results[idx++ % 40] = uresult;
    
    if (__builtin_mul_overflow(u1, 2, &uresult)) {
        overflow_count++;
    }
    results[idx++ % 40] = uresult;
    
    results[39] = overflow_count;
    return checksum(results, 40);
}

int main(void) {
    uint32_t final_results[5];
    
    // Run all tests
    final_results[0] = test_narrowing_conversions();
    final_results[1] = test_loop_range_analysis();
    final_results[2] = test_saturation_arithmetic();
    final_results[3] = test_bitfield_ranges();
    final_results[4] = test_overflow_builtins();
    
    // Combine results
    uint32_t final_checksum = 0;
    for (int i = 0; i < 5; i++) {
        final_checksum ^= final_results[i];
        printf("Test %d checksum: 0x%08X\n", i, final_results[i]);
    }
    
    printf("Final checksum: 0x%08X\n", final_checksum);
    
    // Prevent optimization
    volatile uint32_t dummy = final_checksum;
    
    return (final_checksum != 0) ? 0 : 1;
}
