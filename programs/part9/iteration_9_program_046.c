#include <stdint.h>
#include <stdio.h>
#include <string.h>

// Prevent dead code elimination
#define KEEP(expr) do { volatile int _ = (int)(expr); (void)_; } while(0)

// GCC fixed-point types if available
#ifdef __STDC_IEC_559__
#include <stdfix.h>
#endif

__attribute__((noinline))
unsigned test_narrowing_conversions() {
    unsigned checksum = 0;
    
    // Test 1: Narrowing conversions with boundary values
    int64_t wide_vals[] = {
        INT64_MAX,
        INT64_MIN,
        INT32_MAX + 1LL,
        INT32_MIN - 1LL,
        UINT32_MAX,
        0x7FFFFFFF00000000LL,
        0x8000000000000000LL
    };
    
    for (size_t i = 0; i < sizeof(wide_vals)/sizeof(wide_vals[0]); i++) {
        int32_t narrow = (int32_t)wide_vals[i];
        uint32_t unarrow = (uint32_t)wide_vals[i];
        checksum = checksum * 31 + narrow;
        checksum = checksum * 31 + unarrow;
        
        // Force range analysis with comparisons
        if (wide_vals[i] > INT32_MAX) {
            checksum += 1;
        }
        if (wide_vals[i] < INT32_MIN) {
            checksum += 2;
        }
    }
    
    // Test 2: Shifts that may overflow
    uint32_t x = 0x80000000U;
    for (int shift = 0; shift < 40; shift++) {
        uint64_t shifted = (uint64_t)x << shift;
        int32_t truncated = (int32_t)shifted;
        checksum = checksum * 17 + truncated;
        
        // Boundary comparison
        if (shifted > UINT32_MAX) {
            checksum += shift;
        }
    }
    
    return checksum;
}

__attribute__((noinline))
unsigned test_loop_range_analysis() {
    unsigned checksum = 0;
    
    // Complex loop bounds with bitwise operations
    uint32_t a = 0x12345678;
    uint32_t b = 0x9ABCDEF0;
    
    // Outer loop with mask-derived bounds
    for (uint32_t i = a & 0x0000FFFF; i < (b | 0x0000FFFF); i += 257) {
        // Inner loop with dependent bounds
        for (uint32_t j = i & 0xFF; j < (i | 0x7F); j += 13) {
            checksum = checksum * 3 + j;
            
            // Additional bitwise condition
            if ((j ^ i) > 0x80000000) {
                checksum += 0x10000;
            }
        }
        
        // Nested loop with shift-based bounds
        int32_t k;
        for (k = (int32_t)(i >> 16) - 100; k < (int32_t)(i >> 8) + 100; k += 7) {
            if (k > 0 && k < 1000) {
                checksum = checksum * 5 + k;
            }
        }
    }
    
    // Loop with signed overflow potential
    int32_t start = INT32_MAX - 100;
    int32_t end = INT32_MAX + 100;  // Will wrap
    for (int32_t i = start; i < end; i += 10) {
        checksum = checksum * 7 + (unsigned)i;
    }
    
    return checksum;
}

__attribute__((noinline))
unsigned test_saturation_arithmetic() {
    unsigned checksum = 0;
    
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
    
    // Test boundary cases
    int32_t test_cases[][2] = {
        {INT32_MAX, 1},
        {INT32_MIN, -1},
        {INT32_MAX, INT32_MAX},
        {INT32_MIN, INT32_MIN},
        {1000, 1000},
        {-1000, -1000}
    };
    
    for (size_t i = 0; i < sizeof(test_cases)/sizeof(test_cases[0]); i++) {
        int32_t sum = sat_add(test_cases[i][0], test_cases[i][1]);
        int32_t prod = sat_mul(test_cases[i][0], test_cases[i][1]);
        checksum = checksum * 19 + sum;
        checksum = checksum * 23 + prod;
    }
    
    // GCC fixed-point types if available
    #ifdef __STDC_IEC_559__
    _Accum acc = 0.5k;
    _Fract frac = 0.1r;
    
    for (int i = 0; i < 10; i++) {
        acc += 0.1k;
        frac += 0.01r;
        
        // Comparisons that may trigger range analysis
        if (acc > 1.0k) {
            checksum += i;
        }
        if (frac < 0.0r) {
            checksum += i * 2;
        }
    }
    #endif
    
    return checksum;
}

__attribute__((noinline))
unsigned test_bitfield_ranges() {
    unsigned checksum = 0;
    
    // Struct with various bit-fields
    struct BitFieldStruct {
        signed int a : 5;    // -16 to 15
        unsigned int b : 7;   // 0 to 127
        signed int c : 12;   // -2048 to 2047
        unsigned int d : 3;   // 0 to 7
    } bfs;
    
    // Union to test type punning
    union BitFieldUnion {
        struct BitFieldStruct s;
        uint32_t raw;
    } u;
    
    // Test assignments at boundaries
    bfs.a = 15;      // Max for 5-bit signed
    bfs.b = 127;     // Max for 7-bit unsigned
    bfs.c = -2048;   // Min for 12-bit signed
    bfs.d = 7;       // Max for 3-bit unsigned
    
    checksum = bfs.a + (bfs.b << 5) + (bfs.c << 12) + (bfs.d << 24);
    
    // Test comparisons that require range analysis
    for (int i = -20; i < 20; i++) {
        bfs.a = i;
        if (bfs.a > 10) {          // Compare against value within range
            checksum += 1;
        }
        if (bfs.a < -10) {         // Compare against negative boundary
            checksum += 2;
        }
        if (bfs.a == 15) {         // Compare against max boundary
            checksum += 4;
        }
    }
    
    // Test with union
    u.raw = 0xFFFFFFFF;
    if (u.s.b > 100) {             // Bit-field comparison
        checksum += u.s.b;
    }
    
    return checksum;
}

__attribute__((noinline))
unsigned test_overflow_builtins() {
    unsigned checksum = 0;
    
    int32_t a = INT32_MAX - 100;
    int32_t b = 200;
    int32_t result;
    
    // Test overflow builtins with partially known ranges
    for (int i = 0; i < 50; i++) {
        if (__builtin_add_overflow(a + i, b, &result)) {
            checksum += i * 3;
        } else {
            checksum += result;
        }
    }
    
    // Multiplication with range-restricted operands
    int32_t x = 1000;
    int32_t y = 1000;
    for (int i = 0; i < 10; i++) {
        if (__builtin_mul_overflow(x + i, y - i, &result)) {
            checksum += i * 7;
        } else {
            checksum += result;
        }
    }
    
    // Nested overflow checks with conditional ranges
    uint32_t u = UINT32_MAX - 1000;
    uint32_t v = 500;
    uint32_t uresult;
    
    if (u > UINT32_MAX / 2) {
        // Compiler knows u is in [UINT32_MAX/2 + 1, UINT32_MAX-1000]
        for (int i = 0; i < 20; i++) {
            if (__builtin_add_overflow(u, v + i, &uresult)) {
                checksum += i * 11;
            }
        }
    }
    
    // Test subtraction overflow
    int32_t min_val = INT32_MIN + 500;
    for (int i = 0; i < 100; i++) {
        if (__builtin_sub_overflow(min_val, i, &result)) {
            checksum += i * 13;
        }
    }
    
    return checksum;
}

__attribute__((noinline))
unsigned test_value_dependent_branches() {
    unsigned checksum = 0;
    
    // Variables with constrained ranges
    int32_t x = 100;
    int32_t y = -100;
    
    // Constrain ranges through conditions
    if (x > 0 && x < 1000) {
        // Compiler knows x ∈ [1, 999]
        for (int i = 0; i < 10; i++) {
            int32_t temp = x * i;
            if (temp > INT32_MAX - 100) {  // Edge comparison
                checksum += 1;
            }
            if (temp < INT32_MIN + 100) {  // Other edge
                checksum += 2;
            }
        }
    }
    
    // Modulo operation creates known range
    uint32_t mod_val = 0x12345678;
    uint32_t constrained = mod_val % 1000;  // ∈ [0, 999]
    
    if (constrained > 500) {
        checksum += constrained;
    }
    
    // Chain of conditions creating complex range
    int32_t val = 500;
    if (val < 1000) {
        if (val > 100) {
            if ((val & 0xFF) == val % 256) {
                // Complex range known here
                for (int i = 0; i < 5; i++) {
                    int64_t expanded = (int64_t)val << (i * 4);
                    if (expanded > INT32_MAX) {
                        checksum += i * 100;
                    }
                }
            }
        }
    }
    
    return checksum;
}

int main() {
    unsigned final_checksum = 0;
    
    final_checksum ^= test_narrowing_conversions();
    final_checksum ^= test_loop_range_analysis();
    final_checksum ^= test_saturation_arithmetic();
    final_checksum ^= test_bitfield_ranges();
    final_checksum ^= test_overflow_builtins();
    final_checksum ^= test_value_dependent_branches();
    
    // Use the result to prevent optimization
    KEEP(final_checksum);
    
    printf("Checksum: %u\n", final_checksum);
    return 0;
}
