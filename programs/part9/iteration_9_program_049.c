#include <stdint.h>
#include <stdio.h>
#include <string.h>

// Prevent inlining to maintain function boundaries for coverage
#define NOINLINE __attribute__((noinline))

// Checksum function to prevent dead code elimination
static inline uint32_t checksum(uint32_t *data, size_t len) {
    uint32_t sum = 0;
    for (size_t i = 0; i < len; i++) {
        sum ^= data[i];
        sum = (sum << 13) | (sum >> 19);
    }
    return sum;
}

// 1. Integer Range Boundary Tests
NOINLINE uint32_t test_narrowing_conversions(void) {
    uint32_t results[16] = {0};
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
    int32_t x = 0x40000000;
    int32_t shifted1 = x << 3;  // May overflow into sign bit
    int32_t shifted2 = x >> 28; // Right shift with sign extension
    
    results[idx++] = shifted1;
    results[idx++] = shifted2;
    
    // Comparisons at type limits
    int32_t val = 0x7FFFFFF0;
    if (val > 0x7FFFFFFA) {  // Comparison near INT_MAX
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    // Complex expression with mixed widths
    uint16_t a = 0xFFFF;
    uint32_t b = 0xFFFFFFFF;
    uint64_t c = (uint64_t)a * b;  // Widening then narrowing
    uint32_t d = (uint32_t)c;
    
    results[idx++] = d;
    
    return checksum(results, idx);
}

// 2. Loop Bound Analysis with Complex Conditions
NOINLINE uint32_t test_loop_range_analysis(void) {
    uint32_t results[32] = {0};
    int idx = 0;
    
    // Outer loop with bitmasked bounds
    uint32_t outer_start = 0x1000;
    uint32_t outer_end = 0x5000;
    
    for (uint32_t i = outer_start & 0xFFF; i < (outer_end | 0x7FF); i += 0x100) {
        // Inner loop with dependent bounds
        uint32_t inner_limit = (i & 0x3FF) + 0x200;
        
        for (uint32_t j = 0; j < inner_limit; j += 0x40) {
            results[idx++ % 32] ^= i + j;
        }
        
        // Loop with comparison at boundary
        int32_t k = (int32_t)i - 0x2000;
        while (k < 0x1000) {
            results[idx++ % 32] += k;
            k += 0x80;
        }
    }
    
    // Nested loops with signed/unsigned mix
    int32_t signed_val = -1000;
    uint32_t unsigned_val = 2000;
    
    for (; signed_val < 1000; signed_val += 100) {
        uint32_t limit = (unsigned_val + signed_val) & 0x7FF;
        for (uint32_t m = 0; m < limit; m += 50) {
            results[idx++ % 32] ^= m * signed_val;
        }
    }
    
    return checksum(results, 32);
}

// 3. Fixed-Point and Saturation Arithmetic
NOINLINE uint32_t test_saturation_arithmetic(void) {
    uint32_t results[16] = {0};
    int idx = 0;
    
    // Manual saturation arithmetic
    int32_t sat_add(int32_t a, int32_t b) {
        int64_t result = (int64_t)a + b;
        if (result > 0x7FFFFFFF) return 0x7FFFFFFF;
        if (result < (int64_t)0x80000000) return 0x80000000;
        return (int32_t)result;
    }
    
    int32_t sat_mul(int32_t a, int32_t b) {
        int64_t result = (int64_t)a * b;
        if (result > 0x7FFFFFFF) return 0x7FFFFFFF;
        if (result < (int64_t)0x80000000) return 0x80000000;
        return (int32_t)result;
    }
    
    // Test cases at boundaries
    int32_t max_val = 0x7FFFFFFF;
    int32_t min_val = 0x80000000;
    int32_t large_val = 0x40000000;
    
    results[idx++] = sat_add(max_val, 1);
    results[idx++] = sat_add(min_val, -1);
    results[idx++] = sat_mul(large_val, 2);
    results[idx++] = sat_mul(large_val, 3);
    
    // Fixed-point style operations (emulated)
    // Using Q15.16 format
    int32_t fp_mul(int32_t a, int32_t b) {
        int64_t temp = (int64_t)a * b;
        return (int32_t)(temp >> 16);
    }
    
    int32_t fp_val1 = 0x00010000; // 1.0 in Q15.16
    int32_t fp_val2 = 0x7FFF0000; // ~32767.0
    
    results[idx++] = fp_mul(fp_val1, fp_val2);
    results[idx++] = fp_mul(fp_val2, fp_val2);  // Should saturate
    
    // Range clamping
    int32_t clamp(int32_t val, int32_t min, int32_t max) {
        if (val < min) return min;
        if (val > max) return max;
        return val;
    }
    
    int32_t test_vals[] = {0, 100, -100, 0x80000000, 0x7FFFFFFF};
    for (int i = 0; i < 5; i++) {
        results[idx++] = clamp(test_vals[i], -1000, 1000);
    }
    
    return checksum(results, idx);
}

// 4. Structs with Bit-Fields and Unions
NOINLINE uint32_t test_bitfield_ranges(void) {
    uint32_t results[16] = {0};
    int idx = 0;
    
    // Struct with various bit-field sizes
    struct BitFieldStruct {
        unsigned int a : 3;   // 0-7
        signed int b : 5;     // -16 to 15
        unsigned int c : 12;  // 0-4095
        signed int d : 20;    // -524288 to 524287
    };
    
    union BitFieldUnion {
        struct BitFieldStruct bits;
        uint32_t raw;
    };
    
    union BitFieldUnion u1, u2;
    u1.raw = 0;
    u2.raw = 0xFFFFFFFF;
    
    // Assign values at bit-field boundaries
    u1.bits.a = 7;      // Max for 3 bits
    u1.bits.b = -16;    // Min for 5-bit signed
    u1.bits.c = 4095;   // Max for 12 bits
    u1.bits.d = 524287; // Max for 20-bit signed
    
    u2.bits.a = 0;
    u2.bits.b = 15;     // Max for 5-bit signed
    u2.bits.c = 0;
    u2.bits.d = -524288; // Min for 20-bit signed
    
    results[idx++] = u1.raw;
    results[idx++] = u2.raw;
    
    // Comparisons against bit-field capacity
    if (u1.bits.a == 7) results[idx++] = 1;
    if (u1.bits.b == -16) results[idx++] = 2;
    if (u1.bits.c >= 4095) results[idx++] = 3;
    if (u1.bits.d > 524286) results[idx++] = 4;
    
    // Arithmetic with bit-fields
    u1.bits.a += 1;  // Should wrap to 0
    u1.bits.b -= 1;  // Should become -15
    u1.bits.c /= 2;  // 4095/2 = 2047
    u1.bits.d *= 2;  // May overflow 20-bit range
    
    results[idx++] = u1.raw;
    
    // Nested bit-field access in loops
    for (unsigned int i = 0; i < 8; i++) {
        u2.bits.a = i;
        if (u2.bits.a < 4) {
            results[idx++ % 16] ^= i;
        }
    }
    
    return checksum(results, idx);
}

// 5. Compiler Builtins for Overflow Detection
NOINLINE uint32_t test_overflow_builtins(void) {
    uint32_t results[24] = {0};
    int idx = 0;
    
    // Test values at boundaries
    int32_t max_int = 0x7FFFFFFF;
    int32_t min_int = 0x80000000;
    int32_t large_val = 0x40000000;
    
    int32_t sum, diff, prod;
    int overflow;
    
    // Addition overflow tests
    overflow = __builtin_add_overflow(max_int, 1, &sum);
    results[idx++] = sum;
    results[idx++] = overflow;
    
    overflow = __builtin_add_overflow(min_int, -1, &sum);
    results[idx++] = sum;
    results[idx++] = overflow;
    
    // Multiplication overflow tests
    overflow = __builtin_mul_overflow(large_val, 2, &prod);
    results[idx++] = prod;
    results[idx++] = overflow;
    
    overflow = __builtin_mul_overflow(large_val, 4, &prod);
    results[idx++] = prod;
    results[idx++] = overflow;
    
    // Subtraction overflow tests
    overflow = __builtin_sub_overflow(min_int, 1, &diff);
    results[idx++] = diff;
    results[idx++] = overflow;
    
    // Combined operations with range analysis
    int32_t a = 1000;
    int32_t b = 2000;
    int32_t temp;
    
    if (!__builtin_add_overflow(a, b, &temp)) {
        if (!__builtin_mul_overflow(temp, 2, &prod)) {
            results[idx++] = prod;
        }
    }
    
    // Loop with overflow checks
    int32_t accum = 0;
    for (int32_t i = 0; i < 100; i++) {
        int32_t increment = 0x1000000;
        if (!__builtin_add_overflow(accum, increment, &accum)) {
            results[idx++ % 24] = accum;
        }
    }
    
    // Complex expression with multiple overflow points
    int32_t x = 0x20000000;
    int32_t y = 0x30000000;
    int32_t z;
    
    if (!__builtin_add_overflow(x, y, &temp)) {
        if (!__builtin_mul_overflow(temp, 2, &z)) {
            if (!__builtin_sub_overflow(z, 0x10000000, &z)) {
                results[idx++] = z;
            }
        }
    }
    
    return checksum(results, idx);
}

// Main function that calls all tests
int main(void) {
    uint32_t final_hash = 0;
    
    final_hash ^= test_narrowing_conversions();
    final_hash ^= test_loop_range_analysis();
    final_hash ^= test_saturation_arithmetic();
    final_hash ^= test_bitfield_ranges();
    final_hash ^= test_overflow_builtins();
    
    // Use the result to prevent dead code elimination
    volatile uint32_t sink = final_hash;
    (void)sink;
    
    return 0;
}
