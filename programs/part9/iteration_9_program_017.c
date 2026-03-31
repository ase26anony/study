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
static uint64_t checksum = 0;

NOINLINE void update_checksum(uint64_t val) {
    checksum ^= (val << 1) | (val >> 63);
}

// 1. Integer Range Boundary Tests
NOINLINE uint64_t test_narrowing_conversions(void) {
    uint64_t local_sum = 0;
    
    // Explicit bit-width types
    int64_t wide_val = 0x7FFFFFFFFFFFFFFFLL;
    int32_t narrow_val;
    uint64_t uwide_val = 0xFFFFFFFFFFFFFFFFULL;
    uint32_t unarrow_val;
    
    // Narrowing conversions that require range checking
    narrow_val = (int32_t)wide_val;  // May overflow
    update_checksum(narrow_val);
    local_sum += narrow_val;
    
    narrow_val = (int32_t)(wide_val >> 32);
    update_checksum(narrow_val);
    local_sum += narrow_val;
    
    // Shifts that may overflow
    int32_t shift_val = 1 << 31;
    update_checksum(shift_val);
    local_sum += shift_val;
    
    // Comparisons at type limits
    if (wide_val > INT64_MAX - 10) {
        local_sum += 1;
    }
    
    // Multiple narrowing steps
    int16_t very_narrow = (int16_t)((int8_t)wide_val);
    update_checksum(very_narrow);
    local_sum += very_narrow;
    
    return local_sum;
}

// 2. Loop Bound Analysis with Complex Conditions
NOINLINE uint64_t test_loop_range_analysis(void) {
    uint64_t local_sum = 0;
    int32_t a = 1000, b = 2000, c = 7;
    
    // Loop with bitwise operations in bounds
    for (int32_t i = a & 0xFFF; i < (b | 0x7FF); i += c) {
        local_sum += i;
        update_checksum(i);
        
        // Nested loop with dependent bounds
        for (int32_t j = (i & 0x3F); j < 100; j += (c & 0x3)) {
            local_sum += j;
            update_checksum(j);
        }
    }
    
    // Loop with XOR-based bound
    uint32_t mask = 0xFFFF;
    for (uint32_t k = 0; k < (mask ^ 0xFFF); k += 13) {
        local_sum += k;
        update_checksum(k);
    }
    
    // Loop with shifting bound
    for (int32_t m = 0; m < (1 << ((a >> 8) & 0x7)); m++) {
        local_sum += m;
        update_checksum(m);
    }
    
    return local_sum;
}

// 3. Fixed-Point and Saturation Arithmetic
NOINLINE uint64_t test_saturation_arithmetic(void) {
    uint64_t local_sum = 0;
    
    // Manual saturation arithmetic
    int32_t sat_add(int32_t a, int32_t b) {
        int32_t result;
        if (a > 0 && b > INT32_MAX - a) {
            result = INT32_MAX;
        } else if (a < 0 && b < INT32_MIN - a) {
            result = INT32_MIN;
        } else {
            result = a + b;
        }
        return result;
    }
    
    int32_t sat_mul(int32_t a, int32_t b) {
        int64_t tmp = (int64_t)a * (int64_t)b;
        if (tmp > INT32_MAX) return INT32_MAX;
        if (tmp < INT32_MIN) return INT32_MIN;
        return (int32_t)tmp;
    }
    
    // Test saturation with boundary values
    int32_t vals[] = {INT32_MAX, INT32_MIN, 100, -100, 0};
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            int32_t res = sat_add(vals[i], vals[j]);
            local_sum += res;
            update_checksum(res);
            
            res = sat_mul(vals[i], vals[j]);
            local_sum += res;
            update_checksum(res);
        }
    }
    
#ifdef __STDC_IEC_559__
    // GCC fixed-point types if available
    _Accum acc = 0.5k;
    _Fract frac = 0.1r;
    
    for (int i = 0; i < 10; i++) {
        acc += 0.1k;
        frac += 0.01r;
        local_sum += (uint64_t)(acc * 1000);
        update_checksum((uint64_t)(frac * 1000));
    }
#endif
    
    return local_sum;
}

// 4. Conditional Code with Value-Dependent Dead Branches
NOINLINE uint64_t test_conditional_ranges(void) {
    uint64_t local_sum = 0;
    
    // Variables with constrained ranges
    int32_t x = 100;
    int32_t y = 200;
    
    // Range restriction through conditions
    if (x > 50 && x < 150) {
        // x is known to be in [51, 149]
        if (x > INT32_MAX - 1000) {  // Dead branch, but compiler must analyze
            local_sum += 999;
        }
        
        // Further restrict range
        if (x < 100) {
            // x in [51, 99]
            if (x > 90) {
                local_sum += x * 2;
            }
        } else {
            // x in [100, 149]
            if (x < 120) {
                local_sum += x * 3;
            }
        }
    }
    
    // Modulo operation creates known range
    uint32_t mod_val = y % 256;  // Known to be in [0, 255]
    update_checksum(mod_val);
    local_sum += mod_val;
    
    if (mod_val > 200) {
        local_sum += 1000;
    }
    
    // Chain of comparisons
    int64_t big_val = 0x7FFFFFFFFFFFFF00LL;
    if (big_val > INT64_MAX - 256) {
        if (big_val < INT64_MAX - 100) {
            local_sum += 1;
        }
    }
    
    return local_sum;
}

// 5. Structs with Bit-Fields and Unions
NOINLINE uint64_t test_bitfield_ranges(void) {
    uint64_t local_sum = 0;
    
    struct BitFields {
        unsigned int a : 3;   // 0-7
        signed int b : 5;     // -16 to 15
        unsigned int c : 12;  // 0-4095
        signed int d : 20;    // -524288 to 524287
    };
    
    union BitUnion {
        struct BitFields fields;
        uint64_t raw;
    };
    
    struct BitFields bf = {0};
    bf.a = 7;      // Max for 3 bits
    bf.b = -8;     // Min for signed 5 bits
    bf.c = 4095;   // Max for 12 bits
    bf.d = 262144; // Mid-range for 20 bits
    
    // Comparisons against bit-field capacity
    if (bf.a == 7) {
        local_sum += 1;
    }
    
    if (bf.b < 0) {
        local_sum += 2;
    }
    
    if (bf.c > 4000) {
        local_sum += 4;
    }
    
    if (bf.d >= 0 && bf.d < 524288) {
        local_sum += 8;
    }
    
    // Assignment that may overflow bit-field
    unsigned int temp = 15;
    bf.a = temp;  // Will be truncated to 3 bits
    update_checksum(bf.a);
    local_sum += bf.a;
    
    // Union access
    union BitUnion u;
    u.fields = bf;
    if (u.raw > 0) {
        local_sum += u.raw & 0xFF;
    }
    
    return local_sum;
}

// 6. Compiler Builtins for Overflow Detection
NOINLINE uint64_t test_overflow_builtins(void) {
    uint64_t local_sum = 0;
    
    int32_t of_a = 1000000;
    int32_t of_b = 2000000;
    int32_t of_result;
    int of_flag;
    
    // Overflow checks with partially known ranges
    of_flag = __builtin_add_overflow(of_a, of_b, &of_result);
    local_sum += of_flag;
    update_checksum(of_result);
    
    of_flag = __builtin_mul_overflow(of_a, 100, &of_result);
    local_sum += of_flag;
    update_checksum(of_result);
    
    // In loops with range-restricted variables
    for (int32_t i = 0; i < 100; i++) {
        int32_t j = i * 1000;
        of_flag = __builtin_add_overflow(j, INT32_MAX / 2, &of_result);
        local_sum += of_flag;
        
        // Chain overflow operations
        int32_t tmp1, tmp2;
        of_flag = __builtin_mul_overflow(i, 10000, &tmp1);
        if (!of_flag) {
            of_flag = __builtin_add_overflow(tmp1, j, &tmp2);
            local_sum += of_flag * 2;
            update_checksum(tmp2);
        }
    }
    
    // Overflow with constants at boundaries
    of_flag = __builtin_add_overflow(INT32_MAX, 1, &of_result);
    local_sum += of_flag * 4;
    
    of_flag = __builtin_sub_overflow(INT32_MIN, 1, &of_result);
    local_sum += of_flag * 8;
    
    return local_sum;
}

int main(void) {
    uint64_t total = 0;
    
    total += test_narrowing_conversions();
    total += test_loop_range_analysis();
    total += test_saturation_arithmetic();
    total += test_conditional_ranges();
    total += test_bitfield_ranges();
    total += test_overflow_builtins();
    
    // Use checksum to prevent optimization
    printf("Result: %lu, Checksum: %lu\n", 
           (unsigned long)total, 
           (unsigned long)checksum);
    
    return (int)((total + checksum) & 0xFFFFFFFF);
}
