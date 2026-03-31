#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Prevent optimization from removing our test cases */
#define NOINLINE __attribute__((noinline))
#define KEEP __attribute__((used))

/* Checksum to prevent dead code elimination */
static uint64_t checksum = 0;

NOINLINE void update_checksum(uint64_t value) {
    checksum ^= (value * 0x9e3779b97f4a7c15ULL);
    checksum = (checksum << 13) | (checksum >> 51);
}

/* 1. Integer Range Boundary Tests */
NOINLINE uint64_t test_narrowing_conversions(void) {
    uint64_t local_sum = 0;
    
    /* Test with values at boundaries */
    int64_t large_vals[] = {
        INT64_MAX,
        INT64_MIN,
        (int64_t)UINT32_MAX,
        (int64_t)INT32_MAX + 1,
        (int64_t)INT32_MIN - 1
    };
    
    for (int i = 0; i < 5; i++) {
        /* Narrowing conversions that require range analysis */
        int32_t narrow1 = (int32_t)large_vals[i];
        uint32_t narrow2 = (uint32_t)large_vals[i];
        
        /* Comparisons that should trigger boundary checks */
        if (large_vals[i] > INT32_MAX) {
            local_sum += 0x1000 + i;
        }
        if (large_vals[i] < INT32_MIN) {
            local_sum += 0x2000 + i;
        }
        
        /* Shifts that may overflow */
        int64_t shifted = large_vals[i] << 3;
        int32_t narrow_shifted = (int32_t)shifted;
        local_sum += (uint64_t)narrow_shifted;
    }
    
    /* Specific boundary case: value exactly at max for smaller type */
    int64_t at_boundary = (int64_t)INT32_MAX;
    int32_t converted = (int32_t)at_boundary;
    local_sum += (converted == INT32_MAX) ? 0xABCD : 0;
    
    update_checksum(local_sum);
    return local_sum;
}

/* 2. Loop Bound Analysis with Complex Conditions */
NOINLINE uint64_t test_loop_range_analysis(void) {
    uint64_t local_sum = 0;
    
    /* Outer loop with bitmasked bounds */
    uint32_t outer_bound = 1000;
    for (uint32_t i = outer_bound & 0x3FF; i < (outer_bound | 0x7FF); i += 17) {
        /* Inner loop with dependent bounds */
        uint32_t inner_start = i & 0xFF;
        uint32_t inner_end = (i | 0x7F) + 50;
        
        /* Complex condition using XOR */
        for (uint32_t j = inner_start ^ 0x55; j < inner_end; j += (i & 0xF) + 1) {
            if ((j & (i ^ 0xAA)) < 100) {
                local_sum += j * i;
            }
        }
        
        /* Loop with comparison against computed maximum */
        uint32_t max_val = (i << 2) | 0xF;
        for (uint32_t k = 0; k < max_val; k++) {
            if (k > (i >> 1)) {
                local_sum += k;
            }
        }
    }
    
    /* Nested loops with bitwise operations in conditions */
    int32_t a = -500, b = 500;
    for (int32_t x = a & ~0x3; x < (b | 0x3); x += 7) {
        int32_t y_limit = (x ^ 0xFFFF) & 0x1FF;
        for (int32_t y = 0; y < y_limit; y += 3) {
            if ((x & y) > 0) {
                local_sum += x * y;
            }
        }
    }
    
    update_checksum(local_sum);
    return local_sum;
}

/* 3. Fixed-Point and Saturation Arithmetic */
NOINLINE uint64_t test_saturation_arithmetic(void) {
    uint64_t local_sum = 0;
    
    /* Manual saturation arithmetic */
    int32_t sat_min = -1000;
    int32_t sat_max = 1000;
    
    int32_t test_values[] = {-2000, -500, 0, 500, 2000, 1500, -1500};
    
    for (int i = 0; i < 7; i++) {
        int32_t val = test_values[i];
        
        /* Saturation clamp - should trigger boundary comparisons */
        int32_t saturated;
        if (val > sat_max) {
            saturated = sat_max;
        } else if (val < sat_min) {
            saturated = sat_min;
        } else {
            saturated = val;
        }
        
        local_sum += (uint32_t)saturated;
        
        /* Multiplication with saturation check */
        int32_t multiplied = val * 2;
        if (multiplied > sat_max) {
            multiplied = sat_max;
        } else if (multiplied < sat_min) {
            multiplied = sat_min;
        }
        local_sum += (uint32_t)multiplied;
    }
    
    /* Try to use GCC fixed-point types if available */
    #ifdef __FRACT_FBIT__
    /* These may not be available on all targets, but we try */
    _Fract f1 = 0.5r;
    _Fract f2 = 0.75r;
    _Fract f3 = f1 + f2;  /* May saturate */
    local_sum += (uint64_t)(f3 * 1000);
    #endif
    
    update_checksum(local_sum);
    return local_sum;
}

/* 4. Conditional Code with Value-Dependent Dead Branches */
NOINLINE uint64_t test_conditional_boundaries(void) {
    uint64_t local_sum = 0;
    
    /* Variables with constrained ranges */
    uint32_t constrained;
    
    /* First constraint */
    uint32_t input = 500;
    if (input < 1000) {
        constrained = input & 0xFF;  /* Now 0-255 */
    } else {
        constrained = 1000;
    }
    
    /* Comparisons against extreme boundaries */
    if (constrained > 250) {
        local_sum += 0x100;
    }
    if (constrained < 5) {
        local_sum += 0x200;  /* Likely dead branch */
    }
    
    /* Test with signed values at INT_MAX boundary */
    int32_t signed_val = 100;
    for (int i = 0; i < 10; i++) {
        signed_val += 1000000000;
        
        /* These comparisons should trigger boundary analysis */
        if (signed_val > INT32_MAX - 100) {
            local_sum += 0x400 + i;
        }
        if (signed_val < INT32_MIN + 100) {
            local_sum += 0x800 + i;  /* Likely dead */
        }
    }
    
    /* Modulo operation creates known range */
    int32_t modulo_val = 12345;
    int32_t bounded = modulo_val % 100;  /* 0-99 */
    
    if (bounded > 90) {
        local_sum += bounded * 2;
    }
    if (bounded < 10) {
        local_sum += bounded * 3;
    }
    
    update_checksum(local_sum);
    return local_sum;
}

/* 5. Structs with Bit-Fields and Unions */
NOINLINE uint64_t test_bitfield_ranges(void) {
    uint64_t local_sum = 0;
    
    /* Struct with various bit-field sizes */
    struct bitfield_struct {
        unsigned int small : 3;    /* 0-7 */
        signed int signed_small : 4;  /* -8 to 7 */
        unsigned int medium : 10;  /* 0-1023 */
        unsigned int large : 31;   /* 0-2147483647 */
    } bfs;
    
    /* Union to test type punning */
    union bitfield_union {
        struct bitfield_struct bf;
        uint64_t raw;
    } u;
    
    /* Assign values to bit-fields */
    bfs.small = 5;      /* Within range */
    bfs.signed_small = -3; /* Within range */
    bfs.medium = 512;   /* Mid-range */
    bfs.large = 1000000; /* Within 31-bit range */
    
    /* Comparisons that require understanding bit-field ranges */
    if (bfs.small > 4) {
        local_sum += 0x10;
    }
    if (bfs.small < 8) {  /* Always true for 3-bit unsigned */
        local_sum += 0x20;
    }
    
    if (bfs.signed_small > -8) {  /* Always true for 4-bit signed */
        local_sum += 0x40;
    }
    
    /* Test overflow into bit-field */
    uint32_t big_val = 0xFFFFFFFF;
    bfs.medium = big_val;  /* Will be truncated to 10 bits */
    local_sum += bfs.medium;
    
    /* Check if value fits in bit-field */
    uint32_t test_val = 2000;
    if (test_val < (1U << 10)) {  /* Check against 10-bit max */
        bfs.medium = test_val;
        local_sum += bfs.medium;
    }
    
    update_checksum(local_sum);
    return local_sum;
}

/* 6. Compiler Builtins for Overflow Detection */
NOINLINE uint64_t test_overflow_builtins(void) {
    uint64_t local_sum = 0;
    
    int32_t a = 1000000000;
    int32_t b = 2000000000;
    int32_t result;
    int overflow;
    
    /* Overflow checks with values near boundaries */
    overflow = __builtin_add_overflow(a, b, &result);
    local_sum += overflow ? 0x1 : 0x0;
    local_sum += (uint32_t)result;
    
    /* Multiplication that may overflow */
    int32_t c = 100000;
    int32_t d = 100000;
    overflow = __builtin_mul_overflow(c, d, &result);
    local_sum += overflow ? 0x2 : 0x0;
    local_sum += (uint32_t)result;
    
    /* Use in conditional context with range-restricted values */
    int32_t x = 50000;
    int32_t y = 60000;
    
    /* First restrict range */
    if (x < 100000 && y < 100000) {
        /* Now compiler knows ranges */
        overflow = __builtin_add_overflow(x, y, &result);
        if (!overflow && result < 150000) {
            local_sum += result;
        }
    }
    
    /* Loop with overflow checks */
    for (int i = 0; i < 10; i++) {
        int32_t base = 2000000000;
        int32_t increment = 100000000 * i;
        
        overflow = __builtin_add_overflow(base, increment, &result);
        local_sum += overflow ? (0x10 << i) : 0;
        
        if (!overflow) {
            /* Further operation on result */
            int32_t doubled;
            overflow = __builtin_add_overflow(result, result, &doubled);
            local_sum += overflow ? 0x100 : (uint32_t)doubled;
        }
    }
    
    update_checksum(local_sum);
    return local_sum;
}

/* Main function that runs all tests */
int main(void) {
    uint64_t total = 0;
    
    total += test_narrowing_conversions();
    total += test_loop_range_analysis();
    total += test_saturation_arithmetic();
    total += test_conditional_boundaries();
    total += test_bitfield_ranges();
    total += test_overflow_builtins();
    
    /* Use checksum to prevent optimization */
    printf("Result: %llu\n", (unsigned long long)(total ^ checksum));
    
    return (int)(total ^ checksum) & 0xFF;
}
