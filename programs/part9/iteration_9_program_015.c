#include <stdint.h>
#include <stdio.h>
#include <string.h>

// Prevent inlining to maintain function boundaries for coverage
#define NOINLINE __attribute__((noinline))

// Fixed-point types if available
#ifdef __STDC_IEC_559__
#include <stdfix.h>
#endif

// Checksum to prevent dead code elimination
static volatile int checksum = 0;

NOINLINE void test_narrowing_conversions() {
    uint64_t wide_vals[] = {
        0xFFFFFFFFFFFFFFFFULL,  // Max uint64
        0x8000000000000000ULL,  // High bit set
        0x7FFFFFFFFFFFFFFFULL,  // Max int64
        0x00000000FFFFFFFFULL,  // Fits in 32-bit
        0x000000000000FFFFULL   // Fits in 16-bit
    };
    
    for (int i = 0; i < 5; i++) {
        // Narrowing conversions that require range analysis
        uint32_t narrow32 = (uint32_t)wide_vals[i];
        int32_t signed32 = (int32_t)wide_vals[i];
        uint16_t narrow16 = (uint16_t)wide_vals[i];
        
        // Comparisons at type boundaries
        if (narrow32 > 0x7FFFFFFF) checksum ^= 1;
        if (signed32 < 0) checksum ^= 2;
        if (narrow16 == 0xFFFF) checksum ^= 4;
        
        // Shifts that may overflow
        uint32_t shifted = narrow32 << (wide_vals[i] & 0x1F);
        if (shifted > 0x80000000) checksum ^= 8;
    }
}

NOINLINE void test_loop_range_analysis() {
    int32_t bounds[4] = {100, 200, 300, 400};
    uint32_t masks[4] = {0xFF, 0x7FF, 0xFFF, 0xFFFF};
    
    // Complex loop bounds with bitwise operations
    for (int i = bounds[0] & masks[0]; i < (bounds[1] | masks[1]); i += 3) {
        int inner_bound = (i ^ masks[2]) & masks[3];
        
        // Nested loop with dependent bounds
        for (int j = i & 0x3F; j < inner_bound; j += (i & 0x7) + 1) {
            // Bitwise conditions that affect range analysis
            if ((j & 0xFF) == 0x7F) checksum ^= 16;
            if ((j | 0x80) > 0x100) checksum ^= 32;
            
            // Value-dependent shift
            int shifted = j << ((i >> 2) & 0x3);
            if (shifted > 0x400) checksum ^= 64;
        }
        
        // Loop variant with modulo operation
        for (int k = i % 50; k < 100; k += (i % 7) + 1) {
            if (k > 75 && k < 90) checksum ^= 128;
        }
    }
    
    // Loop with signed/unsigned comparisons
    int32_t signed_val = -50;
    uint32_t unsigned_val = 100;
    for (int i = signed_val; i < (int)unsigned_val; i++) {
        if (i > 0 && i < 100) checksum ^= 256;
    }
}

NOINLINE void test_saturation_arithmetic() {
    // Manual saturation implementation
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
            return ((a ^ b) < 0) ? INT32_MIN : INT32_MAX;
        }
        return prod;
    }
    
    // Test cases at boundaries
    int32_t test_cases[][2] = {
        {INT32_MAX, 1},
        {INT32_MIN, -1},
        {INT32_MAX / 2, 2},
        {INT32_MIN / 2, 2},
        {100, 200},
        {-100, -200}
    };
    
    for (int i = 0; i < 6; i++) {
        int32_t a = test_cases[i][0];
        int32_t b = test_cases[i][1];
        
        int32_t sum = sat_add(a, b);
        int32_t prod = sat_mul(a, b);
        
        // Comparisons against limits
        if (sum == INT32_MAX) checksum ^= 512;
        if (sum == INT32_MIN) checksum ^= 1024;
        if (prod > INT32_MAX - 100) checksum ^= 2048;
        if (prod < INT32_MIN + 100) checksum ^= 4096;
    }
    
    // Fixed-point arithmetic if available
    #ifdef __STDC_IEC_559__
    _Accum acc = 0.5k;
    _Fract frac = 0.1r;
    
    for (int i = 0; i < 10; i++) {
        acc += 0.1k;
        frac += 0.01r;
        
        // Comparisons that may trigger fixed-point range analysis
        if (acc > 1.0k) checksum ^= 8192;
        if (frac < 0.0r) checksum ^= 16384;
    }
    #endif
}

NOINLINE void test_bitfield_ranges() {
    // Struct with various bit-fields
    struct BitFields {
        unsigned int a : 3;  // 0-7
        signed int b : 5;    // -16 to 15
        unsigned int c : 8;  // 0-255
        signed int d : 12;   // -2048 to 2047
    } bf;
    
    // Assign values at boundaries
    bf.a = 7;   // Max for 3 bits
    bf.b = -16; // Min for 5 signed bits
    bf.c = 255; // Max for 8 bits
    bf.d = 2047;// Max for 12 signed bits
    
    // Comparisons that require bit-field range analysis
    if (bf.a == 7) checksum ^= 32768;
    if (bf.b < 0) checksum ^= 65536;
    if (bf.c > 200) checksum ^= 131072;
    if (bf.d == 2047) checksum ^= 262144;
    
    // Union with overlapping bit-fields
    union Overlap {
        struct {
            unsigned int x : 10;
            unsigned int y : 10;
        } parts;
        uint32_t whole;
    } ov;
    
    ov.whole = 0x3FF03FF;  // Set both fields to max
    
    // Complex condition with bit-field extraction
    if ((ov.parts.x == 0x3FF) && (ov.parts.y == 0x3FF)) {
        checksum ^= 524288;
    }
}

NOINLINE void test_overflow_builtins() {
    int32_t vals[] = {100, 1000, 10000, 100000, 1000000};
    int32_t multipliers[] = {2, 10, 100, 1000, 10000};
    
    for (int i = 0; i < 5; i++) {
        int32_t a = vals[i];
        int32_t b = multipliers[i];
        int32_t result;
        
        // Overflow checks with partially known ranges
        if (a > 0 && b > 0) {
            if (__builtin_mul_overflow(a, b, &result)) {
                checksum ^= 1048576;
            } else if (result > INT32_MAX / 2) {
                checksum ^= 2097152;
            }
        }
        
        // Add with overflow in loop context
        int32_t sum = 0;
        for (int j = 0; j < b % 10; j++) {
            if (__builtin_add_overflow(sum, a, &sum)) {
                checksum ^= 4194304;
                break;
            }
        }
        
        // Subtraction with underflow check
        int32_t diff;
        if (__builtin_sub_overflow(INT32_MIN + 100, a, &diff)) {
            checksum ^= 8388608;
        }
    }
    
    // Chain of operations with overflow propagation
    int32_t x = 1000000;
    int32_t y = 1000;
    int32_t z = 100;
    
    int32_t tmp1, tmp2, final;
    if (!__builtin_mul_overflow(x, y, &tmp1)) {
        if (!__builtin_add_overflow(tmp1, z, &tmp2)) {
            if (!__builtin_mul_overflow(tmp2, 2, &final)) {
                if (final > 0) checksum ^= 16777216;
            }
        }
    }
}

NOINLINE void test_edge_case_conditions() {
    // Conditions at extreme edges of ranges
    int32_t x = 100;
    uint32_t y = 200;
    
    // These comparisons require precise range analysis
    if (x > INT32_MAX - 1000) checksum ^= 33554432;
    if (y < 100 + (UINT32_MAX >> 8)) checksum ^= 67108864;
    
    // Modulo-restricted ranges
    int32_t mod_val = x % 50;  // 0-49
    if (mod_val > 40) checksum ^= 134217728;
    
    // Value-dependent dead branch (should be analyzable)
    int32_t constrained = y & 0xFF;  // 0-255
    if (constrained > 300) {  // Always false, but requires analysis
        checksum ^= 268435456;
    }
    
    // Complex boundary comparison
    uint64_t big_val = 0xFFFFFFFF00000000ULL;
    uint32_t truncated = (uint32_t)big_val;  // 0
    
    if (truncated == 0 && big_val != 0) {
        checksum ^= 536870912;
    }
}

int main() {
    // Initialize checksum
    checksum = 0;
    
    // Run all tests
    test_narrowing_conversions();
    test_loop_range_analysis();
    test_saturation_arithmetic();
    test_bitfield_ranges();
    test_overflow_builtins();
    test_edge_case_conditions();
    
    // Use checksum to prevent optimization
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
