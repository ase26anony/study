#include <stdint.h>
#include <stdio.h>
#include <string.h>

// Prevent inlining to maintain function boundaries for coverage
#define NOINLINE __attribute__((noinline))

// Checksum function to prevent dead code elimination
static inline uint32_t checksum(uint32_t seed, uint32_t value) {
    return seed * 31 + value;
}

// 1. Integer Range Boundary Tests
NOINLINE uint32_t test_narrowing_conversions(void) {
    uint32_t hash = 0;
    
    // Constants at type boundaries
    int64_t large_positive = 0x7FFFFFFFFFFFFFFFLL;
    int64_t large_negative = 0x8000000000000000LL;
    uint64_t large_unsigned = 0xFFFFFFFFFFFFFFFFULL;
    
    // Narrowing conversions that require range analysis
    int32_t narrow1 = (int32_t)large_positive;  // May overflow
    int32_t narrow2 = (int32_t)large_negative;  // May overflow
    uint32_t narrow3 = (uint32_t)large_unsigned;
    
    // Shifts that may overflow
    int32_t shift1 = 1 << 31;  // Potential overflow
    int32_t shift2 = 0x7FFFFFFF << 1;
    int32_t shift3 = 0x80000000 >> 31;
    
    // Comparisons against type limits
    if (narrow1 > INT32_MAX - 100) {
        hash = checksum(hash, 1);
    }
    if (narrow2 < INT32_MIN + 100) {
        hash = checksum(hash, 2);
    }
    if (narrow3 > UINT32_MAX - 1000) {
        hash = checksum(hash, 3);
    }
    
    // Complex boundary comparisons
    int64_t val = (int64_t)narrow1 * 2;
    if (val > INT32_MAX || val < INT32_MIN) {
        hash = checksum(hash, 4);
    }
    
    return hash;
}

// 2. Loop Bound Analysis with Complex Conditions
NOINLINE uint32_t test_loop_range_analysis(void) {
    uint32_t hash = 0;
    
    // Outer loop with bitmasked bounds
    for (int32_t i = 100 & 0xFFF; i < (500 | 0x7FF); i += 37) {
        hash = checksum(hash, i);
        
        // Inner loop with dependent bounds
        for (int32_t j = i & 0x3F; j < (i | 0x1F); j += 5) {
            hash = checksum(hash, j);
            
            // Bitwise operations in loop conditions
            int32_t k = j ^ 0xAA;
            while ((k & 0xFF) < 200) {
                hash = checksum(hash, k);
                k += (k & 0xF) + 1;
            }
        }
    }
    
    // Loop with shifting bounds
    int32_t start = 1 << 16;
    int32_t end = 1 << 24;
    for (int32_t i = start; i < end; i += 1 << 12) {
        hash = checksum(hash, i);
        
        // Nested loop with modulo bound
        for (int32_t j = 0; j < (i % 256); j++) {
            hash = checksum(hash, j ^ i);
        }
    }
    
    return hash;
}

// 3. Fixed-Point and Saturation Arithmetic
NOINLINE uint32_t test_saturation_arithmetic(void) {
    uint32_t hash = 0;
    
    // Manual saturation arithmetic
    int32_t sat_add(int32_t a, int32_t b) {
        int64_t result = (int64_t)a + b;
        if (result > INT32_MAX) return INT32_MAX;
        if (result < INT32_MIN) return INT32_MIN;
        return (int32_t)result;
    }
    
    int32_t sat_mul(int32_t a, int32_t b) {
        int64_t result = (int64_t)a * b;
        if (result > INT32_MAX) return INT32_MAX;
        if (result < INT32_MIN) return INT32_MIN;
        return (int32_t)result;
    }
    
    // Test saturation at boundaries
    int32_t values[] = {INT32_MAX, INT32_MIN, 100, -100, 0x7FFFFFFF, 0x80000000};
    
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 6; j++) {
            int32_t sum = sat_add(values[i], values[j]);
            int32_t prod = sat_mul(values[i], values[j]);
            
            hash = checksum(hash, sum);
            hash = checksum(hash, prod);
            
            // Boundary comparisons
            if (sum == INT32_MAX) hash = checksum(hash, 0xAA);
            if (sum == INT32_MIN) hash = checksum(hash, 0xBB);
            if (prod == INT32_MAX) hash = checksum(hash, 0xCC);
            if (prod == INT32_MIN) hash = checksum(hash, 0xDD);
        }
    }
    
    return hash;
}

// 4. Structs with Bit-Fields and Unions
NOINLINE uint32_t test_bitfield_ranges(void) {
    uint32_t hash = 0;
    
    // Struct with various bit-field sizes
    struct BitFields {
        unsigned int a : 3;   // 0-7
        signed int b : 5;     // -16 to 15
        unsigned int c : 12;  // 0-4095
        signed int d : 20;    // -524288 to 524287
        unsigned int e : 1;   // 0-1
    };
    
    union BitUnion {
        struct BitFields bits;
        uint64_t raw;
    };
    
    union BitUnion u;
    u.raw = 0;
    
    // Assign values at bit-field boundaries
    u.bits.a = 7;      // Max for 3 bits
    u.bits.b = -16;    // Min for 5 signed bits
    u.bits.c = 4095;   // Max for 12 bits
    u.bits.d = 524287; // Max for 20 signed bits
    u.bits.e = 1;      // Max for 1 bit
    
    hash = checksum(hash, u.raw);
    
    // Comparisons that require range analysis
    if (u.bits.a == 7) {
        hash = checksum(hash, 0x111);
    }
    if (u.bits.b == -16) {
        hash = checksum(hash, 0x222);
    }
    if (u.bits.c >= 4095) {
        hash = checksum(hash, 0x333);
    }
    if (u.bits.d > 500000) {
        hash = checksum(hash, 0x444);
    }
    
    // Test overflow in bit-field assignment
    u.bits.a = 8;  // Should wrap to 0
    u.bits.b = 16; // Should wrap to -16
    u.bits.c = 4096; // Should wrap to 0
    
    hash = checksum(hash, u.raw);
    
    return hash;
}

// 5. Compiler Builtins for Overflow Detection
NOINLINE uint32_t test_overflow_builtins(void) {
    uint32_t hash = 0;
    
    int32_t a = INT32_MAX / 2;
    int32_t b = INT32_MAX / 2 + 100;
    int32_t c = INT32_MIN / 2;
    
    int32_t result;
    int overflow;
    
    // Test overflow builtins with boundary values
    overflow = __builtin_add_overflow(a, b, &result);
    hash = checksum(hash, result);
    hash = checksum(hash, overflow);
    
    overflow = __builtin_sub_overflow(c, b, &result);
    hash = checksum(hash, result);
    hash = checksum(hash, overflow);
    
    overflow = __builtin_mul_overflow(a, 3, &result);
    hash = checksum(hash, result);
    hash = checksum(hash, overflow);
    
    // Use in loops with range-constrained variables
    for (int32_t i = INT32_MAX - 100; i < INT32_MAX; i += 10) {
        for (int32_t j = -100; j < 100; j += 20) {
            if (__builtin_add_overflow(i, j, &result)) {
                hash = checksum(hash, 0xDEAD);
            } else {
                hash = checksum(hash, result);
            }
        }
    }
    
    // Test with values derived from conditional ranges
    int32_t x = 1000;
    int32_t y = 2000;
    
    if (x > 500 && x < 1500) {
        if (y > 1000 && y < 3000) {
            overflow = __builtin_mul_overflow(x, y, &result);
            hash = checksum(hash, overflow);
            hash = checksum(hash, result);
        }
    }
    
    return hash;
}

// 6. Additional Conditional Code with Value-Dependent Dead Branches
NOINLINE uint32_t test_conditional_dead_branches(void) {
    uint32_t hash = 0;
    
    // Variables with constrained ranges
    int32_t x = 100;
    int32_t y = 200;
    
    // Range-restricting conditions
    if (x > 50 && x < 150) {
        if (y > 150 && y < 250) {
            // Compiler should know ranges: x in [51,149], y in [151,249]
            
            // Comparisons against extreme constants
            if (x > INT32_MAX - 1000) {
                // Dead branch - compiler should detect
                hash = checksum(hash, 0xFFFF);
            }
            
            if (y < INT32_MIN + 1000) {
                // Dead branch - compiler should detect
                hash = checksum(hash, 0xEEEE);
            }
            
            // Modulo operations that constrain range further
            int32_t z = x % 50;  // z in [0,49]
            if (z > 100) {
                // Dead branch
                hash = checksum(hash, 0xDDDD);
            }
            
            hash = checksum(hash, z + y);
        }
    }
    
    // Test with bitmask constraints
    uint32_t mask = 0xFF;
    uint32_t val = 0xABCD1234 & mask;  // val in [0,255]
    
    if (val > 1000) {
        // Dead branch
        hash = checksum(hash, 0xCCCC);
    }
    
    hash = checksum(hash, val);
    
    return hash;
}

int main(void) {
    uint32_t final_hash = 0;
    
    // Run all tests
    final_hash = checksum(final_hash, test_narrowing_conversions());
    final_hash = checksum(final_hash, test_loop_range_analysis());
    final_hash = checksum(final_hash, test_saturation_arithmetic());
    final_hash = checksum(final_hash, test_bitfield_ranges());
    final_hash = checksum(final_hash, test_overflow_builtins());
    final_hash = checksum(final_hash, test_conditional_dead_branches());
    
    // Print result to prevent optimization
    printf("Final checksum: %u\n", final_hash);
    
    return (int)(final_hash % 256);
}
