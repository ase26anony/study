#include <stdint.h>
#include <stdio.h>
#include <string.h>

// Prevent optimization from removing our tests
#define NOINLINE __attribute__((noinline))
#define KEEP __attribute__((used))

// Checksum to prevent dead code elimination
static volatile int checksum = 0;

// Test 1: Narrowing conversions with boundary values
NOINLINE static int test_narrowing_conversions(void) {
    int result = 0;
    
    // Constants at type boundaries
    int64_t large_vals[] = {
        INT64_MAX,
        INT64_MIN,
        (int64_t)INT32_MAX + 1,
        (int64_t)INT32_MIN - 1,
        0x7FFFFFFF00000000LL,
        0x8000000000000000LL
    };
    
    for (size_t i = 0; i < sizeof(large_vals)/sizeof(large_vals[0]); i++) {
        // Narrowing conversions that require range analysis
        int32_t narrowed = (int32_t)large_vals[i];
        result ^= narrowed;
        
        // Comparisons against boundaries
        if (large_vals[i] > INT32_MAX) {
            result += 1;
        }
        if (large_vals[i] < INT32_MIN) {
            result += 2;
        }
        
        // Shift operations that may overflow
        int64_t shifted = large_vals[i] << 3;
        int32_t narrowed_shifted = (int32_t)shifted;
        result ^= narrowed_shifted;
    }
    
    // Mixed-width arithmetic
    uint32_t a = 0xFFFFFFFF;
    uint64_t b = 0x100000000;
    uint64_t sum = a + b;
    uint32_t truncated = (uint32_t)sum;
    result ^= truncated;
    
    checksum += result;
    return result;
}

// Test 2: Complex loop bound analysis
NOINLINE static int test_loop_range_analysis(void) {
    int result = 0;
    
    // Outer loop with bitmasked bound
    for (int32_t outer = 0x1000; outer < (0x2000 & 0x1FFF); outer += 0x100) {
        // Inner loop with bound derived from outer
        for (int32_t inner = outer & 0xFF; 
             inner < ((outer | 0x7F) ^ 0x3F); 
             inner += (outer & 0x3) + 1) {
            result += inner * outer;
            
            // Additional bitwise condition
            if ((inner & 0xF0) == (outer & 0xF0)) {
                result ^= inner;
            }
        }
    }
    
    // Loop with shifting bound
    uint64_t base = 0x80000000;
    for (uint32_t i = 1; i < 32; i++) {
        uint64_t limit = base >> i;
        for (uint32_t j = 0; j < limit && j < 1000; j++) {
            result += j * i;
        }
    }
    
    // Nested loops with complex exit conditions
    int32_t x = 0x7FFFFFF0;
    int32_t y = 0x0000000F;
    while (x > 0 && y < 100) {
        for (int32_t z = x & y; z < (x | y); z += (x ^ y) & 0xF) {
            result += z;
            if (z > 0x7FFFFFFF - 100) {
                break;
            }
        }
        x -= 0x100;
        y += 0x1;
    }
    
    checksum += result;
    return result;
}

// Test 3: Saturation arithmetic
NOINLINE static int test_saturation_arithmetic(void) {
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
    
    // Test cases near boundaries
    int32_t test_cases[][2] = {
        {INT32_MAX, 1},
        {INT32_MIN, -1},
        {INT32_MAX / 2, 2},
        {INT32_MIN / 2, 2},
        {0x40000000, 0x40000000},
        {0xC0000000, 0x40000000}
    };
    
    for (size_t i = 0; i < sizeof(test_cases)/sizeof(test_cases[0]); i++) {
        result ^= sat_add(test_cases[i][0], test_cases[i][1]);
        result ^= sat_mul(test_cases[i][0], test_cases[i][1]);
    }
    
    // Fixed-point types if available (GCC extension)
    #ifdef __FRACT_FBIT__
    _Fract f1 = 0.5r;
    _Fract f2 = 0.7r;
    _Fract fsum = f1 + f2;
    result += (int)(fsum * 1000);
    #endif
    
    checksum += result;
    return result;
}

// Test 4: Bit-field range analysis
NOINLINE static int test_bitfield_ranges(void) {
    int result = 0;
    
    struct BitFields {
        unsigned int a : 3;  // 0-7
        signed int b : 5;    // -16 to 15
        unsigned int c : 12; // 0-4095
        signed int d : 20;   // -524288 to 524287
    } bf;
    
    // Assign values at boundaries
    bf.a = 7;   // Max for 3 bits
    bf.b = -16; // Min for 5-bit signed
    bf.c = 4095; // Max for 12 bits
    bf.d = 524287; // Max for 20-bit signed
    
    // Comparisons that require range analysis
    if (bf.a == 7) {
        result += 1;
    }
    if (bf.b < 0) {
        result += 2;
    }
    if (bf.c > 4000) {
        result += 4;
    }
    if (bf.d == 524287) {
        result += 8;
    }
    
    // Union with bit-fields
    union {
        struct {
            unsigned int low : 16;
            unsigned int high : 16;
        } parts;
        uint32_t whole;
    } u;
    
    u.whole = 0x87654321;
    if (u.parts.low > 0x8000) {
        result ^= u.parts.low;
    }
    if (u.parts.high < 0x9000) {
        result ^= u.parts.high;
    }
    
    // Nested bit-field access in loop
    for (unsigned int i = 0; i < 10; i++) {
        bf.a = i & 0x7;
        bf.c = (i * 100) & 0xFFF;
        result += bf.a * bf.c;
    }
    
    checksum += result;
    return result;
}

// Test 5: Overflow builtins with range analysis
NOINLINE static int test_overflow_builtins(void) {
    int result = 0;
    
    // Variables with constrained ranges
    int32_t constrained_values[10];
    for (int i = 0; i < 10; i++) {
        constrained_values[i] = (i * 100) & 0x7FF; // 0-2047
    }
    
    // Overflow checks with partially known ranges
    for (int i = 0; i < 9; i++) {
        int32_t a = constrained_values[i];
        int32_t b = constrained_values[i + 1];
        int32_t sum;
        
        if (!__builtin_add_overflow(a, b, &sum)) {
            result += sum;
        }
        
        int32_t prod;
        if (!__builtin_mul_overflow(a, b, &prod)) {
            result ^= prod;
        }
        
        // Chain operations
        int32_t temp;
        if (!__builtin_add_overflow(a, 1000, &temp)) {
            if (!__builtin_sub_overflow(temp, b, &temp)) {
                result += temp;
            }
        }
    }
    
    // Edge cases
    int32_t edge_cases[] = {INT32_MAX, INT32_MIN, 0, 1, -1};
    for (size_t i = 0; i < sizeof(edge_cases)/sizeof(edge_cases[0]); i++) {
        for (size_t j = 0; j < sizeof(edge_cases)/sizeof(edge_cases[0]); j++) {
            int32_t a = edge_cases[i];
            int32_t b = edge_cases[j];
            int32_t res;
            int overflow = __builtin_add_overflow(a, b, &res);
            result ^= (overflow << (i * 3 + j)) & 1;
            
            overflow = __builtin_mul_overflow(a, b, &res);
            result ^= (overflow << (i * 3 + j + 1)) & 1;
        }
    }
    
    // Overflow in shift operations simulated
    for (int shift = 0; shift < 40; shift++) {
        int32_t val = 1;
        int overflow = 0;
        
        if (shift >= 31) {
            overflow = 1;
        } else {
            val = 1 << shift;
        }
        
        result ^= (overflow << (shift & 0xF)) & 1;
        result += val;
    }
    
    checksum += result;
    return result;
}

// Test 6: Additional boundary condition tests
NOINLINE static int test_boundary_conditions(void) {
    int result = 0;
    
    // Comparisons at extreme edges
    int64_t values[] = {
        INT64_MAX,
        INT64_MIN,
        INT32_MAX,
        INT32_MIN,
        (int64_t)UINT32_MAX,
        0
    };
    
    for (size_t i = 0; i < sizeof(values)/sizeof(values[0]); i++) {
        // Multiple comparison types
        if (values[i] > INT32_MAX - 10) {
            result += 1;
        }
        if (values[i] < INT32_MIN + 10) {
            result += 2;
        }
        if (values[i] >= 0x7FFFFFFF00000000LL) {
            result += 4;
        }
        if (values[i] <= 0x8000000000000000LL) {
            result += 8;
        }
        
        // Zero/sign extension simulation
        uint64_t zext = (uint64_t)(uint32_t)values[i];
        int64_t sext = (int64_t)(int32_t)values[i];
        result ^= (int)(zext >> 32);
        result ^= (int)(sext >> 32);
    }
    
    // Modulo operations that constrain ranges
    for (int64_t x = -1000; x < 1000; x += 77) {
        int32_t constrained = x % 256;  // Now in range -255..255
        if (constrained > 200) {
            result += constrained;
        }
        if (constrained < -200) {
            result -= constrained;
        }
    }
    
    checksum += result;
    return result;
}

// Main function calling all tests
int main(void) {
    int total = 0;
    
    total ^= test_narrowing_conversions();
    total ^= test_loop_range_analysis();
    total ^= test_saturation_arithmetic();
    total ^= test_bitfield_ranges();
    total ^= test_overflow_builtins();
    total ^= test_boundary_conditions();
    
    // Use checksum to prevent optimization
    printf("Result: %d (checksum: %d)\n", total, checksum);
    
    return total != 0;
}
