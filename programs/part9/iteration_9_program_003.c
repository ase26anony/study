#include <stdint.h>
#include <stdio.h>
#include <string.h>

// Prevent inlining to maintain function boundaries for coverage
#define NOINLINE __attribute__((noinline))

// Fixed-point types if available
#ifdef __STDC_IEC_559__
#include <stdfix.h>
#endif

// Checksum to prevent dead code elimination
static volatile int checksum = 0;

NOINLINE static void test_narrowing_conversions(void) {
    uint64_t wide_val = 0xFFFFFFFFFFFFFFF0ULL;
    int32_t narrow1, narrow2, narrow3;
    int16_t very_narrow;
    
    // Test 1: Direct narrowing with constant at boundary
    narrow1 = (int32_t)wide_val;  // Should be -16
    
    // Test 2: Narrowing after arithmetic
    uint64_t a = 0x7FFFFFFFFFFFFFFFULL;  // Max positive signed 64-bit
    uint64_t b = 10;
    narrow2 = (int32_t)(a + b);  // Overflow in 64-bit, then narrow
    
    // Test 3: Chain of narrowing conversions
    int64_t medium = 0x00000000FFFFFFFFLL;
    narrow3 = (int32_t)medium;  // Exact boundary of 32-bit
    
    // Test 4: Very narrow with shift
    very_narrow = (int16_t)((wide_val >> 48) & 0xFFFF);
    
    checksum += narrow1 + narrow2 + narrow3 + very_narrow;
}

NOINLINE static void test_loop_range_analysis(void) {
    int i, j, k;
    int result = 0;
    
    // Complex loop bounds with bitwise operations
    uint32_t a = 0x12345678;
    uint32_t b = 0x9ABCDEF0;
    uint32_t c = 0x55555555;
    
    // Loop 1: Bounds depend on masked values
    for (i = a & 0xFFF; i < (b | 0x7FF); i += (c & 0x3F) + 1) {
        // Nested loop with dependent bounds
        for (j = (i & 0xF) << 4; j < 256; j += (i & 0x3) + 1) {
            // Inner loop with complex condition
            for (k = j; k < (j + (i & 0x7)); k++) {
                result ^= k * i;
            }
        }
    }
    
    // Loop 2: Bounds near type limits
    int32_t start = -1000;
    int32_t end = 1000;
    int32_t step = 3;
    
    for (i = start; i < end; i += step) {
        // Conditional with comparison at extreme
        if (i > INT32_MAX - 100) {
            result += i * 2;
        } else if (i < INT32_MIN + 100) {
            result -= i * 2;
        }
    }
    
    checksum += result;
}

NOINLINE static void test_saturation_arithmetic(void) {
    int32_t min_val = -1000;
    int32_t max_val = 1000;
    int32_t values[] = {-1500, -500, 500, 1500, 0};
    int32_t saturated[5];
    
    // Manual saturation arithmetic
    for (int i = 0; i < 5; i++) {
        if (values[i] < min_val) {
            saturated[i] = min_val;
        } else if (values[i] > max_val) {
            saturated[i] = max_val;
        } else {
            saturated[i] = values[i];
        }
    }
    
    // Test with bitwise-derived values
    uint32_t raw = 0x87654321;
    int32_t val = (int32_t)raw;
    
    // Double saturation check
    int32_t final_val;
    if (val < -500) {
        final_val = -500;
    } else if (val > 500) {
        final_val = 500;
    } else {
        final_val = val;
    }
    
    // Fixed-point if available
    #ifdef __STDC_IEC_559__
    _Fract f1 = 0.5r;
    _Fract f2 = 0.7r;
    _Fract f3 = f1 + f2;  // May saturate
    checksum += (int)(f3 * 1000);
    #endif
    
    for (int i = 0; i < 5; i++) {
        checksum += saturated[i];
    }
    checksum += final_val;
}

NOINLINE static void test_bitfield_ranges(void) {
    // Struct with various bit-fields
    struct {
        unsigned int a : 4;   // 0-15
        signed int b : 5;     // -16 to 15
        unsigned int c : 8;   // 0-255
        signed int d : 12;    // -2048 to 2047
    } bits;
    
    // Assign values at boundaries
    bits.a = 15;      // Max for 4-bit unsigned
    bits.b = -16;     // Min for 5-bit signed
    bits.c = 255;     // Max for 8-bit unsigned
    bits.d = 2047;    // Max for 12-bit signed
    
    // Comparisons that should trigger range analysis
    if (bits.a == 15) {
        checksum += 1;
    }
    if (bits.b < 0) {
        checksum += 2;
    }
    if (bits.c >= 200) {
        checksum += 4;
    }
    if (bits.d > 2000) {
        checksum += 8;
    }
    
    // Union with overlapping bit-fields
    union {
        uint32_t full;
        struct {
            uint32_t low : 16;
            uint32_t high : 16;
        } parts;
    } u;
    
    u.full = 0x87654321;
    if (u.parts.low > 0x7FFF) {
        checksum += 16;
    }
    if (u.parts.high < 0x8000) {
        checksum += 32;
    }
}

NOINLINE static void test_overflow_builtins(void) {
    int32_t a, b, result;
    int overflow;
    
    // Test 1: Values with known partial ranges
    a = 1000000;
    b = 2000000;
    overflow = __builtin_add_overflow(a, b, &result);
    checksum += overflow ? 1 : 0;
    checksum += result;
    
    // Test 2: After range restriction
    int32_t x = 50000;
    if (x > 0 && x < 100000) {
        // x is known to be in [1, 99999]
        overflow = __builtin_mul_overflow(x, 1000, &result);
        checksum += overflow ? 2 : 0;
    }
    
    // Test 3: With bit-masked values
    uint32_t y = 0x12345678;
    uint32_t z = 0x9ABCDEF0;
    uint32_t mask = 0x0000FFFF;
    
    uint32_t y_masked = y & mask;  // 0-65535
    uint32_t z_masked = z & mask;  // 0-65535
    
    uint32_t mul_result;
    overflow = __builtin_mul_overflow(y_masked, z_masked, &mul_result);
    checksum += overflow ? 4 : 0;
    
    // Test 4: In loop context
    int32_t accum = 0;
    for (int i = 0; i < 100; i++) {
        int32_t increment = (i * 1000) & 0x7FFF;
        if (!__builtin_add_overflow(accum, increment, &accum)) {
            checksum += increment;
        }
    }
}

int main(void) {
    // Run all tests
    test_narrowing_conversions();
    test_loop_range_analysis();
    test_saturation_arithmetic();
    test_bitfield_ranges();
    test_overflow_builtins();
    
    // Print checksum to ensure code isn't optimized away
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
