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
        (int64_t)UINT32_MAX + 1,
        (int64_t)INT32_MAX,
        (int64_t)INT32_MIN,
        0x7FFFFFFF00000000LL,
        -0x7FFFFFFF00000000LL
    };
    
    for (size_t i = 0; i < sizeof(wide_vals)/sizeof(wide_vals[0]); i++) {
        // These conversions require range analysis
        int32_t narrow1 = (int32_t)wide_vals[i];
        uint32_t narrow2 = (uint32_t)wide_vals[i];
        
        // Mix results into hash
        hash = (hash << 5) ^ (hash >> 27) ^ (uint32_t)narrow1;
        hash = (hash << 3) ^ (hash >> 29) ^ narrow2;
    }
    
    // Test 2: Complex narrowing with arithmetic
    int64_t a = 0x123456789ABCDEF0LL;
    int64_t b = 0xFEDCBA9876543210LL;
    
    for (int i = 0; i < 100; i++) {
        int64_t product = a * b;
        int32_t truncated = (int32_t)(product >> (i % 32));
        uint32_t unsigned_trunc = (uint32_t)((uint64_t)product >> (i % 32));
        
        hash ^= (uint32_t)truncated + unsigned_trunc;
        a = (a << 1) | (a >> 63);  // Rotate
        b = (b >> 1) | (b << 63);
    }
    
    sink = hash;
    return hash;
}

__attribute__((noinline,noipa))
uint32_t test_loop_range_analysis(void) {
    uint32_t hash = 0xDEADBEEF;
    
    // Complex loop bounds with bitwise operations
    uint32_t mask1 = 0x00000FFF;
    uint32_t mask2 = 0x000007FF;
    uint32_t mask3 = 0xFFFFF000;
    
    // Outer loop with bitwise-derived bounds
    for (uint32_t outer = mask1 & 0xABCD; 
         outer < (mask2 | 0x7000); 
         outer += (outer & 0x3F) + 1) {
        
        // Inner loop with dependent bounds
        for (int32_t inner = (int32_t)(outer ^ mask3);
             inner < (int32_t)((outer & mask2) << 3);
             inner += (inner & 0x1F) + 1) {
            
            // Complex condition that requires range analysis
            if ((inner > (int32_t)(mask1 & 0x7FF)) && 
                (inner < (int32_t)((mask2 | 0x800) << 1))) {
                hash = (hash * 16777619) ^ (uint32_t)inner;
            }
            
            // Additional bitwise condition
            if (((inner & 0xFFF) == 0) || ((inner | 0x800) > 0x1000)) {
                hash ^= outer << 16;
            }
        }
        
        // Nested loop with shifting bounds
        for (int64_t j = (int64_t)outer - 0x1000; 
             j < (int64_t)outer + 0x1000; 
             j += (j & 0x3FF) + 1) {
            
            // Range comparison at type boundaries
            if (j > INT32_MAX - 1000 || j < INT32_MIN + 1000) {
                hash ^= (uint32_t)j * 0x9E3779B9;
            }
        }
    }
    
    // Loop with modulo-based bounds
    uint32_t base = 0x12345678;
    for (int32_t i = base % 1000; i < (int32_t)((base | 0x3FF) % 2000); i += 17) {
        // Condition that tests boundary analysis
        if (i > (INT32_MAX >> 2) && i < (INT32_MAX - (INT32_MAX >> 3))) {
            hash += i * 3;
        }
    }
    
    sink = hash;
    return hash;
}

__attribute__((noinline,noipa))
uint32_t test_saturation_arithmetic(void) {
    uint32_t hash = 0;
    
    // Manual saturation arithmetic
    int32_t sat_min = -1000;
    int32_t sat_max = 1000;
    
    int32_t test_vals[] = {
        INT32_MAX, INT32_MIN, 0, 500, -500,
        1500, -1500, 999, -999, 1001, -1001
    };
    
    for (size_t i = 0; i < sizeof(test_vals)/sizeof(test_vals[0]); i++) {
        int32_t val = test_vals[i];
        
        // Saturation logic - exactly the kind of boundary checks
        // that trigger the uncovered code
        int32_t saturated;
        if (val > sat_max) {
            saturated = sat_max;
        } else if (val < sat_min) {
            saturated = sat_min;
        } else {
            saturated = val;
        }
        
        // Additional saturation with different bounds
        uint32_t usat_max = 0xFFFF;
        uint32_t uval = (uint32_t)val;
        uint32_t usaturated = (uval > usat_max) ? usat_max : uval;
        
        hash = (hash << 7) ^ (hash >> 25) ^ (uint32_t)saturated;
        hash ^= usaturated * 0xCC9E2D51;
    }
    
    // Complex saturation with shifting bounds
    for (int64_t i = -10000; i <= 10000; i += 777) {
        int32_t bound1 = (int32_t)(i & 0x7FF) - 0x400;
        int32_t bound2 = (int32_t)((i >> 8) & 0x3FF) + 0x200;
        
        // Nested saturation checks
        int32_t temp = (int32_t)i;
        if (temp > bound2) temp = bound2;
        if (temp < bound1) temp = bound1;
        
        // Check if value is at boundary
        if (temp == bound1 || temp == bound2) {
            hash ^= (uint32_t)temp << 16;
        }
        
        hash += (uint32_t)temp;
    }
    
    sink = hash;
    return hash;
}

// Struct with bit-fields for range analysis
struct bitfield_struct {
    signed int bf1 : 7;    // -64 to 63
    unsigned int bf2 : 9;   // 0 to 511
    signed int bf3 : 12;   // -2048 to 2047
    unsigned int bf4 : 5;   // 0 to 31
};

__attribute__((noinline,noipa))
uint32_t test_bitfield_ranges(void) {
    uint32_t hash = 0xCAFEBABE;
    
    struct bitfield_struct bs;
    
    // Test assignments at and beyond bit-field boundaries
    int test_values[] = {
        0, 63, 64, -64, -65, 127, -128,
        511, 512, 1023, 2047, 2048, -2048, -2049,
        31, 32, 100, -100
    };
    
    for (size_t i = 0; i < sizeof(test_values)/sizeof(test_values[0]); i++) {
        int val = test_values[i];
        
        // Assign to bit-fields (implicit truncation)
        bs.bf1 = val;  // Will be truncated to 7 bits
        bs.bf2 = val;  // Will be truncated to 9 bits
        bs.bf3 = val;  // Will be truncated to 12 bits
        bs.bf4 = val;  // Will be truncated to 5 bits
        
        // Conditions that test bit-field ranges
        if (bs.bf1 == 63 || bs.bf1 == -64) {
            hash ^= 0x11111111;
        }
        if (bs.bf2 == 511 || bs.bf2 == 0) {
            hash ^= 0x22222222;
        }
        if (bs.bf3 > 1000 || bs.bf3 < -1000) {
            hash ^= 0x33333333;
        }
        if (bs.bf4 == 31) {
            hash ^= 0x44444444;
        }
        
        // Complex condition with bit-field ranges
        if ((bs.bf1 + bs.bf3) > 100 || (bs.bf2 - bs.bf4) < 50) {
            hash += val;
        }
    }
    
    // Union with bit-fields for additional testing
    union {
        uint32_t full;
        struct {
            signed int x : 10;
            unsigned int y : 10;
            signed int z : 10;
        } parts;
    } u;
    
    for (uint32_t i = 0; i < 1000; i += 37) {
        u.full = i * 0x9E3779B9;
        
        // Range checks on bit-field union members
        if (u.parts.x == 511 || u.parts.x == -512) {
            hash ^= u.full;
        }
        if (u.parts.y == 1023) {
            hash += u.full;
        }
        if (u.parts.z > 0 && u.parts.z < 100) {
            hash = (hash << 3) ^ u.full;
        }
    }
    
    sink = hash;
    return hash;
}

__attribute__((noinline,noipa))
uint32_t test_overflow_builtins(void) {
    uint32_t hash = 0;
    
    // Test overflow builtins with various ranges
    int32_t a = 1000;
    int32_t b = 2000;
    int32_t result;
    
    // Simple overflow checks
    for (int i = 0; i < 100; i++) {
        if (__builtin_add_overflow(a, b, &result)) {
            hash ^= 0xAAAAAAAA;
        } else {
            hash += result;
        }
        
        if (__builtin_mul_overflow(a, b, &result)) {
            hash ^= 0xBBBBBBBB;
        } else {
            hash += result * 3;
        }
        
        // Vary values
        a += i * 100;
        b -= i * 50;
    }
    
    // Test with values near boundaries
    int32_t boundary_vals[] = {
        INT32_MAX, INT32_MIN, INT32_MAX - 100,
        INT32_MIN + 100, 0, 1, -1
    };
    
    for (size_t i = 0; i < sizeof(boundary_vals)/sizeof(boundary_vals[0]); i++) {
        for (size_t j = 0; j < sizeof(boundary_vals)/sizeof(boundary_vals[0]); j++) {
            int32_t x = boundary_vals[i];
            int32_t y = boundary_vals[j];
            int32_t add_result, mul_result, sub_result;
            
            // Multiple overflow checks in sequence
            int overflow_add = __builtin_add_overflow(x, y, &add_result);
            int overflow_mul = __builtin_mul_overflow(x, y, &mul_result);
            int overflow_sub = __builtin_sub_overflow(x, y, &sub_result);
            
            // Complex condition based on overflow results
            if (overflow_add && !overflow_mul) {
                hash ^= (uint32_t)x + (uint32_t)y;
            }
            if (!overflow_add && overflow_sub) {
                hash += (uint32_t)(x - y);
            }
            if (overflow_mul || overflow_add || overflow_sub) {
                hash = (hash << 1) | (hash >> 31);
            }
        }
    }
    
    // Nested overflow checks in conditional
    uint32_t ua = 0xFFFFFFFF;
    uint32_t ub = 0x00000001;
    uint32_t uc = 0x80000000;
    
    for (int i = 0; i < 50; i++) {
        uint32_t temp1, temp2;
        
        if (__builtin_uadd_overflow(ua, ub, &temp1)) {
            if (__builtin_umul_overflow(uc, 2, &temp2)) {
                hash ^= 0xCCCCCCCC;
            } else {
                hash += temp2;
            }
        }
        
        // Shift values
        ua = (ua >> 1) | (ua << 31);
        ub = (ub << 1) | (ub >> 31);
        uc = uc + i;
    }
    
    sink = hash;
    return hash;
}

__attribute__((noinline,noipa))
uint32_t test_edge_case_comparisons(void) {
    uint32_t hash = 0x12345678;
    
    // Direct comparisons at type boundaries
    int64_t large_vals[] = {
        INT64_MAX, INT64_MIN,
        (int64_t)UINT32_MAX,
        (int64_t)INT32_MAX + 1,
        (int64_t)INT32_MIN - 1,
        0x7FFFFFFFFFFFFFFFLL,
        0x8000000000000000LL
    };
    
    for (size_t i = 0; i < sizeof(large_vals)/sizeof(large_vals[0]); i++) {
        int64_t val = large_vals[i];
        
        // Comparisons that should trigger the uncovered logic
        if (val > INT32_MAX) {
            hash ^= 0x11111111;
        }
        if (val < INT32_MIN) {
            hash ^= 0x22222222;
        }
        if ((uint64_t)val > UINT32_MAX) {
            hash ^= 0x33333333;
        }
        
        // Compound comparisons
        if (val >= INT32_MIN && val <= INT32_MAX) {
            hash += (uint32_t)val;
        }
    }
    
    // Test zero/sign extension scenarios
    int32_t small_vals[] = {0, -1, 1, 0x7FFFFFFF, 0x80000000};
    
    for (size_t i = 0; i < sizeof(small_vals)/sizeof(small_vals[0]); i++) {
        int32_t sval = small_vals[i];
        uint32_t uval = (uint32_t)sval;
        
        // Zero extension comparisons
        uint64_t zext = (uint64_t)uval;
        int64_t sext = (int64_t)sval;
        
        if (zext > 0xFFFFFFFF) {
            hash ^= 0x44444444;
        }
        if (sext < -0x7FFFFFFFFFFFFFFFLL) {
            hash ^= 0x55555555;
        }
        
        // Shift then compare (like in uncovered code)
        uint64_t shifted = zext << 16;
        if (shifted > 0xFFFFFFFF00000000ULL) {
            hash += 0x66666666;
        }
    }
    
    sink = hash;
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
    final_hash ^= test_edge_case_comparisons();
    
    // Use the result to prevent optimization
    printf("Result hash: %u\n", final_hash);
    
    return (int)final_hash;
}
