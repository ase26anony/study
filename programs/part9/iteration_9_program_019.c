#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Prevent dead code elimination
static volatile int sink;

// GCC fixed-point types (if available)
#ifdef __STDC_IEC_559__
typedef _Fract fract_t;
typedef _Accum accum_t;
#else
typedef int32_t fract_t;
typedef int64_t accum_t;
#endif

__attribute__((noinline))
unsigned test_narrowing_conversions(void) {
    unsigned hash = 0;
    
    // Test 1: Narrowing with constants at boundaries
    int64_t wide_vals[] = {
        INT64_MAX,
        INT64_MIN,
        (int64_t)INT32_MAX + 1,
        (int64_t)INT32_MIN - 1,
        0x7FFFFFFF00000000LL,
        0x80000000FFFFFFFFLL
    };
    
    for (size_t i = 0; i < sizeof(wide_vals)/sizeof(wide_vals[0]); i++) {
        int32_t narrow = (int32_t)wide_vals[i];
        hash = hash * 31 + (unsigned)narrow;
        
        // Additional narrowing with shifts
        uint64_t uwide = (uint64_t)wide_vals[i];
        uint32_t unarrow = (uint32_t)(uwide >> 16);
        hash = hash * 31 + unarrow;
    }
    
    // Test 2: Complex narrowing with arithmetic
    for (int64_t base = -1000; base <= 1000; base += 500) {
        int64_t val = base * 0x7FFFFFFFLL;
        int32_t truncated = (int32_t)val;
        
        // Conditional narrowing based on range
        if (val > INT32_MAX) {
            truncated = INT32_MAX;
        } else if (val < INT32_MIN) {
            truncated = INT32_MIN;
        }
        hash = hash * 31 + (unsigned)truncated;
    }
    
    sink = hash;
    return hash;
}

__attribute__((noinline))
unsigned test_loop_range_analysis(void) {
    unsigned hash = 0;
    
    // Test 1: Loop with bitmasked bounds
    uint32_t a = 0x12345678;
    uint32_t b = 0x9ABCDEF0;
    
    for (uint32_t i = a & 0xFFF; i < (b | 0x7FF); i += 37) {
        hash += i * 0x9E3779B9;
        
        // Nested loop with dependent bounds
        for (uint32_t j = i & 0xFF; j < (i | 0x7F); j += 13) {
            hash ^= j * 0xDEADBEEF;
        }
    }
    
    // Test 2: Loop with signed bounds and shifting
    int32_t start = -1000;
    int32_t end = 1000;
    
    for (int32_t i = start; i < end; i = (i << 1) + 1) {
        if (i > 0) {
            hash += (unsigned)i;
        }
        
        // Complex condition with bitwise ops
        if ((i & 0xFF) == 0x7F || (i ^ 0x55) < 100) {
            hash ^= 0xAAAAAAAA;
        }
    }
    
    // Test 3: Loop with modulo-based bounds
    for (int32_t k = 0; k < 1000; k++) {
        int32_t bound = (k * 7) % 256;
        for (int32_t m = 0; m < bound; m++) {
            hash = hash * 3 + m;
        }
    }
    
    sink = hash;
    return hash;
}

__attribute__((noinline))
unsigned test_saturation_arithmetic(void) {
    unsigned hash = 0;
    
    // Manual saturation arithmetic
    int32_t sat_add(int32_t a, int32_t b) {
        int64_t result = (int64_t)a + b;
        if (result > INT32_MAX) return INT32_MAX;
        if (result < INT32_MIN) return INT32_MIN;
        return (int32_t)result;
    }
    
    int32_t sat_mul(int32_t a, int32_t b) {
        int64_t result = (int64_t)a * b;
        if (result > INT32_MAX) return INT32_MAX;
        if (result < INT32_MIN) return INT32_MIN;
        return (int32_t)result;
    }
    
    // Test saturation at boundaries
    int32_t test_cases[][2] = {
        {INT32_MAX, 1},
        {INT32_MIN, -1},
        {INT32_MAX / 2, 2},
        {INT32_MIN / 2, 3},
        {1000, 1000},
        {-1000, -1000}
    };
    
    for (size_t i = 0; i < sizeof(test_cases)/sizeof(test_cases[0]); i++) {
        int32_t sum = sat_add(test_cases[i][0], test_cases[i][1]);
        int32_t prod = sat_mul(test_cases[i][0], test_cases[i][1]);
        hash = hash * 31 + (unsigned)sum;
        hash = hash * 31 + (unsigned)prod;
    }
    
    // Fixed-point style operations (if available)
#ifdef __STDC_IEC_559__
    accum_t acc = 0.5k;
    for (int i = 0; i < 10; i++) {
        acc = acc * 1.1k;
        hash += (unsigned)((int64_t)acc);
    }
#endif
    
    sink = hash;
    return hash;
}

__attribute__((noinline))
unsigned test_bitfield_ranges(void) {
    unsigned hash = 0;
    
    // Struct with various bit-fields
    struct BitFields {
        signed int a : 5;    // -16 to 15
        unsigned int b : 7;   // 0 to 127
        signed int c : 12;   // -2048 to 2047
        unsigned int d : 3;   // 0 to 7
    } bf;
    
    // Test assignments at boundaries
    bf.a = 15;      // Max positive for 5-bit signed
    bf.b = 127;     // Max for 7-bit unsigned
    bf.c = -2048;   // Min for 12-bit signed
    bf.d = 7;       // Max for 3-bit unsigned
    
    hash = (unsigned)bf.a + ((unsigned)bf.b << 8) + 
           ((unsigned)bf.c << 16) + ((unsigned)bf.d << 28);
    
    // Conditional checks based on bit-field ranges
    for (int i = -20; i < 20; i++) {
        bf.a = i;
        if (bf.a > 10) {
            hash ^= 0x11111111;
        }
        if (bf.a < -10) {
            hash ^= 0x22222222;
        }
    }
    
    // Union with overlapping bit-fields
    union Overlap {
        struct {
            unsigned int low : 16;
            unsigned int high : 16;
        } parts;
        uint32_t whole;
    } ov;
    
    ov.whole = 0x87654321;
    if (ov.parts.low > 0x7FFF) {
        hash += 0x33333333;
    }
    if (ov.parts.high < 0x8000) {
        hash += 0x44444444;
    }
    
    sink = hash;
    return hash;
}

__attribute__((noinline))
unsigned test_overflow_builtins(void) {
    unsigned hash = 0;
    
    // Test overflow detection with partially known ranges
    for (int32_t i = -100; i <= 100; i += 25) {
        int32_t result;
        
        // Addition with one operand bounded
        if (__builtin_add_overflow(i, INT32_MAX - 50, &result)) {
            hash ^= 0x55555555;
        } else {
            hash += (unsigned)result;
        }
        
        // Multiplication with bounded result
        if (__builtin_mul_overflow(i, 1000000, &result)) {
            hash ^= 0x66666666;
        }
        
        // Subtraction near boundaries
        if (__builtin_sub_overflow(INT32_MIN + 100, i, &result)) {
            hash ^= 0x77777777;
        }
    }
    
    // Complex overflow checks in loops
    int32_t accum = 1;
    for (int i = 1; i < 100; i++) {
        int32_t old_accum = accum;
        if (!__builtin_mul_overflow(accum, i, &accum)) {
            // Check if we're approaching overflow
            if (old_accum > INT32_MAX / (i + 1)) {
                hash += 0x88888888;
            }
        } else {
            accum = INT32_MAX;
            hash ^= 0x99999999;
        }
    }
    
    sink = hash;
    return hash;
}

__attribute__((noinline))
unsigned test_edge_case_conditions(void) {
    unsigned hash = 0;
    
    // Conditions at extreme boundaries
    for (int64_t i = INT64_MIN; i < INT64_MIN + 100; i += 10) {
        int32_t narrowed = (int32_t)i;
        
        // Comparisons against edge values
        if (narrowed > INT32_MAX - 10) {
            hash ^= 0xAAAAAAAALL;
        }
        if (narrowed < INT32_MIN + 10) {
            hash ^= 0xBBBBBBBBLL;
        }
        
        // Range-restricted variable
        int32_t bounded = narrowed % 1000;
        if (bounded > 500) {
            hash += 0xCCCCCCCC;
        }
    }
    
    // Shift operations that may overflow
    for (int i = 0; i < 40; i++) {
        uint32_t val = 1U << i;  // Shift may exceed bit width
        hash += val;
        
        // Right shift with sign extension
        int32_t sval = -1 << i;
        hash ^= (unsigned)sval;
    }
    
    sink = hash;
    return hash;
}

int main(void) {
    unsigned final_hash = 0;
    
    final_hash ^= test_narrowing_conversions();
    final_hash ^= test_loop_range_analysis();
    final_hash ^= test_saturation_arithmetic();
    final_hash ^= test_bitfield_ranges();
    final_hash ^= test_overflow_builtins();
    final_hash ^= test_edge_case_conditions();
    
    // Use the result to prevent optimization
    printf("Result hash: %u\n", final_hash);
    
    return 0;
}
