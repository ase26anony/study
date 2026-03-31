#include <stdint.h>
#include <stdio.h>
#include <string.h>

// Prevent inlining to keep function boundaries clear for coverage
#define NOINLINE __attribute__((noinline))

// Fixed-point types if available
#ifdef __STDC_IEC_559__
#include <stdfix.h>
#endif

// Checksum/hash function to prevent dead code elimination
static inline uint32_t mix(uint32_t h) {
    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;
    return h;
}

// ==================== Test 1: Narrowing Conversions ====================
NOINLINE uint32_t test_narrowing_conversions(void) {
    uint32_t hash = 0x12345678;
    
    // Wide types with values at boundaries
    int64_t wide_vals[] = {
        INT64_MIN, INT64_MIN + 1,
        INT32_MIN - 1LL, INT32_MIN, INT32_MIN + 1,
        -100, 0, 100,
        INT32_MAX - 1, INT32_MAX, INT32_MAX + 1LL,
        INT64_MAX - 1, INT64_MAX
    };
    
    for (size_t i = 0; i < sizeof(wide_vals)/sizeof(wide_vals[0]); i++) {
        // Narrowing conversions that require range checking
        int32_t narrow1 = (int32_t)wide_vals[i];  // Explicit cast
        int32_t narrow2 = wide_vals[i];           // Implicit conversion
        
        // Arithmetic that might overflow after narrowing
        int32_t result1 = narrow1 * 2;
        int32_t result2 = narrow1 + 1000000;
        
        // Comparisons against boundaries
        int in_range = (wide_vals[i] >= INT32_MIN && wide_vals[i] <= INT32_MAX);
        int would_overflow = (result1 < INT32_MIN/2 || result1 > INT32_MAX/2);
        
        hash = mix(hash ^ (uint32_t)narrow1);
        hash = mix(hash ^ (uint32_t)narrow2);
        hash = mix(hash ^ (uint32_t)in_range);
        hash = mix(hash ^ (uint32_t)would_overflow);
    }
    
    // Shifts that may overflow
    uint64_t large_shifts[] = {1ULL << 31, 1ULL << 32, 1ULL << 33, 1ULL << 63};
    for (size_t i = 0; i < sizeof(large_shifts)/sizeof(large_shifts[0]); i++) {
        uint32_t shifted = (uint32_t)(large_shifts[i] >> 16);
        uint32_t shifted2 = (uint32_t)(large_shifts[i] << 1);
        
        hash = mix(hash ^ shifted);
        hash = mix(hash ^ shifted2);
    }
    
    return hash;
}

// ==================== Test 2: Loop Range Analysis ====================
NOINLINE uint32_t test_loop_range_analysis(void) {
    uint32_t hash = 0x9ABCDEF0;
    
    // Complex loop bounds with bitwise operations
    int32_t a = 0x1234;
    int32_t b = 0x5678;
    int32_t c = 17;
    
    // Loop with bounds derived from bitwise ops
    for (int32_t i = a & 0xFFF; i < (b | 0x7FF); i += c) {
        // Nested loop with dependent bounds
        for (int32_t j = (i & 0xF) * 2; j < 100; j += (i % 7) + 1) {
            // Bitwise condition that affects range
            if ((j ^ i) & 0x80) {
                hash = mix(hash ^ (uint32_t)j);
            } else {
                hash = mix(hash ^ (uint32_t)(i << 2));
            }
        }
        
        // Additional range-restricting condition
        if (i > 0x800) {
            for (int32_t k = i - 0x800; k < i; k += 3) {
                hash = mix(hash ^ (uint32_t)k);
            }
        }
    }
    
    // Loop with modulo-based bounds
    uint32_t start = 0xFFFFFF00;  // Near overflow boundary
    for (uint32_t i = start; i < start + 512; i++) {
        // Complex condition involving range analysis
        if (i > UINT32_MAX - 256 && (i & 0xFF) < 128) {
            hash = mix(hash ^ i);
        }
    }
    
    // Loop with signed/unsigned mixing
    int32_t signed_val = -100;
    uint32_t unsigned_val = 1000;
    for (int i = signed_val; i < (int)unsigned_val; i += 30) {
        // Comparison that requires understanding of both ranges
        if (i > INT32_MIN + 50 && i < INT32_MAX - 50) {
            hash = mix(hash ^ (uint32_t)i);
        }
    }
    
    return hash;
}

// ==================== Test 3: Saturation Arithmetic ====================
NOINLINE uint32_t test_saturation_arithmetic(void) {
    uint32_t hash = 0x13579BDF;
    
    // Manual saturation arithmetic
    int32_t sat_add(int32_t a, int32_t b) {
        int64_t result = (int64_t)a + (int64_t)b;
        if (result > INT32_MAX) return INT32_MAX;
        if (result < INT32_MIN) return INT32_MIN;
        return (int32_t)result;
    }
    
    int32_t sat_mul(int32_t a, int32_t b) {
        int64_t result = (int64_t)a * (int64_t)b;
        if (result > INT32_MAX) return INT32_MAX;
        if (result < INT32_MIN) return INT32_MIN;
        return (int32_t)result;
    }
    
    // Test values at boundaries
    int32_t test_cases[][2] = {
        {INT32_MIN, -1},
        {INT32_MIN, 0},
        {INT32_MIN, 1},
        {INT32_MAX, -1},
        {INT32_MAX, 0},
        {INT32_MAX, 1},
        {1000, 2000},
        {-1000, -2000},
        {1000000, 1000000}
    };
    
    for (size_t i = 0; i < sizeof(test_cases)/sizeof(test_cases[0]); i++) {
        int32_t a = test_cases[i][0];
        int32_t b = test_cases[i][1];
        
        int32_t sum = sat_add(a, b);
        int32_t prod = sat_mul(a, b);
        
        // Also test regular arithmetic for comparison
        int32_t reg_sum = a + b;
        int32_t reg_prod = a * b;
        
        hash = mix(hash ^ (uint32_t)sum);
        hash = mix(hash ^ (uint32_t)prod);
        hash = mix(hash ^ (uint32_t)reg_sum);
        hash = mix(hash ^ (uint32_t)reg_prod);
    }
    
    // Fixed-point arithmetic if available
    #ifdef __STDC_IEC_559__
    _Accum acc1 = 0.5k;
    _Accum acc2 = 0.75k;
    _Accum acc_sum = acc1 + acc2;
    _Accum acc_prod = acc1 * acc2;
    
    hash = mix(hash ^ *(uint32_t*)&acc_sum);
    hash = mix(hash ^ *(uint32_t*)&acc_prod);
    #endif
    
    return hash;
}

// ==================== Test 4: Bitfield Ranges ====================
NOINLINE uint32_t test_bitfield_ranges(void) {
    uint32_t hash = 0x2468ACE0;
    
    // Struct with various bitfields
    struct BitfieldStruct {
        signed int small_signed : 4;    // Range: -8 to 7
        unsigned int small_unsigned : 5; // Range: 0 to 31
        signed int medium_signed : 10;  // Range: -512 to 511
        unsigned int large_unsigned : 31; // Range: 0 to 2^31-1
    } bs;
    
    // Union to test bitfield storage
    union BitfieldUnion {
        struct BitfieldStruct fields;
        uint64_t raw;
    } bu;
    
    // Test assignments at and beyond bitfield boundaries
    int test_values[] = {-10, -8, -7, -1, 0, 1, 7, 8, 15, 31, 32, 100, -513, -512, -511, 511, 512};
    
    for (size_t i = 0; i < sizeof(test_values)/sizeof(test_values[0]); i++) {
        int val = test_values[i];
        
        // Assign to bitfields (will be truncated)
        bs.small_signed = val;
        bs.small_unsigned = val;
        bs.medium_signed = val;
        bs.large_unsigned = val;
        
        // Check if values fit in their respective ranges
        int fits_small_signed = (val >= -8 && val <= 7);
        int fits_small_unsigned = (val >= 0 && val <= 31);
        int fits_medium_signed = (val >= -512 && val <= 511);
        int fits_large_unsigned = (val >= 0 && val <= 0x7FFFFFFF);
        
        // Comparisons that require understanding bitfield ranges
        if (bs.small_signed > 3 || bs.small_unsigned < 10) {
            hash = mix(hash ^ (uint32_t)bs.small_signed);
        }
        
        if (bs.medium_signed != val) {  // Always true if val out of range
            hash = mix(hash ^ (uint32_t)bs.medium_signed);
        }
        
        hash = mix(hash ^ (uint32_t)fits_small_signed);
        hash = mix(hash ^ (uint32_t)fits_small_unsigned);
        hash = mix(hash ^ (uint32_t)fits_medium_signed);
        hash = mix(hash ^ (uint32_t)fits_large_unsigned);
    }
    
    return hash;
}

// ==================== Test 5: Overflow Builtins ====================
NOINLINE uint32_t test_overflow_builtins(void) {
    uint32_t hash = 0xFEDCBA98;
    
    // Test overflow builtins with partially known ranges
    int32_t vals[] = {1, 100, 10000, INT32_MAX/2, INT32_MAX-10, INT32_MAX};
    
    for (size_t i = 0; i < sizeof(vals)/sizeof(vals[0]); i++) {
        for (size_t j = 0; j < sizeof(vals)/sizeof(vals[0]); j++) {
            int32_t a = vals[i];
            int32_t b = vals[j];
            int32_t result;
            int overflow;
            
            // Addition with overflow check
            overflow = __builtin_add_overflow(a, b, &result);
            hash = mix(hash ^ (uint32_t)result);
            hash = mix(hash ^ (uint32_t)overflow);
            
            // Multiplication with overflow check
            overflow = __builtin_mul_overflow(a, b, &result);
            hash = mix(hash ^ (uint32_t)result);
            hash = mix(hash ^ (uint32_t)overflow);
            
            // Subtraction with overflow check
            overflow = __builtin_sub_overflow(a, b, &result);
            hash = mix(hash ^ (uint32_t)result);
            hash = mix(hash ^ (uint32_t)overflow);
        }
    }
    
    // Use builtins in conditional contexts with range restrictions
    int32_t x = 1000;
    int32_t y = 2000;
    
    // First restrict ranges with conditions
    if (x > 500 && x < 1500) {
        if (y > 1000 && y < 3000) {
            int32_t sum;
            if (!__builtin_add_overflow(x, y, &sum)) {
                // Further operations on sum
                int32_t doubled;
                if (!__builtin_mul_overflow(sum, 2, &doubled)) {
                    hash = mix(hash ^ (uint32_t)doubled);
                }
            }
        }
    }
    
    // Test with edge cases
    int32_t edge_cases[] = {INT32_MIN, INT32_MIN+1, -1, 0, 1, INT32_MAX-1, INT32_MAX};
    for (size_t i = 0; i < sizeof(edge_cases)/sizeof(edge_cases[0]); i++) {
        int32_t a = edge_cases[i];
        int32_t b = edge_cases[(i+1) % (sizeof(edge_cases)/sizeof(edge_cases[0]))];
        int32_t res;
        int ovf;
        
        ovf = __builtin_add_overflow(a, b, &res);
        hash = mix(hash ^ (uint32_t)(ovf ? 0xDEAD : res));
        
        ovf = __builtin_sub_overflow(a, b, &res);
        hash = mix(hash ^ (uint32_t)(ovf ? 0xBEEF : res));
    }
    
    return hash;
}

// ==================== Main Function ====================
int main(void) {
    uint32_t final_hash = 0;
    
    printf("Running integer range analysis tests...\n");
    
    final_hash ^= test_narrowing_conversions();
    printf("Test 1 complete\n");
    
    final_hash ^= test_loop_range_analysis();
    printf("Test 2 complete\n");
    
    final_hash ^= test_saturation_arithmetic();
    printf("Test 3 complete\n");
    
    final_hash ^= test_bitfield_ranges();
    printf("Test 4 complete\n");
    
    final_hash ^= test_overflow_builtins();
    printf("Test 5 complete\n");
    
    printf("Final hash: 0x%08X\n", final_hash);
    
    // Use final_hash to prevent dead code elimination
    volatile uint32_t sink = final_hash;
    (void)sink;
    
    return 0;
}
