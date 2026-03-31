#include <stdint.h>
#include <stdio.h>
#include <string.h>

// Prevent inlining to maintain function boundaries for coverage
#define NOINLINE __attribute__((noinline))

// Fixed-point types if available
#ifdef __STDC_IEC_559__
#include <stdfix.h>
#endif

// Checksum/hash function to prevent dead code elimination
static uint32_t hash = 0;
static void update_hash(uint32_t value) {
    hash = (hash * 31) ^ value;
}

// ==================== Test 1: Narrowing Conversions ====================
NOINLINE void test_narrowing_conversions() {
    uint64_t wide_values[] = {
        0xFFFFFFFFFFFFFFFFULL,  // Max uint64
        0x8000000000000000ULL,  // High bit set
        0x7FFFFFFFFFFFFFFFULL,  // Max int64
        0x00000000FFFFFFFFULL,  // Fits in uint32
        0x000000007FFFFFFFULL,  // Fits in int32
    };
    
    for (size_t i = 0; i < sizeof(wide_values)/sizeof(wide_values[0]); i++) {
        uint64_t val = wide_values[i];
        
        // Narrowing conversions that require range analysis
        uint32_t narrow1 = (uint32_t)val;
        int32_t narrow2 = (int32_t)val;
        uint16_t narrow3 = (uint16_t)val;
        int16_t narrow4 = (int16_t)val;
        
        // Operations that might overflow
        uint32_t shifted = (uint32_t)(val >> 32);
        int32_t signed_shifted = (int32_t)(val >> 31);
        
        // Comparisons against type limits
        int in_int32_range = (val <= 0x7FFFFFFF) && (val >= (uint64_t)(-0x7FFFFFFF-1));
        int in_uint32_range = (val <= 0xFFFFFFFF);
        
        update_hash(narrow1 ^ narrow2 ^ narrow3 ^ narrow4);
        update_hash(shifted ^ signed_shifted);
        update_hash(in_int32_range ^ in_uint32_range);
    }
}

// ==================== Test 2: Loop Range Analysis ====================
NOINLINE void test_loop_range_analysis() {
    int32_t results = 0;
    
    // Complex loop bounds with bitwise operations
    for (int32_t i = 100 & 0xFFF; i < (500 | 0x7FF); i += 3) {
        // Nested loop with dependent bounds
        for (int32_t j = (i & 0x3F); j < ((i ^ 0xFF) & 0x7F); j += 2) {
            results += i * j;
            
            // Inner-inner loop with shift-based bounds
            for (int32_t k = (j << 2); k < ((j + 10) >> 1); k++) {
                results ^= k;
            }
        }
        
        // Loop variant used in bitwise expression
        int32_t mask = i & 0xFFFF;
        for (int32_t m = mask; m < (mask | 0x0FFF); m++) {
            results += m % 17;
        }
    }
    
    // Loop with wrap-around analysis
    uint8_t counter = 250;
    for (int i = 0; i < 20; i++) {
        counter += 10;  // Will wrap around
        results += counter;
    }
    
    update_hash(results);
}

// ==================== Test 3: Saturation Arithmetic ====================
NOINLINE void test_saturation_arithmetic() {
    int32_t min_val = -1000;
    int32_t max_val = 1000;
    int32_t sum = 0;
    
    // Test values at boundaries
    int32_t test_vals[] = {-2000, -1001, -1000, -500, 0, 500, 1000, 1001, 2000};
    
    for (size_t i = 0; i < sizeof(test_vals)/sizeof(test_vals[0]); i++) {
        int32_t val = test_vals[i];
        
        // Manual saturation clamping
        int32_t saturated;
        if (val < min_val) {
            saturated = min_val;
        } else if (val > max_val) {
            saturated = max_val;
        } else {
            saturated = val;
        }
        
        // Saturation with bitwise trick (requires range analysis)
        int32_t alt_saturated = val;
        alt_saturated = (alt_saturated < min_val) ? min_val : alt_saturated;
        alt_saturated = (alt_saturated > max_val) ? max_val : alt_saturated;
        
        // Accumulate with potential overflow
        sum = sum + saturated;
        if (sum > 10000) sum = 10000;
        if (sum < -10000) sum = -10000;
        
        update_hash(saturated ^ alt_saturated);
    }
    
    update_hash(sum);
    
    // Fixed-point arithmetic if supported
    #ifdef __STDC_IEC_559__
    _Fract f1 = 0.5r;
    _Fract f2 = 0.7r;
    _Fract f3 = f1 + f2;  // May saturate
    update_hash(*(uint32_t*)&f3);
    #endif
}

// ==================== Test 4: Bit-Field Ranges ====================
NOINLINE void test_bitfield_ranges() {
    // Struct with various bit-fields
    struct {
        unsigned int a : 3;   // 0-7
        signed int b : 5;     // -16 to 15
        unsigned int c : 10;  // 0-1023
        signed int d : 12;    // -2048 to 2047
    } bits;
    
    uint32_t result = 0;
    
    // Assign values at boundaries
    bits.a = 7;      // Max for 3 bits
    bits.b = -16;    // Min for 5-bit signed
    bits.c = 1023;   // Max for 10 bits
    bits.d = 2047;   // Max for 12-bit signed
    
    // Check if values fit in their ranges
    if (bits.a <= 7) result |= 0x1;
    if (bits.b >= -16 && bits.b <= 15) result |= 0x2;
    if (bits.c < 1024) result |= 0x4;
    if (bits.d > -2049 && bits.d < 2048) result |= 0x8;
    
    // Operations that might overflow bit-field
    bits.a = bits.a + 1;  // Should wrap for unsigned bit-field
    bits.b = bits.b - 1;  // Should wrap for signed bit-field
    
    // Union to test bit-field vs regular integer
    union {
        struct {
            unsigned int x : 4;
            unsigned int y : 4;
        } bits;
        uint8_t byte;
    } u;
    
    u.byte = 0xFF;
    if (u.bits.x == 0xF && u.bits.y == 0xF) result |= 0x10;
    
    update_hash(result);
    update_hash(bits.a + bits.b + bits.c + bits.d);
}

// ==================== Test 5: Overflow Builtins ====================
NOINLINE void test_overflow_builtins() {
    int32_t a = 1000000;
    int32_t b = 2000000;
    int32_t c = 0;
    int overflow = 0;
    
    // Basic overflow checks
    overflow |= __builtin_add_overflow(a, b, &c);
    update_hash(c);
    
    overflow |= __builtin_mul_overflow(a, 3, &c);
    update_hash(c);
    
    // In loops with range-constrained variables
    for (int32_t i = 1000; i < 2000; i += 100) {
        int32_t temp;
        if (!__builtin_add_overflow(i, 5000000, &temp)) {
            c += temp;
        }
    }
    
    // Chain of operations
    int32_t x = 1;
    for (int i = 0; i < 10; i++) {
        int32_t old_x = x;
        if (__builtin_mul_overflow(x, 3, &x)) {
            x = old_x;  // Revert on overflow
        }
        update_hash(x);
    }
    
    // Overflow with bitwise constrained values
    int32_t y = 0x7FFFFFFF & 0x0FFFFFFF;  // Top bit cleared
    int32_t z;
    overflow |= __builtin_add_overflow(y, 0x10000000, &z);
    update_hash(z ^ overflow);
}

// ==================== Test 6: Edge Case Comparisons ====================
NOINLINE void test_edge_case_comparisons() {
    int32_t extreme_values[] = {
        INT32_MIN,
        INT32_MIN + 1,
        -100,
        0,
        100,
        INT32_MAX - 1,
        INT32_MAX
    };
    
    uint32_t checksum = 0;
    
    for (size_t i = 0; i < sizeof(extreme_values)/sizeof(extreme_values[0]); i++) {
        int32_t val = extreme_values[i];
        
        // Comparisons at type boundaries
        if (val > INT32_MAX - 10) checksum += 1;
        if (val < INT32_MIN + 10) checksum += 2;
        if (val >= 0 && val <= 100) checksum += 4;
        
        // After range-restricting operations
        int32_t constrained = val & 0xFFF;  // 0-4095
        if (constrained > 4000) checksum += 8;
        if (constrained < 100) checksum += 16;
        
        // Modulo creates known range
        int32_t modded = val % 100;
        if (modded >= 0 && modded < 100) checksum += 32;
        
        // Shift operations that might overflow
        int32_t shifted = val << 3;
        if (shifted > 0 && shifted < INT32_MAX >> 2) checksum += 64;
    }
    
    // Test with 64-bit values
    uint64_t big_val = 0xFFFFFFFF00000000ULL;
    if (big_val > UINT32_MAX) checksum += 128;
    if ((big_val >> 32) == 0xFFFFFFFF) checksum += 256;
    
    update_hash(checksum);
}

// ==================== Main Function ====================
int main() {
    // Reset hash
    hash = 0;
    
    // Run all tests
    test_narrowing_conversions();
    test_loop_range_analysis();
    test_saturation_arithmetic();
    test_bitfield_ranges();
    test_overflow_builtins();
    test_edge_case_comparisons();
    
    // Print hash to prevent optimization
    printf("Hash: %u\n", hash);
    
    return 0;
}
