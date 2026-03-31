#include <stdint.h>
#include <stdio.h>
#include <string.h>

// Prevent inlining for better coverage tracking
#define NOINLINE __attribute__((noinline))

// Fixed-point types if available
#ifdef __STDC_IEC_559__
#include <stdfix.h>
#endif

// Checksum/hash to prevent dead code elimination
static uint64_t checksum = 0x123456789ABCDEF0ULL;

NOINLINE static void update_checksum(uint64_t val) {
    checksum = (checksum << 13) | (checksum >> 51);
    checksum ^= val * 0x9E3779B97F4A7C15ULL;
}

// ==================== Test 1: Narrowing Conversions ====================
NOINLINE static uint64_t test_narrowing_conversions(void) {
    uint64_t local_sum = 0;
    
    // Constants at type boundaries
    int64_t wide_vals[] = {
        INT64_MAX,
        INT64_MIN,
        UINT32_MAX,
        (int64_t)UINT32_MAX + 1,
        (int64_t)INT32_MAX * 2,
        (int64_t)INT32_MIN * 2
    };
    
    for (size_t i = 0; i < sizeof(wide_vals)/sizeof(wide_vals[0]); i++) {
        // Narrowing conversions that require range analysis
        int32_t narrow1 = (int32_t)wide_vals[i];
        uint32_t narrow2 = (uint32_t)wide_vals[i];
        
        // Comparisons against boundaries
        if (narrow1 > INT32_MAX - 100) {
            local_sum += narrow1;
        }
        if (narrow2 < UINT32_MAX - 1000) {
            local_sum += narrow2;
        }
        
        // Shift operations that may overflow
        int32_t shifted = narrow1 << 3;
        if (shifted > 0 && shifted < INT32_MAX) {
            local_sum += shifted;
        }
    }
    
    // Complex narrowing with intermediate calculations
    uint64_t a = 0xFFFFFFFFULL;
    uint64_t b = 0x7FFFFFFFULL;
    uint64_t c = a + b;  // May overflow in 64-bit
    
    int32_t narrowed = (int32_t)(c >> 16);
    if (narrowed > 0 && narrowed < INT32_MAX) {
        local_sum += narrowed * 3;
    }
    
    update_checksum(local_sum);
    return local_sum;
}

// ==================== Test 2: Loop Range Analysis ====================
NOINLINE static uint64_t test_loop_range_analysis(void) {
    uint64_t local_sum = 0;
    
    // Outer loop with bitmasked bounds
    for (int32_t outer = 100; outer < 500; outer += 37) {
        // Create bounds using bitwise operations
        int32_t lower_bound = outer & 0x3FF;  // Mask to 10 bits
        int32_t upper_bound = (outer | 0x7FF) & 0xFFF;  // Complex bound
        
        // Inner loop with range-dependent bounds
        for (int32_t i = lower_bound; i < upper_bound; i += (outer & 0x1F) + 1) {
            // Complex condition involving bit operations
            if ((i ^ outer) < 1000 && (i & outer) > 100) {
                local_sum += i;
            }
            
            // Nested condition with shifting
            int32_t shifted = i << ((outer >> 2) & 0x7);
            if (shifted > 0 && shifted < (1 << 20)) {
                local_sum += shifted;
            }
        }
    }
    
    // Loop with wrap-around analysis
    uint32_t counter = 0;
    for (int i = 0; i < 1000; i++) {
        counter = (counter * 1103515245 + 12345) & 0x7FFFFFFF;
        uint16_t truncated = counter & 0xFFFF;
        
        // Condition at boundary
        if (truncated > 0xFF00 && truncated < 0xFFFF) {
            local_sum += truncated;
        }
    }
    
    update_checksum(local_sum);
    return local_sum;
}

// ==================== Test 3: Saturation Arithmetic ====================
NOINLINE static uint64_t test_saturation_arithmetic(void) {
    uint64_t local_sum = 0;
    
    // Manual saturation implementation
    int32_t saturating_add(int32_t a, int32_t b) {
        int64_t result = (int64_t)a + (int64_t)b;
        
        // Boundary checks that should trigger the uncovered code
        if (result > INT32_MAX) {
            return INT32_MAX;
        }
        if (result < INT32_MIN) {
            return INT32_MIN;
        }
        return (int32_t)result;
    }
    
    // Test values near boundaries
    int32_t test_cases[][2] = {
        {INT32_MAX - 100, 200},
        {INT32_MIN + 100, -200},
        {1000, 2000},
        {-1000, -2000},
        {INT32_MAX / 2, INT32_MAX / 2 + 1}
    };
    
    for (size_t i = 0; i < sizeof(test_cases)/sizeof(test_cases[0]); i++) {
        int32_t result = saturating_add(test_cases[i][0], test_cases[i][1]);
        local_sum += result;
        
        // Additional boundary check
        if (result == INT32_MAX || result == INT32_MIN) {
            local_sum += i * 1000;
        }
    }
    
#ifdef __STDC_IEC_559__
    // Fixed-point arithmetic if supported
    _Accum acc = 0.5k;
    for (int i = 0; i < 100; i++) {
        acc = acc * 1.1k;
        // Comparisons that may trigger fixed-point range analysis
        if (acc > 0.8k && acc < 1.2k) {
            local_sum += i;
        }
    }
#endif
    
    update_checksum(local_sum);
    return local_sum;
}

// ==================== Test 4: Bit-Field Ranges ====================
NOINLINE static uint64_t test_bitfield_ranges(void) {
    uint64_t local_sum = 0;
    
    // Struct with various bit-fields
    struct BitFieldStruct {
        unsigned int small : 3;    // 0-7
        signed int signed4 : 4;    // -8 to 7
        unsigned int medium : 10;  // 0-1023
        signed int signed10 : 10;  // -512 to 511
        unsigned int large : 31;   // 0-2147483647
    } bfs;
    
    // Test assignments at boundaries
    bfs.small = 7;  // Max for 3 bits
    bfs.signed4 = -8;  // Min for signed 4 bits
    bfs.medium = 1023;  // Max for 10 bits
    bfs.signed10 = 511;  // Max for signed 10 bits
    bfs.large = 0x3FFFFFFF;  // Large value for 31 bits
    
    // Comparisons that should trigger range analysis
    if (bfs.small == 7) {
        local_sum += bfs.small;
    }
    
    if (bfs.signed4 < 0 && bfs.signed4 >= -8) {
        local_sum += -bfs.signed4;
    }
    
    if (bfs.medium > 1000 && bfs.medium <= 1023) {
        local_sum += bfs.medium;
    }
    
    // Union with overlapping bit-fields
    union OverlapUnion {
        struct {
            unsigned int lower16 : 16;
            unsigned int upper16 : 16;
        } parts;
        uint32_t full;
    } ou;
    
    ou.full = 0x87654321;
    
    // Range checks on overlapping fields
    if (ou.parts.lower16 < 0x8000 && ou.parts.upper16 > 0x8000) {
        local_sum += ou.parts.lower16 + ou.parts.upper16;
    }
    
    update_checksum(local_sum);
    return local_sum;
}

// ==================== Test 5: Overflow Builtins ====================
NOINLINE static uint64_t test_overflow_builtins(void) {
    uint64_t local_sum = 0;
    
    // Test cases for overflow detection
    int32_t overflow_test(int32_t a, int32_t b) {
        int32_t result;
        
        // Use builtin overflow checks
        if (__builtin_add_overflow(a, b, &result)) {
            return (a > 0) ? INT32_MAX : INT32_MIN;
        }
        
        // Additional multiplication with overflow check
        int32_t mul_result;
        if (__builtin_mul_overflow(result, 2, &mul_result)) {
            return (result > 0) ? INT32_MAX : INT32_MIN;
        }
        
        return mul_result;
    }
    
    // Values designed to trigger and not trigger overflows
    struct {
        int32_t a, b;
    } tests[] = {
        {INT32_MAX - 10, 20},
        {INT32_MIN + 10, -20},
        {1000, 2000},
        {INT32_MAX / 2, 2},
        {INT32_MAX / 2, 3}
    };
    
    for (size_t i = 0; i < sizeof(tests)/sizeof(tests[0]); i++) {
        int32_t res = overflow_test(tests[i].a, tests[i].b);
        local_sum += res;
        
        // Conditional based on overflow result
        if (res == INT32_MAX || res == INT32_MIN) {
            local_sum += i * 10000;
        }
    }
    
    // Loop with accumulated overflow checks
    int32_t accumulator = 0;
    for (int i = 0; i < 100; i++) {
        int32_t old_acc = accumulator;
        if (__builtin_add_overflow(accumulator, i * 100000, &accumulator)) {
            accumulator = (old_acc > 0) ? INT32_MAX : INT32_MIN;
            break;
        }
        
        // Range check after potential overflow
        if (accumulator > 0 && accumulator < INT32_MAX / 2) {
            local_sum += accumulator;
        }
    }
    
    update_checksum(local_sum);
    return local_sum;
}

// ==================== Main Function ====================
int main(void) {
    uint64_t total = 0;
    
    printf("Starting integer range analysis tests...\n");
    
    total += test_narrowing_conversions();
    printf("Test 1 complete\n");
    
    total += test_loop_range_analysis();
    printf("Test 2 complete\n");
    
    total += test_saturation_arithmetic();
    printf("Test 3 complete\n");
    
    total += test_bitfield_ranges();
    printf("Test 4 complete\n");
    
    total += test_overflow_builtins();
    printf("Test 5 complete\n");
    
    // Final checksum to ensure all code affects output
    printf("Final checksum: 0x%016llX\n", (unsigned long long)checksum);
    printf("Total sum: %llu\n", (unsigned long long)total);
    
    return (total > 0) ? 0 : 1;
}
