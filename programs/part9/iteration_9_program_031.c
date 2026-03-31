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
        0x0000000080000000ULL,  // Just above int32 max
    };
    
    for (size_t i = 0; i < sizeof(wide_values)/sizeof(wide_values[0]); i++) {
        // Narrowing conversions that require range analysis
        uint32_t to_u32 = (uint32_t)wide_values[i];
        int32_t to_i32 = (int32_t)wide_values[i];
        uint16_t to_u16 = (uint16_t)wide_values[i];
        int16_t to_i16 = (int16_t)wide_values[i];
        
        // Comparisons at type boundaries
        if (to_u32 > 0xFFFFFF00UL) {
            update_hash(to_u32);
        }
        if (to_i32 > 0x7FFFFF00) {
            update_hash(to_i32);
        }
        if (to_u16 < 0x100) {
            update_hash(to_u16);
        }
        if (to_i16 < -0x7F00) {
            update_hash(to_i16);
        }
        
        // Shift operations that may overflow
        uint64_t shifted = wide_values[i] << 3;
        uint32_t shifted_narrow = (uint32_t)(shifted >> 16);
        update_hash(shifted_narrow);
    }
}

// ==================== Test 2: Loop Range Analysis ====================
NOINLINE void test_loop_range_analysis() {
    // Complex loop bounds with bitwise operations
    uint32_t a = 0x12345678;
    uint32_t b = 0x9ABCDEF0;
    uint32_t c = 17;
    
    // Outer loop with mask operation
    for (uint32_t i = a & 0x0000FFFF; i < (b | 0x00007FFF); i += c) {
        update_hash(i);
        
        // Inner loop with dependent bounds
        uint32_t inner_start = i & 0xFF;
        uint32_t inner_end = (i >> 8) & 0x7F;
        
        for (uint32_t j = inner_start; j < inner_end * 2; j += 3) {
            // XOR pattern creates complex range
            uint32_t k = j ^ (i & 0x55);
            if (k > 0x40 && k < 0xC0) {
                update_hash(k);
            }
        }
        
        // Loop with wrap-around detection
        uint32_t counter = i;
        for (int n = 0; n < 10; n++) {
            counter = (counter * 1103515245 + 12345) & 0x7FFFFFFF;
            if (counter > 0x70000000) {
                update_hash(counter);
            }
        }
    }
    
    // Nested loops with shifting bounds
    for (int32_t x = -1000; x < 1000; x += 97) {
        int32_t y_bound = (x < 0) ? -x : x;
        y_bound = y_bound & 0x3FF;  // Mask to 10 bits
        
        for (int32_t y = -y_bound; y < y_bound; y += 31) {
            // Comparison that requires signed range analysis
            if (x > y && x - y < 500) {
                update_hash(x ^ y);
            }
        }
    }
}

// ==================== Test 3: Saturation Arithmetic ====================
NOINLINE void test_saturation_arithmetic() {
    // Manual saturation implementation
    int32_t sat_add(int32_t a, int32_t b) {
        int32_t sum;
        if (__builtin_add_overflow(a, b, &sum)) {
            // Overflow - saturate to max/min based on sign
            return (a > 0) ? INT32_MAX : INT32_MIN;
        }
        return sum;
    }
    
    int32_t sat_mul(int32_t a, int32_t b) {
        int64_t product = (int64_t)a * (int64_t)b;
        if (product > INT32_MAX) return INT32_MAX;
        if (product < INT32_MIN) return INT32_MIN;
        return (int32_t)product;
    }
    
    // Test values at boundaries
    int32_t test_cases[] = {
        INT32_MAX, INT32_MIN, 0,
        1000000000, -1000000000,
        65536, -65536
    };
    
    for (size_t i = 0; i < sizeof(test_cases)/sizeof(test_cases[0]); i++) {
        for (size_t j = 0; j < sizeof(test_cases)/sizeof(test_cases[0]); j++) {
            int32_t sum = sat_add(test_cases[i], test_cases[j]);
            int32_t prod = sat_mul(test_cases[i], test_cases[j]);
            
            update_hash(sum);
            update_hash(prod);
            
            // Direct boundary comparisons
            if (sum == INT32_MAX || sum == INT32_MIN) {
                update_hash(i + j);
            }
            if (prod == INT32_MAX || prod == INT32_MIN) {
                update_hash(i * j);
            }
        }
    }
    
    // Fixed-point arithmetic if available
    #ifdef __STDC_IEC_559__
    _Accum acc1 = 0.5k;
    _Accum acc2 = 0.25k;
    for (int i = 0; i < 10; i++) {
        acc1 = acc1 + acc2;
        if (acc1 > 0.9k || acc1 < 0.1k) {
            update_hash((uint32_t)(acc1 * 1000k));
        }
    }
    #endif
}

// ==================== Test 4: Bit-Field Ranges ====================
NOINLINE void test_bitfield_ranges() {
    // Struct with various bit-fields
    struct BitFieldStruct {
        unsigned int small : 3;    // 0-7
        signed int signed_small : 4; // -8 to 7
        unsigned int medium : 10;  // 0-1023
        signed int signed_medium : 12; // -2048 to 2047
        unsigned int large : 31;   // 0-2147483647
    } bfs;
    
    // Union to test overlapping ranges
    union BitUnion {
        struct {
            unsigned int low : 16;
            unsigned int high : 16;
        } parts;
        uint32_t whole;
    } bu;
    
    // Test assignments to bit-fields
    for (uint32_t i = 0; i < 20; i++) {
        bfs.small = i & 0x7;  // Always 0-7
        bfs.signed_small = (i & 0xF) - 8;  // -8 to 7
        
        // Comparisons that should be optimized based on bit-field ranges
        if (bfs.small > 5) {
            update_hash(bfs.small);
        }
        if (bfs.signed_small < -4) {
            update_hash(bfs.signed_small + 8);
        }
        
        // Test overflow/wrap-around in bit-fields
        bfs.medium = i * 100;
        if (bfs.medium > 512) {  // Half of 1023
            update_hash(bfs.medium);
        }
        
        // Union test
        bu.parts.low = i * 17;
        bu.parts.high = i * 23;
        if (bu.whole > 0x80000000) {
            update_hash(bu.whole >> 16);
        }
    }
    
    // Edge case: maximum values
    bfs.small = 7;
    bfs.medium = 1023;
    bfs.large = 0x7FFFFFFF;
    
    if (bfs.small == 7) update_hash(1);
    if (bfs.medium == 1023) update_hash(2);
    if (bfs.large > 0x70000000) update_hash(3);
}

// ==================== Test 5: Overflow Builtins ====================
NOINLINE void test_overflow_builtins() {
    // Test __builtin_add_overflow with range-constrained values
    for (int32_t x = -10000; x <= 10000; x += 999) {
        // Constrain range further with conditional
        int32_t y;
        if (x > 0) {
            y = x & 0x3FF;  // 0-1023
        } else {
            y = -(x & 0x3FF);  // -1023-0
        }
        
        int32_t sum;
        if (__builtin_add_overflow(x, y, &sum)) {
            update_hash(0xDEAD);
        } else {
            // Check if near boundaries
            if (sum > INT32_MAX - 100 || sum < INT32_MIN + 100) {
                update_hash(sum & 0xFF);
            }
        }
        
        // Multiplication with overflow check
        int32_t prod;
        if (__builtin_mul_overflow(x, y, &prod)) {
            update_hash(0xBEEF);
        } else {
            update_hash(prod & 0xFFFF);
        }
    }
    
    // Test with unsigned types and bitwise constraints
    uint32_t mask = 0x00FFFFFF;
    for (uint32_t a = 0; a < 0xFFFFFFFF; a += 0x10000000) {
        uint32_t b = a & mask;  // Constrained to 24 bits
        
        uint32_t u_sum;
        if (__builtin_add_overflow(a, b, &u_sum)) {
            update_hash(0xCAFE);
        }
        
        uint32_t u_prod;
        if (__builtin_mul_overflow(a & 0xFFFF, b & 0xFFFF, &u_prod)) {
            update_hash(0xF00D);
        }
        
        // Shift with potential overflow
        uint32_t shifted = b << 8;
        if (shifted < b) {  // Detect wrap-around
            update_hash(shifted);
        }
    }
}

// ==================== Test 6: Additional Edge Cases ====================
NOINLINE void test_edge_cases() {
    // Direct comparisons with extreme values
    int64_t large_values[] = {
        INT64_MAX, INT64_MIN, 
        INT64_MAX - 1, INT64_MIN + 1,
        0x7FFFFFFFFFFFFFFFLL, 0x8000000000000000LL
    };
    
    for (size_t i = 0; i < sizeof(large_values)/sizeof(large_values[0]); i++) {
        // Comparisons that should trigger the uncovered logic
        if (large_values[i] > INT64_MAX - 1000) {
            update_hash(i);
        }
        if (large_values[i] < INT64_MIN + 1000) {
            update_hash(i + 100);
        }
        
        // Zero/sign extension simulation
        uint32_t truncated = (uint32_t)large_values[i];
        int32_t signed_truncated = (int32_t)large_values[i];
        
        if (truncated > 0xFFFFFF00U) {
            update_hash(truncated);
        }
        if (signed_truncated < -0x7FFFFF00) {
            update_hash(signed_truncated + 0x80000000);
        }
    }
    
    // Complex conditional with range analysis
    uint32_t x = 0x87654321;
    for (int i = 0; i < 32; i++) {
        uint32_t masked = x & ((1 << i) - 1);
        uint32_t shifted = x >> (31 - i);
        
        // This comparison needs to understand both values are in [0, 2^i-1]
        if (masked > shifted && masked - shifted < (1 << (i/2))) {
            update_hash(masked ^ shifted);
        }
    }
}

// ==================== Main Function ====================
int main() {
    // Initialize hash
    hash = 0x12345678;
    
    // Run all tests
    test_narrowing_conversions();
    test_loop_range_analysis();
    test_saturation_arithmetic();
    test_bitfield_ranges();
    test_overflow_builtins();
    test_edge_cases();
    
    // Print final hash to prevent optimization
    printf("Final hash: %u\n", hash);
    
    return 0;
}
