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

// Checksum to prevent dead code elimination
static volatile uint64_t checksum = 0;

NOINLINE uint32_t test_narrowing_conversions(void) {
    uint32_t local_sum = 0;
    
    // Test 1: Narrowing conversions with boundary values
    int64_t wide_vals[] = {
        INT64_MAX,
        INT64_MIN,
        (int64_t)UINT32_MAX + 1,
        (int64_t)INT32_MAX * 2,
        (int64_t)INT32_MIN * 2
    };
    
    for (size_t i = 0; i < sizeof(wide_vals)/sizeof(wide_vals[0]); i++) {
        // These conversions require range analysis
        int32_t narrow = (int32_t)wide_vals[i];
        uint32_t unarrow = (uint32_t)wide_vals[i];
        
        // Force analysis of the conversion
        if (wide_vals[i] > INT32_MAX || wide_vals[i] < INT32_MIN) {
            local_sum += 1;  // Overflow case
        } else {
            local_sum += narrow;
        }
        
        if ((uint64_t)wide_vals[i] > UINT32_MAX) {
            local_sum += 2;  // Truncation case
        } else {
            local_sum += unarrow;
        }
    }
    
    // Test 2: Shifts that may overflow
    uint32_t x = 0x80000000;
    for (int shift = 0; shift < 40; shift++) {
        uint64_t shifted = (uint64_t)x << shift;
        uint32_t truncated = (uint32_t)shifted;
        
        // This comparison should trigger range analysis
        if (shifted > UINT32_MAX) {
            local_sum += truncated;
        } else {
            local_sum += (uint32_t)shifted;
        }
    }
    
    checksum += local_sum;
    return local_sum;
}

NOINLINE uint32_t test_loop_range_analysis(void) {
    uint32_t local_sum = 0;
    
    // Complex loop bounds with bitwise operations
    uint32_t a = 0x12345678;
    uint32_t b = 0x9ABCDEF0;
    uint32_t c = 17;
    
    // Outer loop with bitmasked bound
    for (uint32_t i = a & 0xFFF; i < (b | 0x7FF); i += (c & 0x1F)) {
        // Inner loop with dependent bounds
        for (uint32_t j = (i ^ 0x555) & 0x3FF; 
             j < ((i + 0x100) & 0x7FF); 
             j += ((i & 0x1F) + 1)) {
            local_sum += j;
            
            // Additional condition to force range analysis
            if (j > 0x3FF && j < 0x500) {
                local_sum ^= i;
            }
        }
        
        // Break condition based on complex comparison
        if (i > 0x800 && (i & 0xFF) == 0x7F) {
            break;
        }
    }
    
    // Another loop with signed bounds
    int32_t start = -1000;
    int32_t end = 1000;
    int32_t step = 37;
    
    for (int32_t k = start; k < end; k += step) {
        // Nested loop with shifted bounds
        for (int32_t m = k << 2; m < (k + 100); m += 5) {
            if (m > INT16_MAX || m < INT16_MIN) {
                local_sum += 1;
            } else {
                local_sum += (uint32_t)m;
            }
        }
    }
    
    checksum += local_sum;
    return local_sum;
}

NOINLINE uint32_t test_saturation_arithmetic(void) {
    uint32_t local_sum = 0;
    
    // Manual saturation arithmetic
    int32_t sat_add(int32_t a, int32_t b) {
        int64_t result = (int64_t)a + (int64_t)b;
        
        // These comparisons should trigger the uncovered range analysis
        if (result > INT32_MAX) {
            return INT32_MAX;
        } else if (result < INT32_MIN) {
            return INT32_MIN;
        } else {
            return (int32_t)result;
        }
    }
    
    int32_t sat_mul(int32_t a, int32_t b) {
        int64_t result = (int64_t)a * (int64_t)b;
        
        // Boundary checks for multiplication
        if (result > INT32_MAX) {
            return INT32_MAX;
        } else if (result < INT32_MIN) {
            return INT32_MIN;
        } else {
            return (int32_t)result;
        }
    }
    
    // Test saturation with boundary values
    int32_t test_cases[][2] = {
        {INT32_MAX, 1},
        {INT32_MIN, -1},
        {INT32_MAX / 2, 2},
        {INT32_MIN / 2, 2},
        {1000, 2000},
        {-1000, -2000}
    };
    
    for (size_t i = 0; i < sizeof(test_cases)/sizeof(test_cases[0]); i++) {
        int32_t saturated = sat_add(test_cases[i][0], test_cases[i][1]);
        local_sum += (uint32_t)saturated;
        
        saturated = sat_mul(test_cases[i][0], test_cases[i][1]);
        local_sum += (uint32_t)saturated;
    }
    
    // Fixed-point operations if available
#ifdef __STDC_IEC_559__
    fract_t f1 = 0.5r;
    fract_t f2 = 0.75r;
    fract_t f3 = f1 + f2;  // May saturate
    
    accum_t a1 = 1000.0k;
    accum_t a2 = 2000.0k;
    accum_t a3 = a1 * a2;  // May overflow
    
    local_sum += (uint32_t)(f3 * 1000);
    local_sum += (uint32_t)(a3 / 1000);
#endif
    
    checksum += local_sum;
    return local_sum;
}

NOINLINE uint32_t test_bitfield_ranges(void) {
    uint32_t local_sum = 0;
    
    // Struct with various bit-fields
    struct bitfield_struct {
        unsigned int a : 3;   // 0-7
        signed int b : 5;     // -16 to 15
        unsigned int c : 12;  // 0-4095
        signed int d : 20;    // -524288 to 524287
        unsigned int e : 1;   // 0-1
    } bfs;
    
    // Union to test type punning
    union bitfield_union {
        struct bitfield_struct bf;
        uint64_t raw;
    } u;
    
    // Test assignments at boundaries
    bfs.a = 7;  // Max for 3 bits
    bfs.b = -16; // Min for 5-bit signed
    bfs.c = 4095; // Max for 12 bits
    bfs.d = 524287; // Max for 20-bit signed
    bfs.e = 1;
    
    // Comparisons that should trigger range analysis
    if (bfs.a == 7) {  // At max boundary
        local_sum += 1;
    }
    
    if (bfs.b < 0 && bfs.b >= -16) {  // Within signed range
        local_sum += 2;
    }
    
    if (bfs.c > 4000) {  // Near upper bound
        local_sum += bfs.c;
    }
    
    // Test overflow in bit-field assignment
    uint32_t large_val = 500000;
    bfs.d = large_val;  // Will be truncated
    
    // Check if truncated value is within range
    if (bfs.d <= 524287 && bfs.d >= -524288) {
        local_sum += 3;
    }
    
    // Complex condition with bit-field ranges
    if ((bfs.a & 0x3) == 0x3 && bfs.c < 4096) {
        local_sum ^= bfs.b;
    }
    
    checksum += local_sum;
    return local_sum;
}

NOINLINE uint32_t test_overflow_builtins(void) {
    uint32_t local_sum = 0;
    
    // Test overflow builtins with range-constrained values
    int32_t x = 1000000;
    int32_t y = 2000000;
    int32_t result;
    
    // These should use the underlying range analysis
    if (__builtin_add_overflow(x, y, &result)) {
        local_sum += 1;  // Overflow occurred
    } else {
        local_sum += result;
    }
    
    // Test with values near boundaries
    int32_t vals[] = {INT32_MAX - 10, INT32_MIN + 10, 100, -100};
    
    for (size_t i = 0; i < sizeof(vals)/sizeof(vals[0]); i++) {
        for (size_t j = 0; j < sizeof(vals)/sizeof(vals[0]); j++) {
            int32_t add_result, mul_result;
            
            // Range analysis should determine possible overflow
            if (__builtin_add_overflow(vals[i], vals[j], &add_result)) {
                local_sum += 0x100;
            } else {
                local_sum += add_result;
            }
            
            if (__builtin_mul_overflow(vals[i], vals[j], &mul_result)) {
                local_sum += 0x200;
            } else {
                local_sum += mul_result;
            }
        }
    }
    
    // Test in loop context
    int32_t accum = 1;
    for (int i = 0; i < 100; i++) {
        int32_t old_accum = accum;
        
        // This multiplication may overflow
        if (__builtin_mul_overflow(accum, 2, &accum)) {
            // Reset on overflow
            accum = 1;
            local_sum += 0x400;
        }
        
        // Check if we're approaching overflow
        if (old_accum > INT32_MAX / 2) {
            local_sum += 0x800;
        }
    }
    
    // Test with unsigned overflow
    uint32_t u1 = UINT32_MAX - 100;
    uint32_t u2 = 200;
    uint32_t u_result;
    
    if (__builtin_add_overflow(u1, u2, &u_result)) {
        local_sum += 0x1000;
    } else {
        local_sum += u_result;
    }
    
    checksum += local_sum;
    return local_sum;
}

NOINLINE uint32_t test_edge_case_conditions(void) {
    uint32_t local_sum = 0;
    
    // Conditions at extreme edges of ranges
    int32_t x = 0;
    
    // These comparisons should trigger the specific uncovered code
    // by creating comparisons against computed maxima/minima
    
    // Test with values that require zero/sign extension analysis
    for (int64_t i = -1000; i < 1000; i++) {
        int32_t val = (int32_t)i;
        
        // Complex condition similar to uncovered code
        if (val > INT32_MAX - 100 || val < INT32_MIN + 100) {
            local_sum += 1;
        }
        
        // Test with shifted values
        int32_t shifted = val << 3;
        if (shifted > (INT32_MAX >> 2) && shifted < (INT32_MIN << 1)) {
            local_sum += 2;
        }
    }
    
    // Test with modulo-constrained values
    uint32_t constrained = checksum % 10000;
    
    // These comparisons require understanding the constrained range
    if (constrained > 9000) {
        local_sum += constrained;
    }
    
    if (constrained < 1000) {
        local_sum ^= constrained;
    }
    
    // Test with value-dependent dead branches
    int32_t known_range_val = 500;  // Known to be 0-1000 from context
    
    // These branches should be analyzable
    if (known_range_val > 750) {
        // Possibly dead code
        local_sum += 0x10000;
    }
    
    if (known_range_val < 250) {
        // Possibly dead code
        local_sum += 0x20000;
    }
    
    checksum += local_sum;
    return local_sum;
}

int main(void) {
    uint32_t total = 0;
    
    total += test_narrowing_conversions();
    total += test_loop_range_analysis();
    total += test_saturation_arithmetic();
    total += test_bitfield_ranges();
    total += test_overflow_builtins();
    total += test_edge_case_conditions();
    
    // Use the result to prevent optimization
    printf("Result: %u\n", total);
    
    return (int)(total & 0xFF);
}
