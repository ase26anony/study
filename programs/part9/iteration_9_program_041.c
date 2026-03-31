#include <stdint.h>
#include <stdio.h>
#include <string.h>

// Prevent inlining to maintain function boundaries for coverage
#define NOINLINE __attribute__((noinline))

// Test 1: Narrowing conversions with range analysis
NOINLINE uint32_t test_narrowing_conversions() {
    uint32_t checksum = 0;
    
    // Constants at boundary values
    int64_t large_positive = 0x7FFFFFFF;  // INT_MAX for 32-bit
    int64_t large_negative = -0x80000000; // INT_MIN for 32-bit
    
    // Test narrowing with values near boundaries
    int32_t narrow1 = (int32_t)large_positive;
    int32_t narrow2 = (int32_t)(large_positive + 1);
    int32_t narrow3 = (int32_t)large_negative;
    int32_t narrow4 = (int32_t)(large_negative - 1);
    
    // Operations that require range checking
    int64_t a = 0x123456789ABCDEF0LL;
    int32_t b = (int32_t)(a >> 16);
    int32_t c = (int32_t)(a & 0xFFFFFFFF);
    
    // Comparisons against boundary constants
    if (narrow1 > 0x7FFFFFFE) checksum ^= 1;
    if (narrow2 < 0x80000000) checksum ^= 2;
    if (narrow3 == 0x80000000) checksum ^= 4;
    if (narrow4 > 0x7FFFFFFF) checksum ^= 8;
    
    // Shift operations that may overflow
    int32_t shifted1 = b << 8;
    int32_t shifted2 = c >> 24;
    if (shifted1 > 0x7FFF0000) checksum ^= 16;
    if (shifted2 < 0xFFFFFF80) checksum ^= 32;
    
    return checksum;
}

// Test 2: Complex loop bound analysis
NOINLINE uint32_t test_loop_range_analysis() {
    uint32_t checksum = 0;
    
    // Outer loop with bitwise bound calculation
    for (int32_t i = 100; i < 500; i += 37) {
        // Inner loop bounds depend on outer index with bitwise ops
        uint32_t mask = 0xFFF;
        uint32_t start = i & mask;
        uint32_t end = (i | 0x7FF) & 0xFFF;
        
        // Complex condition for loop termination
        for (uint32_t j = start; j < end && j < 4096; j += (i & 0x3F) + 1) {
            checksum = (checksum * 31 + j) ^ i;
            
            // Additional bitwise operation in loop
            uint32_t k = (j ^ 0x555) & 0xFFF;
            if (k > 0x800) checksum += k;
        }
        
        // Nested loop with shifting bounds
        int32_t shift_val = (i >> 3) & 0x1F;
        for (int32_t k = 0; k < (1 << shift_val); k++) {
            if (k > (i & 0x7F)) {
                checksum = checksum * 3 + k;
            }
        }
    }
    
    // Loop with comparison at extreme range
    int64_t big_val = 0x7FFFFFFFFFFFFFFFLL;
    for (int64_t i = big_val - 1000; i < big_val; i += 100) {
        checksum += (uint32_t)(i & 0xFFFFFFFF);
    }
    
    return checksum;
}

// Test 3: Saturation arithmetic
NOINLINE uint32_t test_saturation_arithmetic() {
    uint32_t checksum = 0;
    
    // Manual saturation implementation
    int32_t saturate_add(int32_t a, int32_t b) {
        int64_t result = (int64_t)a + (int64_t)b;
        if (result > 0x7FFFFFFF) return 0x7FFFFFFF;
        if (result < -0x80000000) return -0x80000000;
        return (int32_t)result;
    }
    
    int32_t saturate_mul(int32_t a, int32_t b) {
        int64_t result = (int64_t)a * (int64_t)b;
        if (result > 0x7FFFFFFF) return 0x7FFFFFFF;
        if (result < -0x80000000) return -0x80000000;
        return (int32_t)result;
    }
    
    // Test cases near boundaries
    int32_t test_cases[][2] = {
        {0x70000000, 0x10000000},  // Would overflow
        {-0x70000000, -0x10000000}, // Would underflow
        {0x40000000, 0x40000000},  // Would overflow on multiplication
        {0x7FFFFFFF, 1},           // At max boundary
        {-0x80000000, -1},         // At min boundary
    };
    
    for (int i = 0; i < 5; i++) {
        int32_t sat_add = saturate_add(test_cases[i][0], test_cases[i][1]);
        int32_t sat_mul = saturate_mul(test_cases[i][0], test_cases[i][1]);
        checksum = checksum * 17 + sat_add;
        checksum = checksum * 19 + sat_mul;
    }
    
    return checksum;
}

// Test 4: Bit-field range analysis
NOINLINE uint32_t test_bitfield_ranges() {
    uint32_t checksum = 0;
    
    // Struct with various bit-fields
    struct BitFieldStruct {
        unsigned int a : 3;   // 0-7
        signed int b : 5;     // -16 to 15
        unsigned int c : 12;  // 0-4095
        signed int d : 20;    // -524288 to 524287
        unsigned int e : 1;   // 0-1
    };
    
    union BitFieldUnion {
        struct BitFieldStruct bits;
        uint64_t raw;
    };
    
    union BitFieldUnion u;
    u.raw = 0;
    
    // Assign values at boundaries
    u.bits.a = 7;      // Max for 3 bits
    u.bits.b = -16;    // Min for 5-bit signed
    u.bits.c = 4095;   // Max for 12 bits
    u.bits.d = 524287; // Max for 20-bit signed
    u.bits.e = 1;      // Max for 1 bit
    
    // Comparisons that require range analysis
    if (u.bits.a == 7) checksum ^= 0x01;
    if (u.bits.b < 0) checksum ^= 0x02;
    if (u.bits.c > 4000) checksum ^= 0x04;
    if (u.bits.d >= 524000) checksum ^= 0x08;
    if (u.bits.e != 0) checksum ^= 0x10;
    
    // Operations that may exceed bit-field capacity
    u.bits.a = 8;  // Should wrap or saturate
    u.bits.b = 16; // Exceeds signed 5-bit max
    u.bits.c = u.bits.c + 1; // Overflow for 12 bits
    
    checksum = checksum * 23 + u.bits.a;
    checksum = checksum * 29 + (u.bits.b & 0xFF);
    checksum = checksum * 31 + u.bits.c;
    
    return checksum;
}

// Test 5: Overflow builtins with range analysis
NOINLINE uint32_t test_overflow_builtins() {
    uint32_t checksum = 0;
    
    // Variables with constrained ranges
    int32_t x = 100;
    int32_t y = 200;
    
    // Range-restricting conditions
    if (x > 50 && x < 150) {
        if (y > 100 && y < 300) {
            int overflow;
            int32_t result;
            
            // These should not overflow
            overflow = __builtin_add_overflow(x, y, &result);
            if (!overflow) checksum += result;
            
            overflow = __builtin_mul_overflow(x, y, &result);
            if (!overflow) checksum += result * 3;
        }
    }
    
    // Test with values near boundaries
    int32_t boundary_cases[][2] = {
        {0x7FFFFFFF, 1},
        {0x7FFFFFFF, -1},
        {-0x80000000, 1},
        {-0x80000000, -1},
        {0x40000000, 2},
    };
    
    for (int i = 0; i < 5; i++) {
        int overflow;
        int32_t result;
        
        overflow = __builtin_add_overflow(boundary_cases[i][0], 
                                         boundary_cases[i][1], &result);
        checksum = checksum * 7 + (overflow ? 1 : 0);
        
        overflow = __builtin_mul_overflow(boundary_cases[i][0], 
                                         boundary_cases[i][1], &result);
        checksum = checksum * 11 + (overflow ? 2 : 0);
    }
    
    // Loop with overflow checks
    for (int32_t i = 0x70000000; i < 0x7FFFFFFF; i += 0x1000000) {
        int overflow;
        int32_t result;
        
        overflow = __builtin_add_overflow(i, 0x0FFFFFFF, &result);
        if (!overflow) {
            checksum += result & 0xFFFF;
        }
    }
    
    return checksum;
}

// Test 6: Additional edge cases for the uncovered block
NOINLINE uint32_t test_edge_cases() {
    uint32_t checksum = 0;
    
    // Comparisons exactly matching the uncovered code pattern
    int64_t a_high, a_low;
    int64_t max_r, max_s, min_r, min_s;
    
    // Simulate the initialization from uncovered lines
    max_r = 0;
    max_s = -1;
    max_s = max_s & ((1LL << 32) - 1);  // Simulate zext
    
    min_r = -1;
    min_s = 1;
    min_s = min_s << 32;  // Simulate alshift
    min_s = min_s | ((min_s >> 63) * ((1LL << 33) - 1));  // Simulate sext
    
    // Test various values that might trigger the comparison
    int64_t test_values[] = {
        0, 1, -1, 0x7FFFFFFF, 0x80000000,
        0xFFFFFFFF, 0x100000000, -0x100000000
    };
    
    for (int i = 0; i < 8; i++) {
        a_high = test_values[i] >> 32;
        a_low = test_values[i] & 0xFFFFFFFF;
        
        // This mimics the uncovered comparison
        if (a_high > max_r || (a_high == max_r && (uint64_t)a_low > (uint64_t)max_s)) {
            checksum += test_values[i] & 0xFFFF;
        }
        
        // Additional boundary checks
        if (a_high < (min_r >> 32)) {
            checksum += 0x10000;
        }
    }
    
    return checksum;
}

int main() {
    uint32_t final_checksum = 0;
    
    final_checksum ^= test_narrowing_conversions();
    final_checksum = final_checksum * 3 + test_loop_range_analysis();
    final_checksum ^= test_saturation_arithmetic();
    final_checksum = final_checksum * 5 + test_bitfield_ranges();
    final_checksum ^= test_overflow_builtins();
    final_checksum = final_checksum * 7 + test_edge_cases();
    
    // Print to prevent optimization
    printf("Checksum: %u\n", final_checksum);
    
    return 0;
}
