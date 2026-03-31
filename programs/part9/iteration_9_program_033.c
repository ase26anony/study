#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

// Prevent inlining to maintain function boundaries for coverage
#define NOINLINE __attribute__((noinline))

// Fixed-point types if available
#ifdef __STDC_IEC_559__
#include <stdfix.h>
#endif

// Checksum to prevent dead code elimination
static volatile int checksum = 0;

// Test 1: Narrowing conversions with boundary values
NOINLINE void test_narrowing_conversions() {
    uint64_t wide_vals[] = {
        0,
        1,
        0x7FFFFFFF,
        0x80000000,
        0xFFFFFFFF,
        0x100000000,
        0x7FFFFFFFFFFFFFFFULL,
        0x8000000000000000ULL,
        0xFFFFFFFFFFFFFFFFULL
    };
    
    for (size_t i = 0; i < sizeof(wide_vals)/sizeof(wide_vals[0]); i++) {
        // Narrowing conversions that require range analysis
        int32_t narrow1 = (int32_t)wide_vals[i];
        uint32_t narrow2 = (uint32_t)wide_vals[i];
        
        // Operations that might overflow
        int64_t extended = (int64_t)narrow1 * 2;
        int32_t narrowed_again = (int32_t)extended;
        
        // Comparisons at boundaries
        if (narrow1 > 0x7FFFFFF0) {
            checksum += 1;
        }
        if (narrow2 < 0xFFFFFF00) {
            checksum += 2;
        }
        
        // Shift operations with potential overflow
        int32_t shifted = narrow1 << 3;
        if (shifted > 0x7FFFFFFF) {
            checksum += 4;
        }
    }
}

// Test 2: Complex loop bound analysis
NOINLINE void test_loop_range_analysis() {
    int32_t a = 0x12345678;
    int32_t b = 0x9ABCDEF0;
    uint32_t c = 0x7F;
    
    // Loop with bitwise operations in bounds
    for (int32_t i = a & 0xFFF; i < (b | 0x7FF); i += (c & 0x3F)) {
        // Nested loop with dependent bounds
        for (int32_t j = i & 0xFF; j < 1000; j += (i & 0x1F) + 1) {
            // Complex condition using XOR
            if ((i ^ j) > 0x80000000) {
                checksum += j;
            }
            
            // Range-dependent operation
            int32_t k = j * 2;
            if (k > 0x7FFFFFF0 && k < 0x8000000F) {
                checksum += i;
            }
        }
        
        // Break condition based on bit patterns
        if ((i & 0xF0000000) == 0x80000000) {
            break;
        }
    }
    
    // Another loop with modulo operation
    for (uint32_t i = 0; i < 10000; i++) {
        uint32_t masked = i & 0x3FF;
        for (uint32_t j = masked; j < 2000; j += (masked % 64) + 1) {
            checksum += (j & 1);
        }
    }
}

// Test 3: Saturation arithmetic
NOINLINE void test_saturation_arithmetic() {
    // Manual saturation implementation
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
    
    // Test with boundary values
    int32_t test_cases[][2] = {
        {0x7FFFFFFF, 1},
        {0x7FFFFFFF, 0x7FFFFFFF},
        {0x80000000, -1},
        {0x40000000, 2},
        {0x20000000, 8}
    };
    
    for (size_t i = 0; i < sizeof(test_cases)/sizeof(test_cases[0]); i++) {
        int32_t sum = sat_add(test_cases[i][0], test_cases[i][1]);
        int32_t prod = sat_mul(test_cases[i][0], test_cases[i][1]);
        
        // Comparisons that should trigger range analysis
        if (sum == 0x7FFFFFFF) {
            checksum += 8;
        }
        if (prod == 0x80000000) {
            checksum += 16;
        }
    }
    
    // Fixed-point arithmetic if supported
    #ifdef __STDC_IEC_559__
    _Accum acc1 = 0.5k;
    _Accum acc2 = 0.7k;
    _Accum acc_sum = acc1 + acc2;
    
    if (acc_sum > 1.0k) {
        checksum += 32;
    }
    #endif
}

// Test 4: Bit-fields with range constraints
NOINLINE void test_bitfield_ranges() {
    struct BitFieldStruct {
        unsigned int small : 4;
        signed int medium : 10;
        unsigned int large : 20;
        signed int full : 32;
    };
    
    union BitFieldUnion {
        struct BitFieldStruct fields;
        uint64_t raw;
    };
    
    union BitFieldUnion u;
    u.raw = 0;
    
    // Assign values at boundaries
    u.fields.small = 0xF;  // Max for 4 bits
    u.fields.medium = 0x1FF; // Near max for signed 10 bits
    u.fields.large = 0xFFFFF; // Max for 20 bits
    
    // Comparisons that require understanding bit-field ranges
    if (u.fields.small == 0xF) {
        checksum += 64;
    }
    
    if (u.fields.medium > 0x100) {
        checksum += 128;
    }
    
    // Overflow in bit-field arithmetic
    unsigned int temp = u.fields.small;
    temp = temp * 2;  // Would overflow 4 bits
    
    if (temp > 0xF) {
        checksum += 256;
    }
    
    // Complex condition with bit-fields
    if ((u.fields.large & 0xFFF) == 0xABC) {
        checksum += 512;
    }
}

// Test 5: Overflow builtins with range analysis
NOINLINE void test_overflow_builtins() {
    int32_t a = 0x7FFFFFF0;
    int32_t b = 0x7FFFFFF0;
    int32_t result;
    int overflow;
    
    // Test overflow detection with boundary values
    for (int i = 0; i < 32; i++) {
        overflow = __builtin_add_overflow(a, i, &result);
        if (overflow) {
            checksum += 1024;
        }
        
        // Multiplication with increasing values
        overflow = __builtin_mul_overflow(a >> i, b >> i, &result);
        if (overflow) {
            checksum += 2048;
        }
    }
    
    // Test with constrained ranges
    uint32_t x = 1000;
    uint32_t y = 2000;
    
    // First constrain the range
    if (x < 1500 && y < 2500) {
        // Now the compiler knows x < 1500, y < 2500
        overflow = __builtin_add_overflow(x, y, &result);
        if (!overflow) {
            checksum += 4096;
        }
    }
    
    // Test subtraction with potential underflow
    uint32_t small = 100;
    uint32_t large = 1000;
    for (int i = 0; i < 10; i++) {
        overflow = __builtin_sub_overflow(small, i * 200, &result);
        if (overflow) {
            checksum += 8192;
        }
    }
}

// Test 6: Additional edge cases for the uncovered block
NOINLINE void test_edge_cases() {
    // Direct comparisons with extreme values
    int64_t extreme_vals[] = {
        INT64_MIN,
        INT64_MIN + 1,
        -1,
        0,
        1,
        INT64_MAX - 1,
        INT64_MAX
    };
    
    for (size_t i = 0; i < sizeof(extreme_vals)/sizeof(extreme_vals[0]); i++) {
        // Comparisons that should trigger the specific uncovered logic
        if (extreme_vals[i] > 0x7FFFFFFFFFFFFFFFLL) {
            checksum += 16384;
        }
        
        if (extreme_vals[i] < -0x7FFFFFFFFFFFFFFFLL - 1) {
            checksum += 32768;
        }
        
        // Shift operations that might overflow
        int64_t shifted = extreme_vals[i] << 2;
        if (shifted > INT64_MAX || shifted < INT64_MIN) {
            checksum += 65536;
        }
    }
    
    // Mixed signed/unsigned comparisons
    uint32_t u = 0xFFFFFFFF;
    int32_t s = -1;
    
    if (u > (uint32_t)s) {
        checksum += 131072;
    }
    
    // Complex expression with multiple operations
    int32_t x = 0x70000000;
    int32_t y = 0x0FFFFFFF;
    int64_t z = (int64_t)x * (int64_t)y;
    
    if (z > 0x7FFFFFFF) {
        checksum += 262144;
    }
}

int main() {
    // Run all tests
    test_narrowing_conversions();
    test_loop_range_analysis();
    test_saturation_arithmetic();
    test_bitfield_ranges();
    test_overflow_builtins();
    test_edge_cases();
    
    // Print checksum to ensure code isn't optimized away
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
