#include <stdint.h>
#include <stddef.h>
#include <string.h>

// Prevent inlining to maintain function boundaries for coverage
#define NOINLINE __attribute__((noinline))

// Fixed-point types if available
#ifdef __STDC_IEC_559__
#include <stdfix.h>
#endif

// ==================== Test 1: Narrowing Conversions ====================
NOINLINE uint32_t test_narrowing_conversions(void) {
    uint32_t checksum = 0;
    
    // Test with constants at boundary values
    int64_t wide_val1 = 0x7FFFFFFF;      // INT_MAX for 32-bit
    int64_t wide_val2 = 0x80000000;      // INT_MIN for 32-bit
    int64_t wide_val3 = 0xFFFFFFFF;      // -1 in 32-bit signed
    int64_t wide_val4 = 0x100000000LL;   // Just beyond 32-bit range
    
    // Narrowing conversions that require range analysis
    int32_t narrow1 = (int32_t)wide_val1;  // Should fit
    int32_t narrow2 = (int32_t)wide_val2;  // Should fit (wraps)
    int32_t narrow3 = (int32_t)wide_val3;  // Should fit
    int32_t narrow4 = (int32_t)wide_val4;  // Overflow in conversion
    
    checksum ^= narrow1 ^ narrow2 ^ narrow3 ^ narrow4;
    
    // More complex cases with arithmetic
    uint64_t a = 0xFFFFFFFFULL;
    uint64_t b = 0x100000001ULL;
    uint32_t c = (uint32_t)(a + b);  // Requires range analysis
    uint32_t d = (uint32_t)(a * 2);  // Multiplication overflow check
    
    checksum ^= c ^ d;
    
    // Shifts that may overflow
    int32_t shift_val = 0x40000000;
    int32_t shifted1 = shift_val << 1;  // May overflow to negative
    int32_t shifted2 = shift_val >> 31; // Boundary shift
    
    checksum ^= shifted1 ^ shifted2;
    
    return checksum;
}

// ==================== Test 2: Loop Range Analysis ====================
NOINLINE uint32_t test_loop_range_analysis(void) {
    uint32_t checksum = 0;
    
    // Complex loop bounds with bitwise operations
    uint32_t a = 0x12345678;
    uint32_t b = 0x9ABCDEF0;
    
    // Loop with bounds derived from bitwise ops
    for (uint32_t i = a & 0xFFF; i < (b | 0x7FF); i += 37) {
        checksum ^= i;
        // Nested loop with dependent bounds
        for (uint32_t j = i & 0xFF; j < 1000; j += 51) {
            checksum += j * 3;
        }
    }
    
    // Loop with signed bounds at extremes
    int32_t min_val = -1000;
    int32_t max_val = 1000;
    
    for (int32_t k = min_val; k < max_val; k += 7) {
        // Condition that depends on k's range
        if (k > 500 || k < -500) {
            checksum ^= (k * 2);
        }
        
        // Bitwise operation in loop condition
        uint32_t masked = k & 0x3FF;
        if (masked > 512) {  // Always false due to mask, but compiler needs to prove
            checksum += 0xDEADBEEF;
        }
    }
    
    // Loop with wrap-around analysis
    uint8_t counter = 250;
    for (int i = 0; i < 20; i++) {
        counter++;  // Will wrap around
        checksum += counter;
    }
    
    return checksum;
}

// ==================== Test 3: Saturation Arithmetic ====================
NOINLINE uint32_t test_saturation_arithmetic(void) {
    uint32_t checksum = 0;
    
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
    
    // Test cases at boundaries
    int32_t test_vals[] = {
        0x7FFFFFF0,  // Near INT_MAX
        0x8000000F,  // Near INT_MIN
        0x40000000,
        -0x40000000,
        100,
        -100
    };
    
    for (size_t i = 0; i < sizeof(test_vals)/sizeof(test_vals[0]); i++) {
        for (size_t j = 0; j < sizeof(test_vals)/sizeof(test_vals[0]); j++) {
            int32_t saturated = sat_add(test_vals[i], test_vals[j]);
            checksum ^= saturated;
            
            saturated = sat_mul(test_vals[i], test_vals[j]);
            checksum += saturated;
        }
    }
    
#ifdef __STDC_IEC_559__
    // Fixed-point arithmetic if supported
    _Accum fx1 = 0.5k;
    _Accum fx2 = 0.8k;
    _Accum fx_sum = fx1 + fx2;
    checksum ^= *(uint32_t*)&fx_sum;
#endif
    
    return checksum;
}

// ==================== Test 4: Bit-Field Ranges ====================
NOINLINE uint32_t test_bitfield_ranges(void) {
    uint32_t checksum = 0;
    
    // Struct with various bit-fields
    struct BitFieldStruct {
        unsigned int small : 4;    // 0-15
        signed int signed_small : 5; // -16 to 15
        unsigned int medium : 10;  // 0-1023
        signed int signed_medium : 11; // -1024 to 1023
        unsigned int large : 31;   // 0-2^31-1
    } bfs;
    
    // Assign boundary values
    bfs.small = 15;      // Max for 4-bit unsigned
    bfs.signed_small = -16; // Min for 5-bit signed
    bfs.medium = 1023;   // Max for 10-bit unsigned
    bfs.signed_medium = 1023; // Max for 11-bit signed
    bfs.large = 0x7FFFFFFF >> 1; // Large value
    
    // Comparisons that require range analysis
    if (bfs.small > 14) {
        checksum ^= 0x1111;
    }
    
    if (bfs.signed_small < -15) {
        checksum ^= 0x2222;
    }
    
    if (bfs.medium >= 1000 && bfs.medium <= 1023) {
        checksum ^= 0x3333;
    }
    
    // Union with overlapping bit-fields
    union OverlapUnion {
        struct {
            unsigned int low : 16;
            unsigned int high : 16;
        } parts;
        uint32_t whole;
    } ou;
    
    ou.parts.low = 0xFFFF;
    ou.parts.high = 0x7FFF;
    
    if (ou.parts.low == 0xFFFF) {
        checksum ^= ou.whole;
    }
    
    // Complex bit-field expression
    bfs.small = (bfs.medium >> 6) & 0xF;
    if (bfs.small < 16) {  // Always true, but compiler must prove
        checksum += bfs.small;
    }
    
    return checksum;
}

// ==================== Test 5: Overflow Builtins ====================
NOINLINE uint32_t test_overflow_builtins(void) {
    uint32_t checksum = 0;
    int overflow;
    
    // Test with boundary values
    int32_t max_int = 0x7FFFFFFF;
    int32_t min_int = 0x80000000;
    int32_t large_val = 0x40000000;
    
    // Addition overflow checks
    int32_t result;
    overflow = __builtin_add_overflow(max_int, 1, &result);
    checksum ^= (overflow << 0) ^ result;
    
    overflow = __builtin_add_overflow(min_int, -1, &result);
    checksum ^= (overflow << 1) ^ result;
    
    overflow = __builtin_add_overflow(large_val, large_val, &result);
    checksum ^= (overflow << 2) ^ result;
    
    // Multiplication overflow checks
    overflow = __builtin_mul_overflow(max_int, 2, &result);
    checksum ^= (overflow << 3) ^ result;
    
    overflow = __builtin_mul_overflow(large_val, 4, &result);
    checksum ^= (overflow << 4) ^ result;
    
    // Subtraction overflow
    overflow = __builtin_sub_overflow(min_int, 1, &result);
    checksum ^= (overflow << 5) ^ result;
    
    // In loops with range-restricted values
    for (int32_t i = -100; i < 100; i++) {
        int32_t a = i * 1000;
        int32_t b = i * 1000;
        
        // Overflow check with partially known ranges
        if (__builtin_add_overflow(a, b, &result)) {
            checksum += 0xABCD;
        } else {
            checksum ^= result;
        }
    }
    
    // Unsigned overflow checks
    uint32_t u_max = 0xFFFFFFFF;
    uint32_t u_result;
    overflow = __builtin_add_overflow(u_max, 1, &u_result);
    checksum ^= (overflow << 6) ^ u_result;
    
    overflow = __builtin_mul_overflow(u_max, 2, &u_result);
    checksum ^= (overflow << 7) ^ u_result;
    
    return checksum;
}

// ==================== Test 6: Edge Case Comparisons ====================
NOINLINE uint32_t test_edge_comparisons(void) {
    uint32_t checksum = 0;
    
    // Comparisons at extreme boundaries
    int32_t x = 0x7FFFFFF0;
    int32_t y = 0x8000000F;
    
    // These comparisons should trigger range analysis
    if (x > 0x7FFFFFF0 - 10) {
        checksum ^= 0xAAAA;
    }
    
    if (y < 0x80000000 + 10) {
        checksum ^= 0xBBBB;
    }
    
    // Chain of comparisons
    int32_t z = 100;
    if (z > 0 && z < 1000) {
        // z is now known to be in [1, 999]
        if (z * 1000 > 0x7FFFFFFF) {  // Can this overflow?
            checksum ^= 0xCCCC;
        }
    }
    
    // Modulo creates known range
    int32_t w = 12345;
    int32_t constrained = w % 1000;  // Known to be in [0, 999]
    
    if (constrained > 500) {
        checksum ^= constrained;
    }
    
    // Shift with potential overflow
    int32_t shift_test = 0x40000000;
    for (int i = 0; i < 4; i++) {
        int32_t shifted = shift_test << i;
        if (shifted > 0x7FFFFFFF || shifted < 0x80000000) {
            checksum += shifted;
        }
    }
    
    return checksum;
}

// ==================== Main Function ====================
int main(void) {
    uint32_t final_checksum = 0;
    
    // Run all tests
    final_checksum ^= test_narrowing_conversions();
    final_checksum ^= test_loop_range_analysis();
    final_checksum ^= test_saturation_arithmetic();
    final_checksum ^= test_bitfield_ranges();
    final_checksum ^= test_overflow_builtins();
    final_checksum ^= test_edge_comparisons();
    
    // Prevent dead code elimination
    volatile uint32_t sink = final_checksum;
    
    return (int)(final_checksum & 0x7FFFFFFF);
}
