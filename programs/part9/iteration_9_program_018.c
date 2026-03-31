#include <stdint.h>
#include <stdio.h>
#include <string.h>

// Prevent dead code elimination
static volatile int sink;

// Prevent inlining for better coverage tracking
__attribute__((noinline,noipa))
uint32_t test_narrowing_conversions(void) {
    uint32_t hash = 0;
    
    // Test 1: Narrowing conversions with boundary values
    int64_t wide_vals[] = {
        INT64_MAX,
        INT64_MIN,
        (int64_t)UINT32_MAX,
        (int64_t)INT32_MAX + 1,
        (int64_t)INT32_MIN - 1
    };
    
    for (int i = 0; i < 5; i++) {
        int32_t narrow = (int32_t)wide_vals[i];
        uint32_t unarrow = (uint32_t)wide_vals[i];
        hash ^= (uint32_t)narrow ^ (unarrow << (i % 16));
    }
    
    // Test 2: Shifts that may overflow
    uint64_t x = 0xFFFFFFFFULL;
    for (int shift = 28; shift <= 36; shift++) {
        uint64_t shifted = x << shift;
        uint32_t truncated = (uint32_t)shifted;
        hash += truncated >> (shift % 8);
    }
    
    // Test 3: Mixed-width arithmetic
    int32_t a = 1000000000;
    int64_t b = 2000000000LL;
    int32_t c = (int32_t)(a * b / 1000);
    hash ^= (uint32_t)c;
    
    return hash;
}

__attribute__((noinline,noipa))
uint32_t test_loop_range_analysis(void) {
    uint32_t hash = 0;
    
    // Complex loop bounds with bitwise operations
    uint32_t mask1 = 0x00000FFF;
    uint32_t mask2 = 0x000007FF;
    
    // Outer loop with bitmask-limited range
    for (uint32_t i = 1000 & mask1; i < (5000 | mask2); i += 127) {
        // Inner loop with dependent bounds
        uint32_t inner_limit = (i & 0xFF) + 50;
        for (uint32_t j = i & 0x7F; j < inner_limit; j += 13) {
            hash = hash * 31 + (i ^ j);
            
            // Additional bitwise condition
            if ((j & (i >> 4)) > 100) {
                hash ^= 0x5A5A5A5A;
            }
        }
        
        // Loop with wrap-around behavior
        uint32_t k = i;
        do {
            hash = (hash << 1) | (hash >> 31);
            k = (k * 1103515245U + 12345U) & 0x7FFFFFFF;
        } while (k > 100 && k < (i | 0x3FF));
    }
    
    // Nested loops with XOR-based bounds
    for (int x = 10; x < 100; x += 7) {
        int y_limit = (x ^ 0x55) & 0x3F;
        for (int y = 0; y < y_limit; y++) {
            if ((x & y) > (x ^ y)) {
                hash += x * y;
            }
        }
    }
    
    return hash;
}

__attribute__((noinline,noipa))
uint32_t test_saturation_arithmetic(void) {
    uint32_t hash = 0;
    
    // Manual saturation arithmetic
    int32_t sat_min = -1000;
    int32_t sat_max = 1000;
    
    int32_t test_vals[] = {-2000, -500, 0, 500, 1500, 3000};
    
    for (int i = 0; i < 6; i++) {
        int32_t val = test_vals[i];
        
        // Saturation clamp
        int32_t saturated;
        if (val < sat_min) {
            saturated = sat_min;
        } else if (val > sat_max) {
            saturated = sat_max;
        } else {
            saturated = val;
        }
        
        hash = hash * 17 + (uint32_t)saturated;
        
        // Double saturation with intermediate overflow
        int32_t doubled = val * 2;
        if (doubled < sat_min) doubled = sat_min;
        if (doubled > sat_max) doubled = sat_max;
        hash ^= (uint32_t)doubled << 8;
    }
    
    // Fixed-point like operations (emulated)
    int32_t fp_a = 500;
    int32_t fp_b = 300;
    int32_t fp_scale = 256; // Q8.8 scaling
    
    // Simulated fixed-point multiplication
    int64_t fp_prod = (int64_t)fp_a * fp_b;
    int32_t fp_result = (int32_t)(fp_prod / fp_scale);
    
    // Check for overflow in scaling
    if (fp_result > 1000) fp_result = 1000;
    if (fp_result < -1000) fp_result = -1000;
    
    hash += (uint32_t)fp_result;
    
    return hash;
}

// Struct with bit-fields
struct bitfield_struct {
    unsigned int a : 5;   // 0-31
    signed int b : 7;     // -64 to 63
    unsigned int c : 12;  // 0-4095
    signed int d : 20;    // -524288 to 524287
};

__attribute__((noinline,noipa))
uint32_t test_bitfield_ranges(void) {
    uint32_t hash = 0;
    struct bitfield_struct bs;
    
    // Assign boundary values to bit-fields
    bs.a = 31;      // Max for 5 bits
    bs.b = -64;     // Min for 7-bit signed
    bs.c = 4095;    // Max for 12 bits
    bs.d = 524287;  // Max for 20-bit signed
    
    // Comparisons against bit-field limits
    if (bs.a == 31) hash ^= 1;
    if (bs.b < 0) hash ^= 2;
    if (bs.c > 2047) hash ^= 4;
    if (bs.d >= 262144) hash ^= 8;
    
    // Arithmetic that may overflow bit-field
    unsigned int temp = bs.a * 10;  // Could exceed 5 bits
    bs.a = temp & 0x1F;  // Explicit mask
    
    hash = (hash << 16) | (bs.a << 11) | (bs.c & 0xFFF);
    
    // Union with overlapping bit-fields
    union {
        uint32_t full;
        struct {
            unsigned int low : 10;
            unsigned int mid : 10;
            unsigned int high : 12;
        } parts;
    } u;
    
    u.full = 0x12345678;
    if (u.parts.low > 512) hash += u.parts.mid;
    if (u.parts.high < 2048) hash ^= u.parts.high;
    
    return hash;
}

__attribute__((noinline,noipa))
uint32_t test_overflow_builtins(void) {
    uint32_t hash = 0;
    
    // Test overflow builtins with various ranges
    int32_t ovf_a = 1000000000;
    int32_t ovf_b = 2000000000;
    int32_t ovf_result;
    
    // Addition that may overflow
    if (__builtin_add_overflow(ovf_a, ovf_b, &ovf_result)) {
        hash ^= 0xDEADBEEF;
    } else {
        hash += (uint32_t)ovf_result;
    }
    
    // Multiplication in a loop with constrained ranges
    for (int32_t i = 10000; i < 50000; i += 7777) {
        int32_t j = i * 3;
        int32_t k;
        
        // Check multiplication overflow
        if (__builtin_mul_overflow(i, j, &k)) {
            hash = (hash << 5) | (hash >> 27);
        } else {
            hash += (uint32_t)k;
        }
        
        // Nested overflow checks
        int32_t m = i + 1000;
        int32_t n;
        if (!__builtin_add_overflow(i, m, &n)) {
            int32_t p;
            if (__builtin_mul_overflow(n, 2, &p)) {
                hash ^= (uint32_t)i;
            }
        }
    }
    
    // Overflow with bitmask-constrained values
    uint32_t masked_x = 0x7FFFFFFF & 0x0FFFFFFF;
    uint32_t masked_y = 0x3FFFFFFF & 0x00FFFFFF;
    uint32_t sum;
    
    if (__builtin_uadd_overflow(masked_x, masked_y, &sum)) {
        hash |= 0x80000000;
    }
    
    // Chain of operations with overflow checks
    int64_t accum = 1;
    for (int i = 1; i <= 20; i++) {
        int64_t next;
        if (__builtin_mul_overflow_int(accum, i, &next)) {
            break;
        }
        accum = next;
        hash ^= (uint32_t)(accum >> 32) ^ (uint32_t)accum;
    }
    
    return hash;
}

__attribute__((noinline,noipa))
uint32_t test_edge_case_conditions(void) {
    uint32_t hash = 0;
    
    // Comparisons at type boundaries
    int32_t x = 0x7FFFFFFF;  // INT_MAX
    
    // These conditions test the boundary comparison logic
    if (x > INT32_MAX - 10) {
        hash ^= 0x11111111;
    }
    
    if (x - 5 < INT32_MIN + 10) {
        hash ^= 0x22222222;
    }
    
    // Range-restricted variable
    uint32_t y = 100;
    if (y > 0 && y < 200) {
        uint32_t z = y * 3;
        if (z > 250) {
            hash += z;
        }
    }
    
    // Complex boundary condition
    int64_t big = INT64_MAX;
    if (big - 1000 > INT32_MAX) {
        hash ^= (uint32_t)(big >> 32);
    }
    
    // Modulo-constrained range
    for (int i = 0; i < 1000; i++) {
        int constrained = i % 137;
        if (constrained > 100 && constrained < 136) {
            hash = hash * 13 + constrained;
        }
    }
    
    return hash;
}

int main(void) {
    uint32_t final_hash = 0;
    
    // Run all tests
    final_hash ^= test_narrowing_conversions();
    final_hash ^= test_loop_range_analysis();
    final_hash ^= test_saturation_arithmetic();
    final_hash ^= test_bitfield_ranges();
    final_hash ^= test_overflow_builtins();
    final_hash ^= test_edge_case_conditions();
    
    // Use sink to prevent optimization
    sink = (int)final_hash;
    
    // Print to prevent dead code elimination
    printf("Result: %u\n", final_hash);
    
    return 0;
}
