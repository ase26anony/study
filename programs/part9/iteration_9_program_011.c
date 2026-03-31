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
    
    // Wide to narrow with boundary values
    int64_t wide_vals[] = {
        INT64_MAX,
        INT64_MIN,
        (int64_t)INT32_MAX + 1,
        (int64_t)INT32_MIN - 1,
        0x7FFFFFFF00000000LL,
        0x8000000000000000LL
    };
    
    for (size_t i = 0; i < sizeof(wide_vals)/sizeof(wide_vals[0]); i++) {
        // Narrowing conversions that require range analysis
        int32_t narrow1 = (int32_t)wide_vals[i];
        uint32_t narrow2 = (uint32_t)wide_vals[i];
        int16_t narrow3 = (int16_t)wide_vals[i];
        
        hash = mix(hash ^ (uint32_t)narrow1);
        hash = mix(hash ^ narrow2);
        hash = mix(hash ^ (uint32_t)narrow3);
        
        // Comparisons at type boundaries
        if (wide_vals[i] > INT32_MAX) {
            hash = mix(hash ^ 0x11111111);
        }
        if (wide_vals[i] < INT32_MIN) {
            hash = mix(hash ^ 0x22222222);
        }
        if ((uint64_t)wide_vals[i] > UINT32_MAX) {
            hash = mix(hash ^ 0x33333333);
        }
    }
    
    // Shifts that may overflow
    uint64_t x = 0xFFFFFFFFULL;
    for (int shift = 0; shift < 64; shift += 8) {
        uint64_t shifted = x << shift;
        uint32_t narrowed = (uint32_t)shifted;
        hash = mix(hash ^ narrowed);
        
        // Boundary comparison
        if (shifted > UINT32_MAX) {
            hash = mix(hash ^ (0x44444444 + shift));
        }
    }
    
    return hash;
}

// ==================== Test 2: Loop Range Analysis ====================
NOINLINE uint32_t test_loop_range_analysis(void) {
    uint32_t hash = 0x87654321;
    
    // Complex loop bounds with bitwise operations
    int32_t a = 0x12345678;
    int32_t b = 0x9ABCDEF0;
    int32_t c = 17;
    
    // Outer loop with bitmasked start
    for (int32_t i = a & 0x0000FFFF; i < (b | 0x7FFF0000); i += c) {
        if (i > 0x7FFFFFFF - 1000) {
            hash = mix(hash ^ (uint32_t)i);
            break; // Prevent infinite loops
        }
        
        // Inner loop with dependent bounds
        for (int32_t j = (i ^ 0xFF) & 0xFFF; 
             j < ((i >> 4) | 0x7FF); 
             j += (c & 0x3F)) {
            hash = mix(hash ^ (uint32_t)j);
            if (j > 5000) break;
        }
        
        if (i > 10000) break;
    }
    
    // Nested loops with XOR-based bounds
    uint32_t mask = 0x00FFFFFF;
    for (uint32_t outer = 0x1000; outer < 0x5000; outer += 0x100) {
        uint32_t start = outer & mask;
        uint32_t end = (outer ^ 0x3333) | 0x8000;
        
        for (uint32_t inner = start; inner < end; inner += 0x77) {
            hash = mix(hash ^ inner);
            // Complex condition involving bitwise ops
            if ((inner & 0xF00) == (outer & 0xF00)) {
                hash = mix(hash ^ 0x55555555);
            }
            if (inner > 0x10000) break;
        }
    }
    
    return hash;
}

// ==================== Test 3: Saturation Arithmetic ====================
NOINLINE uint32_t test_saturation_arithmetic(void) {
    uint32_t hash = 0xABCD1234;
    
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
    
    // Test with boundary values
    int32_t test_cases[][2] = {
        {INT32_MAX, 1},
        {INT32_MIN, -1},
        {INT32_MAX / 2, 2},
        {INT32_MIN / 2, 2},
        {1000, 1000},
        {-1000, -1000}
    };
    
    for (size_t i = 0; i < sizeof(test_cases)/sizeof(test_cases[0]); i++) {
        int32_t sum = sat_add(test_cases[i][0], test_cases[i][1]);
        int32_t prod = sat_mul(test_cases[i][0], test_cases[i][1]);
        
        hash = mix(hash ^ (uint32_t)sum);
        hash = mix(hash ^ (uint32_t)prod);
        
        // Direct boundary comparisons
        if (test_cases[i][0] > INT32_MAX - 100) {
            hash = mix(hash ^ 0x66666666);
        }
        if (test_cases[i][0] < INT32_MIN + 100) {
            hash = mix(hash ^ 0x77777777);
        }
    }
    
#ifdef __STDC_IEC_559__
    // Fixed-point arithmetic if supported
    _Accum acc = 0.5k;
    for (int i = 0; i < 10; i++) {
        acc = acc * 2.0k;
        // Comparisons that may trigger fixed-point range analysis
        if (acc > 10.0k) {
            hash = mix(hash ^ 0x88888888);
        }
    }
#endif
    
    return hash;
}

// ==================== Test 4: Bit-Field Ranges ====================
NOINLINE uint32_t test_bitfield_ranges(void) {
    uint32_t hash = 0xDEADBEEF;
    
    // Struct with various bit-fields
    struct BitFieldStruct {
        unsigned int small : 4;    // 0-15
        signed int signed5 : 5;    // -16 to 15
        unsigned int medium : 10;  // 0-1023
        signed int signed12 : 12;  // -2048 to 2047
        unsigned int large : 31;   // 0-2^31-1
    } bfs;
    
    // Union to test overlapping ranges
    union BitUnion {
        struct {
            unsigned int low16 : 16;
            unsigned int high16 : 16;
        } parts;
        uint32_t full;
    } bu;
    
    // Test assignments to bit-fields
    unsigned int test_values[] = {0, 15, 16, 255, 1023, 1024, 0x7FFFFFFF};
    
    for (size_t i = 0; i < sizeof(test_values)/sizeof(test_values[0]); i++) {
        bfs.small = test_values[i] & 0xF;
        bfs.medium = test_values[i] & 0x3FF;
        bfs.large = test_values[i] & 0x7FFFFFFF;
        
        // Comparisons against bit-field capacity
        if (bfs.small == 15) {  // Max for 4-bit unsigned
            hash = mix(hash ^ 0x99999999);
        }
        if (bfs.medium > 1000) {
            hash = mix(hash ^ 0xAAAAAAAA);
        }
        if (bfs.large < 100) {
            hash = mix(hash ^ 0xBBBBBBBB);
        }
        
        // Test signed bit-fields
        bfs.signed5 = (test_values[i] & 0x1F) - 16;  // Range -16..15
        bfs.signed12 = (test_values[i] & 0xFFF) - 2048; // Range -2048..2047
        
        if (bfs.signed5 == -16) {  // Min for 5-bit signed
            hash = mix(hash ^ 0xCCCCCCCC);
        }
        if (bfs.signed12 == 2047) { // Max for 12-bit signed
            hash = mix(hash ^ 0xDDDDDDDD);
        }
    }
    
    // Test union bit-field ranges
    bu.full = 0x12345678;
    if (bu.parts.low16 == 0x5678) {
        hash = mix(hash ^ 0xEEEEEEEE);
    }
    if (bu.parts.high16 > 0x1234) {
        hash = mix(hash ^ 0xFFFFFFFF);
    }
    
    return hash;
}

// ==================== Test 5: Overflow Builtins ====================
NOINLINE uint32_t test_overflow_builtins(void) {
    uint32_t hash = 0xCAFEBABE;
    
    // Use overflow builtins with range-constrained values
    int32_t vals[] = {100, 1000, 10000, INT32_MAX/2, INT32_MAX-10};
    
    for (size_t i = 0; i < sizeof(vals)/sizeof(vals[0]); i++) {
        int32_t result;
        int overflow;
        
        // Test addition overflow
        overflow = __builtin_add_overflow(vals[i], vals[(i+1)%5], &result);
        hash = mix(hash ^ (uint32_t)result);
        if (overflow) {
            hash = mix(hash ^ 0x12345678);
        }
        
        // Test multiplication overflow
        overflow = __builtin_mul_overflow(vals[i], 2, &result);
        hash = mix(hash ^ (uint32_t)result);
        if (overflow) {
            hash = mix(hash ^ 0x87654321);
        }
        
        // Test subtraction overflow
        overflow = __builtin_sub_overflow(vals[i], INT32_MIN/2, &result);
        hash = mix(hash ^ (uint32_t)result);
        if (overflow) {
            hash = mix(hash ^ 0x13579BDF);
        }
    }
    
    // Nested conditions with range analysis
    int32_t x = 1000;
    int32_t y = 2000;
    
    for (int i = 0; i < 10; i++) {
        // Constrain ranges through conditions
        if (x > 500 && x < 1500) {
            int32_t sum;
            if (!__builtin_add_overflow(x, y, &sum)) {
                hash = mix(hash ^ (uint32_t)sum);
            }
        }
        
        // Further constrain x
        if (x < 800) {
            x = 1200;  // Jump to different range
        } else {
            x += 100;
        }
        
        // Modulo operation creates known range
        y = (y * 3) % 10000;
    }
    
    return hash;
}

// ==================== Main Function ====================
int main(void) {
    uint32_t final_hash = 0;
    
    printf("Running integer range analysis tests...\n");
    
    final_hash ^= test_narrowing_conversions();
    final_hash ^= test_loop_range_analysis();
    final_hash ^= test_saturation_arithmetic();
    final_hash ^= test_bitfield_ranges();
    final_hash ^= test_overflow_builtins();
    
    printf("Final hash: 0x%08X\n", final_hash);
    
    // Use result to prevent dead code elimination
    volatile uint32_t sink = final_hash;
    (void)sink;
    
    return 0;
}
