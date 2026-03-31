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

// Test 1: Narrowing conversions with range boundary checks
NOINLINE int test_narrowing_conversions() {
    int result = 0;
    
    // Constants at type boundaries
    int64_t large_positive = 0x7FFFFFFFFFFFFFFFLL;
    int64_t large_negative = 0x8000000000000000LL;
    uint64_t large_unsigned = 0xFFFFFFFFFFFFFFFFULL;
    
    // Narrowing conversions that require range analysis
    int32_t narrow1 = (int32_t)large_positive;  // May overflow
    int32_t narrow2 = (int32_t)large_negative;  // May overflow
    uint32_t narrow3 = (uint32_t)large_unsigned; // Truncation
    
    // Comparisons against boundaries
    if (narrow1 > 0x7FFFFFFF - 100) {
        result ^= 1;
    }
    if (narrow2 < -0x7FFFFFFF + 100) {
        result ^= 2;
    }
    if (narrow3 > 0xFFFFFFFFU - 1000) {
        result ^= 4;
    }
    
    // Shifts that may overflow
    int32_t shifted = narrow1 << 3;
    if (shifted > 0x7FFFFFFF) {
        result ^= 8;
    }
    
    // Compound assignment with narrowing
    int16_t very_narrow = 0;
    for (int64_t i = 0; i < 1000; i++) {
        very_narrow += (int16_t)(i * 100);  // Requires range analysis
    }
    result ^= very_narrow;
    
    checksum += result;
    return result;
}

// Test 2: Complex loop bound analysis
NOINLINE int test_loop_range_analysis() {
    int result = 0;
    
    // Variables with constrained ranges
    uint32_t a = 0x12345678;
    uint32_t b = 0x9ABCDEF0;
    int32_t c = 100;
    
    // Outer loop with bitwise operation in bound
    for (uint32_t i = a & 0xFFF; i < (b | 0x7FF); i += (c & 0x3F)) {
        // Inner loop with dependent bounds
        for (int32_t j = (i & 0xFF) - 128; j < (int32_t)(i >> 8) + 64; j += 3) {
            result += j * i;
            
            // Additional condition that depends on both indices
            if ((i ^ j) > 0x80000000) {
                result ^= j;
            }
        }
        
        // Loop with shift operation in step
        for (int32_t k = -1000; k < 1000; k += (i & 0x1F) << 1) {
            result += k;
            
            // Condition at extreme boundary
            if (k > 0x7FFFFFF0) {
                result |= 0x80000000;
            }
        }
    }
    
    // Nested loops with complex exit conditions
    int32_t x = 0, y = 0;
    while (x < 1000) {
        y = 0;
        while (y < (x & 0x3FF) * 2) {
            result += x * y;
            y += (x & 0x1F) + 1;
            
            // Comparison requiring range analysis
            if (y > 0x7FFFFFFF - x) {
                result ^= 0x55555555;
            }
        }
        x += (y & 0x7F) + 1;
    }
    
    checksum += result;
    return result;
}

// Test 3: Saturation arithmetic and fixed-point
NOINLINE int test_saturation_arithmetic() {
    int result = 0;
    
    // Manual saturation arithmetic
    int32_t sat_add(int32_t a, int32_t b) {
        int64_t sum = (int64_t)a + (int64_t)b;
        if (sum > 0x7FFFFFFF) return 0x7FFFFFFF;
        if (sum < -0x80000000) return -0x80000000;
        return (int32_t)sum;
    }
    
    int32_t sat_mul(int32_t a, int32_t b) {
        int64_t prod = (int64_t)a * (int64_t)b;
        if (prod > 0x7FFFFFFF) return 0x7FFFFFFF;
        if (prod < -0x80000000) return -0x80000000;
        return (int32_t)prod;
    }
    
    // Test saturation at boundaries
    int32_t val1 = 0x70000000;
    int32_t val2 = 0x10000000;
    
    result = sat_add(val1, val2);  // Should saturate to 0x7FFFFFFF
    result ^= sat_add(0x80000000, -1);  // Edge case
    
    result ^= sat_mul(0x10000, 0x10000);  // Should saturate
    
    // Array of values to saturate
    int32_t values[] = {0, 1, -1, 0x7FFFFFFF, 0x80000000, 0x40000000};
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 6; j++) {
            result += sat_add(values[i], values[j]);
            result ^= sat_mul(values[i], values[j]);
        }
    }
    
#ifdef __STDC_IEC_559__
    // Fixed-point arithmetic if supported
    _Accum fx1 = 0.5k;
    _Accum fx2 = 0.75k;
    _Accum fx_sum = fx1 + fx2;
    
    if (fx_sum > 1.0k) {
        result ^= 0xAAAAAAAA;
    }
#endif
    
    checksum += result;
    return result;
}

// Test 4: Bit-field range analysis
NOINLINE int test_bitfield_ranges() {
    int result = 0;
    
    // Struct with various bit-fields
    struct BitFields {
        unsigned int a : 3;    // 0-7
        signed int b : 5;      // -16 to 15
        unsigned int c : 10;   // 0-1023
        signed int d : 12;     // -2048 to 2047
        unsigned int e : 1;    // 0-1
    } bf;
    
    // Assign values at boundaries
    bf.a = 7;      // Max for 3 bits
    bf.b = -16;    // Min for 5 signed bits
    bf.c = 1023;   // Max for 10 bits
    bf.d = 2047;   // Max for 12 signed bits
    bf.e = 1;      // Max for 1 bit
    
    // Comparisons that require understanding bit-field ranges
    if (bf.a > 6) {           // Knows max is 7
        result ^= 1;
    }
    if (bf.b < -15) {         // Knows min is -16
        result ^= 2;
    }
    if (bf.c == 1023) {       // Knows exact max
        result ^= 4;
    }
    if (bf.d >= 2046) {       // Close to max
        result ^= 8;
    }
    
    // Operations that may overflow bit-field
    unsigned int temp = bf.a + 10;  // 7 + 10 = 17, but a only holds 3 bits
    bf.a = temp;  // Requires range analysis for truncation
    
    // Union with bit-fields and regular ints
    union BitUnion {
        struct {
            unsigned int x : 4;
            unsigned int y : 4;
            unsigned int z : 8;
        } bits;
        uint16_t full;
    } u;
    
    u.full = 0xFFFF;
    // Comparisons that need to understand masked values
    if (u.bits.x == 0xF) {    // Knows x is 4 bits
        result ^= 0x10;
    }
    if (u.bits.z > 0xFE) {    // Knows z is 8 bits
        result ^= 0x20;
    }
    
    // Loop using bit-field as counter
    for (bf.c = 0; bf.c < 1024; bf.c++) {
        result += bf.c;
        
        // Condition using multiple bit-fields
        if (bf.c > 1000 && bf.a == 7) {
            result ^= bf.c;
        }
    }
    
    checksum += result;
    return result;
}

// Test 5: Overflow builtins with range analysis
NOINLINE int test_overflow_builtins() {
    int result = 0;
    
    // Variables with partially known ranges
    int32_t x = 1000;
    int32_t y = 2000;
    
    // Basic overflow checks
    int32_t sum;
    if (__builtin_add_overflow(x, y, &sum)) {
        result ^= 1;
    }
    
    // In loops with varying ranges
    for (int i = -100; i < 100; i++) {
        int32_t prod;
        if (__builtin_mul_overflow(i, i, &prod)) {
            result ^= i;
        }
        
        // Chain of operations
        int32_t tmp;
        if (!__builtin_add_overflow(i, 1000, &tmp)) {
            if (__builtin_mul_overflow(tmp, 2, &tmp)) {
                result ^= 0x100;
            }
        }
    }
    
    // With values at boundaries
    int32_t extremes[] = {0x7FFFFFFF, 0x80000000, 0, 1, -1};
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            int32_t res;
            if (__builtin_add_overflow(extremes[i], extremes[j], &res)) {
                result ^= (i << 4) | j;
            }
            if (__builtin_mul_overflow(extremes[i], extremes[j], &res)) {
                result ^= (i << 8) | (j << 12);
            }
        }
    }
    
    // Sub-overflow with unsigned
    uint32_t u1 = 0xFFFFFFFF;
    uint32_t u2 = 1;
    uint32_t ures;
    if (__builtin_add_overflow(u1, u2, &ures)) {
        result ^= 0x80000000;
    }
    
    // Combined with range-restricting conditions
    int32_t a = 500, b = 600;
    if (a > 0 && a < 1000) {          // Compiler knows range
        if (b > 0 && b < 1000) {      // Compiler knows range
            int32_t mul;
            // Should not overflow given known ranges
            if (__builtin_mul_overflow(a, b, &mul)) {
                result ^= 0x40000000;
            }
        }
    }
    
    checksum += result;
    return result;
}

// Test 6: Additional edge cases for double_int comparisons
NOINLINE int test_edge_cases() {
    int result = 0;
    
    // Comparisons exactly at type boundaries
    int64_t val = 0x7FFFFFFFFFFFFFFFLL;
    
    if (val == 0x7FFFFFFFFFFFFFFFLL) {
        result ^= 1;
    }
    if (val > 0x7FFFFFFFFFFFFFFELL) {
        result ^= 2;
    }
    
    // Zero extension scenarios
    uint32_t uval = 0xFFFFFFFF;
    uint64_t uext = (uint64_t)uval;  // Zero extension
    
    if (uext == 0x00000000FFFFFFFFULL) {
        result ^= 4;
    }
    
    // Sign extension scenarios
    int32_t sval = -1;
    int64_t sext = (int64_t)sval;  // Sign extension
    
    if (sext == -1LL) {
        result ^= 8;
    }
    if (sext < 0) {
        result ^= 0x10;
    }
    
    // Shift operations that may need range analysis
    int32_t shift_val = 0x40000000;
    for (int i = 0; i < 4; i++) {
        int32_t shifted = shift_val << i;
        if (shifted > 0x7FFFFFFF) {
            result ^= (0x20 << i);
        }
    }
    
    // Arithmetic right shift with sign extension
    int32_t neg_val = -1000;
    for (int i = 0; i < 4; i++) {
        int32_t arshift = neg_val >> i;
        if (arshift < -500) {
            result ^= (0x100 << i);
        }
    }
    
    checksum += result;
    return result;
}

int main() {
    int total = 0;
    
    total += test_narrowing_conversions();
    total += test_loop_range_analysis();
    total += test_saturation_arithmetic();
    total += test_bitfield_ranges();
    total += test_overflow_builtins();
    total += test_edge_cases();
    
    // Use checksum to prevent optimization
    if (checksum != 0) {
        printf("Result: %d\n", total);
    }
    
    return total & 0xFF;  // Return non-zero to indicate execution
}
