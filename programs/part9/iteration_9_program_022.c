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

// ==================== Test 1: Narrowing Conversions ====================
NOINLINE int test_narrowing_conversions(void) {
    int result = 0;
    
    // Test 1a: Constants at boundaries
    int64_t wide_constants[] = {
        INT64_MAX,
        INT64_MIN,
        (int64_t)UINT32_MAX + 1,
        (int64_t)INT32_MAX * 2,
        (int64_t)INT32_MIN * 2
    };
    
    for (size_t i = 0; i < sizeof(wide_constants)/sizeof(wide_constants[0]); i++) {
        int32_t narrow = (int32_t)wide_constants[i];
        result ^= narrow;
    }
    
    // Test 1b: Variables with arithmetic
    uint64_t a = 0xFFFFFFFFFFFFFFF0ULL;
    uint32_t b = (uint32_t)(a >> 4);
    uint16_t c = (uint16_t)(b * 3);
    int8_t d = (int8_t)(c - 128);
    
    result += b + c + d;
    
    // Test 1c: Complex narrowing with shifts
    for (int64_t i = -100; i <= 100; i++) {
        int64_t val = i * 0x55555555LL;
        int32_t narrowed = (int32_t)(val >> (i & 0x1F));
        result += narrowed;
    }
    
    checksum ^= result;
    return result;
}

// ==================== Test 2: Loop Range Analysis ====================
NOINLINE int test_loop_range_analysis(void) {
    int result = 0;
    
    // Test 2a: Complex loop bounds with bitwise operations
    uint32_t base = 0x12345678;
    uint32_t mask = 0x00000FFF;
    
    for (uint32_t i = base & mask; 
         i < (base | 0x000007FF); 
         i += (i & 0x3F) + 1) {
        result += i;
        
        // Nested loop with dependent bounds
        for (uint32_t j = i & 0xFF; j < (i | 0x7F); j += 3) {
            result ^= j;
        }
    }
    
    // Test 2b: Loop with signed arithmetic and boundary conditions
    int32_t start = -1000;
    int32_t end = 1000;
    int32_t step = 7;
    
    for (int32_t k = start; k < end; k += step) {
        // Complex condition that requires range analysis
        if ((k & 0x3FF) > 512 || (k ^ 0x5555) < 100) {
            result += k * 2;
        } else {
            result -= k;
        }
    }
    
    // Test 2c: Loop with modulo operation in bounds
    for (uint64_t m = 0; m < 1000; m++) {
        uint64_t limit = (m * 137) % 256;
        for (uint64_t n = m % 16; n < limit; n++) {
            result += (int)(n * m);
        }
    }
    
    checksum ^= result;
    return result;
}

// ==================== Test 3: Saturation Arithmetic ====================
NOINLINE int test_saturation_arithmetic(void) {
    int result = 0;
    
    // Manual saturation implementation
    int32_t sat_add(int32_t a, int32_t b) {
        int64_t sum = (int64_t)a + (int64_t)b;
        if (sum > INT32_MAX) return INT32_MAX;
        if (sum < INT32_MIN) return INT32_MIN;
        return (int32_t)sum;
    }
    
    int32_t sat_mul(int32_t a, int32_t b) {
        int64_t prod = (int64_t)a * (int64_t)b;
        if (prod > INT32_MAX) return INT32_MAX;
        if (prod < INT32_MIN) return INT32_MIN;
        return (int32_t)prod;
    }
    
    // Test saturation with boundary values
    int32_t test_values[] = {
        INT32_MAX, INT32_MIN, 0,
        INT32_MAX - 100, INT32_MIN + 100,
        10000, -10000
    };
    
    for (size_t i = 0; i < sizeof(test_values)/sizeof(test_values[0]); i++) {
        for (size_t j = 0; j < sizeof(test_values)/sizeof(test_values[0]); j++) {
            result += sat_add(test_values[i], test_values[j]);
            result ^= sat_mul(test_values[i], test_values[j]);
        }
    }
    
    // Fixed-point arithmetic if available
    #ifdef __STDC_IEC_559__
    _Accum acc = 0.5k;
    for (int i = 0; i < 100; i++) {
        acc += 0.1k;
        if (acc > 1.0k) acc = 1.0k;
        if (acc < -1.0k) acc = -1.0k;
        result += (int)(acc * 1000);
    }
    #endif
    
    checksum ^= result;
    return result;
}

// ==================== Test 4: Bit-Field Ranges ====================
NOINLINE int test_bitfield_ranges(void) {
    int result = 0;
    
    // Struct with various bit-fields
    struct BitFieldStruct {
        unsigned int a : 3;    // 0-7
        signed int b : 5;      // -16 to 15
        unsigned int c : 10;   // 0-1023
        signed int d : 12;     // -2048 to 2047
        unsigned int e : 1;    // 0-1
    } bfs;
    
    // Union to test bit-field packing
    union BitFieldUnion {
        struct BitFieldStruct bits;
        uint32_t raw;
    } u;
    
    // Test assignments at boundaries
    for (unsigned int i = 0; i <= 7; i++) {
        bfs.a = i;
        bfs.b = (i < 4) ? -8 : 7;
        bfs.c = i * 128;
        bfs.d = (i * 512) - 1024;
        bfs.e = i & 1;
        
        // Comparisons that require range analysis
        if (bfs.a > 5 || bfs.b < -10) {
            result += 1;
        }
        if (bfs.c >= 512 && bfs.d <= 0) {
            result += 2;
        }
        if (bfs.e == 0 || bfs.a + bfs.b > 10) {
            result += 4;
        }
    }
    
    // Test with union
    u.raw = 0xFFFFFFFF;
    result += u.bits.a + u.bits.b + u.bits.c + u.bits.d + u.bits.e;
    
    checksum ^= result;
    return result;
}

// ==================== Test 5: Overflow Builtins ====================
NOINLINE int test_overflow_builtins(void) {
    int result = 0;
    int overflow;
    
    // Test 5a: Basic overflow checks
    int32_t x = 1000000;
    int32_t y = 2000000;
    
    if (__builtin_add_overflow(x, y, &overflow)) {
        result += 1;
    }
    
    if (__builtin_mul_overflow(x, y, &overflow)) {
        result += 2;
    }
    
    // Test 5b: Overflow in loops with range constraints
    for (int32_t i = -100; i <= 100; i++) {
        int32_t a = i * 10000;
        int32_t b = (i & 0x7F) * 50000;
        
        if (!__builtin_add_overflow(a, b, &overflow)) {
            result += overflow;
        }
        
        if (i > 0 && i < 50) {
            // Range is known to be positive and limited
            if (!__builtin_mul_overflow(a, i, &overflow)) {
                result ^= overflow;
            }
        }
    }
    
    // Test 5c: Complex overflow conditions
    uint64_t large = 0xFFFFFFFFFFFFFFFFULL;
    uint32_t medium = 0x7FFFFFFF;
    
    if (__builtin_uadd_overflow(medium, 1, &overflow)) {
        result += 4;
    }
    
    if (__builtin_umul_overflow(medium, 2, &overflow)) {
        result += 8;
    }
    
    // Test 5d: Overflow with bitwise constrained values
    for (uint32_t val = 0; val < 256; val++) {
        uint32_t constrained = val & 0x3F;  // 0-63
        uint32_t multiplier = (val >> 4) & 0x7;  // 0-7
        
        if (!__builtin_umul_overflow(constrained, multiplier, &overflow)) {
            result += overflow % 256;
        }
    }
    
    checksum ^= result;
    return result;
}

// ==================== Test 6: Edge Case Comparisons ====================
NOINLINE int test_edge_case_comparisons(void) {
    int result = 0;
    
    // Test comparisons at type boundaries
    int32_t values[] = {
        INT32_MAX, INT32_MIN, 0,
        INT32_MAX - 10, INT32_MIN + 10,
        1, -1
    };
    
    for (size_t i = 0; i < sizeof(values)/sizeof(values[0]); i++) {
        int32_t v = values[i];
        
        // These comparisons should trigger boundary analysis
        if (v > INT32_MAX - 100) {
            result += 1;
        }
        if (v < INT32_MIN + 100) {
            result += 2;
        }
        if (v >= 0 && v <= 100) {
            result += 4;
        }
        if (v <= 0 && v >= -100) {
            result += 8;
        }
        
        // Complex condition with arithmetic
        if (v * 2 > INT32_MAX / 2) {
            result += 16;
        }
        if (v / 2 < INT32_MIN / 2) {
            result += 32;
        }
    }
    
    // Test with unsigned boundaries
    uint32_t u = 0xFFFFFF00;
    if (u > UINT32_MAX - 255) {
        result += 64;
    }
    if (u + 300 < 300) {  // Will wrap around
        result += 128;
    }
    
    checksum ^= result;
    return result;
}

// ==================== Main Function ====================
int main(void) {
    int total = 0;
    
    total += test_narrowing_conversions();
    total += test_loop_range_analysis();
    total += test_saturation_arithmetic();
    total += test_bitfield_ranges();
    total += test_overflow_builtins();
    total += test_edge_case_comparisons();
    
    // Use checksum to prevent optimization
    printf("Result: %d (checksum: %d)\n", total, checksum);
    
    return total == 0 ? 0 : 1;
}
