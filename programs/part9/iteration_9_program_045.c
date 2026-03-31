#include <stdint.h>
#include <stdio.h>
#include <string.h>

// Prevent inlining to maintain function boundaries for coverage
#define NOINLINE __attribute__((noinline))

// Fixed-point types if available
#ifdef __STDC_IEC_559__
typedef _Fract fract_t;
typedef _Accum accum_t;
#else
typedef int32_t fract_t;
typedef int64_t accum_t;
#endif

// Checksum function to prevent dead code elimination
static uint32_t checksum = 0;

NOINLINE static void update_checksum(uint64_t value) {
    checksum = (checksum * 31) + (uint32_t)value;
    checksum = (checksum * 31) + (uint32_t)(value >> 32);
}

// 1. Integer Range Boundary Tests
NOINLINE static void test_narrowing_conversions(void) {
    int64_t wide_values[] = {
        INT64_MAX,
        INT64_MIN,
        (int64_t)UINT32_MAX + 1,
        (int64_t)INT32_MAX * 2,
        (int64_t)INT32_MIN * 2,
        0x7FFFFFFF00000000LL,
        0x8000000000000000LL
    };
    
    for (size_t i = 0; i < sizeof(wide_values)/sizeof(wide_values[0]); i++) {
        // Narrowing conversions that require range analysis
        int32_t narrow1 = (int32_t)wide_values[i];
        uint32_t narrow2 = (uint32_t)wide_values[i];
        int16_t narrow3 = (int16_t)wide_values[i];
        
        // Comparisons at type boundaries
        if (wide_values[i] > INT32_MAX) {
            update_checksum(1);
        }
        if (wide_values[i] < INT32_MIN) {
            update_checksum(2);
        }
        if ((uint64_t)wide_values[i] > UINT32_MAX) {
            update_checksum(3);
        }
        
        // Shift operations that may overflow
        int64_t shifted = wide_values[i] << 3;
        int32_t narrowed_shift = (int32_t)shifted;
        update_checksum((uint64_t)narrowed_shift);
    }
    
    // Complex boundary case with intermediate calculations
    int64_t a = INT32_MAX;
    int64_t b = 100;
    int64_t sum = a + b;
    if (sum > INT32_MAX) {  // Should trigger range analysis
        update_checksum(4);
    }
    
    // Narrowing after arithmetic
    int32_t result = (int32_t)(a * b / 2);
    update_checksum((uint64_t)result);
}

// 2. Loop Bound Analysis with Complex Conditions
NOINLINE static void test_loop_range_analysis(void) {
    uint32_t outer_bound = 1000;
    uint32_t inner_base = 500;
    
    // Outer loop with bitmasked bound
    for (uint32_t i = 0x100; i < (outer_bound & 0xFFF); i += 0x20) {
        // Inner loop with complex bound calculation
        uint32_t inner_bound = (i | 0x7FF) & 0xFFF;
        uint32_t step = (i & 0x1F) + 1;
        
        // Nested loop where inner bound depends on outer index
        for (uint32_t j = i & 0xFF; j < inner_bound; j += step) {
            // Mix of arithmetic that requires range tracking
            uint32_t k = j * 3;
            if (k > 0x80000000) {
                update_checksum(j);
            }
            
            // Bitwise operations that constrain ranges
            uint32_t masked = k & 0xFFFF;
            if (masked < 0x1000) {
                update_checksum(masked);
            }
        }
        
        // Loop with decreasing counter and complex condition
        for (int32_t k = (int32_t)(i & 0x7F); k > -(int32_t)(inner_base & 0x3FF); k -= 5) {
            int32_t abs_k = k < 0 ? -k : k;
            if (abs_k > 0x40) {
                update_checksum(abs_k);
            }
        }
    }
    
    // Loop with wrap-around behavior analysis
    uint8_t counter = 250;
    for (int i = 0; i < 20; i++) {
        counter += 10;  // Will wrap around
        if (counter > 200) {  // Range analysis needed for this condition
            update_checksum(counter);
        }
    }
}

// 3. Fixed-Point and Saturation Arithmetic
NOINLINE static void test_saturation_arithmetic(void) {
    // Manual saturation arithmetic
    int32_t sat_min = -1000;
    int32_t sat_max = 1000;
    
    int32_t test_values[] = {-2000, -500, 0, 500, 2000, 1500, -1500};
    
    for (size_t i = 0; i < sizeof(test_values)/sizeof(test_values[0]); i++) {
        // Saturation clamp - directly tests boundary comparisons
        int32_t result;
        if (test_values[i] > sat_max) {
            result = sat_max;
        } else if (test_values[i] < sat_min) {
            result = sat_min;
        } else {
            result = test_values[i];
        }
        update_checksum((uint64_t)result);
        
        // Multi-step saturation with intermediate overflow check
        int32_t scaled = test_values[i] * 3;
        if (scaled > sat_max) {
            scaled = sat_max;
        } else if (scaled < sat_min) {
            scaled = sat_min;
        }
        update_checksum((uint64_t)scaled);
    }
    
    // Fixed-point like operations using integers
    int32_t fp_a = 0x40000000;  // 0.5 in Q31
    int32_t fp_b = 0x20000000;  // 0.25 in Q31
    
    // Multiplication with shift to maintain fixed-point
    int64_t fp_product = (int64_t)fp_a * (int64_t)fp_b;
    int32_t fp_result = (int32_t)(fp_product >> 31);
    
    // Check if result is in valid Q31 range
    if (fp_result > 0x7FFFFFFF || fp_result < -0x80000000) {
        update_checksum(0xFFFFFFFF);
    } else {
        update_checksum((uint64_t)fp_result);
    }
}

// 4. Conditional Code with Value-Dependent Dead Branches
NOINLINE static void test_conditional_ranges(void) {
    uint32_t x = 0xFFFFFF00;
    uint32_t y = 0x00000100;
    
    // Chain of conditions that constrain value ranges
    if (x > 0xFFFFFF00) {
        // Dead branch for analysis
        update_checksum(1);
    }
    
    if (x < 0xFFFFFFFF) {
        // x is now known to be in [0xFFFFFF00, 0xFFFFFFFF]
        uint32_t z = x + y;
        
        // This addition may overflow - needs range analysis
        if (z < x) {  // Overflow occurred
            update_checksum(z);
        }
    }
    
    // Complex condition with modulo constraint
    uint32_t a = 1000;
    uint32_t b = a % 256;  // b is in [0, 255]
    
    if (b > 200) {
        // b is in [201, 255]
        uint32_t c = b * 2;
        if (c > 400 && c < 512) {  // Should be always true
            update_checksum(c);
        }
    }
    
    // Edge case at type boundary
    int32_t edge = INT32_MAX - 5;
    for (int i = 0; i < 10; i++) {
        edge++;
        if (edge > INT32_MAX) {  // Should trigger after 5 iterations
            update_checksum(i);
        }
    }
}

// 5. Structs with Bit-Fields and Unions
NOINLINE static void test_bitfield_ranges(void) {
    struct bitfield_struct {
        unsigned int a : 4;   // 0-15
        signed int b : 5;     // -16 to 15
        unsigned int c : 8;   // 0-255
        signed int d : 12;    // -2048 to 2047
    } bf;
    
    union bitfield_union {
        struct bitfield_struct bits;
        uint32_t raw;
    } u;
    
    // Assign values at bit-field boundaries
    bf.a = 15;      // Max for 4-bit unsigned
    bf.b = -16;     // Min for 5-bit signed
    bf.c = 255;     // Max for 8-bit unsigned
    bf.d = 2047;    // Max for 12-bit signed
    
    u.bits = bf;
    update_checksum(u.raw);
    
    // Operations that require understanding bit-field ranges
    if (bf.a == 15) {
        // bf.a cannot be > 15, so this comparison is always false
        if (bf.a > 20) {
            update_checksum(999);  // Dead code
        }
    }
    
    // Arithmetic that may overflow bit-field
    bf.c = 200;
    bf.c = bf.c + 100;  // Would be 300, but truncated to 8 bits
    
    // Check if value fits in bit-field
    uint32_t test_val = 300;
    if (test_val <= 255) {
        bf.c = test_val;
    } else {
        bf.c = 255;
    }
    update_checksum(bf.c);
    
    // Nested bit-field access in loop
    for (int i = 0; i < 20; i++) {
        bf.a = i & 0xF;  // Always in range 0-15
        if (bf.a > 10) {
            update_checksum(i);
        }
    }
}

// 6. Compiler Builtins for Overflow Detection
NOINLINE static void test_overflow_builtins(void) {
    int32_t a = INT32_MAX / 2;
    int32_t b = INT32_MAX / 2 + 1;
    int32_t result;
    
    // Overflow checks in loops
    for (int i = 0; i < 10; i++) {
        int overflow = __builtin_add_overflow(a, b + i, &result);
        if (overflow) {
            update_checksum(i);
        } else {
            update_checksum(result);
        }
    }
    
    // Multiplication overflow with range-constrained inputs
    uint32_t x = 0xFFFFFFFF;
    uint32_t y = 2;
    uint64_t mul_result;
    
    if (!__builtin_mul_overflow(x, y, &mul_result)) {
        update_checksum(mul_result);
    } else {
        update_checksum(0);
    }
    
    // Chained operations with overflow checks
    int32_t accum = 1000;
    int32_t values[] = {500, 600, 700, 800, 900};
    
    for (size_t i = 0; i < sizeof(values)/sizeof(values[0]); i++) {
        int32_t new_accum;
        if (!__builtin_add_overflow(accum, values[i], &new_accum)) {
            accum = new_accum;
            // Check if still in safe range
            if (accum < 5000) {
                update_checksum(accum);
            }
        }
    }
    
    // Subtraction overflow with signed types
    int32_t min_val = INT32_MIN + 10;
    for (int i = 0; i < 20; i++) {
        int32_t diff;
        if (!__builtin_sub_overflow(min_val, i, &diff)) {
            update_checksum(diff);
        } else {
            update_checksum(0xDEADBEEF);
        }
    }
}

int main(void) {
    // Initialize checksum
    checksum = 0x12345678;
    
    // Run all tests
    test_narrowing_conversions();
    test_loop_range_analysis();
    test_saturation_arithmetic();
    test_conditional_ranges();
    test_bitfield_ranges();
    test_overflow_builtins();
    
    // Output checksum to prevent optimization
    printf("Checksum: %u\n", checksum);
    
    return 0;
}
