#include <stdint.h>
#include <stdio.h>
#include <string.h>

// Prevent inlining to maintain function boundaries for coverage
#define NOINLINE __attribute__((noinline))

// Fixed-point types if available
#ifdef __STDC_IEC_559__
#include <stdfix.h>
#endif

// Checksum/hash to prevent dead code elimination
static uint32_t g_checksum = 0;

NOINLINE static void update_checksum(uint32_t val) {
    g_checksum = (g_checksum << 3) ^ (g_checksum >> 29) ^ val;
}

// ==================== Test 1: Narrowing Conversions ====================
NOINLINE static uint32_t test_narrowing_conversions(void) {
    uint32_t local_sum = 0;
    
    // Constants at type boundaries
    const int64_t max_int32 = INT32_MAX;
    const int64_t min_int32 = INT32_MIN;
    const uint64_t max_uint32 = UINT32_MAX;
    
    // Test 1a: Direct narrowing with boundary values
    int32_t narrow1 = (int32_t)(max_int32 + 0);  // Exactly at boundary
    int32_t narrow2 = (int32_t)(min_int32 - 0);  // Exactly at boundary
    uint32_t narrow3 = (uint32_t)(max_uint32 - 1000);
    
    update_checksum(narrow1);
    update_checksum(narrow2);
    update_checksum(narrow3);
    local_sum = narrow1 ^ narrow2 ^ narrow3;
    
    // Test 1b: Variable narrowing with arithmetic
    int64_t wide_var = 0x7FFFFFFF00000000LL;  // High bits set
    for (int i = 0; i < 10; i++) {
        int32_t narrowed = (int32_t)(wide_var >> (i * 4));
        update_checksum(narrowed);
        local_sum += narrowed;
    }
    
    // Test 1c: Chained narrowing
    uint64_t a = 0xFFFFFFFFFFFFFFFFULL;
    uint32_t b = (uint32_t)a;
    uint16_t c = (uint16_t)b;
    uint8_t d = (uint8_t)c;
    
    update_checksum(b);
    update_checksum(c);
    update_checksum(d);
    local_sum = local_sum * 31 + b + c + d;
    
    return local_sum;
}

// ==================== Test 2: Loop Range Analysis ====================
NOINLINE static uint32_t test_loop_range_analysis(void) {
    uint32_t local_sum = 0;
    
    // Complex loop bounds with bitwise operations
    uint32_t a = 0x12345678;
    uint32_t b = 0x9ABCDEF0;
    uint32_t c = 17;
    
    // Loop 1: Lower bound from masking, upper bound from OR
    for (uint32_t i = a & 0xFFF; i < (b | 0x7FF); i += c) {
        if (i % 3 == 0) {
            local_sum += i;
            update_checksum(i);
        }
    }
    
    // Nested loops with dependent bounds
    int32_t outer_limit = 50;
    for (int32_t j = -100; j < outer_limit; j += 7) {
        // Inner loop bound depends on outer index
        int32_t inner_start = j & 0x3F;  // 0-63
        int32_t inner_end = (j + 64) | 0x1F;  // Complex upper bound
        
        for (int32_t k = inner_start; k < inner_end && k < 100; k += 3) {
            if (k > 0) {
                local_sum += k * j;
                update_checksum(k);
            }
        }
    }
    
    // Loop with shifting bound
    uint64_t base = 0x1000;
    for (uint32_t i = 0; i < 32; i++) {
        uint64_t limit = base << i;
        if (limit > 0x1000000) limit = 0x1000000;
        
        for (uint32_t j = 0; j < (uint32_t)(limit >> 8); j += 256) {
            local_sum += j >> (i & 0x7);
            update_checksum(j);
        }
    }
    
    return local_sum;
}

// ==================== Test 3: Saturation Arithmetic ====================
NOINLINE static uint32_t test_saturation_arithmetic(void) {
    uint32_t local_sum = 0;
    
    // Manual saturation functions
    int32_t sat_add(int32_t a, int32_t b) {
        int32_t sum;
        if (__builtin_add_overflow(a, b, &sum)) {
            return (a > 0) ? INT32_MAX : INT32_MIN;
        }
        return sum;
    }
    
    int32_t sat_mul(int32_t a, int32_t b) {
        int32_t prod;
        if (__builtin_mul_overflow(a, b, &prod)) {
            if ((a > 0 && b > 0) || (a < 0 && b < 0)) {
                return INT32_MAX;
            } else {
                return INT32_MIN;
            }
        }
        return prod;
    }
    
    // Test saturation with boundary values
    int32_t test_vals[] = {INT32_MIN, INT32_MIN + 1, -100, 0, 100, INT32_MAX - 1, INT32_MAX};
    
    for (size_t i = 0; i < sizeof(test_vals)/sizeof(test_vals[0]); i++) {
        for (size_t j = 0; j < sizeof(test_vals)/sizeof(test_vals[0]); j++) {
            int32_t saturated = sat_add(test_vals[i], test_vals[j]);
            update_checksum(saturated);
            local_sum += saturated;
            
            saturated = sat_mul(test_vals[i], test_vals[j]);
            update_checksum(saturated);
            local_sum = local_sum * 7 + saturated;
        }
    }
    
    // Fixed-point arithmetic if available
    #ifdef __STDC_IEC_559__
    _Accum acc1 = 0.5k;
    _Accum acc2 = 0.25k;
    for (int i = 0; i < 10; i++) {
        _Accum result = acc1 + acc2;
        update_checksum(*(uint32_t*)&result);
        local_sum += *(uint32_t*)&result;
        acc1 *= 2.0k;
    }
    #endif
    
    return local_sum;
}

// ==================== Test 4: Bit-Field Ranges ====================
NOINLINE static uint32_t test_bitfield_ranges(void) {
    uint32_t local_sum = 0;
    
    // Struct with various bit-fields
    struct BitFieldStruct {
        unsigned int small : 3;    // 0-7
        signed int signed4 : 4;    // -8 to 7
        unsigned int medium : 10;  // 0-1023
        signed int signed10 : 10;  // -512 to 511
        unsigned int large : 31;   // 0-2147483647
    } bfs;
    
    // Union to test bit-field vs regular access
    union BitFieldUnion {
        struct BitFieldStruct bf;
        uint64_t raw;
    } u;
    
    memset(&bfs, 0, sizeof(bfs));
    
    // Assign boundary values to bit-fields
    bfs.small = 7;      // Max for 3 bits
    bfs.signed4 = -8;   // Min for signed 4 bits
    bfs.medium = 1023;  // Max for 10 bits
    bfs.signed10 = 511; // Max for signed 10 bits
    bfs.large = 0x7FFFFFFF; // Max for 31 bits
    
    // Check if values are within bit-field capacity
    if (bfs.small <= 7) {
        update_checksum(bfs.small);
        local_sum += bfs.small;
    }
    
    if (bfs.signed4 >= -8 && bfs.signed4 <= 7) {
        update_checksum(bfs.signed4);
        local_sum += bfs.signed4;
    }
    
    // Complex condition with bit-field comparison
    unsigned int external = 500;
    if (bfs.medium < external && bfs.medium > 100) {
        update_checksum(bfs.medium);
        local_sum += bfs.medium;
    }
    
    // Test overflow in bit-field assignment
    for (int i = 0; i < 20; i++) {
        bfs.small = i;  // Will wrap for i >= 8
        bfs.signed4 = i - 10;  // Some negative, some positive
        
        if (bfs.small < 8) {  // Condition depends on bit-field range
            update_checksum(bfs.small);
            local_sum += bfs.small;
        }
        
        update_checksum(bfs.signed4);
        local_sum = local_sum * 13 + bfs.signed4;
    }
    
    // Union access
    u.raw = 0x123456789ABCDEF0ULL;
    if (u.bf.small > 0 && u.bf.small < 8) {
        update_checksum(u.bf.small);
        local_sum ^= u.bf.small;
    }
    
    return local_sum;
}

// ==================== Test 5: Overflow Builtins ====================
NOINLINE static uint32_t test_overflow_builtins(void) {
    uint32_t local_sum = 0;
    
    // Test with partially known ranges
    int32_t x = 1000000;
    int32_t y = 2000000;
    
    // Range-restricting condition
    if (x > 0 && x < 2000000) {
        int32_t sum;
        if (!__builtin_add_overflow(x, y, &sum)) {
            update_checksum(sum);
            local_sum += sum;
        } else {
            update_checksum(0xDEAD);
            local_sum += 0xDEAD;
        }
    }
    
    // Loop with overflow checks
    int32_t multipliers[] = {1, 10, 100, 1000, 10000, 100000, 1000000};
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < sizeof(multipliers)/sizeof(multipliers[0]); j++) {
            int32_t result;
            if (__builtin_mul_overflow(i, multipliers[j], &result)) {
                // Overflow occurred
                update_checksum(0xFFFF0000 | (j << 8) | (i & 0xFF));
                local_sum += i * j;
            } else {
                update_checksum(result);
                local_sum += result;
            }
        }
    }
    
    // Chain of operations with overflow detection
    int64_t accum = 1;
    for (int i = 1; i < 30; i++) {
        int32_t truncated;
        if (!__builtin_mul_overflow((int32_t)accum, i, &truncated)) {
            accum = accum * i;
            update_checksum(truncated);
            local_sum += truncated;
        } else {
            // This should trigger range analysis for overflow detection
            update_checksum(0xBAADF00D);
            local_sum ^= 0xBAADF00D;
            break;
        }
    }
    
    // Unsigned overflow tests
    uint32_t u1 = UINT32_MAX - 100;
    uint32_t u2 = 200;
    uint32_t u_result;
    
    if (__builtin_uadd_overflow(u1, u2, &u_result)) {
        update_checksum(0xFFFFFFFF);
        local_sum += 0xFFFFFFFF;
    } else {
        update_checksum(u_result);
        local_sum += u_result;
    }
    
    return local_sum;
}

// ==================== Test 6: Edge Case Conditionals ====================
NOINLINE static uint32_t test_edge_conditionals(void) {
    uint32_t local_sum = 0;
    
    // Comparisons at extreme boundaries
    int32_t var = 0;
    for (int i = 0; i < 1000; i++) {
        var = (var * 1103515245 + 12345) & 0x7FFFFFFF;
        
        // These comparisons test the uncovered range comparison logic
        if (var > INT32_MAX - 100) {
            update_checksum(0x80000000 | var);
            local_sum += 1;
        }
        
        if (var < INT32_MIN + 100 && var > INT32_MIN) {
            update_checksum(0x40000000 | var);
            local_sum += 2;
        }
        
        // Modulo creates known range
        int32_t constrained = var % 1000;  // 0-999
        if (constrained > 900) {
            update_checksum(constrained);
            local_sum += constrained;
        }
    }
    
    // Power-of-two boundary checks
    uint32_t power = 1;
    for (int i = 0; i < 32; i++) {
        if (power > 0x40000000) {  // 2^30
            update_checksum(power);
            local_sum += power;
        }
        power <<= 1;
    }
    
    return local_sum;
}

// ==================== Main Function ====================
int main(void) {
    printf("Starting integer range analysis tests...\n");
    
    uint32_t sum = 0;
    
    sum += test_narrowing_conversions();
    printf("Test 1 completed, checksum: %u\n", g_checksum);
    
    sum += test_loop_range_analysis();
    printf("Test 2 completed, checksum: %u\n", g_checksum);
    
    sum += test_saturation_arithmetic();
    printf("Test 3 completed, checksum: %u\n", g_checksum);
    
    sum += test_bitfield_ranges();
    printf("Test 4 completed, checksum: %u\n", g_checksum);
    
    sum += test_overflow_builtins();
    printf("Test 5 completed, checksum: %u\n", g_checksum);
    
    sum += test_edge_conditionals();
    printf("Test 6 completed, checksum: %u\n", g_checksum);
    
    printf("All tests completed. Final sum: %u, Global checksum: %u\n", 
           sum, g_checksum);
    
    return (sum ^ g_checksum) != 0;
}
